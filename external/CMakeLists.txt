macro(git_submodule_update submodule)
        # we have a submodule setup for submodule, assume it is under external/submodule
        # now we need to clone this submodule
        execute_process(COMMAND git submodule update --init -- ${submodule}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

git_submodule_update(flatbuffers)

add_library(header_only_libs INTERFACE)
target_include_directories(header_only_libs SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/external>
        $<INSTALL_INTERFACE:external>)

install(TARGETS header_only_libs
        EXPORT flowTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}"
        COMPONENT dev)


add_subdirectory(flatbuffers
        ${CMAKE_CURRENT_BINARY_DIR}/flatbuffers-build
        EXCLUDE_FROM_ALL)