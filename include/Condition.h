/*
 * Condition.h
 *
 *  Created on: Dec 16, 2018
 *      Author: xiagai
 */

#ifndef CONDITION_H_
#define CONDITION_H_

#include "MutexLockerGuard.h"

namespace miniws {

class Condition : noncopyable {
public:
	explicit Condition(MutexLocker &locker)
	: m_locker(locker) {
		pthread_cond_init(&m_cond, NULL);
	}
	~Condition (){
		pthread_cond_destroy(&m_cond);
	}
	void wait() {
		pthread_cond_wait(&m_cond, m_locker.getPthreadMutex());
	}
	void notify() {
		pthread_cond_signal(&m_cond);
	}
	void notifyAll() {
		pthread_cond_broadcast(&m_cond);
	}
private:
	MutexLocker& m_locker;
	pthread_cond_t m_cond;
};

}



#endif /* CONDITION_H_ */
