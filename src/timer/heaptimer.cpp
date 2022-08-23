/**
 * @file heaptimer.cpp
 * @brief  定时器
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#include "heaptimer.h"

namespace wsv
{

HeapTimer::HeapTimer() { _heap.reserve(64); }
HeapTimer::~HeapTimer() { clear(); }

void HeapTimer::_swapNode(size_t i, size_t j) {
    if (i >= _heap.size()) {
        LOG_ERROR("HeapTimer > _swapNode: i >= _heap.size()");
        exit(EXIT_FAILURE);
    }
    if (j >= _heap.size()) {
        LOG_ERROR("HeapTimer > _swapNode: j >= _heap.size()");
        exit(EXIT_FAILURE);
    }
    std::swap(_heap[i], _heap[j]);
    _ref[_heap[i].id] = i;
    _ref[_heap[j].id] = j;
}

void HeapTimer::_siftup(size_t i) {
    if (i >= _heap.size()) {
        LOG_ERROR("HeapTimer > _sfitup: i >= _heap.size()");
        exit(EXIT_FAILURE);
    }
    size_t j = (i-1)/2;
    while (j >= 0) { // FIXME: always true
        if (_heap[j] < _heap[i])
            break;
        _swapNode(i, j);
        i = j;
        j = (i-1)/2;
    }
}

bool HeapTimer::_siftdown(size_t idx, size_t n) {
    if (idx >= _heap.size()) {
        LOG_ERROR("HeapTimer > _sfitdown: idx >= _heap.size()");
        exit(EXIT_FAILURE);
    }
    if (n > _heap.size()) {
        LOG_ERROR("HeapTimer > _sfitdown: n >= _heap.size()");
        exit(EXIT_FAILURE);
    }
    size_t i = idx, j = i*2+1;
    while (j < n) {
        if (j+1 < n && _heap[j+1] < _heap[j])
            j++;
        if (_heap[i] < _heap[j])
            break;
        _swapNode(i, j);
        i = j;
        j = i*2+1;
    }
    return i > idx;
}

void HeapTimer::adjust(int id, int newExpires) {
    if (_heap.empty() || _ref.count(id) <= 0) {
        LOG_ERROR("HeapTimer > adjust: heap is empty or key 's value not found!'");
        exit(EXIT_FAILURE);
    }
    _heap[_ref[id]].expires = Clock::now() + Millisecond(newExpires);
    _siftdown(_ref[id], _heap.size());
}

void HeapTimer::add(int id, int timeout, const TimeoutCallBack &cb) {
    if (id < 0) {
        LOG_ERROR("HeapTimer add: id < 0");
        exit(EXIT_FAILURE);
    }
    size_t i;
    if (_ref.count(id) == 0) { // inesrt
        i = _heap.size();
        _ref[id] = i;
        _heap.push_back({id, Clock::now() + Millisecond(timeout), cb});
        _siftup(i);
        TimerNode node = _heap.front();
        node.callBack();
    } else { // change
        i = _ref[id];
        _heap[i].expires = Clock::now() + Millisecond(timeout);
        _heap[i].callBack = cb;
        if (!_siftdown(i, _heap.size()))
            _siftup(i);
    }
}

void HeapTimer::_del(size_t idx) {
    if (_heap.empty() || idx > _heap.size()-1) {
        LOG_ERROR("HeapTimer > _del: _heap.empty or idx > _heap.size()-1");
        exit(EXIT_FAILURE);
    }
    // 将要删除的结点换到队尾，然后调整堆
    size_t i = idx, n = _heap.size()-1;
    if (i < n) {
        _swapNode(i, n);
        if (!_siftdown(i, n))
            _siftup(i);
    }
    // 队尾元素删除
    _ref.erase(_heap.back().id);
    _heap.pop_back();
}

void HeapTimer::pop() {
    if (_heap.empty()) {
        LOG_ERROR("HeapTimer > pop: _heap is empty");
        exit(EXIT_FAILURE);
    }
    _del(0);
}

void HeapTimer::clear() {
    _ref.clear();
    _heap.clear();
}

void HeapTimer::doWork(int id) {
    if (_heap.empty() || _ref.count(id) == 0) return;
    size_t i = _ref[id];
    TimerNode node = _heap[i];
    node.callBack();
    _del(i);
}

void HeapTimer::tick() { // 清除超时结点
    if (_heap.empty()) return;
    while (!_heap.empty()) {
        TimerNode node = _heap.front();
        if (std::chrono::duration_cast<Millisecond>(node.expires - Clock::now()).count() > 0)
            break;
        node.callBack();
        pop();
    }
}

int HeapTimer::getNextTick() {
    tick();
    int res = -1;
    if (!_heap.empty()) {
        res = std::chrono::duration_cast<Millisecond>(_heap.front().expires - Clock::now()).count();
        if (res < 0)
            res = 0;
    }
    return res;
}

}
