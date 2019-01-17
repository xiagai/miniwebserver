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


#include <poll.h>
#include <signal.h>
#include <assert.h>
#include <iostream>

namespace miniws {

namespace {

__thread EventLoop *t_loopInThisThread = nullptr;
const int kPollTimeMs = 10000;

}

EventLoop::EventLoop()
	: m_looping(false),
	  m_quit(false),
	  m_threadId(CurrentThread::tid()) {
	printf("EventLoop created %p in thread %d\n", this, m_threadId);
	if (t_loopInThisThread) {
		printf("Another EventLoop %p exists in this thread %d\n", t_loopInThisThread, m_threadId);
	}
	else {
		t_loopInThisThread = this;
	}
	m_poller = std::make_unique<Poller>(this);
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
	}
	printf("LOG_TRACE EventLoop %p stop looping", this);
	m_looping = false;
}

void EventLoop::quit() {
	m_quit = true;
}

bool EventLoop::isInLoopThread() const {
	return m_threadId == CurrentThread::tid();
}

void EventLoop::updateChannel(Channel *channel) {
	assert(channel->ownerLoop() == this);
	assertInLoopThread();
	m_poller->updateChannel(channel);
}

void EventLoop::assertInLoopThread() {
	assert(isInLoopThread());
}

EventLoop *EventLoop::getEventLoopOfCurrentThread() {
	return t_loopInThisThread;
}

}
