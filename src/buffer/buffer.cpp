/**
 * @file buffer.cpp
 * @brief  buffer
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-18
 */
#include "buffer.h"

#define ERR_EXIT(msg) do { fprintf(stderr, "[buffer error]: %s", msg); exit(EXIT_FAILURE); } while (0)
#define NOT_ERR_EXIT(exp, msg) do { if (!(exp)) { ERR_EXIT(msg); } } while (0)

namespace wsv
{

Buffer::Buffer(int initBuffSize) : _buffer(initBuffSize), _readPos(0), _writePos(0) {  }

size_t Buffer::writableBytes() const { return _buffer.size() - _writePos; }

size_t Buffer::readableBytes() const { return _writePos-_readPos; }

size_t Buffer::prependableBytes() const { return _readPos; }

char* Buffer::_beginPtr() { return &(*_buffer.begin()); }
const char* Buffer::_beginPtr() const { return &(*_buffer.begin()); }

const char* Buffer::beginWriteConst() const { return _beginPtr() + _writePos; }
char* Buffer::beginWrite() { return _beginPtr() + _writePos; }

void Buffer::_makeSpace(size_t len) {
    if (writableBytes() + prependableBytes() < len) {
        _buffer.resize(_writePos+len+1);
    } else {
        size_t readable = readableBytes();
        std::copy(_beginPtr() + _readPos, _beginPtr()+_writePos, _beginPtr()); // i i o
        _readPos = 0;
        _writePos = _readPos + readable;
        NOT_ERR_EXIT(readable == readableBytes(), "readable == readableBytes()");
    }
}

const char* Buffer::peek() const { return _beginPtr() + _readPos; }

void Buffer::ensureWritable(size_t len) { 
    if (writableBytes() < len) 
        _makeSpace(len); 
    NOT_ERR_EXIT(writableBytes() >= len, "writableBytes() >= len"); 
}

void Buffer::hasWritten(size_t len) { _writePos += len; }

void Buffer::retrieve(size_t len) { 
    NOT_ERR_EXIT(len <= readableBytes(), "len <= readableBytes()");
    _readPos += len;
}

void Buffer::retrieveUntil(const char *end) {
    NOT_ERR_EXIT(peek() <= end, "peek() <= end");
    retrieve(end - peek());
}

void Buffer::retrieveAll() {
    memset(&_buffer, 0, _buffer.size());
    _readPos = 0;
    _writePos = 0;
}

std::string Buffer::retrieveAllToStr() {
    std::string str(peek(), readableBytes());
    retrieveAll();
    return str;
}

// origin
void Buffer::append(const char *str, size_t len) {
    NOT_ERR_EXIT(str, "append str");
    ensureWritable(len);
    std::copy(str, str+len, beginWrite()); // to beginWrite()
    hasWritten(len);
}

void Buffer::append(const std::string &str) {
    append(str.data(), str.length());
}

void Buffer::append(const void *data, size_t len) {
    NOT_ERR_EXIT(data, "append data");
    append(data, len);
}

void Buffer::append(const Buffer &buff) { // to buff
    append(buff.peek(), buff.readableBytes());
}

ssize_t Buffer::readFd(int fd, int *Errno) {
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = writableBytes();
    // 分散读
    iov[0].iov_base = _beginPtr() + _writePos;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *Errno = errno;
    } else if (len <= writable) {
        _writePos += len;
    } else {
        _writePos = _buffer.size();
        append(buff, len - writable);
    }
    return len;
}

ssize_t Buffer::writeFd(int fd, int *Errno) {
    size_t readSize = readableBytes();
    ssize_t len = write(fd, peek(), readSize);
    if (len < 0) {
        *Errno = errno;
        return len;
    }
    _readPos += len;
    return len;
}

};
