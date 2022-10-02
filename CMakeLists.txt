cmake_minimum_required(VERSION 3.15)

project(Flow LANGUAGES CXX VERSION 0.0.1)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)


option(ENABLE_TESTING "Compile unit tests")
option(ENABLE_EXAMPLES "Compile exampels")
