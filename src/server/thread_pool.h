#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <vector>
#include <functional>
#include <memory>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
    void EnqueueTask(std::function<void()> task);
private:
    boost::asio::io_service ioService;
    boost::asio::executor_work_guard<boost::asio::io_service::executor_type> workGuard;
    std::vector<boost::thread> threads;
    bool stop;
};