cmake_minimum_required(VERSION 3.14)

project(REBL)

add_library(REBL
  src/REBL.cpp
  src/Core.cpp
)
target_compile_features(REBL PUBLIC cxx_std_20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  graaflib
  GIT_REPOSITORY https://github.com/bobluppes/graaf.git
  GIT_TAG        v1.1.1  # Adjust to version you'd like
)
FetchContent_MakeAvailable(graaflib)
target_include_directories(REBL PUBLIC
    ${graaflib_SOURCE_DIR}/include/
)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0  # Adjust to version you'd like
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(RunTests_REBL tests/REBL_test.cpp)
target_link_libraries(RunTests_REBL REBL GTest::gtest_main)
target_compile_features(RunTests_REBL PUBLIC cxx_std_20)

include(GoogleTest)
enable_testing()
gtest_discover_tests(RunTests_REBL)
