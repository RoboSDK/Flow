cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_REQUIRED_FLAGS -fcoroutines)
# Set the project name to your project name, my project isn't very descriptive
project(Flow CXX)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

add_subdirectory(external)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_options INTERFACE ${CMAKE_REQUIRED_FLAGS})

include(CheckIncludeFileCXX)
macro(check_include_file_cxx _header _var)
  _check_include_file_cxx(${_header} ${_var})
  if(NOT ${_var})
    set(${_var} 0)
  endif()
endmacro()

check_include_file_cxx(coroutine HAS_STD_COROUTINE_HEADER)
check_include_file_cxx(experimental/coroutine HAS_STD_EXPERIMENTAL_COROUTINES_HEADER)

check_include_file_cxx(filesystem HAS_STD_FILESYSTEM_HEADER)
check_include_file_cxx(experimental/filesystem HAS_STD_EXPERIMENTAL_FILESYSTEM_HEADER)

message(STATUS "HAS_STD_COROUTINE_HEADER: ${HAS_STD_COROUTINE_HEADER}")
message(STATUS "HAS_STD_EXPERIMENTAL_COROUTINES_HEADER: ${HAS_STD_EXPERIMENTAL_COROUTINES_HEADER}")
message(STATUS "HAS_STD_FILESYSTEM_HEADER: ${HAS_STD_FILESYSTEM_HEADER}")
message(STATUS "HAS_STD_EXPERIMENTAL_FILESYSTEM_HEADER: ${HAS_STD_EXPERIMENTAL_FILESYSTEM_HEADER}")

target_compile_options(project_options INTERFACE

        -DCPPCORO_HAS_STD_COROUTINE_HEADER=${HAS_STD_COROUTINE_HEADER}
        )
target_compile_options(project_options INTERFACE
        -DCPPCORO_HAS_STD_EXPERIMENTAL_COROUTINES_HEADER=${HAS_STD_EXPERIMENTAL_COROUTINES_HEADER}
        )

target_compile_options(project_options INTERFACE
        -DCPPCORO_HAS_STD_FILESYSTEM_HEADER=${HAS_STD_FILESYSTEM_HEADER}
        )

target_compile_options(project_options INTERFACE
        -DCPPCORO_HAS_STD_EXPERIMENTAL_FILESYSTEM_HEADER=${HAS_STD_EXPERIMENTAL_FILESYSTEM_HEADER}
        )

if(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
  option(ENABLE_BUILD_WITH_TIME_TRACE "Enable -ftime-trace to generate time tracing .json files on clang" OFF)
  if(ENABLE_BUILD_WITH_TIME_TRACE)
    add_compile_definitions(project_options INTERFACE -ftime-trace)
  endif()
endif()

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# enable cache system
include(cmake/Cache.cmake)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

# enable doxygen
include(cmake/Doxygen.cmake)
enable_doxygen()

# allow for static analysis options
include(cmake/StaticAnalyzers.cmake)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" OFF)
option(ENABLE_FUZZING "Enable Fuzzing Builds" OFF)

# Very basic PCH example
option(ENABLE_PCH "Enable Precompiled Headers" OFF)
if(ENABLE_PCH)
  # This sets a global PCH parameter, each project will build its own PCH, which is a good idea if any #define's change
  #
  # consider breaking this out per project as necessary
  target_precompile_headers(
    project_options
    INTERFACE
    <iostream>
  )
endif()

# Set up some extra Conan dependencies based on our needs before loading Conan
set(CONAN_EXTRA_REQUIRES "")
set(CONAN_EXTRA_OPTIONS "")

include(cmake/Conan.cmake)
run_conan()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
find_package(cppcoro REQUIRED)
include(geturing)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src)

if(ENABLE_TESTING)
  enable_testing()
  message("Building Tests. Be sure to check out test/constexpr_tests for constexpr testing")
  add_subdirectory(test)
endif()

option(ENABLE_UNITY "Enable Unity builds of projects" OFF)
if(ENABLE_UNITY)
  # Add for any project you want to apply unity builds for
  set_target_properties(intro PROPERTIES UNITY_BUILD ON)
endif()
