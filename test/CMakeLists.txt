FILE(GLOB test_files **.cc)
add_executable(redis_test ${test_files})
target_link_libraries(redis_test PRIVATE redis_core gmock gmock_main gtest gtest_main)
