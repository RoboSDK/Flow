add_subdirectory(unit_tests)
add_subdirectory(integration_tests)

if(ENABLE_FUZZING)
    message("Building Fuzz Tests, using fuzzing sanitizer https://www.llvm.org/docs/LibFuzzer.html")
    add_subdirectory(fuzz_tests)
endif()
