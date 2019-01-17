/*
 * Poller.h
 *
 *  Created on: Jan 15, 2019
 *      Author: xiagai
 */

#ifndef POLLER_H_
#define POLLER_H_

#include "noncopyable.h"
#include "TimeStamp.h"

#include <vector>
#include <map>

struct pollfd;

namespace miniws {

class EventLoop;
class Channel;

class Poller : noncopyable {
public:
	Poller(EventLoop *loop);
	~Poller();

	/// Polls the I/O events
	/// Must be called in the loop thread
	TimeStamp poll(int timeoutMs, std::vector<Channel *> &activeChannels);

	/// Changes the interested I/O events;
	/// Must be called in the loop thread
	void updateChannel(Channel *channel);
	// void removeChannel(Channel *channel);

	void assertInLoopThread();

private:
	void fillActiveChannels(int numEvents, std::vector<Channel *> &activeChannels) const;

private:
	EventLoop *m_ownerLoop;
	std::vector<pollfd> m_pollfds;
	std::map<int, Channel *> m_channels;
};

}


#endif /* POLLER_H_ */
