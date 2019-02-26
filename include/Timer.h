/*
 * Time.h
 * 
 *  Created on: Jan 17, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"
#include "Common.h"

#include <functional>

namespace miniws {

class Timer : noncopyable {
public:
    typedef std::function<void()> TimerCallback;

    Timer(TimerId id, TimerCallback cb, TimeStamp expiration, double interval);

    void cb();
    TimerId getTimerId() const;
    TimeStamp getExpiration();
    double getInterval();
    bool getRepeat();
    void restart(TimeStamp now);
    bool toCancel() const;
    void setToCancel(bool on);

private:
    // set only once
    const TimerId m_timerId;
    const TimerCallback m_cb;
    TimeStamp m_expiration;
    const double m_interval;
    const bool m_repeat;

    bool m_toCancel;
    
};

}