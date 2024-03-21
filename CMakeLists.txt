cmake_minimum_required(VERSION 3.10)
project(sled VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)

option(SLED_BUILD_BENCHMARK "Build benchmark" OFF)
option(SLED_BUILD_TESTS "Build tests" OFF)
option(SLED_LOCATION_PATH "" "sled/src/system/location.cc")

set(BUILD_STATIC ON)
set(BUILD_RTTR_DYNAMIC OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_EXAMPLES OFF)
add_library(sled STATIC "")

add_subdirectory(3party/gperftools EXCLUDE_FROM_ALL)
add_subdirectory(3party/cppuprofile EXCLUDE_FROM_ALL)
add_subdirectory(3party/protobuf-3.21.12 EXCLUDE_FROM_ALL)
if (NOT TARGET marl)
    add_subdirectory(3party/marl EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET rpc_core)
    add_subdirectory(3party/rpc_core EXCLUDE_FROM_ALL)
endif()

if (NOT TARGET fmt)
    add_subdirectory(3party/fmt EXCLUDE_FROM_ALL)
endif()

if (SLED_LOCATION_PATH)
    target_compile_definitions(sled PRIVATE __SLED_LOCATION_PATH="${SLED_LOCATION_PATH}")
endif()
# add_subdirectory(3party/eigen EXCLUDE_FROM_ALL)
target_include_directories(sled PUBLIC include 3party/eigen 3party/inja 3party/rxcpp)
target_sources(
  sled
  PRIVATE 
          src/filesystem/path.cc
          src/log/log.cc
          src/network/async_resolver.cc
          src/network/ip_address.cc
          src/network/null_socket_server.cc
          src/network/physical_socket_server.cc
          src/network/socket_address.cc
          src/network/socket_server.cc
		  src/profiling/profiling.cc
          src/strings/base64.cc
          src/strings/utils.cc
          src/synchronization/event.cc
          src/synchronization/mutex.cc
          src/synchronization/sequence_checker_internal.cc
          src/synchronization/thread_local.cc
          src/system/location.cc
		  src/system/pid.cc
          src/system/thread.cc
          src/system/thread_pool.cc
          src/task_queue/pending_task_safety_flag.cc
          src/task_queue/task_queue_base.cc
          src/timer/task_queue_timeout.cc
          src/timer/timer.cc
          src/units/time_delta.cc
          src/units/timestamp.cc
          src/operations_chain.cc
          src/random.cc
          src/sigslot.cc
          src/status.cc
          src/system_time.cc
          src/time_utils.cc
		  src/uri.cc
)
# set(BUILD_RTTR_DYNAMIC OFF) set(BUILD_UNIT_TESTS OFF)
# set(BUILD_WITH_STATIC_RUNTIME_LIBS ON) set(BUILD_WITH_DOCUMENTATION OFF)
# add_subdirectory(3party/rttr EXCLUDE_FROM_ALL)

target_link_libraries(sled 
    PUBLIC rpc_core fmt marl protobuf::libprotobuf cppuprofile tcmalloc_and_profiler_static
)

if(SLED_BUILD_BENCHMARK)
	if (NOT TARGET benchmark) 
    	find_package(benchmark REQUIRED)
	endif()

    add_executable(sled_benchmark 
        src/random_bench.cc
        src/strings/base64_bench.cc
        src/system/fiber/fiber_bench.cc
        src/system/thread_pool_bench.cc
        src/system_time_bench.cc)
    target_link_libraries(sled_benchmark PRIVATE sled benchmark::benchmark
                                               benchmark::benchmark_main)
endif(SLED_BUILD_BENCHMARK)

if(SLED_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    FetchContent_MakeAvailable(googletest)
    add_executable(sled_tests 
		src/any_test.cc
        src/filesystem/path_test.cc
		src/profiling/profiling_test.cc
        src/strings/base64_test.cc
        src/cleanup_test.cc
        src/status_or_test.cc
        src/system/fiber/fiber_test.cc
        src/system/thread_pool_test.cc
		src/rx_test.cc
		src/uri_test.cc
    )
    target_link_libraries(sled_tests PRIVATE sled GTest::gtest GTest::gtest_main)
    add_test(NAME sled_tests COMMAND sled_tests)
endif(SLED_BUILD_TESTS)
