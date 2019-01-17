/*
 * TimeStamp.cc
 *
 *  Created on: Jan 16, 2019
 *      Author: xiagai
 */

#include "TimeStamp.h"

#include <sys/time.h>

namespace miniws {

TimeStamp::TimeStamp()
	: m_mircoseconds(0) {}

TimeStamp::TimeStamp(int64_t microseconds) {
	m_mircoseconds = microseconds;
}

TimeStamp TimeStamp::now() {
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return TimeStamp(tv.tv_sec * kMicroSecPerSec + tv.tv_usec);
}

}


