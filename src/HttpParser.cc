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
#include <sys/mman.h>
#include <stdarg.h>

namespace miniws {

const char* HttpParser::ok_200_title = "OK";
const char* HttpParser::error_400_title = "Bad Request";
const char* HttpParser::error_400_form = "Your requests has bad syntax or is inherently impossible to satisfy.\n";
const char* HttpParser::error_403_title = "Forbidden";
const char* HttpParser::error_403_form = "You do not have permission to get file from this server.\n";
const char* HttpParser::error_404_title = "Not Found";
const char* HttpParser::error_404_form = "The requested file was not found on this server.\n";
const char* HttpParser::error_500_title = "Internal Error";
const char* HttpParser::error_500_form = "There was an unusual problem serving the requested file.\n";

HttpParser::HttpParser(char *homeDir, Buffer &readBuf)
    : m_homeDir(homeDir),
      m_checkState(CHECK_STATE_REQUESTLINE),
      m_method(GET),
      m_readBuf(readBuf),
      m_readIdx(readBuf.size()),
      m_lineStart(0),
      m_lineEnd(0),
      m_contentLen(0),
      m_linger(false),
      m_fileAddress(nullptr),
      m_writeIdx(0),
      m_status(0),
      m_title(nullptr) {

}

HttpParser::~HttpParser() {
    if (m_fileAddress) {
        munmap(m_fileAddress, m_fileStat.st_size);
        m_fileAddress = nullptr;
    }
}

struct httpret HttpParser::process() {
    HTTP_CODE res = processRead();
    httpret ret;
    if (processWrite(res)) {
        ret.iov = m_iov;
        ret.iovlen = 2;
        ret.keepAlive = m_linger;
        return ret;
    }
    else {
        printf("SYS_DEBUG HttpParser::process\n");
    }
}

HttpParser::HTTP_CODE HttpParser::processRead() {
    LINE_STATUS lineStatus = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    while (m_checkState == CHECK_STATE_CONTENT || (lineStatus = parseLine()) == LINE_OK) {
        switch (m_checkState) {
            case CHECK_STATE_REQUESTLINE:
                ret = parseRequestLine();
                if (ret == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                break;
            case CHECK_STATE_HEADER:
                ret = parseHeaders();
                if (ret == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                else if (ret == GET_REQUEST) {
                    return doRequest();
                }
                break;
            case CHECK_STATE_CONTENT:
                ret = parseContent();
                if (ret == GET_REQUEST) {
                    return doRequest();
                }
                lineStatus = LINE_OPEN;
                break;
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

bool HttpParser::processWrite(HTTP_CODE httpCode) {
    switch(httpCode) {
        case FILE_REQUEST:
            m_status = ok_200;
            m_title = ok_200_title;
            break;
        case BAD_REQUEST:
            m_status = error_400;
            m_title = error_400_title;
            strncpy(m_form, error_400_form, strlen(error_400_form));
        case FORBIDDEN_REQUEST:
            m_status = error_403;
            m_title = error_403_title;
            strncpy(m_form, error_403_form, strlen(error_403_form));
        case NO_RESOURCE:
            m_status = error_404;
            m_title = error_404_title;
            strncpy(m_form, error_404_form, strlen(error_404_form));
        case INTERNAL_ERROR:
            m_status = error_500;
            m_title = error_500_title;
            strncpy(m_form, error_500_form, strlen(error_500_form));
        default:
            printf("SYS_DEBUG HttpParser::processWrite\n");
    }
    bool ret = addStatusLine() && addHeaders();
    addResponse();
    addContent();
    return ret;
}

HttpParser::LINE_STATUS HttpParser::parseLine() {
    size_t crlf = m_readBuf.findCRLF(m_lineStart);
    if (crlf == m_readBuf.size()) {
        return LINE_OPEN;
    }
    else {
        m_lineEnd = crlf;
        return LINE_OK;
    }
}

HttpParser::HTTP_CODE HttpParser::parseRequestLine() {
    //解析method
    size_t begin = m_readBuf.skipSpace(m_lineStart);
    size_t space = m_readBuf.findSpace(begin, m_lineEnd);
    if (space == m_readBuf.size()) {
        return BAD_REQUEST;
    }
    std::string method = m_readBuf.getStringPiece(begin, space - begin);
    if (method == "GET") {
        m_method = GET;
    }
    else {
        printf("only support get method now.\n");
        return BAD_REQUEST;
    }

    //解析url
    begin = m_readBuf.skipSpace(space);
    space = m_readBuf.findSpace(begin, m_lineEnd);
    if (space == m_readBuf.size()) {
        return BAD_REQUEST;
    }
    m_url = m_readBuf.getStringPiece(begin, space - begin);

    //解析version
    begin = m_readBuf.skipSpace(space);
    m_version = m_readBuf.getStringPiece(begin, m_lineEnd - begin);
    if (m_version.compare("HTTP/1.1") != 0) {
        printf("only support http 1.1 now.\n");
        return BAD_REQUEST;
    }
    m_checkState = CHECK_STATE_HEADER;
    //结尾需跳过/r/n两个字符的位置才是下一行的开始
    m_lineStart = m_lineEnd + 2;
    return NO_REQUEST;
}

HttpParser::HTTP_CODE HttpParser::parseHeaders() {
    //headers部分结束，根据content的长度判断有没有content部分
    if (m_lineStart == m_lineEnd) {
        m_lineStart = m_lineEnd + 2;
        if (m_contentLen != 0) {
            m_contentLen = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        else {
            return GET_REQUEST;
        }
    }
    //解析header
    else {
        std::string header = m_readBuf.getStringPiece(m_lineStart, m_lineEnd - m_lineStart);
        if (header.compare(0, 11, "Connection:") == 0) {
            size_t space = m_readBuf.skipSpace(m_lineStart + 11);
            if (header.compare(space - m_lineStart, header.length() - (space - m_lineStart), "keep-alive") == 0) {
                m_linger = true;
            }
        }
        else if (header.compare(0, 15, "Content-Length:") == 0) {
            size_t space = m_readBuf.skipSpace(m_lineStart + 15);
            m_contentLen = std::stoi(header.substr(space - m_lineStart, header.length() - (space - m_lineStart)));
        }
        else if (header.compare(0, 5, "Host:") == 0) {
            size_t space = m_readBuf.skipSpace(m_lineStart + 5);
            m_host = header.substr(space - m_lineStart, header.length() - (space - m_lineStart));
        }
        else {
            printf("only support connection, content-length, host headers now, and unknow header %s\n", header.c_str());
        }
        m_lineStart = m_lineEnd + 2;
        return NO_REQUEST;
    }
}

HttpParser::HTTP_CODE HttpParser::parseContent() {
    //目前没对content做任何处理
    if (m_readIdx >= (m_lineStart + m_contentLen)) {
        m_lineStart += m_contentLen;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

HttpParser::HTTP_CODE HttpParser::doRequest() {
    //将已解析的http报文移出缓冲区
    m_readBuf.takeOut(m_lineStart);
    strncpy(m_readFile, m_homeDir, FILENAME_LEN);
    int len = strlen(m_homeDir);
    strncpy(m_readFile + len, m_url.c_str(), FILENAME_LEN - len - 1);
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
    int fd = ::open(m_readFile, O_RDONLY);
    m_fileAddress = (char *)mmap(0, m_fileStat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

bool HttpParser::addLine(const char *format, ...) {
    if (m_writeIdx >= WRITE_BUFFER_SIZE) {
        return false;
    }
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_response + m_writeIdx, WRITE_BUFFER_SIZE - 1 - m_writeIdx, format, arg_list);
    if (len >= (WRITE_BUFFER_SIZE - 1 - m_writeIdx))  {
        return false;
    }
    m_writeIdx += len;
    va_end(arg_list);
    return true;
}

bool HttpParser::addStatusLine() {
    return addLine("%s %d %s\r\n", "HTTP/1.1", m_status, m_title);
}

bool HttpParser::addHeaders() {
    bool ret = true;
    if (m_status == ok_200) {
        ret = addLine("Content-Length: %d\r\n", m_fileStat.st_size);
    }
    else {
        ret = addLine("Content-Length: %d\r\n", strlen(m_form));
    }
    if (!ret) {
        return ret;
    }
    ret = addLine("Connection: %s\r\n", (m_linger ? "keep-alive" : "close"));
    if (!ret) {
        return ret;
    }
    ret = addLine("%s", "\r\n");
    if (!ret) {
        return ret;
    }
    return ret;
}

void HttpParser::addResponse() {
    m_iov[0].iov_base = m_response;
    m_iov[0].iov_len = m_writeIdx;
}

void HttpParser::addContent() {
    if (m_status == ok_200) {
        //FIXME file size == 0
        m_iov[1].iov_base = m_fileAddress;
        m_iov[1].iov_len = m_fileStat.st_size;
    }
    else {
        m_iov[1].iov_base = m_form;
        m_iov[1].iov_len = strlen(m_form);
    }
}

}