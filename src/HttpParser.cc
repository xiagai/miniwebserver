/*
 * HttpParser.cc
 * 
 *  Created on: Feb 20, 2019
 *      Author: xiagai
 */

#include "HttpParser.h"
#include "Common.h"

#include <iostream>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

namespace miniws {

HttpParser::HttpParser(char *readBuf) 
    : m_checkState(),
      m_method(),
      m_readBuf(readBuf),
      m_readIdx(0),
      m_checkedIdx() {

}

HttpParser::~HttpParser() {}

HttpParser::HTTP_CODE HttpParser::processRead() {
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text = nullptr;
    while (((m_checkState == CHECK_STATE_CONTENT) && (line_status ==  LINE_OK)) ||
        ((line_status = parseLine()) == LINE_OK)) {
        text = getLine();
        m_startLine = m_checkedIdx;

        switch (m_checkState) {
            case CHECK_STATE_REQUESTLINE:
                ret = parseRequestLine(text);
                if (ret == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                ret = parseHeaders(text);
                if (ret == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                else if ( ret == GET_REQUEST) {
                    return doRequest();
                }
                break;
            case CHECK_STATE_CONTENT:
                ret = parseContent(text);
                if (ret == GET_REQUEST) {
                    return doRequest();
                }
                line_status = LINE_OPEN;
                break;
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

//解析一行内容
HttpParser::LINE_STATUS HttpParser::parseLine() {
    char temp;
    for (; m_checkedIdx < m_readIdx; ++m_checkedIdx) {
        temp = m_readBuf[m_checkedIdx];
        if (temp == '\r') {
            if (m_checkedIdx + 1 == m_readIdx) {
                return LINE_OPEN;
            }
            else if (m_readBuf[m_checkedIdx + 1] == '\n') {
                m_readBuf[m_checkedIdx++] == '\0';
                m_readBuf[m_checkedIdx++] == '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == '\n') {
            printf("没有检测到回车符再检测到换行符的情况\n");
            if ((m_checkedIdx > 1) && (m_readBuf[m_checkedIdx - 1] == '\r')) {
                m_readBuf[m_checkedIdx - 1] = '\0';
                m_readBuf[m_checkedIdx++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

char *HttpParser::getLine() {
    return m_readBuf + m_startLine;
}

HttpParser::HTTP_CODE HttpParser::parseRequestLine(char *text) {
    m_url = strpbrk(text, " \t");
    if (!m_url) {
        return BAD_REQUEST;
    }
    *m_url++ = '\0';

    char *method = text;
    if (strcasecmp(method, "GET") == 0) { //因为前面的操作已经在method字符串部分后面添加了字符串结尾标志，故可以这么干
        m_method = GET;
    }
    else {
        printf("only support get method now.\n");
        return BAD_REQUEST;
    }

    m_url += strspn(m_url, " \t"); //跳过所有method和url之间的空格
    m_version = strpbrk(m_url, " \t");
    if (!m_version) {
        return BAD_REQUEST;
    }
    *m_version++ = '\0';
    m_version += strspn(m_version, " \t");
    if (strcasecmp(m_version, "HTTP/1.1") != 0) {
        printf("only support http 1.1 now.\n");
        return BAD_REQUEST;
    }
    if (strncasecmp(m_url, "http://", 7) == 0) {
        m_url += 7;
        m_url = strchr(m_url, '/');
    }

    if (!m_url || m_url[0] != '/') {
        return BAD_REQUEST;
    }
    m_checkState = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpParser::HTTP_CODE HttpParser::parseHeaders(char *text) {
    if (text[0] == '\0') {
        if (m_contentLen != 0) {
            m_checkState = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if (strncasecmp(text, "Connection:", 11) == 0) {
        text += 11;
        text += strspn(text, " \t");
        if (strcasecmp(text, "keep-alive") == 0) {
            m_linger = true;
        }
    }
    else if (strncasecmp(text, "Content-Length:", 15) == 0) {
        text += 15;
        text += strspn(text, " \t");
        m_contentLen = atoi(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0) {
        text += 5;
        text += strspn(text, " \t");
        m_host = text;
    }
    else {
        printf("only support connection, content-length, host headers now, and unknow header %s\n", text);
    }
    return NO_REQUEST;
}

HttpParser::HTTP_CODE HttpParser::parseContent(char *text) {
    if (m_readIdx >= (m_checkedIdx + m_contentLen)) {
        text[m_contentLen] = '\0';
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

HttpParser::HTTP_CODE HttpParser::doRequest() {
    strncpy(m_readFile, home_dir, FILENAME_LEN);
    int len = strlen(home_dir);
    strncpy(m_readFile + len, m_url, FILENAME_LEN - len - 1);
    if (stat(m_readFile, &m_fileStat) < 0) {
        char strerr[64];
        printf("SYS_INFO HttpParser::deRequest %s\n", strerror_r(errno, strerr, sizeof strerr));
        return NO_RESOURCE;
    }
    if (!(m_fileStat.st_mode & S_IROTH)) {
        return FORBIDDEN_REQUEST;
    }
    if (!S_ISREG(m_fileStat.st_mode)) {
        printf("SYS_INFO HttpParser::doRequest not regular file\n");
        return BAD_REQUEST;
    }
    // int fd = ::open(m_readFile, O_RDONLY);
    // m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    // close(fd);
    return FILE_REQUEST;
}

}