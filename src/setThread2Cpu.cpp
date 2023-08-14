#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>

#include <condition_variable>
#include <dirent.h>
#include <errno.h>
#include <map>
#include <mutex>
#include <pthread.h>
#include <queue>
#include <thread>
#include <unistd.h>

int main(int argc, const char **argv) {

  constexpr unsigned num_threads = 4;

  // A mutex ensures orderly access to std::cout from multiplethreads.

  std::mutex iomutex;

  std::vector<std::thread> threads(num_threads);

  for (unsigned i = 0; i < num_threads; ++i) {
    threads[i] = std::thread([&iomutex, i] {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      while (1) {
        {

          // Use a lexical scope and lock_guard to safely lock the mutexonly

          // for the duration of std::cout usage.

          std::lock_guard<std::mutex> iolock(iomutex);

          std::cout << "Thread #" << i << ":on CPU " << sched_getcpu() << "\n";
        }
        // Simulate important work done by the tread by sleeping for abit...

        std::this_thread::sleep_for(std::chrono::milliseconds(900));
      }
    });

    // Create a cpu_set_t object representing a set of CPUs. Clear itand mark
    // only CPU i as set.

    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);

    CPU_SET((i + 2) % num_threads, &cpuset);

    int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                    sizeof(cpu_set_t), &cpuset);

    if (rc != 0) {

      std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    }
  }

  for (auto &t : threads) {

    t.join();
  }

  return 0;
}
