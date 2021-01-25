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

