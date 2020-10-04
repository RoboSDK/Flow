list(APPEND source_files flow.cpp)

if (BUILD_SHARED_LIBS)
    add_library(flow SHARED ${source_files})
else()
    add_library(flow STATIC ${source_files})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)
find_package(cppcoro REQUIRED)
include(geturing)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(flow::flow ALIAS flow)

target_include_directories(flow PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(flow
        PUBLIC
        Threads::Threads
        liburing::liburing
        cppcoro

        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::logging

        CONAN_PKG::docopt.cpp)
