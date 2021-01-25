find_path(LIBURING_INCLUDE_DIR liburing.h
        PATH_SUFFIXES liburing)

find_library(LIBURING_LIBRARY NAMES uring)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(liburing DEFAULT_MSG
        LIBURING_LIBRARY LIBURING_INCLUDE_DIR)

mark_as_advanced(LIBURING_LIBRARY LIBURING_INCLUDE_DIR)

add_library(liburing::liburing INTERFACE IMPORTED GLOBAL)
target_link_libraries(liburing::liburing INTERFACE ${LIBURING_LIBRARY})
target_include_directories(liburing::liburing INTERFACE ${LIBURING_INCLUDE_DIR})
if(NOT liburing_FOUND)
    include(ExternalProject)

    set(LIBURING_TMP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/liburing)
    externalproject_add(_fetch_uring
            GIT_REPOSITORY https://github.com/axboe/liburing.git
            GIT_TAG master
            CONFIGURE_COMMAND ./configure --prefix=${LIBURING_TMP_INSTALL_DIR}
            BUILD_COMMAND $(MAKE)
            INSTALL_COMMAND $(MAKE) install
            BUILD_IN_SOURCE ON
            )
    execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${LIBURING_TMP_INSTALL_DIR}/include)
    add_library(_liburing INTERFACE IMPORTED GLOBAL)
    target_link_libraries(_liburing INTERFACE ${LIBURING_TMP_INSTALL_DIR}/lib/liburing.a)
    target_include_directories(_liburing INTERFACE ${LIBURING_TMP_INSTALL_DIR}/include)
    add_dependencies(_liburing _fetch_uring)
    add_library(liburing::liburing ALIAS _liburing)

    install(FILES ${LIBURING_TMP_INSTALL_DIR}/lib/liburing.a DESTINATION lib)
    install(DIRECTORY ${LIBURING_TMP_INSTALL_DIR}/include/ DESTINATION include)
endif()
