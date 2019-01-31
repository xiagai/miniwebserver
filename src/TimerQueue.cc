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
    //memset(&newValue, sizeof(newValue), 0);
    //memset(&oldValue, sizeof(oldValue), 0);
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

TimerQueue::TimerQueue(EventLoop *eventloop) 
    : m_ownerLoop(eventloop),
      m_timerfd(detail::createTimerfd()), 
      m_timerChannel(eventloop, m_timerfd),
      m_timersMap() {
    m_timerChannel.setReadCallback(std::bind(&TimerQueue::handleRead, this));
    m_timerChannel.enableReading();
}

TimerQueue::~TimerQueue() {
    m_timerChannel.disableAll();
    m_timerChannel.remove();
    close(m_timerfd);
}

void TimerQueue::addTimer(const Timer::TimerCallback &cb, TimeStamp when, double interval) {
    std::unique_ptr<Timer> timer = std::make_unique<Timer>(cb, when, interval);
    //  The problem is that std::function must be CopyConstructible, 
    //  which requires its argument (which will be stored by the function) also be CopyConstructible.
    //m_ownerLoop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, std::move(timer)));
    m_ownerLoop->runInLoop(EventLoop::Functor([&]() {
        addTimerInLoop(timer);
    }));
}

void TimerQueue::addTimerInLoop(std::unique_ptr<Timer> &timer) {
    m_ownerLoop->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if (earliestChanged) {
        detail::resetTimerfd(m_timerfd, m_timersMap.cbegin()->second->getExpiration());
    }
}

void TimerQueue::handleRead() {
    m_ownerLoop->assertInLoopThread();
    TimeStamp now = TimeStamp::now();
    detail::readTimerfd(m_timerfd, now);

    std::vector<std::unique_ptr<Timer>> expired = getExpired(now);

    for (auto &each : expired) {
        each->cb();
    }

    reset(expired, now);
}

std::vector<std::unique_ptr<Timer>> TimerQueue::getExpired(TimeStamp now) {
    std::vector<std::unique_ptr<Timer>> expired;
    TimersMap::const_iterator end = m_timersMap.lower_bound(now);
    assert(end == m_timersMap.end() || now < end->first);
    for (auto it = m_timersMap.begin(); it != end; ++it) {
        expired.push_back(std::move(it->second));
    }
    m_timersMap.erase(m_timersMap.begin(), end);
    return expired;
}

void TimerQueue::reset(std::vector<std::unique_ptr<Timer>> &expired, TimeStamp now) {
    for (std::unique_ptr<Timer> &each : expired) {
        if (each->getRepeat()) {
            each->restart(now);
            insert(each);
        }
    }
    TimeStamp nextExpire;
    if (!m_timersMap.empty()) {
        nextExpire = m_timersMap.begin()->second->getExpiration();
    }
    if (nextExpire.isValid()) {
        detail::resetTimerfd(m_timerfd, nextExpire);
    }
}

bool TimerQueue::insert(std::unique_ptr<Timer> &timer) {
    m_ownerLoop->assertInLoopThread();
    bool earlistChanged = false;
    TimeStamp when = timer->getExpiration();
    TimersMap::const_iterator it = m_timersMap.cbegin();
    if (it == m_timersMap.cend() || when < it->first) {
        earlistChanged = true;
    }
    m_timersMap.insert(std::pair<TimeStamp, std::unique_ptr<Timer>>(when, std::move(timer)));
    return earlistChanged;
}

}