/*
 * Buffer.h
 * 
 *  Created on: Feb 18, 2019
 *      Author: xiagai
 */

#pragma once

#include <vector>

namespace miniws {

class Buffer {
public:
    Buffer(size_t capacity);
    ~Buffer();

    bool isEmpty() const;
    bool isFull() const;
    size_t remainingSize() const;
    size_t size() const;
    bool putIn(char *buf, size_t len);
    bool takeOut(size_t n);
    /// if i > m_size, return value is undefined;
    char operator[](size_t i);

private:
    const size_t m_capacity;
    size_t m_size;
    size_t m_start;
    size_t m_end;
    std::vector<char> m_buf;
};

}