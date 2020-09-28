set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/)

find_package(cppcoro REQUIRED)

add_executable(app main.cpp)
target_link_libraries(app
        PRIVATE
        project_options
        project_warnings
        header_only_libs
        cppcoro
)
