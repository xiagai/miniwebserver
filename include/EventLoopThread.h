/*
 * EventLoopThread.h
 *
 *  Created on: Feb 1, 2019
 *      Author: xiagai
 */

#pragma once
#include "noncopyable.h"
#include "Thread.h"
#include "MutexLocker.h"
#include "MutexLockerGuard.h"
#include "Condition.h"
#include "EventLoop.h"

namespace miniws {

class EventLoopThread : noncopyable{
public:
    EventLoopThread();
    ~EventLoopThread();
    void threadFunc();
    EventLoop *startLoop();
private:
    EventLoop *m_eventLoop;
    MutexLocker m_mutex;
    Condition m_cond;
    Thread m_thread;
};

}