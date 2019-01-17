/*
 * CurrentThread.cc
 *
 *  Created on: Jan 9, 2019
 *      Author: xiagai
 */


#include "CurrentThread.h"

namespace miniws {

pid_t CurrentThread::tid() {
	return syscall(SYS_gettid);
}

}

