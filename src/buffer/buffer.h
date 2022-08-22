/**
 * @file buffer.h
 * @brief  buffer
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-18
 */
#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <iostream>
#include <vector>
#include <atomic>
#include <string>

#include <cstring>  // perror

#include <unistd.h> // write
#include <sys/uio.h>// readv

namespace wsv
{

class Buffer
{
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t writableBytes() const;
    size_t readableBytes() const;
    size_t prependableBytes() const;

    const char* peek() const;
    void ensureWritable(size_t len);
    void hasWritten(size_t len);

    void retrieve(size_t len);
    void retrieveUntil(const char *end);
    void retrieveAll();

    std::string retrieveAllToStr();
    const char* beginWriteConst() const;
    char* beginWrite();

    void append(const std::string &str);
    void append(const char *str, size_t len);
    void append(const void *data, size_t len);
    void append(const Buffer &buff);

    ssize_t readFd(int fd, int *Errno);
    ssize_t writeFd(int fd, int *Errno);

private:
    char* _beginPtr();
    const char* _beginPtr() const;
    void _makeSpace(size_t len);

private:
    std::vector<char> _buffer;
    std::atomic<std::size_t> _readPos;
    std::atomic<std::size_t> _writePos;
};

}

#endif // __BUFFER_H__
