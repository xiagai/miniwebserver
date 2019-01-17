/*
 * TImeStamp.h
 *
 *  Created on: Jan 16, 2019
 *      Author: xiagai
 */

#ifndef TIMESTAMP_H_
#define TIMESTAMP_H_

#include <cstdint>

namespace miniws {

class TimeStamp {
public:
	TimeStamp();
	explicit TimeStamp(int64_t microseconds);

	static TimeStamp now();

private:
	const static int kMicroSecPerSec = 1000 * 1000;
	int64_t m_mircoseconds;
};

}

#endif /* TIMESTAMP_H_ */
