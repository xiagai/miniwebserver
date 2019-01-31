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
	int64_t getMicroSeconds() const;
	inline bool operator<(const TimeStamp &rhs) const {
		return this->getMicroSeconds() < rhs.getMicroSeconds();
	}
	inline bool operator==(const TimeStamp &rhs) const {
		return this->getMicroSeconds() == rhs.getMicroSeconds();
	}
	inline bool operator>(const TimeStamp &rhs) const {
		return this->getMicroSeconds() > rhs.getMicroSeconds();
	}
	inline bool operator<=(const TimeStamp &rhs) const {
		return !(*this > rhs);
	}
	inline bool operator>=(const TimeStamp &rhs) const {
		return !(*this < rhs);
	}
	bool isValid();

	static TimeStamp now();
	static TimeStamp invalid();
	static TimeStamp addTime(TimeStamp, double);

public:
	const static int kMicroSecPerSec = 1000 * 1000;

private:
	int64_t m_mircoseconds;
};

}

#endif /* TIMESTAMP_H_ */
