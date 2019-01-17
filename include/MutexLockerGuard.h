/*
 * MutexLockerGuard.h
 *
 *  Created on: Dec 16, 2018
 *      Author: xiagai
 */

#ifndef MUTEXLOCKERGUARD_H_
#define MUTEXLOCKERGUARD_H_

#include "MutexLocker.h"

namespace miniws {

class MutexLockerGuard : noncopyable {
	explicit MutexLockerGuard(MutexLocker locker)
	: m_locker(locker) {
		m_locker.lock();
	}
	~MutexLockerGuard() {
		m_locker.unlock();
	}

private:
	MutexLocker& m_locker;
};

}

#define MutexLockerGuard(x) error "Missing guard object name"

#endif /* MUTEXLOCKERGUARD_H_ */
