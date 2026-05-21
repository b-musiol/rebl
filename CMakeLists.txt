cmake_minimum_required(VERSION 3.14)

project(REBL)

add_library(REBL
  src/REBL.cpp
  src/Core.cpp
  src/ComponentData.cpp
)
target_compile_features(REBL PUBLIC cxx_std_20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  knokan
  GIT_REPOSITORY https://github.com/b-musiol/knokan.git
  GIT_TAG        main  # Adjust to version you'd like
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(knokan)
target_include_directories(REBL PUBLIC
    ${knokan_SOURCE_DIR}/include/
)

FetchContent_Declare(
  litesaver
  GIT_REPOSITORY https://github.com/b-musiol/litesaver.git
  GIT_TAG        main  # Adjust to version you'd like
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(litesaver)
target_include_directories(REBL PUBLIC
    ${litesaver_SOURCE_DIR}/include/
)
target_link_libraries(REBL PUBLIC litesaver)

FetchContent_Declare(
  Kochs
  GIT_REPOSITORY https://github.com/b-musiol/kochs.git
  GIT_TAG        main  # Adjust to version you'd like
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(Kochs)
target_include_directories(REBL PUBLIC
    ${kochs_SOURCE_DIR}/include/
)
target_link_libraries(REBL PUBLIC Kochs)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.17.0  # Adjust to version you'd like
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(RunTests_REBL tests/REBL_test.cpp)
target_link_libraries(RunTests_REBL REBL GTest::gtest_main)
target_compile_features(RunTests_REBL PUBLIC cxx_std_20)

include(GoogleTest)
enable_testing()
gtest_discover_tests(RunTests_REBL)
