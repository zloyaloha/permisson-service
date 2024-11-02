#include "thread_pool.h"

ThreadPool::ThreadPool(size_t numThreads, const std::string& url)
    : workGuard(boost::asio::make_work_guard(_ioService)), _stop(false), _connection(std::make_shared<pqxx::connection>(url))
{
    for (size_t i = 0; i < numThreads; ++i) {
        _threads.emplace_back([this] {
            _ioService.run();
        });
    }
}

ThreadPool::~ThreadPool() {
    _stop = true;
    _ioService.stop();
    for (auto& thread : _threads) {
        if (thread.joinable()) {
            thread.join(); 
        }
    }
    _connection->disconnect();
}

void ThreadPool::EnqueueTask(std::function<void()> task) {
    _ioService.post(std::move(task));
}