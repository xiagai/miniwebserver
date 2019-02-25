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
      m_end(0),
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
        m_end = (m_end + 1) % m_capacity;
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

char &Buffer::operator[](size_t i) {
    return m_buf[(m_start + i) % m_capacity];
}

size_t Buffer::findCRLF(size_t pos) const {
    for (; pos + 1 < m_size; ++pos) {
        if (m_buf[(m_start + pos) % m_capacity] == '\r' && m_buf[(m_start + pos + 1) % m_capacity] == '\n') {
            return pos;
        }
    }
    return m_size;
}

size_t Buffer::findSpace(size_t l, size_t r) const {
    if (l >= m_size || r >= m_size) {
        return m_size;
    }
    for (; l < r; ++l) {
        if (m_buf[(m_start + l) % m_capacity] == ' ' || m_buf[(m_start + l) % m_capacity] == '\t') {
            return l;
        }
    }
    return m_size;
}

size_t Buffer::skipSpace(size_t pos) const {
    if (pos >= m_size) {
        return m_size;
    }
    while (pos < m_size) {
        if (!(m_buf[(m_start + pos) % m_capacity] == ' ' || m_buf[(m_start + pos) % m_capacity] == '\t')) {
            return pos;
        }
        pos++;
    }
    return m_size;
}

std::string Buffer::getStringPiece(size_t pos, size_t len) const {
    if (pos + len > m_size) {
        return "";
    }
    if (((m_start + pos) % m_capacity + len) <= m_capacity) {
        return std::string(m_buf.begin() + (m_start + pos) % m_capacity, m_buf.begin() + (m_start + pos) % m_capacity + len);
    }
    else {
        std::string str1(m_buf.begin() + (m_start + pos) % m_capacity, m_buf.end());
        std::string str2(m_buf.begin(), m_buf.begin() + (m_start + pos + len) % m_capacity);
        return str1 + str2;
    }
}

}