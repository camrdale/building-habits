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
find_package(CURL REQUIRED)

add_subdirectory(habits)

target_link_libraries(BuildingHabits PRIVATE habits nlohmann_json::nlohmann_json expresscpp::expresscpp curl ws2_32 wsock32 fmt)

target_include_directories(BuildingHabits PUBLIC
                          "${PROJECT_BINARY_DIR}"
                          "${PROJECT_SOURCE_DIR}/habits"
                          )

add_subdirectory(js)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        f8d7d77c06936315286eb55f8de22cd23c188571 # release-1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
