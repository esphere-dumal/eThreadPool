## eThreadPool
### Overview

线程的创建销毁需要一定开销，在一些特定场景下，频繁创建、销毁线程会造成一定的性能影响。线程池通过对线程集中创建、销毁，可以在一定程度上消除这部分性能损失。
---
### Basic
最基础的线程池需要实现线程的集中创建、集中销毁、任务添加以及结果返回这些功能。

- 集中创建、销毁：参考 cpp RAII 的资源管理机制，在 ThreadPool 类的构造函数、析构函数里面就好了，
- 任务添加以及结果返回:
    - 是一个生产者消费模型
    - 实现相对复杂一点，需要用一些函数模板来对任务进行抽象
    - enqueue 以（函数和他的参数）作为参数，首先将这个函数与参数绑定后得到一个 `return_type f()` 形式的可调用对象，接着用 package_task 封装，以 future 期物的形式返回给用户。


```cpp
class ThreadPool {
public:
    explicit ThreadPool(size_t); // 构造的同时运行一系列线程，并且让线程都进入 sleep 状态避免占有 CPU 资源
    ~ThreadPool(); // RAII,等待线程运行结束并且销毁线程
    
    /*
     * 添加一个任务进入队列
     * 通过模板对函数进行抽象
     * 以期物的形式将结果返回
     * 写着一段的时候有一种在写 cpp 黑魔法的感觉
     */
    template<class F, class ... Args> 
    auto enqueue(F&& f, Args&& ... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

private:
    std::vector<std::thread>          _workers;             // 线程
    std::queue<std::function<void()>> _tasks;               // 任务队列
    std::mutex                        _queue_mutex;         // 队列锁
    std::condition_variable           _conditionVariable;   // cv，用于线程的休眠唤醒
    bool _is_stop;                                          // ThreadPool 状态
};
```
## Further More
线程池还可以根据任务多少自动进行扩容收缩，从而最大化利用资源。
