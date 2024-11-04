#include "thread_pool.h"

ThreadPool::ThreadPool(size_t numThreads)
    : _workGuard(boost::asio::make_work_guard(_ioContext)), _stop(false) 
{
    for (size_t i = 0; i < numThreads; ++i) {
        _threads.emplace_back([this] {
            _ioContext.run();
        });
    }
}

ThreadPool::~ThreadPool() {
    if (!_stop) {
        _stop = true;
        _workGuard.reset();
        _ioContext.stop();

        for (auto& thread : _threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
}

void ThreadPool::EnqueueTask(std::function<void()> task) {
    boost::asio::post(_ioContext, std::move(task));  
}