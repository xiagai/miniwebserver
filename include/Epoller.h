/*
 * Epoller.h
 * 
 *  Created on: Feb 19, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <vector>
#include <map>

struct epoll_event;

namespace miniws {

class EventLoop;
class Channel;

class Epoller : noncopyable {
public:
    Epoller(EventLoop *ownerLoop, bool enableET = true);
    ~Epoller();
    
	/// EPolls the I/O events
	/// Must be called in the loop thread
	TimeStamp epoll(int timeoutMs, std::vector<Channel *> &activeChannels);

	/// Changes the interested I/O events;
	/// Must be called in the loop thread
	void updateChannel(Channel *channel);

	/// Remove the channel, when it destructs.
  	/// Must be called in the loop thread.
	void removeChannel(Channel *channel);

	bool isEt() const;

private:
    void fillActiveChannels(int numEvents, epoll_event *revents, std::vector<Channel *> &activeChannels) const;

private:
    EventLoop *m_ownerLoop;
    int m_epollfd;
	bool m_enableET;
    std::map<int, Channel *> m_channels;
	
};

}