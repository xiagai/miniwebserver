/*
 * EventLoop.h
 *
 *  Created on: Jan 9, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "pthread.h"
#include "MutexLocker.h"
#include "TimerQueue.h"
#include "TimeStamp.h"
#include "Timer.h"

#include <memory>
#include <vector>
#include <functional>

namespace miniws {

class Channel;
class Epoller;

class EventLoop : noncopyable {
public:
	typedef std::function<void()> Functor;
	EventLoop();
	~EventLoop();

	void loop();
	void quit();
	void assertInLoopThread();
	bool isInLoopThread() const;
	// Must be called in the loop thread
	void updateChannel(Channel* channel);
	// Must be called in the loop thread
	void removeChannel(Channel* channel);
	void runInLoop(const Functor &cb);
	static EventLoop *getEventLoopOfCurrentThread();
	void runAt(const TimeStamp &timestamp, const Timer::TimerCallback &cb);
	void runAfter(double delay, const Timer::TimerCallback &cb);
	void runEvery(double interval, const Timer::TimerCallback &cb);

private:
	void handleWakeUp();
	void wakeup();
	void doPendingFunctors();
	void queueInLoop(const Functor &cb);

private:
	bool m_looping;
	bool m_quit;
	const pid_t m_threadId;
	std::unique_ptr<Epoller> m_poller;
	std::unique_ptr<TimerQueue> m_timerQueue;
	std::vector<Channel *> m_activeChannels;

	bool m_eventHandling;
	Channel *m_currentChannel;

	bool m_callingPendingFunctors;
	int m_wakeupfd;
	std::unique_ptr<Channel> m_wakeupChannel;
	MutexLocker m_mutex;
	std::vector<Functor> m_pendingFunctors;
};

}

