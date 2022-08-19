/**
 * @file blockqueue.h
 * @brief  一个队列
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __BLOCKQUEUE_H__
#define __BLOCKQUEUE_H__

#include <mutex>
#include <deque>
#include <condition_variable>

#include <sys/time.h>

namespace wsv
{

template<class T>
class BlockDeque
{
public:
    explicit BlockDeque(size_t maxCapacity = 1000);
    ~BlockDeque();

    void clear();
    void close();
    bool empty();
    bool full();
    size_t size();
    size_t capacity();
    T front();
    T back();
    void flush();

    void push_back(const T &item);
    void push_front(const T &item);
    bool pop(T &item);
    bool popWithTimeout(T &item, int timeout);

private:
    bool _isClosed;
    size_t _capacity;
    std::mutex _mtx;
    std::condition_variable _condConsumer;
    std::condition_variable _condProducer;
    std::deque<T> _deque;
};

}

#endif // __BLOCKQUEUE_H__
