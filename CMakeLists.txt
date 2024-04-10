cmake_minimum_required(VERSION 3.10)
project(
  sled
  VERSION 0.1.1
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSIONS OFF)
# set(CMAKE_C_FLAGS "-gdwarf-3 -gstrict-dwarf") set(CMAKE_CXX_FLAGS "-gdwarf-3
# -gstrict-dwarf")

option(SLED_BUILD_BENCHMARK "Build benchmark" OFF)
option(SLED_BUILD_TESTS "Build tests" OFF)
option(SLED_BUILD_FUZZ "Build fuzzer test" OFF)
option(SLED_LOCATION_PATH "" "sled/src/sled/system/location.cc")

set(BUILD_STATIC ON)
set(BUILD_RTTR_DYNAMIC OFF)
set(BUILD_UNIT_TESTS OFF)
set(BUILD_EXAMPLES OFF)
add_library(test_main STATIC src/sled/testing/test_main.cc)
add_library(benchmark_main STATIC src/sled/testing/benchmark_main.cc)
target_include_directories(test_main PUBLIC src/)
target_include_directories(benchmark_main PUBLIC src/)

add_library(sled STATIC "")

add_subdirectory(3party/minilua EXCLUDE_FROM_ALL)
# add_subdirectory(3party/gperftools EXCLUDE_FROM_ALL)
add_subdirectory(3party/asyncplusplus EXCLUDE_FROM_ALL)
# add_subdirectory(3party/cppuprofile EXCLUDE_FROM_ALL)
# add_subdirectory(3party/protobuf-3.21.12 EXCLUDE_FROM_ALL)
if(NOT TARGET marl)
  add_subdirectory(3party/marl EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET rpc_core)
  add_subdirectory(3party/rpc_core EXCLUDE_FROM_ALL)
endif()

if(NOT TARGET fmt)
  add_subdirectory(3party/fmt EXCLUDE_FROM_ALL)
endif()

if(SLED_LOCATION_PATH)
  target_compile_definitions(
    sled PRIVATE __SLED_LOCATION_PATH="${SLED_LOCATION_PATH}")
endif()
# add_subdirectory(3party/eigen EXCLUDE_FROM_ALL)
target_include_directories(sled PUBLIC src/ 3party/eigen 3party/inja
                                       3party/rxcpp 3party/toml11)
target_sources(
  sled
  PRIVATE src/sled/async/async.cc
          src/sled/config.cc
          src/sled/debugging/demangle.cc
          src/sled/debugging/symbolize.cc
          src/sled/event_bus/event_bus.cc
          src/sled/filesystem/path.cc
          src/sled/log/log.cc
          src/sled/network/async_resolver.cc
          src/sled/network/ip_address.cc
          src/sled/network/null_socket_server.cc
          src/sled/network/physical_socket_server.cc
          src/sled/network/socket_address.cc
          src/sled/network/socket_server.cc
          src/sled/operations_chain.cc
          src/sled/profiling/profiling.cc
          src/sled/random.cc
          src/sled/sigslot.cc
          src/sled/status.cc
          src/sled/strings/base64.cc
          src/sled/strings/utils.cc
          src/sled/synchronization/event.cc
          src/sled/synchronization/mutex.cc
          src/sled/synchronization/sequence_checker.cc
          src/sled/synchronization/thread_local.cc
          src/sled/system/location.cc
          src/sled/system/pid.cc
          src/sled/system/thread.cc
          src/sled/system/thread_pool.cc
          src/sled/system_time.cc
          src/sled/task_queue/pending_task_safety_flag.cc
          src/sled/task_queue/task_queue_base.cc
          src/sled/testing/benchmark.cc
          src/sled/testing/test.cc
          src/sled/timer/task_queue_timeout.cc
          src/sled/timer/timer.cc
          src/sled/time_utils.cc
          src/sled/units/time_delta.cc
          src/sled/units/timestamp.cc
          src/sled/uri.cc)

target_link_libraries(
  sled
  PUBLIC rpc_core fmt marl Async++ minilua
  PRIVATE dl
  # protobuf::libprotobuf ${WHOLE_ARCHIVE_WRAPPER_START}
  # tcmalloc_and_profiler_static ${WHOLE_ARCHIVE_WRAPPER_END}
)

# set fPIC
set_target_properties(sled PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(SLED_BUILD_BENCHMARK)
  # if(NOT TARGET benchmark) find_package(mbenchmark REQUIRED) endif()

  add_executable(
    sled_benchmark
    src/sled/event_bus/event_bus_bench.cc
    src/sled/random_bench.cc
    src/sled/strings/base64_bench.cc
    # src/sled/system/fiber/fiber_bench.cc
    src/sled/system/thread_bench.cc
    src/sled/system/thread_pool_bench.cc
    src/sled/system_time_bench.cc
    src/sled/uri_bench.cc)
  target_link_libraries(sled_benchmark PRIVATE sled benchmark_main)
  target_compile_options(sled_benchmark PRIVATE -include
                                                sled/testing/benchmark.h)
endif(SLED_BUILD_BENCHMARK)

function(sled_add_test)
  set(prefix SLED_TEST)
  set(options NO_MAIN)
  set(one_value_keywords NAME)
  set(multi_value_keywords SRCS INC_DIRS LIBS)
  cmake_parse_arguments("${prefix}" "${options}" "${one_value_keywords}"
                        "${multi_value_keywords}" ${ARGN})

  add_executable(${SLED_TEST_NAME} ${SLED_TEST_SRCS})

  if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(EXTRA_FLAGS # -Wthread-safety
        -g -fsanitize=address -fno-omit-frame-pointer
        -fno-optimize-sibling-calls)
    target_compile_options(${SLED_TEST_NAME} PRIVATE ${EXTRA_FLAGS})

    target_link_options(${SLED_TEST_NAME} PRIVATE ${EXTRA_FLAGS})
  endif()
  if(MSVC)
    target_compile_options(${SLED_TEST_NAME} PRIVATE /FI"sled/testing/test.h")
  else()
    target_compile_options(${SLED_TEST_NAME} PRIVATE -include
                                                     sled/testing/test.h)
  endif()
  target_include_directories(${SLED_TEST_NAME} PRIVATE ${SLED_TEST_INC_DIRS})
  target_link_libraries(
    ${SLED_TEST_NAME} PRIVATE ${SLED_TEST_LIBS} sled
                              $<IF:$<BOOL:${SLED_NO_MAIN}>,,test_main>)
  add_test(NAME ${SLED_TEST_NAME} COMMAND ${SLED_TEST_NAME})
endfunction()

if(SLED_BUILD_TESTS)
  enable_testing()
  # include(FetchContent) FetchContent_Declare( googletest URL
  # https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  # ) FetchContent_MakeAvailable(googletest)

  sled_add_test(
    NAME
    sled_all_tests
    SRCS
    src/sled/debugging/demangle_test.cc
    src/sled/async/async_test.cc
    src/sled/filesystem/path_test.cc
    src/sled/log/fmt_test.cc
    src/sled/synchronization/sequence_checker_test.cc
    src/sled/cleanup_test.cc
    src/sled/status_test.cc
    src/sled/status_or_test.cc
    src/sled/strings/utils_test.cc
    src/sled/strings/base64_test.cc
    src/sled/system/fiber/fiber_test.cc
    src/sled/system/thread_pool_test.cc
    src/sled/rx_test.cc
    src/sled/uri_test.cc)

  if(NOT ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
    sled_add_test(NAME sled_async_test SRCS src/sled/async/async_test.cc)
    sled_add_test(NAME sled_thread_pool_test SRCS
                  src/sled/system/thread_pool_test.cc)
  endif()

  sled_add_test(NAME sled_event_bus_test SRCS
                src/sled/event_bus/event_bus_test.cc)
  sled_add_test(NAME sled_lua_test SRCS tests/lua_test.cc)
  sled_add_test(NAME sled_move_on_copy_test SRCS
                src/sled/utility/move_on_copy_test.cc)
  sled_add_test(NAME sled_symbolize_test SRCS
                src/sled/debugging/symbolize_test.cc NO_MAIN)
  sled_add_test(NAME sled_sigslot_test SRCS src/sled/sigslot_test.cc)
  sled_add_test(NAME sled_rpc_test SRCS src/sled/network/rpc_test.cc)
  sled_add_test(NAME sled_string_view_test SRCS
                src/sled/nonstd/string_view_test.cc)
  sled_add_test(NAME sled_expected_test SRCS src/sled/nonstd/expected_test.cc)
  sled_add_test(NAME sled_config_test SRCS src/sled/config_test.cc)
  sled_add_test(NAME sled_ioc_test SRCS src/sled/ioc/ioc_test.cc)
endif(SLED_BUILD_TESTS)

if(SLED_BUILD_FUZZ)

  function(add_fuzz_test name sources)
    add_executable(${name} ${sources})
    target_link_libraries(${name} PRIVATE sled)
    target_compile_options(${name} PRIVATE -g -O1 -fsanitize=fuzzer,address
                                           -fsanitize-coverage=trace-cmp)
    target_link_options(${name} PRIVATE -fsanitize=fuzzer,address
                        -fsanitize-coverage=trace-cmp)
  endfunction()

  add_fuzz_test(base64_fuzz src/sled/strings/base64_fuzz.cc)
  add_fuzz_test(uri_fuzz src/sled/uri_fuzz.cc)
endif(SLED_BUILD_FUZZ)
