cmake_minimum_required(VERSION 3.15.0)
project(BuildingHabits VERSION 0.4.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(JSON_ImplicitConversions OFF)

include(CTest)
enable_testing()

add_executable(BuildingHabits main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_VERBATIM_VARIABLES YES)
set(CPACK_COMPONENTS_ALL binary)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
include(CPack)

find_package(fmt REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(expresscpp REQUIRED)
find_package(CURL REQUIRED)

add_subdirectory(habits)

target_link_libraries(BuildingHabits PRIVATE habits nlohmann_json::nlohmann_json expresscpp::expresscpp curl fmt)

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
FetchContent_MakeAvailable(googletest)

install(TARGETS BuildingHabits DESTINATION bin COMPONENT binary)
set_target_properties(BuildingHabits PROPERTIES LINK_FLAGS_RELEASE -s)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/static/"
  DESTINATION static
  COMPONENT binary
  PATTERN ".gitignore" EXCLUDE
)
cpack_add_component(binary)
