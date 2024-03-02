cmake_minimum_required(VERSION 3.10)
project(sled VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

option(SLED_BUILD_BENCHMARK "Build benchmark" OFF)
option(SLED_BUILD_TESTS "Build tests" OFF)

set(BUILD_STATIC ON)
set(BUILD_RTTR_DYNAMIC OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_EXAMPLES OFF)
add_library(sled STATIC "")

add_subdirectory(3party/rpc_core EXCLUDE_FROM_ALL)
add_subdirectory(3party/fmt EXCLUDE_FROM_ALL)
# add_subdirectory(3party/eigen EXCLUDE_FROM_ALL)
target_include_directories(sled PUBLIC 3party/eigen)
target_sources(
  sled
  PRIVATE src/log/log.cc
          src/network/async_resolver.cc
          src/network/ip_address.cc
          src/network/null_socket_server.cc
          src/network/physical_socket_server.cc
          src/network/socket_address.cc
          src/network/socket_server.cc
          src/strings/base64.cc
          src/strings/utils.cc
          src/synchronization/event.cc
          src/synchronization/mutex.cc
          src/synchronization/sequence_checker_internal.cc
          src/synchronization/thread_local.cc
          src/system/location.cc
          src/system/thread.cc
          src/task_queue/pending_task_safety_flag.cc
          src/task_queue/task_queue_base.cc
          src/units/time_delta.cc
          src/units/timestamp.cc
          src/operations_chain.cc
          src/random.cc
          src/sigslot.cc
          src/status.cc
          src/system_time.cc
          src/time_utils.cc)
# set(BUILD_RTTR_DYNAMIC OFF) set(BUILD_UNIT_TESTS OFF)
# set(BUILD_WITH_STATIC_RUNTIME_LIBS ON) set(BUILD_WITH_DOCUMENTATION OFF)
# add_subdirectory(3party/rttr EXCLUDE_FROM_ALL)

target_include_directories(
  sled
  PUBLIC include
  PRIVATE src)

target_link_libraries(sled PUBLIC rpc_core fmt)

if(SLED_BUILD_BENCHMARK)
    find_package(benchmark REQUIRED)

    add_executable(sled_benchmark benchmark/strings/base64_benchmark.cc)
    target_link_libraries(sled_benchmark PRIVATE sled benchmark::benchmark
                                               benchmark::benchmark_main)
endif(SLED_BUILD_BENCHMARK)

if(SLED_BUILD_TESTS)
    find_package(gtest REQUIRED)
    add_executable(sled_tests "")
endif(SLED_BUILD_TESTS)
