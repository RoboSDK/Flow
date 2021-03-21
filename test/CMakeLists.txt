# automatically enable catch2 to generate ctest targets
if(CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif()

list(APPEND libraries
        CONAN_PKG::catch2
        flow::flow

        CONAN_PKG::spdlog)

add_library(catch_main STATIC catch_main.cpp)
target_link_libraries(catch_main PUBLIC ${libraries})

macro (make_test test_name libraries)
  add_executable(${test_name} ${test_name}.cpp)
  target_link_libraries(${test_name} PRIVATE catch_main)
endmacro()

macro(add_catch_test test_name)
    make_test(${test_name} ${libraries})

  # automatically discover unit_tests that are defined in catch based test files you can modify the unittests. TEST_PREFIX to
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

add_catch_test(test_mixed_array)
add_catch_test(test_cancellation)

add_constexpr_catch_test(test_metaprogramming)
add_constexpr_catch_test(test_concepts)

if (ENABLE_EXAMPLES)
    add_test(NAME minimal_publisher_subscriber COMMAND minimal_publisher_subscriber)
    add_test(NAME publisher_transformer_subscriber COMMAND publisher_transformer_subscriber)
    add_test(NAME simple_publisher_subscriber COMMAND simple_publisher_subscriber)
    add_test(NAME simple_user_publisher_subscriber COMMAND simple_user_publisher_subscriber)
    #add_test(NAME two_chains COMMAND two_chains)
endif()

