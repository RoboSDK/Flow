# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDE_DIR include/flow CACHE PATH "Installation directory for header files")
set(INSTALL_CMAKE_DIR lib/cmake/flow CACHE PATH "Installation directory for CMake files")

# Make relative paths absolute (needed later on)
foreach(directory LIB BIN INCLUDE CMAKE)
    set(var INSTALL_${directory}_DIR)
    if(NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
    endif()
endforeach()

configure_file(cmake/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

include(cmake/StandardProjectSettings.cmake)
include(cmake/PreventInSourceBuilds.cmake)

add_subdirectory(external)

set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
target_compile_features(project_options INTERFACE cxx_std_20)
target_compile_options(project_options INTERFACE -fcoroutines -gdwarf-3)

include(cmake/VerifyCompilerSupportsCoroutines.cmake)

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

