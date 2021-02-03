# Add all targets to the build-tree export set
export(TARGETS flow project_options project_warnings header_only_libs flatbuffers FILE "${PROJECT_BINARY_DIR}/flowTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE flow)

# Create the flowConfig.cmake and flowConfigVersion.cmake files
file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}" "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(cmake/flowConfig.cmake.in "${PROJECT_BINARY_DIR}/flowConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${FLOW_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(cmake/flowConfig.cmake.in "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/flowConfig.cmake" @ONLY)
# ... for both
configure_file(cmake/flowConfigVersion.cmake.in "${PROJECT_BINARY_DIR}/flowConfigVersion.cmake" @ONLY)

# Install the flowConfig.cmake and flowConfigVersion.cmake
install(FILES
        "${PROJECT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/flowConfig.cmake"
        "${PROJECT_BINARY_DIR}/flowConfigVersion.cmake"
        DESTINATION
        "${INSTALL_CMAKE_DIR}" COMPONENT dev)

install(DIRECTORY include/ DESTINATION ${INSTALL_INCLUDE_DIR})

# Install the export set for use with the install-tree
install(EXPORT flowTargets
        DESTINATION "${INSTALL_CMAKE_DIR}"
        COMPONENT dev)

