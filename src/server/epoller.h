/**
 * @file epoller.h
 * @brief  EPOLL
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <fcntl.h>      // fcntl()
#include <errno.h>

#include <sys/epoll.h>  // epoll_ctl()

#include "../log/log.h"

namespace wsv
{

class Epoller
{
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool DelFd(int fd);

    int wait(int timeoutMs = -1);
    int getEventFd(size_t i) const;
    uint32_t getEvents(size_t i) const;

private:
    int _epollFd;
    std::vector<struct epoll_event> _events;
};

}

#endif // __EPOLLER_H__
