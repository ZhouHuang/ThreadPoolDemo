#include "WorkerPool.hh"


namespace mt {
    using std::cerr;
    // class WorkerThread
    // public

    WorkerThread::WorkerThread(int cpubind) {
        printf("worker constructed...\n");
        m_job = nullptr;
        if (cpubind >=0) {
            set_thread_affinity(m_thread, cpubind);
        }
    }
    void WorkerThread::join() {
        printf("in join...\n");
        while (m_job) {
            ;
        }
        printf("before stop...\n");
        m_stop = true;
        if (m_thread.joinable()) {
            m_thread.join();
        }
    }
    bool WorkerThread::set_job(const std::function<void()>& f, bool bFork) {
        // TODO: bFork
        if (!m_lck.load()) {
            printf("set job [success]...\n");
            m_job = f;
            m_lck.store(true);
            return true;
        }
        printf("set job [fail]...\rset job [fail]...");
        return false;
    }
    
    // private
    void WorkerThread::loop() {
        while(!m_stop) {
            m_lck.store(true); // m_lck looks like m_running
            m_running = true;
            if (m_job) {
                try {
                    m_job();
                } catch (const std::exception& ex) {
                    std::cout << ex.what() << '\n';
                }
                m_job = nullptr;
            }
            m_running = false;
            m_lck.store(false);
            std::this_thread::yield();
            // std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        }
    }
    

    // class WorkerPool
    // public:

    void WorkerPool::join() {
        while(!m_jobs.empty()){
            ;
        }
        m_stop = true;
        if (m_controller_thread.joinable())
            m_controller_thread.join();
        for(auto& thr : m_threads) {
            thr.join();
        }


    }
    void WorkerPool::add_job(const std::function<void()>& f, const std::string& info) {
        if (m_stop)
            throw std::runtime_error("WokerPool already stopped...");
        m_lck.store(true);
        printf("add job %s...\n", info.c_str());
        m_jobs.push_back({f, info});
        m_lck.store(false);
    }
    //private:
    void WorkerPool::schedule_loop(){
        while(!m_stop) {
            if (!m_lck.load()){
                int i = 0;
                for(auto& worker : m_threads) {
                    i++;
                    if (m_jobs.empty()){
                        break;
                    }
                    auto& job = m_jobs.front();
                    // printf("job %s on front...\n", job.second.c_str());
                    if (worker.set_job(job.first)) {
                        printf("worker %d handles job %s...\n", i, job.second.c_str());
                        m_jobs.pop_front();
                        continue;
                    } else{
                        // printf("worker %d busy...\n", i);
                    }
                }
                if (!m_jobs.empty()) {
                    printf("waiting workers...\rwaiting workers...");
                    fflush(stdout);
                }
            }
            std::this_thread::yield();
        }
    }
    /*
    void WorkerPool::schedule_loop() {
        printf("in schedule loop...\n");
        while(true) {
            for(auto& worker : m_threads) {
                while(!m_jobs.empty()) {
                    auto f = m_jobs.front();
                    if (worker.set_job(f)) {
                        m_jobs.pop_front();
                    }
                    break;
                }
            }
        };
    }
    */

    /*
    void WorkerPool::schedule_loop() {
        for(int i = 0; i<m_threads.size(); ++i) {
            m_threads.at(i).set_job(
                [this]
                {
                    for(;;)
                    {
                        std::function<void()> job;

                        {
                            while (this->m_lck)  // 自旋锁
                                ;

                            if (this->m_stop && this->m_jobs.empty())
                                return;
                            
                            job = std::move(this->m_jobs.front());
                            this->m_jobs.pop_front();

                        }

                        job();
                    }
                }
            );
        }
    }
    */

    // other function
    void set_thread_affinity(std::thread& thr, int cpu_id) {

        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_id, &cpuset);

        int rc = pthread_setaffinity_np(thr.native_handle(), sizeof(cpu_set_t), &cpuset);
        if (rc != 0) {
            std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
        }
    }

}