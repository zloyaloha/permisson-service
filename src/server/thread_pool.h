#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <pqxx/pqxx>
#include <condition_variable>
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
    boost::asio::io_context _ioContext;
    boost::asio::executor_work_guard<boost::asio::io_service::executor_type> _workGuard;
    std::vector<boost::thread> _threads;
    bool _stop;
};