cmake_minimum_required(VERSION 3.15.0)
project(BuildingHabits VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)
enable_testing()

add_executable(BuildingHabits main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(expresscpp REQUIRED)
find_package(curl REQUIRED)

add_subdirectory(src)

target_link_libraries(BuildingHabits PRIVATE position nlohmann_json::nlohmann_json expresscpp::expresscpp curl ws2_32 wsock32 fmt)

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
target_include_directories(BuildingHabits PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/src"
                          )

add_subdirectory(js)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
