/*
 * noncopybale.h
 *
 *  Created on: Dec 16, 2018
 *      Author: xiagai
 */

#ifndef NONCOPYABLE_H_
#define NONCOPYABLE_H_

namespace miniws {

class noncopyable {
public:
	noncopyable(const noncopyable &) = delete;
	void operator=(const noncopyable &) = delete;

protected:
	noncopyable() = default;
	~noncopyable() = default;
};

}

#endif /* NONCOPYABLE_H_ */
