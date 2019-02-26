#include "Timer.h"

namespace miniws {

Timer::Timer(TimerId id, TimerCallback cb, TimeStamp expiration, double interval) 
    : m_timerId(id),
      m_cb(cb),
      m_expiration(expiration),
      m_interval(interval),
      m_repeat(interval > 0.0),
      m_toCancel(false) {}

void Timer::cb() {
    m_cb();
}

TimerId Timer::getTimerId() const {
    return m_timerId;
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

bool Timer::toCancel() const {
    return m_toCancel;
}

void Timer::setToCancel(bool on) {
    m_toCancel = on;
}

}