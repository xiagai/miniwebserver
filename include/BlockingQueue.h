/*
 * BlockingQueue.h
 *
 *  Created on: Dec 16, 2018
 *      Author: xiagai
 */

#ifndef BLOCKINGQUEUE_H_
#define BLOCKINGQUEUE_H_

#include "Condition.h"

#include <assert.h>
#include <queue>

namespace miniws {

template<typename T>
class BlockingQueue : noncopyable {
	BlockingQueue()
	: m_locker(),
	  m_cond(m_locker),
	  m_queue() {}

	void push(const T &x) {
		MutexLockerGuard lock(m_locker);
		m_queue.push(x);
		m_cond.notify();
	}

	T &pop() {
		MutexLockerGuard lock(m_locker);
		while (m_queue.isEmpty()) {
			m_cond.wait();
		}
		assert(!m_queue.isEmpty());
		T x(m_queue.front());
		m_queue.pop();
		return x;
	}

	size_t size() const {
		MutexLockerGuard lock(m_locker);
		return m_queue.size();
	}

private:
	MutexLocker m_locker;
	Condition m_cond;
	std::queue<T> m_queue;
};

}


#endif /* BLOCKINGQUEUE_H_ */
