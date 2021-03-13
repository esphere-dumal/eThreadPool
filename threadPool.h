#ifndef THREADPOOL_THREADPOOL_H
#define THREADPOOL_THREADPOOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <future>
#include <functional>
#include <stdexcept>
class ThreadPool{
public:
    explicit ThreadPool(size_t);
    ~ThreadPool();
    //对外接口，一个函数和所有参数
    template<class F, class ... Args>
    auto enqueue(F&& f, Args&& ... args)
        -> std::future< typename std::result_of<F(Args...)>::type>;

private:
    //工作线程，任务队列
    std::vector< std::thread > workers;
    std::queue< std::function<void()> > tasks;
    //同步队列，任务队列的锁，线程同步条件变量
    std::mutex queue_mutex;
    std::condition_variable conditionVariable;
    //线程池是否已经被销毁
    bool is_stop;
};

inline ThreadPool::ThreadPool(size_t threads)
    : is_stop(false)
{
    for(size_t i=0;i<threads;i++)
        workers.emplace_back(
                [this]{
                    //轮询是否有任务
                    for(;;){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->queue_mutex);

                            this->conditionVariable.wait
                            (lock,
                             [this]{return this->is_stop||!this->tasks.empty();});
                            if(this->is_stop && this->tasks.empty())
                                return;

                            task = this->tasks.front();
                            this->tasks.pop();
                        }
                        task();
                    }

                }
                );
}

inline ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        is_stop = true;
    }

    conditionVariable.notify_all();
    for(std::thread &worker : workers)
        worker.join();
}

template<class F,class ... Args>
auto ThreadPool::enqueue(F &&f, Args && ... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared< std::packaged_task<return_type()> >(
            std::bind(std::forward<F>(f),std::forward<Args>(args)...)
            );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if(is_stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    conditionVariable.notify_one();
    return res;
}

#endif //THREADPOOL_THREADPOOL_H
