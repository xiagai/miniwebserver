/*
 * EventLoopThreadPool.cc
 * 
 *  Created on: Feb 15, 2019
 *      Author: xiagai
 */

#include "EventLoopThreadPool.h"

#include <assert.h>

namespace miniws {

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : m_baseLoop(baseLoop),
      m_started(false),
      m_numThreads(numThreads),
      m_next(0) {
    assert(numThreads > 0);
}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start() {
    m_baseLoop->assertInLoopThread();
    m_started = true;
    for (int i = 0; i < m_numThreads; ++i) {
        std::shared_ptr<EventLoopThread> t = std::make_shared<EventLoopThread>();
        m_threads.push_back(t);
        m_loops.push_back(t->startLoop());
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    m_baseLoop->assertInLoopThread();
    assert(m_started);
    EventLoop *loop = m_baseLoop;
    if (!m_loops.empty()) {
        loop = m_loops[m_next];
        m_next = (m_next + 1) % m_numThreads;
    }
    return loop;
}

}