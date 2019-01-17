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

#include <memory>
#include <vector>

namespace miniws {

class Channel;
class Poller;

class EventLoop : noncopyable {
public:
	EventLoop();
	~EventLoop();

	void loop();
	void quit();
	void assertInLoopThread();
	bool isInLoopThread() const;
	void updateChannel(Channel* channel);
	static EventLoop *getEventLoopOfCurrentThread();

private:
	void abortNotInLoopThread();

private:
	bool m_looping;
	bool m_quit;
	const pid_t m_threadId;
	std::unique_ptr<Poller> m_poller;
	std::vector<Channel *> m_activeChannels;
};

}



#endif /* EVENTLOOP_H_ */
