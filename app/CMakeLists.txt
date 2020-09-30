set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(cppcoro REQUIRED)
include(geturing)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(app main.cpp LidarTask.cpp LidarTask.hpp)
target_link_libraries(app
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::flow

        Threads::Threads
        liburing::liburing
        cppcoro

        CONAN_PKG::fmt
)
