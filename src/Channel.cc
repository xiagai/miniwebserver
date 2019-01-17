/*
 * Channel.cc
 *
 *  Created on: Jan 15, 2019
 *      Author: xiagai
 */


#include "Channel.h"
#include "EventLoop.h"

namespace miniws {

Channel::Channel(EventLoop *loop, int fd)
	: m_ownerLoop(loop),
	  m_fd(fd),
	  m_events(0),
	  m_revents(0),
	  m_index(-1) {

}
void Channel::handleEvent() {
	if (m_revents & POLLNVAL) {
		printf("LOG_WARN Channel::handleEvent() POLLNVAL");
	}
	if (m_revents & (POLLERR | POLLNVAL)) {
		if (m_errorCallback) {
			m_errorCallback();
		}
	}
	if (m_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
		if (m_readCallback) {
			m_readCallback();
		}
	}
	if (m_revents & (POLLOUT)) {
		if (m_writeCallback) {
			m_writeCallback();
		}
	}
}
void Channel::setReadCallback(const EventCallback& cb) {
	m_readCallback = cb;
}
void Channel::setWriteCallback(const EventCallback& cb) {
	m_writeCallback = cb;
}
void Channel::setErrorCallback(const EventCallback& cb) {
	m_errorCallback = cb;
}
int Channel::fd() const {
	return m_fd;
}
int Channel::events() const {
	return m_events;
}
void Channel::setRevents(int revt) {
	m_revents = revt;
}
bool Channel::isNoneEvent() const {
	return m_events == kNoneEvent;
}
void Channel::enableReading() {
	m_events |= kReadEvent;
	update();
}
void Channel::enableWriting() {
	m_events |= kWriteEvent;
	update();
}
void Channel::disableWriting() {
	m_events &= ~kWriteEvent;
	update();
}
void Channel::disableAll() {
	m_events = kNoneEvent;
	update();
}
int Channel::index() {
	return m_index;
}
void Channel::setIndex(int idx) {
	m_index = idx;
}

EventLoop *Channel::ownerLoop() {
	return m_ownerLoop;
}
void Channel::update() {
	m_ownerLoop->updateChannel(this);
}


}


