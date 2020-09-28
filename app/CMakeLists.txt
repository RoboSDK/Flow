set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(cppcoro REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app
        PRIVATE
        project_options
        project_warnings
        header_only_libs

        flow::flow

        Threads::Threads
        cppcoro

        CONAN_PKG::fmt
)
