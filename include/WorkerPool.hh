#include <thread>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <vector>

namespace mt {

class WorkerThread {
 public:
    explicit WorkerThread(int cpubind = -1) { m_thread = std::thread(&WorkerThread::loop, this); }

    WorkerThread(WorkerThread &&) = delete;
    WorkerThread &operator=(WorkerThread &&) = delete;
    virtual ~WorkerThread() { join(); }
    bool run_done() const;
    void join();
    bool set_job(const std::function<void()> &f, bool bFork = false);

 private:
    void loop();
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::atomic<bool> m_stop{false};
    std::atomic<bool> m_jobRunning{false}; // 标记任务是否正在运行
    std::mutex m_jobMutex;                 // 用于保护m_jobRunning和通知join的互斥锁
    std::condition_variable m_jobCv;       // 用于通知join任务已完成的条件变量
    bool m_bFork{false};
    std::function<void()> m_job{};
};

class WorkerPool {
 public:
    explicit WorkerPool(int poolsize = std::thread::hardware_concurrency(), bool bFork = false);
    WorkerPool(WorkerPool &&) = delete;
    WorkerPool &operator=(WorkerPool &&) = delete;
    ~WorkerPool() { join(); }
    void join();
    void run();
    void add_job(const std::function<void()> &f, const std::string & /*name*/);
    int get_pool_sz() { return m_threads.size(); }

 private:
    void schedule_loop();
    int m_poolsize;
    bool m_fork{false};
    bool m_stop{false};
    std::vector<WorkerThread> m_threads;
    std::deque<std::pair<std::function<void()>, std::string>> m_jobs;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::thread m_controller_thread;
    std::atomic<int> m_remaining_jobs{0};
    std::condition_variable m_jobCv;
    std::condition_variable m_workerCv;
};

} // namespace mt
