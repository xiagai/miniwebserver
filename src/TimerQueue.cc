#include "TimerQueue.h"
#include "EventLoop.h"

#include <functional>
#include <sys/timerfd.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

namespace miniws {

namespace detail {

int createTimerfd() {
    int timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0) {
        printf("LOG_SYSFATAL Failed in timerfd_create\n");
    }
    return timerfd;
}

timespec howMuchTimeFromNow(TimeStamp when) {
    TimeStamp now = TimeStamp::now();
    int64_t microDiff = when.getMicroSeconds() - now.getMicroSeconds();
    timespec diff;
    diff.tv_sec = static_cast<time_t>(microDiff / TimeStamp::kMicroSecPerSec);
    diff.tv_nsec = static_cast<long>((microDiff % TimeStamp::kMicroSecPerSec) * 1000);
    return diff;
}

void resetTimerfd(int timerfd, TimeStamp expiration) {
    itimerspec newValue;
    itimerspec oldValue;
    //memset(&newValue, 0, sizeof(newValue));
    //memset(&oldValue, 0, izeof(oldValue));
    newValue.it_interval.tv_nsec = 0;
    newValue.it_interval.tv_sec = 0;
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret) {
        printf("LOG_SYSERR timerfd_settime()\n");
    }
}

void readTimerfd(int timerfd, TimeStamp now) {
    uint64_t howmany;
    ssize_t n = read(timerfd, &howmany, sizeof(howmany));
    printf("LOG_TRACE TimerQueue::handleRead() %ld at %ld\n", howmany, now.getMicroSeconds());
    if (n != sizeof(howmany)) {
        printf("LOG_ERROR TimerQueue::handleRead() reads %ld bytes instead of 8\n", n);
    }
}

}

std::atomic_uint64_t TimerQueue::s_sequence(1);

TimerQueue::TimerQueue(EventLoop *eventloop) 
    : m_ownerLoop(eventloop),
      m_timerfd(detail::createTimerfd()), 
      m_timerChannel(eventloop, m_timerfd) {
    m_timerChannel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    m_timerChannel.enableReading();
}

TimerQueue::~TimerQueue() {
    m_timerChannel.disableAll();
    m_timerChannel.remove();
    close(m_timerfd);
}

TimerId TimerQueue::addTimer(const Timer::TimerCallback &cb, TimeStamp when, double interval) {
    TimerId timerId = s_sequence++;
    std::shared_ptr<Timer> timer = std::make_shared<Timer>(timerId, cb, when, interval);

    m_ownerLoop->runInLoop(EventLoop::Functor([&]() {
        addTimerInLoop(timerId, timer); 
    }));
    return timerId;
}

void TimerQueue::cancelTimer(TimerId timerId) {
    m_ownerLoop->runInLoop(EventLoop::Functor([&]() {
        cancelTimerInLoop(timerId); 
    }));
}

void TimerQueue::addTimerInLoop(TimerId timerId, std::shared_ptr<Timer> timer) {
    m_ownerLoop->assertInLoopThread();
    m_timersMap.insert(std::pair<TimerId, std::shared_ptr<Timer>>(timerId, timer));
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        detail::resetTimerfd(m_timerfd, m_timersQueue.cbegin()->second->getExpiration());
    }
}

void TimerQueue::cancelTimerInLoop(TimerId timerId) {
    m_ownerLoop->assertInLoopThread();
    if (m_timersMap.find(timerId) != m_timersMap.end()) {
        m_timersMap[timerId]->setToCancel(true);
    }
}

void TimerQueue::handleRead() {
    m_ownerLoop->assertInLoopThread();
    TimeStamp now = TimeStamp::now();
    detail::readTimerfd(m_timerfd, now);

    std::vector<std::shared_ptr<Timer>> expired = getExpired(now);

    for (auto &each : expired) {
        if (!each->toCancel()) {
            each->cb();
        }
    }

    reset(expired, now);
}

std::vector<std::shared_ptr<Timer>> TimerQueue::getExpired(TimeStamp now) {
    std::vector<std::shared_ptr<Timer>> expired;
    TimersQueue::const_iterator end = m_timersQueue.lower_bound(now);
    assert(end == m_timersQueue.end() || now < end->first);
    for (auto it = m_timersQueue.begin(); it != end; ++it) {
        expired.push_back(std::move(it->second));
    }
    m_timersQueue.erase(m_timersQueue.begin(), end);
    return expired;
}

void TimerQueue::reset(std::vector<std::shared_ptr<Timer>> &expired, TimeStamp now) {
    for (std::shared_ptr<Timer> &each : expired) {
        if (!each->toCancel() && each->getRepeat()) {
            each->restart(now);
            insert(each);
        }
        else {
            m_timersMap.erase(each->getTimerId());
        }
    }
    TimeStamp nextExpire;
    if (!m_timersQueue.empty()) {
        nextExpire = m_timersQueue.begin()->second->getExpiration();
    }
    if (nextExpire.isValid()) {
        detail::resetTimerfd(m_timerfd, nextExpire);
    }
}

bool TimerQueue::insert(std::shared_ptr<Timer> &timer) {
    m_ownerLoop->assertInLoopThread();
    bool earlistChanged = false;
    TimeStamp when = timer->getExpiration();
    TimersQueue::const_iterator it = m_timersQueue.cbegin();
    if (it == m_timersQueue.cend() || when < it->first) {
        earlistChanged = true;
    }
    m_timersQueue.insert(std::pair<TimeStamp, std::shared_ptr<Timer>>(when, std::move(timer)));
    return earlistChanged;
}

}