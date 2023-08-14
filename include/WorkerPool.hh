#pragma once

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>
#include <deque>
#include <mutex>

namespace mt {
    class WorkerThread
    {
    public:
        explicit WorkerThread(int cpubind=-1);
        WorkerThread(WorkerThread&&) = default;
        virtual ~WorkerThread(){printf("destructed...\n");join();}
        void join();
        bool set_job(const std::function<void()>& f, bool bFork=false);
    private:
        void loop();
        std::atomic<bool> m_lck{false};
        bool m_running{false};
        bool m_stop{false};
        bool m_bFork{false};
        std::function<void()> m_job{};
        std::thread m_thread{[this](){loop();}};
    };

    class WorkerPool
    {
    public:
        explicit WorkerPool(int poolsize = std::thread::hardware_concurrency(), bool bFork=false):m_threads{static_cast<size_t>(poolsize)}, m_fork{bFork}{}
        WorkerPool(WorkerPool&&) = default;
        virtual ~WorkerPool(){join();}
        void join();
        void add_job(const std::function<void()>& f, const std::string&);
        int get_pool_sz() {return m_threads.size();}
    private:
        void schedule_loop();
        std::atomic<bool> m_lck{false};
        bool m_stop{false};
        bool m_fork{false};
        std::vector<WorkerThread> m_threads;
        std::deque<std::pair<std::function<void()>, std::string>> m_jobs{};
        std::thread m_controller_thread{[this](){schedule_loop();}};
    };

    void set_thread_affinity(std::thread& thr, int cpu_id);

}