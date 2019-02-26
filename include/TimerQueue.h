/*
 * TimeQueue.h
 * 
 *  Created on: Jan 29, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "Channel.h"
#include "TimeStamp.h"
#include "Timer.h"
#include "Common.h"

#include <map>
#include <unordered_map>
#include <memory>
#include <atomic>

namespace miniws {

class EventLoop;

class TimerQueue : noncopyable {
public:

    explicit TimerQueue(EventLoop *loop);
    ~TimerQueue();

    //thread safe
    TimerId addTimer(const Timer::TimerCallback &cb, TimeStamp when, double interval);
    void cancelTimer(TimerId);

private:
    typedef std::map<TimeStamp, std::shared_ptr<Timer>> TimersQueue;
    typedef std::unordered_map<TimerId, std::shared_ptr<Timer>> TimersMap;

    void addTimerInLoop(TimerId timerId, std::shared_ptr<Timer> timer);
    void cancelTimerInLoop(TimerId);

    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<std::shared_ptr<Timer>> getExpired(TimeStamp now);
    void reset(std::vector<std::shared_ptr<Timer>> &expired, TimeStamp now);

    // return whether the earliest item in the map is changed
    bool insert(std::shared_ptr<Timer> &timer);
    
private:
    static std::atomic_uint64_t s_sequence;

    EventLoop *m_ownerLoop;
    const int m_timerfd;
    Channel m_timerChannel;
    TimersMap m_timersMap;
    TimersQueue m_timersQueue;

};

}