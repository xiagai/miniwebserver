/*
 * Time.h
 * 
 *  Created on: Jan 17, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "TimeStamp.h"

#include <functional>

namespace miniws {

class Timer : noncopyable {
public:
    typedef std::function<void()> TimerCallback;

    Timer(TimerCallback cb, TimeStamp expiration, double interval);

    void cb();
    TimeStamp getExpiration();
    double getInterval();
    bool getRepeat();
    void restart(TimeStamp now);

private:
    // set only once
    const TimerCallback m_cb;
    TimeStamp m_expiration;
    const double m_interval;
    const bool m_repeat;
};

}