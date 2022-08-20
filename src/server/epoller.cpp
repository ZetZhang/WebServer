/**
 * @file epoller.cpp
 * @brief  EPOLL
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#include "epoller.h"

namespace wsv
{

Epoller::Epoller(int maxEvent) : _epollFd(epoll_create1(0)), _events(maxEvent) {
    if (_epollFd < 0 || _events.size() <= 0) {
        LOG_ERROR("Epoller > Epoller: _epollFd < 0 or _events.size() <= 0");
        exit(EXIT_FAILURE);
    }
}
Epoller::~Epoller() { close(_epollFd); }

bool Epoller::addFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::modFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev;
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) return false;
    epoll_event ev;
    return 0 == epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int timeoutMs) {
    return epoll_wait(_epollFd, &_events[0], static_cast<int>(_events.size()), timeoutMs);
}

int Epoller::getEventFd(size_t i) const {
    if (i >= _events.size()) {
        LOG_ERROR("Epoller getEventFd: i >= _events.size()");
        exit(EXIT_FAILURE);
    }
    return _events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const {
    if (i >= _events.size()) {
        LOG_ERROR("Epoller getEvents: i >= _events.size()");
        exit(EXIT_FAILURE);
    }
    return _events[i].events;
}

}
