/*
 * TimeQueue.h
 * 
 *  Created on: Jan 29, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "EventLoop.h"
#include "Channel.h"
#include "TimeStamp.h"
#include "Timer.h"

#include <map>

namespace miniws {

class TimerQueue : noncopyable {
public:

    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    // Must be thread safe
    void addTimer(const Timer::TimerCallback &cb, TimeStamp when, double interval);

private:
    typedef std::map<TimeStamp, std::unique_ptr<Timer>> TimersMap;

    void addTimerInLoop(std::unique_ptr<Timer> &timer);

    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<std::unique_ptr<Timer>> getExpired(TimeStamp now);
    void reset(std::vector<std::unique_ptr<Timer>> &expired, TimeStamp now);

    // return whether the earliest item in the map is changed
    bool insert(std::unique_ptr<Timer> &timer);
    
private:
    EventLoop *m_ownerLoop;
    const int m_timerfd;
    Channel m_timerChannel;
    TimersMap m_timersMap;
};

}