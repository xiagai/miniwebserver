/*
 * MutexLocker.h
 *
 *  Created on: Dec 16, 2018
 *      Author: xiagai
 */

#ifndef MUTEXLOCKER_H_
#define MUTEXLOCKER_H_

#include "noncopyable.h"

#include <pthread.h>

namespace miniws {

class MutexLocker : noncopyable {
public:
	MutexLocker() {
		pthread_mutex_init(&m_mutex, NULL);
	}
	~MutexLocker() {
		pthread_mutex_destroy(&m_mutex);
	}
	void lock() {
		pthread_mutex_lock(&m_mutex);
	}
	void unlock() {
		pthread_mutex_unlock(&m_mutex);
	}
	pthread_mutex_t *getPthreadMutex() {
		return &m_mutex;
	}

private:
	pthread_mutex_t m_mutex;
};

}



#endif /* MUTEXLOCKER_H_ */
