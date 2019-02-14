/*
 * Channel.h
 *
 *  Created on: Jan 15, 2019
 *      Author: xiagai
 */

#ifndef CHANNEL_H_
#define CHANNEL_H_

#include "noncopyable.h"

#include <poll.h>
#include <functional>

namespace miniws {

class EventLoop;

class Channel : noncopyable {
public:
	typedef std::function<void()> EventCallback;

	Channel(EventLoop *loop, int fd);
	~Channel();

	void handleEvent();
	void setReadCallback(const EventCallback& cb);
	void setWriteCallback(const EventCallback& cb);
	void setErrorCallback(const EventCallback& cb);
	void setCloseCallback(const EventCallback& cb);

	int fd() const;
	int events() const;
	void setRevents(int revt);
	bool isNoneEvent() const;

	void enableReading();
	void enableWriting();
	void disableWriting();
	void disableAll();
	void remove();

	// for Poller
	int index();
	void setIndex(int idx);

	EventLoop *ownerLoop();

private:
	void update();

private:
	static const int kNoneEvent = 0;
	static const int kReadEvent = POLLIN | POLLPRI;
	static const int kWriteEvent = POLLOUT;

	EventLoop *m_ownerLoop;
	const int m_fd;
	int m_events;
	int m_revents;
	int m_index;
	bool m_eventHandling;

	EventCallback m_readCallback;
	EventCallback m_writeCallback;
	EventCallback m_errorCallback;
	EventCallback m_closeCallback;
};

}

#endif /* CHANNEL_H_ */
