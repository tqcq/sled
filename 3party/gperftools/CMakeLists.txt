cmake_minimum_required(VERSION 3.12)

# Please note that cmake support is very preliminary. Autotools-based build is
# the only fully supported build for now.

# Based on configure.ac

project(
  gperftools
  VERSION 2.15
  LANGUAGES C CXX
  DESCRIPTION "Performance tools for C++"
  HOMEPAGE_URL https://github.com/gperftools/gperftools)

# Update this value for every release!
set(TCMALLOC_SO_VERSION 9.16.5)
set(PROFILER_SO_VERSION 5.11.5)
set(TCMALLOC_AND_PROFILER_SO_VERSION 10.11.6)

# The user can choose not to compile in the heap-profiler, the heap-checker, or
# the cpu-profiler.  There's also the possibility for a 'fully minimal' compile,
# which leaves out the stacktrace code as well.  By default, we include all of
# these that the target system supports.
set(DEFAULT_BUILD_CPU_PROFILER ON)
set(DEFAULT_BUILD_HEAP_PROFILER ON)
set(DEFAULT_BUILD_HEAP_CHECKER OFF)
set(DEFAULT_BUILD_DEBUGALLOC ON)
set(DEFAULT_BUILD_MINIMAL OFF)

set(DEFAULT_TCMALLOC_ALIGNMENT 16)
set(NEED_NANOSLEEP ON) # Used later, to decide if to run ACX_NANOSLEEP

set(HOST string (TOLOWER "${CMAKE_SYSTEM_NAME}"))

if(MINGW OR MSVC)
  set(DEFAULT_BUILD_MINIMAL ON)
  set(DEFAULT_BUILD_DEBUGALLOC OFF)
  set(NEED_NANOSLEEP OFF)
elseif(CYGWIN)
  set(DEFAULT_BUILD_CPU_PROFILER OFF)
endif()

# Heap checker is Linux-only (and deprecated).
if(CMAKE_SYSTEM MATCHES Linux)
  set(DEFAULT_BUILD_HEAP_CHECKER ON)
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckSymbolExists)
include(CheckTypeSize)
include(CheckVariableExists)
include(CMakeDependentOption)
include(CTest)
include(CPack)
include(GNUInstallDirs)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(DefineTargetVariables)

define_target_variables()

# Currently only backtrace works on s390.
if(s390 OR OSX)
  set(default_enable_libunwind OFF)
  set(default_enable_backtrace ON)
else()
  set(default_enable_libunwind ON)
  set(default_enable_backtrace OFF)
endif()

# Disable libunwind linking on ppc64 by default.
if(PPC64)
  set(default_enable_libunwind OFF)
  set(default_tcmalloc_pagesize 64)
else()
  if(PPC)
    set(default_enable_libunwind OFF)
  else()
    set(default_enable_libunwind ON)
  endif()
  set(default_tcmalloc_pagesize 8)
endif()

cmake_dependent_option(
  GPERFTOOLS_BUILD_CPU_PROFILER "Build cpu-profiler"
  ${DEFAULT_BUILD_CPU_PROFILER} "NOT gperftools_build_minimal" OFF)
cmake_dependent_option(
  GPERFTOOLS_BUILD_HEAP_PROFILER "Build heap-profiler"
  ${DEFAULT_BUILD_HEAP_PROFILER} "NOT gperftools_build_minimal" OFF)
cmake_dependent_option(
  GPERFTOOLS_BUILD_HEAP_CHECKER "Build heap-checker"
  ${DEFAULT_BUILD_HEAP_CHECKER} "NOT gperftools_build_minimal" OFF)
cmake_dependent_option(
  GPERFTOOLS_BUILD_DEBUGALLOC "Build debugalloc" ${DEFAULT_BUILD_DEBUGALLOC}
  "NOT gperftools_build_minimal" OFF)
option(gperftools_build_minimal
       "Build only tcmalloc-minimal (and maybe tcmalloc-minimal-debug)"
       ${DEFAULT_BUILD_MINIMAL})
if(gperftools_build_minimal)
  set(GPERFTOOLS_BUILD_CPU_PROFILER OFF)
  set(GPERFTOOLS_BUILD_HEAP_PROFILER OFF)
  set(GPERFTOOLS_BUILD_HEAP_CHECKER OFF)
endif()

cmake_dependent_option(gperftools_build_benchmark "Build benchmark" ON
                       "NOT MINGW AND NOT MSVC" OFF)

option(gperftools_enable_stacktrace_via_backtrace
       "Enable use of backtrace() for stacktrace capturing (may deadlock)"
       ${default_enable_backtrace})
option(gperftools_enable_libunwind "Enable libunwind linking"
       ${default_enable_libunwind})

set(enable_backtrace ${gperftools_enable_stacktrace_via_backtrace})
set(enable_libunwind ${gperftools_enable_libunwind})

option(
  gperftools_enable_libgcc_unwinder_by_default
  "Prefer libgcc's _Unwind_Backtrace as default stacktrace capturing method"
  OFF)
set(PREFER_LIBGCC_UNWINDER ${gperftools_enable_libgcc_unwinder_by_default})

set(gperftools_tcmalloc_pagesize
    ${default_tcmalloc_pagesize}
    CACHE STRING "Set the tcmalloc internal page size")
set(allowed_page_sizes LIST "4;8;16;32;64;128;256")
set_property(CACHE gperftools_tcmalloc_pagesize PROPERTY STRINGS
                                                         ${allowed_page_sizes})
if(NOT gperftools_tcmalloc_pagesize IN_LIST allowed_page_sizes)
  message(
    WARNING
      "Invalid gperftools_tcmalloc_pagesize (${gperftools_tcmalloc_pagesize}), "
      "setting to default value (${default_tcmalloc_pagesize})")
  set(gperftools_tcmalloc_pagesize ${default_tcmalloc_pagesize})
endif()
if(gperftools_tcmalloc_pagesize EQUAL 4)
  set(TCMALLOC_PAGE_SIZE_SHIFT 12)
elseif(gperftools_tcmalloc_pagesize EQUAL 8)
  # default page size
elseif(gperftools_tcmalloc_pagesize EQUAL 16)
  set(TCMALLOC_PAGE_SIZE_SHIFT 14)
elseif(gperftools_tcmalloc_pagesize EQUAL 32)
  set(TCMALLOC_PAGE_SIZE_SHIFT 15)
elseif(gperftools_tcmalloc_pagesize EQUAL 64)
  set(TCMALLOC_PAGE_SIZE_SHIFT 16)
elseif(gperftools_tcmalloc_pagesize EQUAL 128)
  set(TCMALLOC_PAGE_SIZE_SHIFT 17)
elseif(gperftools_tcmalloc_pagesize EQUAL 256)
  set(TCMALLOC_PAGE_SIZE_SHIFT 18)
else()
  message(
    WARNING
      "${gperftools_tcmalloc_pagesize}K size not supported, using default tcmalloc page size."
  )
endif()

set(gperftools_tcmalloc_alignment
    ${DEFAULT_TCMALLOC_ALIGNMENT}
    CACHE STRING "Set the tcmalloc allocation alignment")
set_property(CACHE gperftools_tcmalloc_alignment PROPERTY STRINGS "8" "16")
if(NOT gperftools_tcmalloc_alignment STREQUAL "8"
   AND NOT gperftools_tcmalloc_alignment STREQUAL "16")
  message(
    WARNING
      "Invalid gperftools_tcmalloc_alignment (${gperftools_tcmalloc_alignment}), "
      "setting to default value (${DEFAULT_TCMALLOC_ALIGNMENT})")
  set(gperftools_tcmalloc_alignment ${DEFAULT_TCMALLOC_ALIGNMENT})
endif()
if(gperftools_tcmalloc_alignment STREQUAL "8")
  set(TCMALLOC_ALIGN_8BYTES ON)
endif()

# AX_CXX_COMPILE_STDCXX(11, ext, mandatory)
if(cxx_std_17 IN_LIST CMAKE_CXX_COMPILE_FEATURES)
  set(CMAKE_CXX_STANDARD 17) # std::align_val_t
else()
  set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# AX_C___ATTRIBUTE__
check_c_source_compiles(
  "#include <stdlib.h>
                         static void foo(void) __attribute__ ((unused));
                         void foo(void) { exit(1); }
                         int main() { return 0; }"
  HAVE___ATTRIBUTE__)

check_c_source_compiles(
  "#include <stdlib.h>
                        void foo(void) __attribute__((aligned(128)));
                        void foo(void) { exit(1); }
                        int main() { return 0; }"
  HAVE___ATTRIBUTE__ALIGNED_FN)

set(CMAKE_EXTRA_INCLUDE_FILES "malloc.h")
check_type_size("struct mallinfo" STRUCT_MALLINFO LANGUAGE CXX)
check_type_size("struct mallinfo2" STRUCT_MALLINFO2 LANGUAGE CXX)
set(CMAKE_EXTRA_INCLUDE_FILES "elf.h")
check_type_size("Elf32_Versym" ELF32_VERSYM LANGUAGE CXX) # for vdso_support.h
set(CMAKE_EXTRA_INCLUDE_FILES)
check_function_exists("sbrk" HAVE_SBRK) # for tcmalloc to get memory
check_function_exists("__sbrk" HAVE___SBRK) # for tcmalloc to get memory
check_function_exists("geteuid" HAVE_GETEUID) # for turning off services when
                                              # run as root
check_function_exists("fork" HAVE_FORK) # for the pthread_atfork setup
check_include_file("features.h" HAVE_FEATURES_H) # for vdso_support.h, Where
                                                 # __GLIBC__ is defined
check_include_file("malloc.h" HAVE_MALLOC_H) # some systems define stuff there,
                                             # others not
check_include_file("glob.h" HAVE_GLOB_H) # for heap-profile-table (cleaning up
                                         # profiles)
check_include_file("execinfo.h" HAVE_EXECINFO_H) # for stacktrace? and
                                                 # heapchecker_unittest
check_include_file("unwind.h" HAVE_UNWIND_H) # for stacktrace
check_include_file("sched.h" HAVE_SCHED_H) # for being nice in our spinlock code
check_include_file("sys/syscall.h" HAVE_SYS_SYSCALL_H)
check_include_file("sys/socket.h" HAVE_SYS_SOCKET_H) # optional; for forking out
                                                     # to symbolizer
check_include_file("sys/wait.h" HAVE_SYS_WAIT_H) # optional; for forking out to
                                                 # symbolizer
check_include_file("poll.h" HAVE_POLL_H) # optional; for forking out to
                                         # symbolizer
check_include_file("fcntl.h" HAVE_FCNTL_H) # for tcmalloc_unittest
check_include_file("grp.h" HAVE_GRP_H) # for heapchecker_unittest
check_include_file("pwd.h" HAVE_PWD_H) # for heapchecker_unittest
check_include_file("sys/resource.h" HAVE_SYS_RESOURCE_H) # for
                                                         # memalign_unittest.cc
check_include_file("sys/cdefs.h" HAVE_SYS_CDEFS_H) # Where glibc defines __THROW

check_include_file("sys/ucontext.h" HAVE_SYS_UCONTEXT_H)
check_include_file("ucontext.h" HAVE_UCONTEXT_H)
check_include_file("cygwin/signal.h" HAVE_CYGWIN_SIGNAL_H) # ucontext on cywgin
check_include_file("asm/ptrace.h" HAVE_ASM_PTRACE_H) # get ptrace macros, e.g.
                                                     # PT_NIP

check_include_file("unistd.h" HAVE_UNISTD_H)
# We also need <ucontext.h>/<sys/ucontext.h>, but we get those from
# AC_PC_FROM_UCONTEXT, below.

# We override a lot of memory allocation routines, not all of which are
# standard.  For those the system doesn't declare, we'll declare ourselves.
set(CMAKE_REQUIRED_DEFINITIONS -D_XOPEN_SOURCE=600)
check_symbol_exists("cfree" "stdlib.h;malloc.h" HAVE_DECL_CFREE)
check_symbol_exists("posix_memalign" "stdlib.h;malloc.h"
                    HAVE_DECL_POSIX_MEMALIGN)
check_symbol_exists("memalign" "stdlib.h;malloc.h" HAVE_DECL_MEMALIGN)
check_symbol_exists("valloc" "stdlib.h;malloc.h" HAVE_DECL_VALLOC)
check_symbol_exists("pvalloc" "stdlib.h;malloc.h" HAVE_DECL_PVALLOC)
set(CMAKE_REQUIRED_DEFINITIONS)

if(HAVE_STRUCT_MALLINFO)
  set(HAVE_STRUCT_MALLINFO 1)
else()
  set(HAVE_STRUCT_MALLINFO 0)
endif()

if(HAVE_STRUCT_MALLINFO2)
  set(HAVE_STRUCT_MALLINFO2 1)
else()
  set(HAVE_STRUCT_MALLINFO2 0)
endif()

# We hardcode HAVE_MMAP to 1. There are no interesting systems anymore without
# functional mmap. And our windows (except mingw) builds aren't using autoconf.
# So we keep HAVE_MMAP define, but only to distingush windows and rest.
if(NOT WIN32)
  set(HAVE_MMAP 1)
endif()

# Some tests test the behavior of .so files, and only make sense for dynamic.
option(GPERFTOOLS_BUILD_STATIC "Enable Static" ON)

if(gperftools_enable_libunwind)
  check_include_file("libunwind.h" HAVE_LIBUNWIND_H)
  if(HAVE_LIBUNWIND_H)
    find_library(libunwind_location NAMES unwind)
    if(libunwind_location)
      check_library_exists(unwind backtrace ${libunwind_location}
                           have_libunwind)
    endif()
    if(have_libunwind)
      set(unwind_libs ${libunwind_location})
      set(will_use_libunwind ON)
      set(USE_LIBUNWIND 1)
    endif()
  endif()
endif()

# See if the compiler supports -Wno-unused-result. Newer ubuntu's turn on
# -D_FORTIFY_SOURCE=2, enabling __attribute__((warn_unused_result)) for things
# like write(), which we don't care about.
check_c_compiler_flag("-Wno-unused-result" have_w_no_unused_result)

check_c_compiler_flag("-fno-omit-frame-pointer -momit-leaf-frame-pointer"
                      have_omit_leaf_fp)
check_c_source_compiles(
  "
  #if !(__i386__ || __x86_64__ || __riscv || __aarch64__)
  #error unsupported arch
  #endif
  int main() { return 0; }
  "
  use_omit_leaf_fp)

if(use_omit_leaf_fp)
  add_compile_options(-fno-omit-frame-pointer -momit-leaf-frame-pointer)
endif()

option(gperftools_dynamic_sized_delete_support
       "Try to build run-time switch for sized delete operator" OFF)
if(gperftools_dynamic_sized_delete_support)
  set(ENABLE_DYNAMIC_SIZED_DELETE 1)
endif()

option(gperftools_sized_delete "Build sized delete operator" OFF)
if(gperftools_sized_delete)
  set(ENABLE_SIZED_DELETE 1)
endif()

if(NOT MSVC)
  set(CMAKE_REQUIRED_FLAGS -fsized-deallocation)
  check_cxx_source_compiles(
    "
  #include <new>
  int main() { (::operator delete)(0, 256); return 0; }"
    have_sized_deallocation)
  set(CMAKE_REQUIRED_FLAGS)
endif()

check_cxx_source_compiles(
  "
  #include <new>
  int main() { (::operator delete)((::operator new)(256, std::align_val_t(16)), std::align_val_t(16)); return 0; }"
  HAVE_STD_ALIGN_VAL_T)
if(HAVE_STD_ALIGN_VAL_T)
  set(HAVE_STD_ALIGN_VAL_T 1)
  set(ENABLE_ALIGNED_NEW_DELETE 1)
else()
  set(HAVE_STD_ALIGN_VAL_T 0)
endif()

check_cxx_compiler_flag("-faligned-new" have_f_aligned_new)

check_c_source_compiles(
  "
  #include <unwind.h>
  int main()
  {
#if __APPLE__ || __FreeBSD__
#error OSX _Unwind_Backtrace recurses back to malloc
#endif
    &_Unwind_Backtrace;
    return 0;
  }"
  HAVE_UNWIND_BACKTRACE)

if(enable_backtrace)
  set(default_emergency_malloc ON)
else()
  set(default_emergency_malloc OFF)
endif()

if(will_use_libunwind AND ARM)
  set(default_emergency_malloc ON)
endif()

option(gperftools_emergency_malloc "Build emergency malloc"
       ${default_emergency_malloc})

check_c_source_compiles("int main() { return __builtin_expect(main != 0, 1); }"
                        HAVE_BUILTIN_EXPECT)

check_c_source_compiles(
  "
  #include <unistd.h>
  int main()
  {
    char** env = __environ;
    return 0;
  }"
  HAVE___ENVIRON)

# If we support __thread, that can speed up tcmalloc a bit. Note, however, that
# our code tickles a bug in gcc < 4.1.2 involving TLS and -fPIC (which our
# libraries will use) on x86:
# http://gcc.gnu.org/ml/gcc-bugs/2006-09/msg02275.html
#
# And mingw also does compile __thread but resultant code actually fails to work
# correctly at least in some not so ancient version:
# http://mingw-users.1079350.n2.nabble.com/gcc-4-4-multi-threaded-exception-handling-amp-thread-specifier-not-working-td3440749.html
#
# Also it was reported that earlier gcc versions for mips compile __thread but
# it doesn't really work
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION
                                            VERSION_LESS "4.1.2")
  message(
    WARNING
      "gcc has this bug: http://gcc.gnu.org/ml/gcc-bugs/2006-09/msg02275.html")
elseif(APPLE)
  message(WARNING "OSX __thread support is known to call malloc which makes "
                  "it unsafe to use from malloc replacement")
elseif(MINGW)
  message(WARNING "mingw doesn't really support tls")
elseif(MSVC)
  check_c_source_compiles("static __declspec(thread) int p = 0; int main() {}"
                          HAVE_TLS)
else()
  check_c_source_compiles("static __thread int p = 0; int main() {}" HAVE_TLS)
endif()

if(NEED_NANOSLEEP)
  check_c_source_compiles(
    "#include <time.h>
     int main()
     { static struct timespec ts; nanosleep(&ts, NULL); return 0; }"
    nanosleep_ok)
  if(NOT nanosleep_ok)
    set(CMAKE_REQUIRED_LIBRARIES rt)
    check_c_source_compiles(
      "#include <time.h>
       int main()
       { static struct timespec ts; nanosleep(&ts, NULL); return 0; }"
      nanosleep_ok)
    if(nanosleep_ok)
      set(nanosleep_libs rt)
    else()
      message(FATAL_ERROR "cannot find the nanosleep function")
    endif()
    set(CMAKE_REQUIRED_LIBRARIES)
  endif()
endif()

# Nanosleep requires extra libraries on some architectures (solaris). This sets
# NANOSLEEP_LIBS.  nanosleep doesn't exist on mingw, which is fine for us
# because we don't compile libspinlock, which uses it.
if(enable_backtrace)
  check_symbol_exists("backtrace" "execinfo.h" HAVE_DECL_BACKTRACE)
  check_function_exists("backtrace" backtrace_exists)
  if(NOT backtrace_exists)
    set(CMAKE_REQUIRED_LIBRARIES execinfo)
    check_function_exists("backtrace" backtrace_exists)
    set(CMAKE_REQUIRED_LIBRARIES)
    if(backtrace_exists)
      list(INSERT unwind_libs 0 execinfo)
    endif()
  endif()
endif()

find_package(Threads REQUIRED)
set(HAVE_PTHREAD ${CMAKE_USE_PTHREADS_INIT})

if(FreeBSD)
  set(PTHREADS_CRASHES_IF_RUN_TOO_EARLY ON)
endif()

set(libstdcxx_la_linker_flag)
if(EXISTS /usr/sfw/lib/libstdc++.la)
  file(READ /usr/sfw/lib/libstdc++.la _ch LIMIT 1)
  if(string (LENGTH _ch) EQUAL 0)
    set(libstdcxx_la_linker_flag "-L${CMAKE_CURRENT_SOURCE_DIR}/src/solaris")
  endif()
endif()

check_variable_exists("program_invocation_name" HAVE_PROGRAM_INVOCATION_NAME)

if(MINGW)
  check_symbol_exists("sleep" "unistd.h" HAVE_DECL_SLEEP)
  check_symbol_exists("nanosleep" "time.h" HAVE_DECL_NANOSLEEP)
endif()

if(LINUX)
  check_c_source_compiles(
    "
    #include <signal.h>
    #include <time.h>
    int main() { return SIGEV_THREAD_ID || CLOCK_THREAD_CPUTIME_ID; }"
    HAVE_LINUX_SIGEV_THREAD_ID)
endif()

# Disable large allocation report by default.
option(gperftools_enable_large_alloc_report
       "Report very large allocations to stderr" OFF)
set(ENABLE_LARGE_ALLOC_REPORT ${gperftools_enable_large_alloc_report})

# Enable aggressive decommit by default
option(gperftools_enable_aggressive_decommit_by_default
       "Enable aggressive decommit by default" OFF)
set(ENABLE_AGGRESSIVE_DECOMMIT_BY_DEFAULT
    ${gperftools_enable_aggressive_decommit_by_default})

configure_file(cmake/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)
configure_file(cmake/tcmalloc.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/gperftools/tcmalloc.h @ONLY)

if(GPERFTOOLS_BUILD_CPU_PROFILER
   OR GPERFTOOLS_BUILD_HEAP_PROFILER
   OR GPERFTOOLS_BUILD_HEAP_CHECKER)
  set(WITH_STACK_TRACE ON)
endif()

# Based on Makefile.am

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# This is so we can #include <gperftools/foo>
include_directories($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)

if(NOT WITH_STACK_TRACE)
  add_compile_definitions(NO_TCMALLOC_SAMPLES)
endif()

# These are good warnings to turn on by default.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -Wall -Wwrite-strings -Woverloaded-virtual -Wno-sign-compare"
  )
endif()

if(have_w_no_unused_result)
  add_compile_options(-Wno-unused-result)
endif()

if(have_sized_deallocation)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsized-deallocation")
endif()

if(have_f_aligned_new)
  add_compile_options(-faligned-new)
endif()

# LIBSTDCXX_LA_LINKER_FLAG is used to fix a Solaris bug.
add_link_options(${libstdcxx_la_linker_flag})

option(gperftools_enable_frame_pointers
       "Compile with -fno-omit-frame-pointer (see INSTALL)" OFF)

if(gperftools_enable_frame_pointers)
  add_compile_options(-fno-omit-frame-pointer -DFORCED_FRAME_POINTERS)
endif()

# For windows systems (at least, mingw), we need to tell all our tests to link
# in libtcmalloc using -u.  This is because libtcmalloc accomplishes its tasks
# via patching, leaving no work for the linker to identify, so the linker will
# ignore libtcmalloc by default unless we explicitly create a dependency via -u.
set(TCMALLOC_FLAGS)
if(MINGW)
  list(APPEND TCMALLOC_FLAGS "-Wl,-u__tcmalloc")
endif()

set(googleinclude_HEADERS
    src/google/heap-checker.h
    src/google/heap-profiler.h
    src/google/malloc_extension.h
    src/google/malloc_extension_c.h
    src/google/malloc_hook.h
    src/google/malloc_hook_c.h
    src/google/profiler.h
    src/google/stacktrace.h
    src/google/tcmalloc.h)
install(FILES ${googleinclude_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/google)

# This is a 'convenience library' -- it's not actually installed or anything
set(LOGGING_INCLUDES src/base/logging.h src/base/commandlineflags.h
                     src/base/basictypes.h src/base/dynamic_annotations.h)
set(liblogging_la_SOURCES src/base/logging.cc src/base/dynamic_annotations.cc
                          ${LOGGING_INCLUDES})
add_library(logging STATIC ${liblogging_la_SOURCES})

set(SYSINFO_INCLUDES src/base/sysinfo.h src/getenv_safe.h src/base/logging.h
                     src/base/commandlineflags.h src/base/basictypes.h)
set(libsysinfo_la_SOURCES src/base/sysinfo.cc ${SYSINFO_INCLUDES})
set(libsysinfo_la_LIBADD ${NANOSLEEP_LIBS})
add_library(sysinfo STATIC ${libsysinfo_la_SOURCES})
target_link_libraries(sysinfo ${libsysinfo_la_LIBADD})

# For MinGW, we use also have to use libwindows Luckily, we need the windows.a
# library in exactly the same place we need spinlock.a (pretty much everywhere),
# so we can use the same variable name for each.  We can also optimize the MinGW
# rule a bit by leaving out files we know aren't used on windows.  libwindows
# also obsoletes the need for other files like system_alloc.cc.
if(MINGW OR MSVC)
  set(WINDOWS_INCLUDES
      src/windows/port.h src/windows/mingw.h src/windows/mini_disassembler.h
      src/windows/mini_disassembler_types.h src/windows/preamble_patcher.h)
  set(libwindows_la_SOURCES
      ${WINDOWS_INCLUDES}
      src/windows/port.cc
      src/windows/system-alloc.cc
      src/windows/ia32_modrm_map.cc
      src/windows/ia32_opcode_map.cc
      src/windows/mini_disassembler.cc
      src/windows/patch_functions.cc
      src/windows/preamble_patcher.cc
      src/windows/preamble_patcher_with_stub.cc)
  add_library(windows_object OBJECT ${libwindows_la_SOURCES})
  add_library(windows INTERFACE)
  target_sources(windows INTERFACE $<TARGET_OBJECTS:windows_object>)
  # patch_functions.cc uses Psapi.lib.  MSVC has a #pragma for that, but not us.
  target_link_libraries(windows INTERFACE psapi)

  set(SPINLOCK_INCLUDES
      src/base/spinlock.h src/base/spinlock_internal.h
      src/base/spinlock_win32-inl.h src/base/spinlock_linux-inl.h
      src/base/spinlock_posix-inl.h)
  set(libspinlock_la_SOURCES src/base/spinlock.cc src/base/spinlock_internal.cc
                             ${SPINLOCK_INCLUDES})
  add_library(spinlock STATIC ${libspinlock_la_SOURCES})
  set(LIBSPINLOCK windows spinlock sysinfo logging)
  # We also need to tell mingw that sysinfo.cc needs shlwapi.lib. (We do this
  # via a #pragma for msvc, but need to do it here for mingw).
  target_link_libraries(sysinfo shlwapi)
  # spinlock uses WaitOnAddress et al. We need to link to synchronization.lib
  # (We also do this via a #pragma for msvc, but need to do it here for mingw).
  target_link_libraries(spinlock synchronization)

else()
  set(SPINLOCK_INCLUDES src/base/spinlock.h src/base/spinlock_internal.h)
  set(libspinlock_la_SOURCES src/base/spinlock.cc src/base/spinlock_internal.cc
                             ${SPINLOCK_INCLUDES})
  add_library(spinlock STATIC ${libspinlock_la_SOURCES})
  target_link_libraries(spinlock ${nanosleep_libs})
  set(LIBSPINLOCK spinlock sysinfo logging)
  set(TCMALLOC_CC "src/tcmalloc.cc")
  set(SYSTEM_ALLOC_CC "src/system-alloc.cc")

endif()

# ------- stack trace

if(WITH_STACK_TRACE)

  set(S_STACKTRACE_INCLUDES
      src/stacktrace_impl_setup-inl.h
      src/stacktrace_generic-inl.h
      src/stacktrace_libgcc-inl.h
      src/stacktrace_libunwind-inl.h
      src/stacktrace_arm-inl.h
      src/stacktrace_powerpc-inl.h
      src/stacktrace_powerpc-darwin-inl.h
      src/stacktrace_powerpc-linux-inl.h
      src/stacktrace_win32-inl.h
      src/stacktrace_instrument-inl.h
      src/base/elf_mem_image.h
      src/base/vdso_support.h)

  set(SG_STACKTRACE_INCLUDES src/gperftools/stacktrace.h)
  set(STACKTRACE_INCLUDES ${S_STACKTRACE_INCLUDES} ${SG_STACKTRACE_INCLUDES})
  list(APPEND perftoolsinclude_HEADERS ${SG_STACKTRACE_INCLUDES})

  # Making the library
  set(libstacktrace_la_SOURCES src/stacktrace.cc src/base/elf_mem_image.cc
                               src/base/vdso_support.cc ${STACKTRACE_INCLUDES})
  add_library(stacktrace INTERFACE)
  add_library(stacktrace_object OBJECT ${libstacktrace_la_SOURCES})
  target_link_libraries(stacktrace INTERFACE ${unwind_libs} ${LIBSPINLOCK})
  target_sources(stacktrace INTERFACE $<TARGET_OBJECTS:stacktrace_object>)

  set(libfake_stacktrace_scope_la_SOURCES src/fake_stacktrace_scope.cc)
  add_library(fake_stacktrace_scope ${libfake_stacktrace_scope_la_SOURCES})

endif()

# ------- pprof

# If we are not compiling with stacktrace support, pprof is worthless
if(WITH_STACK_TRACE)
  install(
    FILES src/pprof
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    RENAME pprof-symbolize)

  if(INSTALL_PPROF)
    install(FILES src/pprof DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endif()

# ------- tcmalloc_minimal (thread-caching malloc)

# The header files we use.  We divide into categories based on directory
set(S_TCMALLOC_MINIMAL_INCLUDES
    src/common.h
    src/internal_logging.h
    src/system-alloc.h
    src/packed-cache-inl.h
    ${SPINLOCK_INCLUDES}
    src/tcmalloc_guard.h
    src/base/commandlineflags.h
    src/base/basictypes.h
    src/safe_strerror.h
    src/pagemap.h
    src/sampler.h
    src/central_freelist.h
    src/linked_list.h
    src/libc_override.h
    src/libc_override_gcc_and_weak.h
    src/libc_override_glibc.h
    src/libc_override_osx.h
    src/libc_override_redefine.h
    src/page_heap.h
    src/page_heap_allocator.h
    src/span.h
    src/static_vars.h
    src/symbolize.h
    src/thread_cache.h
    src/stack_trace_table.h
    src/base/thread_annotations.h
    src/malloc_hook-inl.h)
set(SG_TCMALLOC_MINIMAL_INCLUDES
    src/gperftools/malloc_hook.h src/gperftools/malloc_hook_c.h
    src/gperftools/malloc_extension.h src/gperftools/malloc_extension_c.h
    src/gperftools/nallocx.h)
set(TCMALLOC_MINIMAL_INCLUDES
    ${S_TCMALLOC_MINIMAL_INCLUDES} ${SG_TCMALLOC_MINIMAL_INCLUDES}
    ${SG_STACKTRACE_INCLUDES})
list(APPEND perftoolsinclude_HEADERS ${SG_TCMALLOC_MINIMAL_INCLUDES})

# Making the library

set(libtcmalloc_minimal_internal_la_SOURCES
    src/common.cc
    src/internal_logging.cc
    ${SYSTEM_ALLOC_CC}
    src/memfs_malloc.cc
    src/safe_strerror.cc
    src/central_freelist.cc
    src/page_heap.cc
    src/sampler.cc
    src/span.cc
    src/stack_trace_table.cc
    src/static_vars.cc
    src/symbolize.cc
    src/thread_cache.cc
    src/malloc_hook.cc
    src/malloc_extension.cc
    ${TCMALLOC_MINIMAL_INCLUDES})
add_library(tcmalloc_minimal_internal_object OBJECT
            ${libtcmalloc_minimal_internal_la_SOURCES})
# We #define NO_TCMALLOC_SAMPLES, since sampling is turned off for _minimal.
target_compile_definitions(tcmalloc_minimal_internal_object
                           PRIVATE NO_TCMALLOC_SAMPLES NO_HEAP_CHECK NDEBUG)
add_library(tcmalloc_minimal_internal INTERFACE)
target_link_libraries(tcmalloc_minimal_internal INTERFACE ${LIBSPINLOCK})
target_sources(tcmalloc_minimal_internal
               INTERFACE $<TARGET_OBJECTS:tcmalloc_minimal_internal_object>)

set(libtcmalloc_minimal_la_SOURCES ${TCMALLOC_CC} ${TCMALLOC_MINIMAL_INCLUDES})
set(libtcmalloc_minimal_la_DEFINES NO_TCMALLOC_SAMPLES NDEBUG)
add_library(tcmalloc_minimal SHARED ${libtcmalloc_minimal_la_SOURCES})
target_compile_definitions(tcmalloc_minimal
                           PRIVATE ${libtcmalloc_minimal_la_DEFINES})
set(libtcmalloc_minimal_la_LIBADD tcmalloc_minimal_internal)
target_link_libraries(tcmalloc_minimal PRIVATE tcmalloc_minimal_internal
                                               Threads::Threads)
if(MINGW)
  target_link_libraries(tcmalloc_minimal PRIVATE stacktrace)
endif()
set_target_properties(
  tcmalloc_minimal PROPERTIES VERSION ${TCMALLOC_SO_VERSION}
                              SOVERSION ${TCMALLOC_SO_VERSION})
install(TARGETS tcmalloc_minimal)
if(GPERFTOOLS_BUILD_STATIC)
  add_library(tcmalloc_minimal_static STATIC
              ${libtcmalloc_minimal_internal_la_SOURCES})
  target_compile_definitions(tcmalloc_minimal_static PRIVATE NO_TCMALLOC_SAMPLES
                                                             NDEBUG)
  target_link_libraries(tcmalloc_minimal_static
                        PRIVATE tcmalloc_minimal_internal Threads::Threads)
  if(MINGW)
    target_link_libraries(tcmalloc_minimal_static PRIVATE stacktrace)
  endif()
  if(NOT MSVC)
    set_target_properties(tcmalloc_minimal_static PROPERTIES OUTPUT_NAME
                                                             tcmalloc_minimal)
  endif()
  install(TARGETS tcmalloc_minimal_static)
endif()

# ------- tcmalloc_minimal_debug (thread-caching malloc with debugallocation)

if(GPERFTOOLS_BUILD_DEBUGALLOC)
  set(libtcmalloc_minimal_debug_la_SOURCES src/debugallocation.cc
                                           ${TCMALLOC_MINIMAL_INCLUDES})
  add_library(tcmalloc_minimal_debug SHARED
              ${libtcmalloc_minimal_debug_la_SOURCES})
  target_compile_definitions(
    tcmalloc_minimal_debug PRIVATE ${libtcmalloc_minimal_la_DEFINES}
                                   TCMALLOC_FOR_DEBUGALLOCATION)
  target_link_libraries(tcmalloc_minimal_debug
                        PRIVATE ${libtcmalloc_minimal_la_LIBADD})
  target_link_libraries(tcmalloc_minimal_debug PRIVATE Threads::Threads)
  install(TARGETS tcmalloc_minimal_debug)
  set_target_properties(
    tcmalloc_minimal_debug PROPERTIES VERSION ${TCMALLOC_SO_VERSION}
                                      SOVERSION ${TCMALLOC_SO_VERSION})
  if(GPERFTOOLS_BUILD_STATIC)
    add_library(tcmalloc_minimal_debug_static STATIC
                ${libtcmalloc_minimal_debug_la_SOURCES})
    target_compile_definitions(
      tcmalloc_minimal_debug_static PRIVATE ${libtcmalloc_minimal_la_DEFINES}
                                            TCMALLOC_FOR_DEBUGALLOCATION)
    if(NOT MSVC)
      set_target_properties(tcmalloc_minimal_debug_static
                            PROPERTIES OUTPUT_NAME tcmalloc_minimal_debug)
    endif()
    target_link_libraries(tcmalloc_minimal_debug_static
                          PRIVATE ${libtcmalloc_minimal_la_LIBADD})
    install(TARGETS tcmalloc_minimal_debug_static)
  endif()

endif()

# ------- tcmalloc (thread-caching malloc + heap profiler + heap checker)

if(GPERFTOOLS_BUILD_HEAP_CHECKER OR GPERFTOOLS_BUILD_HEAP_PROFILER)
  # The header files we use.  We divide into categories based on directory
  set(S_TCMALLOC_INCLUDES
      ${S_TCMALLOC_MINIMAL_INCLUDES}
      ${LOGGING_INCLUDES}
      src/addressmap-inl.h
      src/raw_printer.h
      src/base/googleinit.h
      src/base/linuxthreads.h
      src/base/stl_allocator.h
      src/base/sysinfo.h
      src/heap-profile-table.h
      src/heap-profile-stats.h
      src/maybe_emergency_malloc.h
      src/mmap_hook.h
      src/emergency_malloc.h)

  set(SG_TCMALLOC_INCLUDES src/gperftools/heap-profiler.h
                           src/gperftools/heap-checker.h)
  set(TCMALLOC_INCLUDES ${S_TCMALLOC_INCLUDES} ${SG_TCMALLOC_MINIMAL_INCLUDES}
                        ${SG_TCMALLOC_INCLUDES} ${SG_STACKTRACE_INCLUDES})
  list(APPEND perftoolsinclude_HEADERS ${SG_TCMALLOC_INCLUDES})

  if(gperftools_emergency_malloc)
    set(EMERGENCY_MALLOC_CC src/emergency_malloc.cc
                            src/emergency_malloc_for_stacktrace.cc)
    set(EMERGENCY_MALLOC_DEFINE ENABLE_EMERGENCY_MALLOC)
  else()
    set(EMERGENCY_MALLOC_CC src/fake_stacktrace_scope.cc)
  endif()

  # Making the library

  set(libtcmalloc_internal_la_SOURCES
      ${libtcmalloc_minimal_internal_la_SOURCES}
      ${TCMALLOC_INCLUDES}
      src/base/low_level_alloc.cc
      src/mmap_hook.cc
      src/heap-profile-table.cc
      src/heap-profiler.cc
      src/raw_printer.cc
      ${EMERGENCY_MALLOC_CC}
      src/memory_region_map.cc)
  set(libtcmalloc_internal_la_DEFINE NDEBUG ${EMERGENCY_MALLOC_DEFINE})
  set(libtcmalloc_internal_la_LIBADD stacktrace Threads::Threads)

  set(libtcmalloc_la_SOURCES ${TCMALLOC_CC} ${TCMALLOC_INCLUDES})
  set(libtcmalloc_la_DEFINE NDEBUG ${EMERGENCY_MALLOC_DEFINE})
  set(libtcmalloc_la_LIBADD tcmalloc_internal Threads::Threads)
  if(GPERFTOOLS_BUILD_HEAP_CHECKER)
    # heap-checker-bcad is last, in hopes its global ctor will run first. (Note
    # this is added to libtcmalloc.la, not libtcmalloc_internal.la, but that's
    # ok; the internal/external distinction is only useful for cygwin, and
    # cygwin doesn't use HEAP_CHECKER anyway.)
    set(HEAP_CHECKER_SOURCES src/base/linuxthreads.cc src/heap-checker.cc
                             src/heap-checker-bcad.cc)
    list(APPEND libtcmalloc_la_SOURCES ${HEAP_CHECKER_SOURCES})
  else()
    list(APPEND libtcmalloc_internal_la_DEFINE NO_HEAP_CHECK)
    list(APPEND libtcmalloc_la_DEFINE NO_HEAP_CHECK)
  endif()

  add_library(tcmalloc_internal_object OBJECT
              ${libtcmalloc_internal_la_SOURCES})
  target_compile_definitions(tcmalloc_internal_object
                             PRIVATE ${libtcmalloc_internal_la_DEFINE})
  add_library(tcmalloc_internal INTERFACE)
  target_sources(tcmalloc_internal
                 INTERFACE $<TARGET_OBJECTS:tcmalloc_internal_object>)
  target_link_libraries(tcmalloc_internal
                        INTERFACE ${libtcmalloc_internal_la_LIBADD})
  target_include_directories(tcmalloc_internal
                             INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)

  add_library(tcmalloc SHARED ${libtcmalloc_la_SOURCES})
  target_compile_definitions(tcmalloc PRIVATE ${libtcmalloc_la_DEFINE})
  target_link_libraries(tcmalloc ${libtcmalloc_la_LIBADD})
  set_target_properties(tcmalloc PROPERTIES VERSION ${TCMALLOC_SO_VERSION}
                                            SOVERSION ${TCMALLOC_SO_VERSION})
  install(TARGETS tcmalloc)
  if(GPERFTOOLS_BUILD_STATIC)
    add_library(tcmalloc_static STATIC ${libtcmalloc_la_SOURCES})
    target_compile_definitions(tcmalloc_static PRIVATE ${libtcmalloc_la_DEFINE})
    if(NOT MSVC)
      set_target_properties(tcmalloc_static PROPERTIES OUTPUT_NAME tcmalloc)
    endif()
    target_link_libraries(tcmalloc_static PRIVATE ${libtcmalloc_la_LIBADD})
    install(TARGETS tcmalloc_static)
  endif()

  # Unittests
endif()

# ------- tcmalloc with debugallocation
if(GPERFTOOLS_BUILD_DEBUGALLOC)
  if(GPERFTOOLS_BUILD_HEAP_CHECKER OR GPERFTOOLS_BUILD_HEAP_PROFILER)
    add_library(
      tcmalloc_debug SHARED src/debugallocation.cc ${HEAP_CHECKER_SOURCES}
                            ${TCMALLOC_INCLUDES})
    target_compile_definitions(
      tcmalloc_debug PRIVATE ${libtcmalloc_la_DEFINE}
                             TCMALLOC_FOR_DEBUGALLOCATION)
    target_link_libraries(tcmalloc_debug PRIVATE ${libtcmalloc_la_LIBADD})
    set_target_properties(
      tcmalloc_debug PROPERTIES VERSION ${TCMALLOC_SO_VERSION}
                                SOVERSION ${TCMALLOC_SO_VERSION})
    install(TARGETS tcmalloc_debug)
    if(GPERFTOOLS_BUILD_STATIC)
      add_library(
        tcmalloc_debug_static STATIC
        src/debugallocation.cc ${HEAP_CHECKER_SOURCES} ${TCMALLOC_INCLUDES})
      target_compile_definitions(
        tcmalloc_debug_static PRIVATE ${libtcmalloc_la_DEFINE}
                                      TCMALLOC_FOR_DEBUGALLOCATION)
      target_link_libraries(tcmalloc_debug_static
                            PRIVATE ${libtcmalloc_la_LIBADD})
      if(NOT MSVC)
        set_target_properties(tcmalloc_debug_static PROPERTIES OUTPUT_NAME
                                                               tcmalloc_debug)
      endif()
      install(TARGETS tcmalloc_debug_static)
    endif()

  endif()
endif()

# ------- CPU profiler
if(GPERFTOOLS_BUILD_CPU_PROFILER)
  # The header files we use.  We divide into categories based on directory
  set(S_CPU_PROFILER_INCLUDES
      src/profiledata.h
      src/profile-handler.h
      src/getpc.h
      src/base/basictypes.h
      src/base/commandlineflags.h
      src/base/googleinit.h
      src/base/logging.h
      src/base/simple_mutex.h
      src/base/sysinfo.h
      ${SPINLOCK_INCLUDES}
      ${LOGGING_INCLUDES})
  set(SG_CPU_PROFILER_INCLUDES src/gperftools/profiler.h)
  set(CPU_PROFILER_INCLUDES
      ${S_CPU_PROFILER_INCLUDES} ${SG_CPU_PROFILER_INCLUDES}
      ${SG_STACKTRACE_INCLUDES})
  list(APPEND perftoolsinclude_HEADERS ${SG_CPU_PROFILER_INCLUDES})

  # Making the library
  set(libprofiler_la_SOURCES src/profiler.cc src/profile-handler.cc
                             src/profiledata.cc ${CPU_PROFILER_INCLUDES})
  set(libprofiler_la_LIBADD stacktrace fake_stacktrace_scope)
  add_library(profiler SHARED ${libprofiler_la_SOURCES})
  target_link_libraries(profiler PRIVATE ${libprofiler_la_LIBADD})
  target_link_libraries(profiler PRIVATE Threads::Threads)
  target_include_directories(profiler INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
  set_target_properties(profiler PROPERTIES VERSION ${PROFILER_SO_VERSION}
                                            SOVERSION ${PROFILER_SO_VERSION})
  install(TARGETS profiler)
  if(GPERFTOOLS_BUILD_STATIC)
    add_library(profiler_static STATIC ${libprofiler_la_SOURCES})
    target_link_libraries(profiler_static PRIVATE ${libprofiler_la_LIBADD})
    target_include_directories(profiler_static
                               INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(NOT MSVC)
      set_target_properties(profiler_static PROPERTIES OUTPUT_NAME profiler)
    endif()
    install(TARGETS profiler_static)
  endif()

  # See discussion above (under LIBTCMALLOC_MINIMAL) for why we do this.
  # Basically it's to work around systems where --rpath doesn't work right.
  set(LIBPROFILER stacktrace profiler)

endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/gperftools/tcmalloc.h
              ${perftoolsinclude_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/gperftools)

# ------- CPU profiler and heap checker, in one!

# Ideally, folks who wanted to use both tcmalloc and libprofiler, could just
# link them both into their application.  But while this works fine for .so
# files, it does not for .a files.  The easiest way around this -- and I've
# tried a bunch of the hard ways -- is to just to create another set of
# libraries that has both functionality in it.

if(GPERFTOOLS_BUILD_HEAP_PROFILER OR GPERFTOOLS_BUILD_HEAP_CHECKER)
  if(GPERFTOOLS_BUILD_CPU_PROFILER)
    add_library(tcmalloc_and_profiler SHARED ${libtcmalloc_la_SOURCES}
                                             ${libprofiler_la_SOURCES})
    target_compile_definitions(tcmalloc_and_profiler
                               PRIVATE ${libtcmalloc_la_DEFINE})
    set_target_properties(
      tcmalloc_and_profiler
      PROPERTIES VERSION ${TCMALLOC_AND_PROFILER_SO_VERSION}
                 SOVERSION ${TCMALLOC_AND_PROFILER_SO_VERSION})
    # We don't include libprofiler_la_LIBADD here because all it adds is
    # libstacktrace.la, which we already get via libtcmalloc.  Trying to specify
    # it twice causes link-time duplicate-definition errors. :-(
    target_link_libraries(tcmalloc_and_profiler
                          PRIVATE ${libtcmalloc_la_LIBADD})
    install(TARGETS tcmalloc_and_profiler)
    if(GPERFTOOLS_BUILD_STATIC)
      add_library(tcmalloc_and_profiler_static STATIC ${libtcmalloc_la_SOURCES}
                                                      ${libprofiler_la_SOURCES})
      target_compile_definitions(tcmalloc_and_profiler_static
                                 PRIVATE ${libtcmalloc_la_DEFINE})
      target_link_libraries(tcmalloc_and_profiler_static
                            PRIVATE ${libtcmalloc_la_LIBADD})
      target_include_directories(tcmalloc_and_profiler_static
                                 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
      if(NOT MSVC)
        set_target_properties(tcmalloc_and_profiler_static
                              PROPERTIES OUTPUT_NAME tcmalloc_and_profiler)
      endif()
      install(TARGETS tcmalloc_and_profiler_static)
    endif()

  endif()
endif()

if(MSVC)
  add_subdirectory(src/windows)
endif()

# ^^^^ END OF RULES TO MAKE YOUR LIBRARIES, BINARIES, AND UNITTESTS TODO rpm deb

# http://linux.die.net/man/1/pkg-config, http://pkg-config.freedesktop.org/wiki
# I get the description and URL lines from the rpm spec. I use sed to try to
# rewrite exec_prefix, libdir, and includedir in terms of prefix, if possible.
set(PTHREAD_FLAGS)
foreach(
  flag IN
  ITEMS INTERFACE_LINK_LIBRARIES INTERFACE_LINK_OPTIONS
        INTERFACE_INCLUDE_DIRECTORIES INTERFACE_COMPILE_OPTIONS
        INTERFACE_COMPILE_DEFINITIONS INTERFACE_SOURCES)
  get_target_property(T Threads::Threads ${flag})
  if(T)
    set(PTHREAD_FLAGS "${PTHREAD_FLAGS} ${T}")
  endif()
endforeach()
set(NAME tcmalloc)
configure_file(cmake/pkgconfig.pc libtcmalloc.pc @ONLY)
set(NAME tcmalloc_debug)
configure_file(cmake/pkgconfig.pc libtcmalloc_debug.pc @ONLY)
set(NAME tcmalloc_minimal)
configure_file(cmake/pkgconfig.pc libtcmalloc_minimal.pc @ONLY)
set(NAME tcmalloc_minimal_debug)
configure_file(cmake/pkgconfig.pc libtcmalloc_minimal_debug.pc @ONLY)
set(NAME profiler)
configure_file(cmake/pkgconfig.pc libprofiler.pc @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/libtcmalloc.pc
        ${CMAKE_CURRENT_BINARY_DIR}/libtcmalloc_minimal.pc
        ${CMAKE_CURRENT_BINARY_DIR}/libtcmalloc_debug.pc
        ${CMAKE_CURRENT_BINARY_DIR}/libtcmalloc_minimal_debug.pc
        ${CMAKE_CURRENT_BINARY_DIR}/libprofiler.pc
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

message(
  WARNING
    "note: gperftools' cmake support is incomplete and is best-effort only")

# TODO @GENERATE_CHANGELOG_RULES@ TODO dist
