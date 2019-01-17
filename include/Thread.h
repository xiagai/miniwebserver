/*
 * Thread.h
 *
 *  Created on: Dec 17, 2018
 *      Author: xiagai
 */

#ifndef THREAD_H_
#define THREAD_H_

#include "noncopyable.h"

#include <pthread.h>
#include <functional>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace miniws {

class Thread : noncopyable {
public:
	typedef std::function<void()> ThreadFunc;
	//typedef void *(*ThreadFunc)(void *);
	Thread(ThreadFunc func);
	~Thread();
	void start();
	int join();

private:
	static void *startThread(void *);

private:
	pthread_t m_thread;
	pid_t m_tid;
	ThreadFunc m_func;
};

}



#endif /* THREAD_H_ */
