add_library(position position.hpp position.cpp)
add_library(moves moves.hpp moves.cpp)

include(GoogleTest)

enable_testing()
add_executable(position_test position_test.cpp)
target_link_libraries(position_test GTest::gtest_main gmock position)
add_executable(moves_test moves_test.cpp)
target_link_libraries(moves_test GTest::gtest_main gmock position moves)
 
gtest_discover_tests(position_test)
gtest_discover_tests(moves_test)
