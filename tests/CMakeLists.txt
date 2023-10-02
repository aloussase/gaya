add_executable(gaya_test_runner
    main.cpp)

target_link_libraries(gaya_test_runner
    gaya_lib
    fmt::fmt)

add_test(
    NAME gaya_tests
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMAND "${CMAKE_CURRENT_BINARY_DIR}/gaya_test_runner")

add_custom_command(
    TARGET gaya_test_runner
    POST_BUILD
    COMMAND ctest --output-on-failure)
