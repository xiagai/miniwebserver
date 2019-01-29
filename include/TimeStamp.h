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
	int64_t getMicroSeconds();
	inline bool operator<(TimeStamp rhs) {
		return this->getMicroSeconds() < rhs.getMicroSeconds();
	}
	inline bool operator==(TimeStamp rhs) {
		return this->getMicroSeconds() == rhs.getMicroSeconds();
	}
	inline bool operator>(TimeStamp rhs) {
		return this->getMicroSeconds() > rhs.getMicroSeconds();
	}
	inline bool operator<=(TimeStamp rhs) {
		return !(*this > rhs);
	}
	inline bool operator>=(TimeStamp rhs) {
		return !(*this < rhs);
	}

	static TimeStamp now();
	static TimeStamp invalid();
	static TimeStamp addTime(TimeStamp, double);

private:
	const static int kMicroSecPerSec = 1000 * 1000;
	int64_t m_mircoseconds;
};

}

#endif /* TIMESTAMP_H_ */
