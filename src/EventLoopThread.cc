/*
 * EventLoopThread.cc
 *
 *  Created on: Feb 1, 2019
 *      Author: xiagai
 */

#include "EventLoopThread.h"

namespace miniws {

EventLoopThread::EventLoopThread()
    : m_eventLoop(nullptr),
      m_mutex(),
      m_cond(m_mutex),
      m_thread(std::bind(&EventLoopThread::threadFunc, this)) {
    
}

EventLoopThread::~EventLoopThread() {
    if (m_eventLoop != nullptr) {
        m_eventLoop->quit();
        m_thread.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    m_thread.start();
    {
        MutexLockerGuard lock(m_mutex);
        while (m_eventLoop == nullptr) {
            m_cond.wait();
        }
    }
    return m_eventLoop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    {
        MutexLockerGuard lock(m_mutex);
        m_eventLoop = &loop;
        m_cond.notify();
    }
    loop.loop();
}

}