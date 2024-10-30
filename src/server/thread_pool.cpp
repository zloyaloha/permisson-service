#include "thread_pool.h"

ThreadPool::ThreadPool(size_t numThreads)
    : workGuard(boost::asio::make_work_guard(ioService)), 
        stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back([this] {
            ioService.run();
        });
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    ioService.stop();
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join(); 
        }
    }
}

void ThreadPool::EnqueueTask(std::function<void()> task) {
    ioService.post(std::move(task));
}