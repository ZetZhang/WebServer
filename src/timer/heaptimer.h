/**
 * @file heaptimer.h
 * @brief  链式定时器
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __HEAPTIMER_H__
#define __HEAPTIMER_H__

#include <unordered_map>
#include <algorithm>
#include <functional>
#include <chrono>

#include <ctime>

#include <arpa/inet.h>

#include "../log/log.h"

namespace wsv
{

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds Millisecond;
typedef Clock::time_point TimeStamp;

struct TimerNode
{
    int id;
    TimeStamp expires;
    TimeoutCallBack callBack;

    bool operator<(const TimerNode &t) { return expires < t.expires; }
};

class HeapTimer
{
public:
    HeapTimer();
    ~HeapTimer();

    void adjust(int id, int newExpires);
    void add(int id, int timeout, const TimeoutCallBack &cb);
    void pop();
    void clear();
    void doWork(int id);
    void tick();
    int getNextTick();

private:
    void _del(size_t i);
    void _siftup(size_t i);
    bool _siftdown(size_t index, size_t n);
    void _swapNode(size_t i, size_t j);

private:
    std::vector<TimerNode> _heap;
    std::unordered_map<int, size_t> _ref;
};

}

#endif // __HEAPTIMER_H__
