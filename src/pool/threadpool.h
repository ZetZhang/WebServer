/**
 * @file threadpool.h
 * @brief  线程池
 * @author Ichheit, <ichheit@outlook.com>
 * @date 2022-08-19
 */
#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

#include <cassert>

namespace wsv
{

class ThreadPool
{
public:
    explicit ThreadPool(size_t threadCount = 8) : _poolImpl(std::make_shared<Pool>()) {
        assert(threadCount > 0);
        for (size_t i = 0; i < threadCount; i++) {
            std::thread([=] {
                std::unique_lock<std::mutex> locker(_poolImpl->mtx);
                for (;;) {
                    if (_poolImpl->tasks.empty()) {
                        auto task = std::move(_poolImpl->tasks.front());
                        _poolImpl->tasks.pop();
                        locker.unlock();
                        task();
                        locker.lock();
                    } else if (_poolImpl->isClosed) {
                        break;
                    } else {
                        _poolImpl->cond.wait(locker);
                    }
                 }
            }).detach();
        }
    }

    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;

    ~ThreadPool() {
        if (_poolImpl) {
            {
                std::lock_guard<std::mutex> locker(_poolImpl->mtx);
                _poolImpl->isClosed = true;
            }
            _poolImpl->cond.notify_all();
        }
    }

    template<class F>
    void AddTask(F &&task) {
        if (_poolImpl) {
            {
                std::lock_guard<std::mutex> locker(_poolImpl->mtx);
                _poolImpl->tasks.emplace(std::forward<F>(task));
            }
            _poolImpl->cond.notify_one();
        }
    }

private:
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<Pool> _poolImpl;
};

}

#endif // __THREADPOOL_H__
