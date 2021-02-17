list(APPEND source_files flow.cpp)

if (BUILD_SHARED_LIBS)
    add_library(flow SHARED ${source_files})
else ()
    add_library(flow STATIC ${source_files})
endif ()

add_library(flow::flow ALIAS flow)

target_link_libraries(flow
        PUBLIC
        project_options
        liburing::liburing

        cppcoro::cppcoro
        Threads::Threads

        project_warnings
        )

target_include_directories(flow
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include/flow>

        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/messages>
        $<INSTALL_INTERFACE:include/flow>
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        )

install(TARGETS flow project_options project_warnings
        EXPORT flowTargets
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT shlib
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}"
        COMPONENT dev)
