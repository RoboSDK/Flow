cmake_minimum_required(VERSION 3.15)

project(Flow CXX)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

add_library(project_options INTERFACE)
add_library(project_warnings INTERFACE)

include(cmake/SetUp.cmake)

option(ENABLE_PCH "Enable Precompiled Headers" OFF)
option(ENABLE_EXAMPLES "Enable Examples" ON)
option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)

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

include(cmake/Conan.cmake)
run_conan()

find_package(cppcoro REQUIRED)
find_package(Threads REQUIRED)
include(geturing)

add_subdirectory(src)
if(ENABLE_EXAMPLES)
  add_subdirectory(example)
endif()

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

include(cmake/ExportLibrary.cmake)

