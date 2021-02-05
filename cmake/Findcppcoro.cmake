find_library(CPPCORO_LIBRARIES NAMES cppcoro REQUIRED)
find_path(CPPCORO_INCLUDE_DIRS NAMES cppcoro REQUIRED)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cppcoro DEFAULT_MSG CPPCORO_LIBRARIES CPPCORO_INCLUDE_DIRS)

mark_as_advanced(CPPCORO_LIBRARIES CPPCORO_INCLUDE_DIRS)
add_library(cppcoro::cppcoro INTERFACE IMPORTED GLOBAL)
target_link_libraries(cppcoro::cppcoro INTERFACE ${CPPCORO_LIBRARIES}) 
target_include_directories(cppcoro::cppcoro INTERFACE ${CPPCORO_INCLUDE_DIRS})
