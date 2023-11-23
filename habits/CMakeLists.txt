add_library(habits position.hpp position.cpp moves.hpp moves.cpp)

include(GoogleTest)

enable_testing()

add_executable(position_test position_test.cpp)
target_link_libraries(position_test habits GTest::gtest_main gmock)

add_executable(moves_test moves_test.cpp)
target_link_libraries(moves_test habits GTest::gtest_main gmock)
 
gtest_discover_tests(position_test)
gtest_discover_tests(moves_test)
