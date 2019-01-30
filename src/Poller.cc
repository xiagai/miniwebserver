/*
 * Poller.cc
 *
 *  Created on: Jan 15, 2019
 *      Author: xiagai
 */

#include "Poller.h"
#include "EventLoop.h"
#include "Channel.h"

#include <poll.h>
#include <assert.h>

namespace miniws {

Poller::Poller(EventLoop *loop)
	: m_ownerLoop(loop) {}
Poller::~Poller() {}

TimeStamp Poller::poll(int timeoutMs, std::vector<Channel *> &activeChannels) {
	int numEvents = ::poll(&(*m_pollfds.begin()), m_pollfds.size(), timeoutMs);
	TimeStamp now(TimeStamp::now());
	if (numEvents > 0) {
		printf("LOG_TRACE %d events happened\n", numEvents);
		fillActiveChannels(numEvents, activeChannels);
	} else if (numEvents == 0) {
		printf("LOG_TRACE nothing happened\n");
	} else {
		printf("LOG_SYSERR Poller::poll()\n");
	}
	return now;
}

void Poller::updateChannel(Channel *channel) {
	assertInLoopThread();
	printf("LOG_TRACE fd = %d events = %d\n", channel->fd(), channel->events());
	if (channel->index() < 0) {
		// a new channel, add to m_pollfds
		assert(m_channels.find(channel->fd()) == m_channels.end());
		pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		m_pollfds.push_back(pfd);
		int idx = static_cast<int>(m_pollfds.size()) - 1;
		assert(idx >= 0);
		channel->setIndex(idx);
		m_channels[pfd.fd] = channel;
	} else {
		// update existing channel
		assert(m_channels.find(channel->fd()) != m_channels.end());
		assert(m_channels[channel->fd()] == channel);
		int idx = channel->index();
		assert(0 <= idx && idx < static_cast<int>(m_pollfds.size()));
		pollfd &pfd = m_pollfds[idx];
		assert(pfd.fd == channel->fd() || pfd.fd == -1);
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		if (channel->isNoneEvent()) {
			pfd.fd = -1;
		}
	}
}

void Poller::removeChannel(Channel *channel) {
	assertInLoopThread();
	if (channel->index() < 0) {
		// a new channel, don't need to remove
	} else {
		// remove existing channel
		assert(m_channels.find(channel->fd()) != m_channels.end());
		assert(m_channels[channel->fd()] == channel);
		int idx = channel->index();
		assert(0 <= idx && idx < static_cast<int>(m_pollfds.size()));
		pollfd &pfd = m_pollfds[idx];
		assert(pfd.fd == channel->fd() || pfd.fd == -1);
		size_t n = m_channels.erase(pfd.fd);
		assert(n == 1);
		m_pollfds.erase(m_pollfds.begin() + idx);
	}
}

void Poller::assertInLoopThread() {
	m_ownerLoop->assertInLoopThread();
}

void Poller::fillActiveChannels(int numEvents, std::vector<Channel *> &activeChannels) const {
	for (std::vector<pollfd>::const_iterator it = m_pollfds.cbegin(); it != m_pollfds.cend() && numEvents > 0; ++it) {
		if (it->revents > 0) {
			--numEvents;
			std::map<int, Channel *>::const_iterator chIt = m_channels.find(it->fd);
			assert(chIt != m_channels.end());
			Channel* channel = chIt->second;
			assert(channel->fd() == it->fd);
			channel->setRevents(it->revents);
			activeChannels.push_back(channel);
		}
	}
}

}
