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
	  m_callingPendingFunctors(false),
	  m_threadId(CurrentThread::tid()),
	  m_poller(std::make_unique<Poller>(this)),
	  m_wakeupFd(detail::createEventFd()),
	  m_wakeupChannel(std::make_unique<Channel>(this, m_wakeupFd)),
	  m_mutex() {
	printf("EventLoop created %p in thread %d\n", this, m_threadId);
	if (t_loopInThisThread) {
		printf("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, m_threadId);
	}
	else {
		t_loopInThisThread = this;
	}
}

EventLoop::~EventLoop() {
	assert(!m_looping);
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
		for (std::vector<Channel *>::iterator it = m_activeChannels.begin();
				it != m_activeChannels.end(); ++it) {
			(*it)->handleEvent();
		}
		doPendingFunctors();
	}
	printf("LOG_TRACE EventLoop %p stop looping", this);
	m_looping = false;
}

void EventLoop::quit() {
	m_quit = true;
	if (!isInLoopThread) {
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

// void EventLoop::removeChannel(Channel *channel) {
// 	assert(channel->ownerLoop() == this);
// 	assertInLoopThread();
	
// }

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
	ssize_t n = write(m_wakeupFd, &one, sizeof(one));
	if (n != sizeof(one)) {
		printf("LOG_ERROR EventLoop::wakeup() writes %d bytes instead of 8", n);
	}
}

void EventLoop::handleWakeUp() {
	uint64_t ret;
	ssize_t n = read(m_wakeupFd, &ret, sizeof(ret));
	if (n != sizeof(ret)) {
		printf("LOG_ERROR EventLoop::handleWakeUp() reads %d bytes instead of 8", n);
	}
}

void EventLoop::assertInLoopThread() {
	assert(isInLoopThread());
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
	return t_loopInThisThread;
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