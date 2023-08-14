cmake_minimum_required(VERSION 3.0.0)

set(version_ 0.4.8)
project(test_demo VERSION ${version_})

# set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-std=c++17 -pthread")
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

include_directories(
  ./
  include
)

add_executable(test_demo src/setThread2Cpu.cpp )
target_link_libraries(test_demo -lpthread)

add_library(work_pool src/WorkerPool.cc)
target_link_libraries(work_pool pthread)

add_executable(test_pool src/test_pool.cc)
target_include_directories(test_pool PUBLIC work_pool)
target_link_libraries(test_pool work_pool)
