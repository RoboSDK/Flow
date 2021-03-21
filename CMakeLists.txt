cmake_minimum_required(VERSION 3.15)

project(Flow LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

include(cmake/SetUp.cmake)

option(ENABLE_PCH "Enable Precompiled Headers" ON)
option(ENABLE_EXAMPLES "Enable Examples" ON)
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)

include(cmake/Conan.cmake)
run_conan()

find_package(cppcoro REQUIRED)
find_package(Threads REQUIRED)
find_package(liburing REQUIRED)
add_subdirectory(src)

if (ENABLE_PCH)
    list(APPEND public_headers
            chain
            concepts
            configuration
            flow
            network
            network_handle
            operator_pipe
            publisher
            spin
            spinner
            subscriber
            transformer)

    foreach (header ${public_headers})
        list(APPEND headers "include/flow/${header}.hpp")
    endforeach ()

    list(APPEND detail_headers
            cancellable_function
            cancellation_handle
            channel_resource
            channel_set
            forward
            hash
            metaprogramming
            multi_channel
            publisher_token
            routine
            single_channel
            spin_routine
            spin_wait
            subscriber_token
            timeout_routine)

    foreach (header ${detail_headers})
        list(APPEND headers "include/flow/detail/${header}.hpp")
    endforeach ()

    target_precompile_headers(
            flow
            PUBLIC
            ${headers}
            PRIVATE
            <vector>
            <unordered_map>
    )
endif ()

if (ENABLE_EXAMPLES)
    add_subdirectory(examples)
endif ()

if (ENABLE_TESTING)
    enable_testing()
    add_subdirectory(test)
endif ()

option(ENABLE_UNITY "Enable Unity builds of projects" OFF)
if (ENABLE_UNITY)
    # Add for any project you want to apply unity builds for
    set_target_properties(intro PROPERTIES UNITY_BUILD ON)
endif ()

include(cmake/ExportLibrary.cmake)

