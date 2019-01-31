/*
 * EventLoop.cc
 *
 *  Created on: Jan 9, 2019
 *      Author: xiagai
 */


#include "EventLoop.h"
#include "CurrentThread.h"
#include "Poller.h"
#include "Channel.h"
#include "MutexLockerGuard.h"


#include <poll.h>
#include <signal.h>
#include <assert.h>
#include <iostream>
#include <sys/eventfd.h>
#include <algorithm>

namespace miniws {

namespace {

__thread EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

}

namespace detail {

int createEventFd() {
	int fd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	return fd;
}

}

EventLoop::EventLoop()
	: m_looping(false),
	  m_quit(false),
	  m_threadId(CurrentThread::tid()),
	  m_poller(std::make_unique<Poller>(this)),
	  m_timerQueue(std::make_unique<TimerQueue>(this)),
	  m_eventHandling(false),
	  m_currentChannel(nullptr),
	  m_callingPendingFunctors(false),
	  m_wakeupfd(detail::createEventFd()),
	  m_wakeupChannel(std::make_unique<Channel>(this, m_wakeupfd)),
	  m_mutex() {
	printf("EventLoop created %p in thread %d\n", this, m_threadId);
	if (t_loopInThisThread) {
		printf("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, m_threadId);
	}
	else {
		t_loopInThisThread = this;
	}
	m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleWakeUp, this));
	m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop() {
	assert(!m_looping);
	m_wakeupChannel->disableAll();
	m_wakeupChannel->remove();
	close(m_wakeupfd);
	t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
	assert(!m_looping);
	assertInLoopThread();
	m_looping = true;
	m_quit = false;

	while (!m_quit) {
		m_activeChannels.clear();
		m_poller->poll(kPollTimeMs, m_activeChannels);
		m_eventHandling = true;
		for (std::vector<Channel *>::iterator it = m_activeChannels.begin();
				it != m_activeChannels.end(); ++it) {
			m_currentChannel = (*it);
			m_currentChannel->handleEvent();
		}
		m_eventHandling = false;
		doPendingFunctors();
	}
	printf("LOG_TRACE EventLoop %p stop looping\n", this);
	m_looping = false;
}

void EventLoop::quit() {
	m_quit = true;
	if (!isInLoopThread()) {
		wakeup();
	}
}

bool EventLoop::isInLoopThread() const {
	return m_threadId == CurrentThread::tid();
}

void EventLoop::updateChannel(Channel *channel) {
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	//???
	if (m_eventHandling) {
		assert(m_currentChannel == channel || std::find(m_activeChannels.begin(), m_activeChannels.end(), channel) != m_activeChannels.end());
	}
	m_poller->removeChannel(channel);
}

void EventLoop::runInLoop(const Functor &cb) {
	if (isInLoopThread()) {
		cb();
	}
	else {
		queueInLoop(cb);
	}
}

void EventLoop::wakeup() {
	uint64_t one = 1;
	ssize_t n = write(m_wakeupfd, &one, sizeof(one));
	if (n != sizeof(one)) {
		printf("LOG_ERROR EventLoop::wakeup() writes %ld bytes instead of 8\n", n);
	}
}

void EventLoop::handleWakeUp() {
	uint64_t ret;
	ssize_t n = read(m_wakeupfd, &ret, sizeof(ret));
	if (n != sizeof(ret)) {
		printf("LOG_ERROR EventLoop::handleWakeUp() reads %ld bytes instead of 8\n", n);
	}
}

void EventLoop::assertInLoopThread() {
	assert(isInLoopThread());
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
	return t_loopInThisThread;
}

void EventLoop::runAt(const TimeStamp &timestamp, const Timer::TimerCallback &cb) {
	m_timerQueue->addTimer(cb, timestamp, 0.0);
}

void EventLoop::runAfter(double delay, const Timer::TimerCallback &cb) {
	TimeStamp timestamp = TimeStamp::addTime(TimeStamp::now(), delay);
	runAt(timestamp, cb);
}

void EventLoop::runEvery(double interval, const Timer::TimerCallback &cb) {
	TimeStamp timestamp = TimeStamp::addTime(TimeStamp::now(), interval);
	m_timerQueue->addTimer(cb, timestamp, interval);
}

void EventLoop::doPendingFunctors() {
	std::vector<Functor> functors;
	m_callingPendingFunctors = true;
	{
		MutexLockerGuard guard(m_mutex);
		functors.swap(m_pendingFunctors);
	}
	for (auto functor : functors) {
		functor();
	}
	m_callingPendingFunctors = false;
}

void EventLoop::queueInLoop(const Functor &cb) {
	{
		MutexLockerGuard guard(m_mutex);
		m_pendingFunctors.push_back(cb);
	}
	if (!isInLoopThread() || m_callingPendingFunctors) {
		wakeup();
	}
}

}