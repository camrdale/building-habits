add_library(position position.hpp position.cpp)

include(GoogleTest)

enable_testing()
add_executable(position_test position_test.cpp)
target_link_libraries(position_test GTest::gtest_main gmock position)
 
gtest_discover_tests(position_test)
