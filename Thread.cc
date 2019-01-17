/*
 * Thread.cc
 *
 *  Created on: Dec 17, 2018
 *      Author: xiagai
 */

#include "Thread.h"

namespace miniws {

Thread::Thread(ThreadFunc func)
	: m_thread(0),
	  m_tid(0),
	  m_func(func) {}

Thread::~Thread() {}

void Thread::start() {
	if (pthread_create(&m_thread, NULL, startThread, this)) {
		printf("thread creation fails\n");
	}
}

int Thread::join() {
	int ret = pthread_join(m_thread, NULL);
	return ret;
}

void *Thread::startThread(void *args) {
	Thread *thread = static_cast<Thread *>(args);
	thread->m_tid = syscall(SYS_gettid);
	thread->m_func();
	return thread;
}

}


