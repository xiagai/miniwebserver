/*
 * Epoller.cc
 * 
 *  Created on: Feb 19, 2019
 *      Author: xiagai
 */

#include "Epoller.h"
#include "EventLoop.h"
#include "Channel.h"
#include "Common.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

namespace miniws {

Epoller::Epoller(EventLoop *ownerLoop)
    : m_ownerLoop(ownerLoop),
      m_epollfd(::epoll_create1(EPOLL_CLOEXEC)) {}

Epoller::~Epoller() {
    ::close(m_epollfd);
}

TimeStamp Epoller::epoll(int timeoutMs, std::vector<Channel *> &activeChannels) {
    m_ownerLoop->assertInLoopThread();
    epoll_event revents[MAX_EVENT_NUM];
    int numEvents = ::epoll_wait(m_epollfd, revents, MAX_EVENT_NUM, timeoutMs);
	TimeStamp now(TimeStamp::now());
	if (numEvents > 0) {
		printf("LOG_TRACE %p %d events happened\n", m_ownerLoop, numEvents);
		fillActiveChannels(numEvents, revents, activeChannels);
	} else if (numEvents == 0) {
		printf("LOG_TRACE %p nothing happened\n", m_ownerLoop);
	} else {
        char strerr[64];
		printf("LOG_ERR EPoller::epoll() %s\n", strerror_r(errno, strerr, sizeof strerr));
	}
	return now;
}

void Epoller::updateChannel(Channel *channel) {
    m_ownerLoop->assertInLoopThread();
    if (channel->isNoneEvent()) {
        removeChannel(channel);
        return;
    }
    printf("LOG_TRACE Epoller::updateChannel %p fd = %d events = %u\n", m_ownerLoop, channel->fd(), channel->events());
    epoll_event event;
    event.data.fd = channel->fd();
    event.events = channel->events();
    int ret;
    if (m_channels.find(channel->fd()) != m_channels.end()) {
        ret = epoll_ctl(m_epollfd, EPOLL_CTL_MOD, channel->fd(), &event);
    }
    else {
        ret = epoll_ctl(m_epollfd, EPOLL_CTL_ADD, channel->fd(), &event);
        m_channels[channel->fd()] = channel;
        assert(m_channels.find(channel->fd()) != m_channels.end());
    }
    if (ret < 0) {
        int err = errno;
        assert(err != EEXIST || err != ENOENT);
        char strerr[64];
        printf("LOG_ERR Epoller::updateChannel %s\n", strerror_r(errno, strerr, sizeof strerr));
    }
}

void Epoller::removeChannel(Channel *channel) {
    m_ownerLoop->assertInLoopThread();
    if (m_channels.find(channel->fd()) != m_channels.end()) {
        printf("LOG_TRACE Epoller::removeChannel %p fd = %d events = %d\n", m_ownerLoop, channel->fd(), channel->events());
        //Since Linux 2.6.9 event can be NULL
        int ret = epoll_ctl(m_epollfd, EPOLL_CTL_DEL, channel->fd(), NULL);
        size_t n = m_channels.erase(channel->fd());
        assert(n == 1);
        if (ret < 0) {
            int err = errno;
            assert(err != ENOENT);
            char strerr[64];
            printf("LOG_ERR Epoller::removeChannel %s\n", strerror_r(errno, strerr, sizeof strerr));
        }
    }
}

void Epoller::fillActiveChannels(int numEvents, epoll_event *revents, std::vector<Channel *> &activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        auto chIt = m_channels.find(revents[i].data.fd);
        assert(chIt != m_channels.end());
        Channel *channel = chIt->second;
        assert(channel->fd() == revents[i].data.fd);
        channel->setRevents(revents[i].events);
        activeChannels.push_back(channel);
    }
}

}