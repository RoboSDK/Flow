list(APPEND source_files flow.cpp)

if (BUILD_SHARED_LIBS)
    add_library(flow SHARED ${source_files})
else ()
    add_library(flow STATIC ${source_files})
endif ()

add_library(flow::flow ALIAS flow)

target_link_libraries(flow
        PUBLIC
        Threads::Threads
        liburing::liburing
        cppcoro
        project_options

        PRIVATE
        project_warnings
        header_only_libs)

target_include_directories(flow
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/OdroidGPIO>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

install(TARGETS flow
        EXPORT flowTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}"
        COMPONENT dev)
