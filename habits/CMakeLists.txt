add_library(habits
  position.hpp position.cpp
  moves.hpp moves.cpp
  search.hpp search.cpp
  http.hpp http.cpp
  bot.hpp bot.cpp
)

include(GoogleTest)

enable_testing()

add_executable(position_test position_test.cpp)
target_link_libraries(position_test habits GTest::gtest_main gmock)

add_executable(moves_test moves_test.cpp)
target_link_libraries(moves_test habits GTest::gtest_main gmock)

add_executable(search_test search_test.cpp)
target_link_libraries(search_test habits GTest::gtest_main gmock)
 
add_test(position_test position_test)
add_test(moves_test moves_test)
add_test(search_test search_test)
