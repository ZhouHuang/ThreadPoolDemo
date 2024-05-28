#include "MultiThread.h"

namespace mt {

void WorkerThread::join() {
    if (m_thread.joinable()) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_stop = true;
            m_cv.notify_all(); // 通知loop函数停止
        }

        // 等待任务执行完毕或线程结束
        if (m_jobRunning.load()) {
            std::unique_lock<std::mutex> lock(m_jobMutex);
            m_jobCv.wait(lock, [this] { return !m_jobRunning.load(); });
        }

        m_thread.join();
    }
}

bool WorkerThread::run_done() const {
    return !m_jobRunning.load();
}

bool WorkerThread::set_job(const std::function<void()> &f, bool bFork) {
    std::unique_lock<std::mutex> lock(m_mutex);
    if (!m_stop && !m_jobRunning.load()) { // 只有在没有停止并且没有任务正在运行时才设置新任务
        m_job = f;
        m_bFork = bFork;
        m_jobRunning.store(true); // 标记任务正在运行
        m_cv.notify_one();        // 通知loop函数有新任务
        return true;
    }
    return false;
}

void WorkerThread::loop() {
    while (true) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return m_stop || m_job; });

        if (m_stop && !m_job) {
            break;
        }

        lock.unlock();

        // 执行任务
        if (m_job) {
            {
                std::unique_lock<std::mutex> jobLock(m_jobMutex);
                m_job(); // 执行任务
            }

            // 标记任务执行完毕
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                m_job = nullptr;
                m_jobRunning.store(false);
            }
            m_jobCv.notify_all(); // 通知可能在join中等待的线程

            if (m_bFork) {
                // 如果需要fork（在这个例子中我们不实际fork，只是演示）
                // ... 执行fork相关的逻辑
                m_bFork = false; // 重置fork标志
            }
        }
    }
}

WorkerPool::WorkerPool(int poolsize, bool bFork)
    : m_poolsize{poolsize}, m_fork{bFork}, m_threads{static_cast<size_t>(poolsize)} {
    m_controller_thread = std::thread(&WorkerPool::schedule_loop, this);
}

void WorkerPool::join() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_stop = true;
        m_cv.notify_all(); // 通知所有等待的线程
    }

    if (m_controller_thread.joinable()) {
        m_controller_thread.join();
    }

    for (auto &thr : m_threads) {
        thr.join();
    }
}

void WorkerPool::add_job(const std::function<void()> &f, const std::string &info) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (!m_stop) {
            m_jobs.emplace_back(f, info); // 假设name不使用，仅放空字符串
            ++m_remaining_jobs;
        }
        m_cv.notify_one(); // 通知调度线程可能有新任务
    }
}

void WorkerPool::schedule_loop() {
    while (true) {
        std::function<void()> job;
        std::string info;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] { return m_stop || !m_jobs.empty(); });

            if (m_stop && m_jobs.empty()) {
                break;
            }

            if (!m_jobs.empty()) {
                job = std::move(m_jobs.front().first);
                info = std::move(m_jobs.front().second);
                m_jobs.pop_front();
                --m_remaining_jobs;
            }
            if (job) {
                bool job_dispatched = false;
                for (auto &worker : m_threads) {
                    if (worker.set_job(job, false)) {
                        job_dispatched = true;
                        break;
                    }
                }

                if (!job_dispatched) {
                    // 没有找到空闲的线程，将任务重新放回队列
                    m_jobs.emplace_front(job, info);
                    ++m_remaining_jobs;
                }
            }
            m_jobCv.notify_one();
        }
    }
}

void WorkerPool::run() {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // 等待任务分配完毕
        if (m_remaining_jobs > 0) {
            m_jobCv.wait(lock, [this] { return 0 == m_remaining_jobs; });
        }
        printf("jobs are dispatched done\n");
    }
    auto all_done = [this] () {
            for (const auto &worker : m_threads) {
                if (!worker.run_done()) {
                    return false;
                }
            }
            return true;
        };
    // 等待所有线程执行结束. 此处的while循环可被更高效的cv替代?
    while(1) {
        if (all_done()) {
            break;
        } else {
            std::this_thread::yield();
        }
    }
}
} // namespace mt
