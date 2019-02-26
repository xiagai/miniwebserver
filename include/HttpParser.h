/*
 * HttpParser.h
 * 
 *  Created on: Feb 20, 2019
 *      Author: xiagai
 */

#pragma once

#include "noncopyable.h"
#include "Buffer.h"

#include <sys/stat.h>
#include <sys/socket.h>
#include <string>

namespace miniws {

struct httpret {
    struct iovec *iov = nullptr;
    size_t iovlen = 0;
    bool keepAlive = false;
};

class HttpParser : noncopyable {
public:
    enum METHOD {
        GET,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATCH
    };

    enum CHECK_STATE {
        CHECK_STATE_REQUESTLINE, //正在分析请求行
        CHECK_STATE_HEADER, //正在分析头部字段
        CHECK_STATE_CONTENT //正在分析请求体
    };

    enum HTTP_CODE {
        NO_REQUEST, //请求不完整，需要继续读取客户数据
        GET_REQUEST, //获得了一个完整的客户请求
        BAD_REQUEST, //客户请求有语法错误
        NO_RESOURCE, //资源不存在
        FORBIDDEN_REQUEST, //客户对资源没有足够的访问权限
        FILE_REQUEST, //文件请求
        INTERNAL_ERROR, //服务器内部错误
        CLOSED_CONNECTION //客户端已经关闭连接
    };

    enum LINE_STATUS {
        LINE_OK, //读到一个完整的行
        LINE_BAD, //行出错
        LINE_OPEN //行数据尚且不完整
    };

public:
    HttpParser(char *homeDir, Buffer &readBuf);
    ~HttpParser();

    struct httpret process();
    HTTP_CODE processRead();
    bool processWrite(HTTP_CODE);

private:
    // used by read
    LINE_STATUS parseLine();
    HTTP_CODE parseRequestLine();
    HTTP_CODE parseHeaders();
    HTTP_CODE parseContent();
    HTTP_CODE doRequest();

    // used by write
    bool addLine(const char *format, ...);
    bool addStatusLine();
    bool addHeaders();
    void addResponse();
    void addContent();

private:
    static const int FILENAME_LEN = 200;
    static const int WRITE_BUFFER_SIZE = 1024;
    static const int FORM_MAX_SIZE = 256;
    static const int ok_200 = 200;
    static const int error_400 = 400;
    static const int error_403 = 403;
    static const int error_404 = 404;
    static const int error_500 = 500;
    static const char* ok_200_title;
    static const char* error_400_title;
    static const char* error_400_form;
    static const char* error_403_title;
    static const char* error_403_form;
    static const char* error_404_title;
    static const char* error_404_form;
    static const char* error_500_title;
    static const char* error_500_form;

    CHECK_STATE m_checkState;
    METHOD m_method;
    char *m_homeDir;

    //read buffer
    Buffer &m_readBuf;
    size_t m_readIdx;
    size_t m_lineStart;
    size_t m_lineEnd;

    //request arguments
    std::string m_url;
    std::string m_version;
    std::string m_host;
    int m_contentLen;
    bool m_linger;

    //file arguments
    char *m_fileAddress;
    char m_readFile[FILENAME_LEN];
    struct stat m_fileStat;

    //response arguments
    char m_response[WRITE_BUFFER_SIZE];
    size_t m_writeIdx;
    struct iovec m_iov[2];
    int m_status;
    const char *m_title;
    char m_form[FORM_MAX_SIZE];
};

}