# automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

list(APPEND libraries CONAN_PKG::catch2 project_options project_warnings header_only_libs flow::flow cppcoro liburing::liburing)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC ${libraries})

macro (make_test test_name libraries)
  add_executable(${test_name} ${test_name}.cpp)
  target_link_libraries(${test_name} PRIVATE catch_main)
endmacro()

macro(add_catch_test test_name)
    make_test(${test_name} ${libraries})

  # automatically discover test_behaviors that are defined in catch based test files you can modify the unittests. TEST_PREFIX to
  # whatever you want, or use different for different binaries
  catch_discover_tests(
          ${test_name}
          TEST_PREFIX
          "unittests."
          EXTRA_ARGS
          -s
          --reporter=xml
          --out=tests.xml)
endmacro()

macro(add_constexpr_catch_test test_name)
  make_test(${test_name} ${libraries})
  catch_discover_tests(
          ${test_name}
          TEST_PREFIX
          "constexpr."
          EXTRA_ARGS
          -s
          --reporter=xml
          --out=constexpr.xml)
endmacro()

macro(add_relaxed_constexpr_catch_test test_name)
  set(relaxed_name "relaxed_${test_name}")
  add_executable(${relaxed_name} ${test_name}.cpp)
  target_link_libraries(${relaxed_name} PRIVATE catch_main)
  target_compile_definitions(${relaxed_name} PRIVATE -DCATCH_CONFIG_RUNTIME_STATIC_REQUIRE)
  catch_discover_tests(
          ${relaxed_name}
          TEST_PREFIX
          "relaxed_constexpr."
          EXTRA_ARGS
          -s
          --reporter=xml
          --out=relaxed_constexpr.xml)
endmacro()


add_catch_test(test_mixed_array)
add_catch_test(test_array)
add_catch_test(test_any_type_set)
add_catch_test(test_string)
add_catch_test(test_double_buffer)
add_catch_test(test_cancellation)
add_catch_test(test_channel_set)

add_constexpr_catch_test(constexpr_tests)
add_constexpr_catch_test(test_metaprogramming)

add_relaxed_constexpr_catch_test(constexpr_tests)
add_relaxed_constexpr_catch_test(test_metaprogramming)

