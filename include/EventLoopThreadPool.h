/*
 * EventLoopThreadPool.h
 * 
 *  Created on: Feb 15, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "EventLoopThread.h"

#include <vector>
#include <memory>

namespace miniws {

class EventLoopThreadPool : noncopyable {
public:
    EventLoopThreadPool(EventLoop *baseLoop, int numThreads);
    ~EventLoopThreadPool();
    void start();
    EventLoop *getNextLoop();
private:
    EventLoop *m_baseLoop;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<std::shared_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop *> m_loops;
};

}