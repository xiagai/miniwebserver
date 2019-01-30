#include "Timer.h"

namespace miniws {

Timer::Timer(TimerCallback cb, TimeStamp expiration, double interval) 
    : m_cb(cb),
      m_expiration(expiration),
      m_interval(interval),
      m_repeat(interval > 0.0) {}

void Timer::cb() {
    m_cb();
}

TimeStamp Timer::getExpiration() {
    return m_expiration;
}

double Timer::getInterval() {
    return m_interval;
}

bool Timer::getRepeat() {
    return m_repeat;
}

void Timer::restart(TimeStamp now) {
    if (m_repeat) {
        m_expiration = TimeStamp::addTime(now, m_interval);
    }
    else {
        m_expiration = TimeStamp::invalid();
    }
}

}