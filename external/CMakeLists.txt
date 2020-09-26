macro(git_submodule_update submodule)
        # we have a submodule setup for submodule, assume it is under external/submodule
        # now we need to clone this submodule
        execute_process(COMMAND git submodule update --init -- ${submodule}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

git_submodule_update(frozen)
git_submodule_update(pipes)
git_submodule_update(sml)

add_library(header_only_libs INTERFACE)
target_include_directories(header_only_libs SYSTEM INTERFACE
        frozen/include
        pipes/include
        sml/include)

