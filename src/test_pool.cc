#include <iostream>
#include <vector>
#include "WorkerPool.hh"
#include <random>
#include <map>

using std::cout;
using std::endl;
using std::vector;


struct DA_Param{
    int uid;
};

std::mt19937 rng(std::rand());
std::uniform_int_distribution<int> distribution(1, 1000);
std::map<int, int> timer{{0,1}, {1,2}, {2,3}, {3,4}, {4,8}, {5,7}, {6,6}, {7,5}, {8,10}, {9,9}};

void update(const DA_Param& param) {
    auto random_number = distribution(rng);
    // std::this_thread::sleep_for(std::chrono::milliseconds(random_number)); // 
    std::this_thread::sleep_for(std::chrono::seconds(timer[param.uid])); // 
    
    cout << "processing " << param.uid << " [takes " << timer[param.uid] << "s]" << '\n';
};

int test_single_worker() {
    DA_Param param{1};
    mt::WorkerThread worker;
    worker.set_job([param](){
        try{
            cout << "updating...\n";
            update(param);
            cout << "updated...\n";
        }catch(const std::exception& ex){
            cout << ex.what() << endl;;
        };
    }
    );

    printf("-------------------\n");

    DA_Param param2{2};
    mt::WorkerThread worker2;
    worker2.set_job([param2](){
        try{
            cout << "updating...\n";
            update(param2);
            cout << "updated...\n";
        }catch(const std::exception& ex){
            cout << ex.what() << endl;;
        };
    }
    );
    // std::this_thread::sleep_for(std::chrono::seconds(1));

    worker2.join();

    worker.join();

    printf("-------------------\n");

    return 0;
};


int test_worker_pool() {
    cout << "hello world." << endl;
    
    mt::WorkerPool pool;
    cout << " worker pool sz " << pool.get_pool_sz() << endl;

    std::vector<DA_Param> all_param{};
    all_param.reserve(100);
    for(int i = 0; i<10; ++i) {
        DA_Param param{};
        param.uid = i;
        all_param.push_back(param);
    }
    char buffer[12];
    for(const auto& param : all_param){
        sprintf(buffer, "%d", param.uid);
        pool.add_job(
            [param](){
                try{
                    update(param);
                }catch(const std::exception& ex){
                    cout << ex.what() << endl;;
                }
            },
            buffer
        );
    }
    pool.join();

    return 0;
};

int main(int argc, char* argv[]) {
    test_single_worker();
    test_worker_pool();
}