/**
 * @file httpconn.cpp
 * @brief  http conn
 * @author Ichheit, <ichheit@outlook.com>
 * @version 
 * @date 2022-08-21
 */
#include "httpconn.h"

namespace wsv
{

bool HttpConn::isET;
const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;

HttpConn::HttpConn() : _isClosed(true), _fd(-1), _iovCnt(0), _readBuff(), _writeBuff() { }
HttpConn::~HttpConn() { close(); }

void HttpConn::init(int sockFd, const sockaddr_in &addr) {
    if (sockFd <= 0) exit(EXIT_FAILURE);
    ++userCount;
    _isClosed = false;
    _addr = addr;
    _fd = sockFd;
    _writeBuff.retrieveAll();
    _readBuff.retrieveAll();

    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", _fd, getIP(), getPort(), (int)userCount);
}

void HttpConn::close() {
    _response.unMapFile();
    if (!_isClosed) {
        _isClosed = !_isClosed;
        --userCount;
        ::close(_fd);
        LOG_INFO("Client[%d](%s:%d) in, userCount:%d", _fd, getIP(), getPort(), (int)userCount);
    }
}

int HttpConn::getFd() const { return _fd; }

int HttpConn::getPort() const { return _addr.sin_port; }

const char* HttpConn::getIP() const { return inet_ntoa(_addr.sin_addr); }

sockaddr_in HttpConn::getAddr() const { return _addr; }

bool HttpConn::isKeepAlive() const { return _request.isKeepAlive(); }

int HttpConn::toWriteBytes() { return _iov[0].iov_len + _iov[1].iov_len; }

ssize_t HttpConn::read(int *saveErrno) {
    ssize_t len = -1;
    do if ((len = _readBuff.readFd(_fd, saveErrno)) <= 0) break; while (isET);
    return len;
}

ssize_t HttpConn::write(int *saveErrno) {
    ssize_t len = -1;
    do {
        if ((len = writev(_fd, _iov, _iovCnt)) <= 0) {
            *saveErrno = errno;
            break;
        }
        if (_iov[0].iov_len + _iov[1].iov_len == 0) {
            break; // 传输结束
        } else if (static_cast<size_t>(len) > _iov[0].iov_len) {
            _iov[1].iov_base = static_cast<uint8_t*>(_iov[1].iov_base) + (len - _iov[0].iov_len);
            _iov[1].iov_len -= (len - _iov[0].iov_len);
            if (_iov[0].iov_len) {
                _writeBuff.retrieveAll();
                _iov[0].iov_len = 0;
            }
        } else {
            _iov[0].iov_base = static_cast<uint8_t*>(_iov[0].iov_base) + len;
            _iov[0].iov_len -= len;
            _writeBuff.retrieve(len);
        }
    } while (isET || toWriteBytes() > 10240);
    return len;
}

bool HttpConn::process() {
    _request.init();
    if (_readBuff.readableBytes() <= 0)
        return false;
    else if (_request.parse(_readBuff))
        _response.init(srcDir, _request.path(), _request.isKeepAlive(), 200);
    else
        _response.init(srcDir, _request.path(), false, 400);
    _response.makeResponse(_writeBuff);
    // 响应头
    _iov[0].iov_base = const_cast<char*>(_writeBuff.peek());
    _iov[0].iov_len = _writeBuff.readableBytes();
    _iovCnt = 1;
    // 文件
    if (_response.fileLen() > 0 && _response.file()) {
        _iov[1].iov_base = _response.file();
        _iov[1].iov_len = _response.fileLen();
        _iovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", _response.fileLen() , _iovCnt, toWriteBytes());
    return true;
}

}
