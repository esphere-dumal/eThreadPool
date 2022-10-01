#ifndef THREADPOOL_THREADPOOL_HH
#define THREADPOOL_THREADPOOL_HH

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <stdexcept>

class ThreadPool {
public:
    explicit ThreadPool(size_t);
    ~ThreadPool();
    
    template<class F, class ... Args>
    auto enqueue(F&& f, Args&& ... args)
        -> std::future< typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread>          _workers;
    std::queue<std::function<void()>> _tasks;
    std::mutex                        _queue_mutex;
    std::condition_variable           _conditionVariable;
    bool _is_stop;
};

ThreadPool::ThreadPool(size_t threads): _is_stop(false) {
    for(size_t i=0;i<threads;i++)
        _workers.emplace_back(
                [this]{
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->_queue_mutex);

                            this->_conditionVariable.wait
                            (lock,
                             [this]{return this->_is_stop||!this->_tasks.empty();});
                            if(this->_is_stop && this->_tasks.empty())
                                return;

                            task = this->_tasks.front();
                            this->_tasks.pop();
                        }
                        task();
                    }

                }
                );
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        _is_stop = true;
    }

    _conditionVariable.notify_all();
    for (std::thread &worker : _workers) worker.join();
}

template<class F,class ... Args>
auto ThreadPool::enqueue(F &&f, Args && ... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
            );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(_queue_mutex);
        if (_is_stop) throw std::runtime_error("[eThreadPool]: threadpool stopped running but get a new job");
        _tasks.push([task](){ (*task)(); });
    }
    _conditionVariable.notify_one();
    return res;
}

#endif //THREADPOOL_THREADPOOL_HH
