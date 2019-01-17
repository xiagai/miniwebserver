/*
 * CurrentThread.h
 *
 *  Created on: Jan 9, 2019
 *      Author: xiagai
 */

#ifndef CURRENTTHREAD_H_
#define CURRENTTHREAD_H_

#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

namespace miniws {

class CurrentThread {
public:
	static pid_t tid();
};

}

#endif /* CURRENTTHREAD_H_ */
