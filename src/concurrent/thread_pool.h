#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <functional>
#include <thread>
#include <vector>

class ThreadPool {
public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();

    void enqueue(const std::function<void()> task);
private:
    void worker();
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

inline ThreadPool::ThreadPool(size_t num_threads) {
    for (size_t i = 0; i < num_threads; i++) {
        workers.emplace_back([this] { worker(); });
    }
}

inline ThreadPool::~ThreadPool() {
    stop = true;
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

inline void ThreadPool::enqueue(const std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.emplace(task);
    }
    condition.notify_one();
}

inline void ThreadPool::worker() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty()) return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}


#endif //THREAD_POOL_H
