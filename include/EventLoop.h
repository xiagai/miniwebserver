/*
 * EventLoop.h
 *
 *  Created on: Jan 9, 2019
 *      Author: xiagai
 */

#ifndef EVENTLOOP_H_
#define EVENTLOOP_H_

#include "noncopyable.h"
#include "pthread.h"
#include "MutexLocker.h"

#include <memory>
#include <vector>

namespace miniws {

class Channel;
class Poller;

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

private:
	void handleWakeUp();
	void wakeup();
	void doPendingFunctors();
	void queueInLoop(const Functor &cb);

private:
	bool m_looping;
	bool m_quit;
	const pid_t m_threadId;
	std::unique_ptr<Poller> m_poller;
	std::vector<Channel *> m_activeChannels;

	bool m_eventHandling;
	Channel *m_currentChannel;

	bool m_callingPendingFunctors;
	int m_wakeupFd;
	std::unique_ptr<Channel> m_wakeupChannel;
	MutexLocker m_mutex;
	std::vector<Functor> m_pendingFunctors;
};

}



#endif /* EVENTLOOP_H_ */
