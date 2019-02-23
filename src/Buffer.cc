/*
 * Buffer.cc
 * 
 *  Created on: Feb 18, 2019
 *      Author: xiagai
 */

#include "Buffer.h"

#include <stdio.h>

namespace miniws {

Buffer::Buffer(size_t capacity) 
    : m_capacity(capacity),
      m_size(0),
      m_start(0),
      m_end(1),
      m_buf(capacity, 0) {}

Buffer::~Buffer() {};

bool Buffer::isEmpty() const {
    return m_size == 0;
}

bool Buffer::isFull() const {
    return m_size == m_capacity;
}

size_t Buffer::remainingSize() const {
    return m_capacity - m_size;
}

size_t Buffer::size() const {
    return m_size;
}

bool Buffer::putIn(char *buf, size_t n) {
    if (n > remainingSize()) {
        return false;
    }
    for (size_t i = 0; i < n; ++i) {
        m_buf[m_end % m_capacity] = buf[i];
        ++m_end;
        ++m_size;
    }
    return true;
}

bool Buffer::takeOut(size_t n) {
    if (n > m_size) {
        return false;
    }
    m_start = (m_start + n) % m_capacity;
    m_size -= n;
    return true;
}

char Buffer::operator[](size_t i) {
    return m_buf[(m_start + i) % m_capacity];
}

}