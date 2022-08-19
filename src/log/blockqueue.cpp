/**
 * @file blockqueue.cpp
 * @brief  双端阻塞队列
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#include "blockqueue.h"

namespace wsv
{

template<class T>
BlockDeque<T>::BlockDeque(size_t maxCapacity) : _isClosed(false), _capacity(maxCapacity) { }

template<class T>
void BlockDeque<T>::clear() {
    std::lock_guard<std::mutex> locker(_mtx);
    _deque.clear();
}
template<class T>
void BlockDeque<T>::flush() {
    _condConsumer.notify_one();
}

template<class T>
void BlockDeque<T>::close() {
    {
        std::lock_guard<std::mutex> locker(_mtx);
        _deque.clear();
        _isClosed = true;
    }
    _condConsumer.notify_all();
    _condProducer.notify_all();
}

template<class T>
BlockDeque<T>::~BlockDeque() { close(); }

template<class T>
bool BlockDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _deque.empty();
}

template<class T>
bool BlockDeque<T>::full() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _deque.size() >= _capacity;
}

template<class T>
size_t BlockDeque<T>::size() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _deque.size();
}

template<class T>
size_t BlockDeque<T>::capacity() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _capacity;
}

template<class T>
T BlockDeque<T>::front() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _deque.front();
}

template<class T>
T BlockDeque<T>::back() {
    std::lock_guard<std::mutex> locker(_mtx);
    return _deque.back();
}

template<class T>
void BlockDeque<T>::push_back(const T &item) {
    std::unique_lock<std::mutex> locker(_mtx);
    while (_deque.size() >= _capacity)
        _condProducer.wait(locker);
    _deque.push_back(item);
    _condConsumer.notify_one();
}

template<class T>
void BlockDeque<T>::push_front(const T &item) {
    std::unique_lock<std::mutex> locker(_mtx);
    while (_deque.size() >= _capacity)
        _condProducer.wait(locker);
    _deque.push_front(item);
    _condConsumer.notify_one();
}

template<class T>
bool BlockDeque<T>::pop(T &item) {
    std::unique_lock<std::mutex> locker(_mtx);
    while (_deque.empty()) {
        _condConsumer.wait(locker);
        if (_isClosed)
            return false;
    }
    item = _deque.front();
    _deque.pop_front();
    _condProducer.notify_one();
    return true;
}

template<class T>
bool BlockDeque<T>::popWithTimeout(T &item, int timeout) {
    std::unique_lock<std::mutex> locker(_mtx);
    while (_deque.empty())
        if (_isClosed || _condConsumer.wait_for(locker, std::chrono::seconds(timeout)) == std::cv_status::timeout)
            return false;
    item = _deque.front();
    _deque.pop_front();
    _condProducer.notify_one();
    return true;
}

}
