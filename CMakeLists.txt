cmake_minimum_required(VERSION 3.14)
project(backtester CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra)

# --- Core library -----------------------------------------------------------
add_library(backtest
  src/Portfolio.cpp
  src/Metrics.cpp
  src/CsvLoader.cpp
  src/Backtester.cpp)
target_include_directories(backtest PUBLIC include)

# --- Demo executable --------------------------------------------------------
add_executable(demo src/main.cpp)
target_link_libraries(demo PRIVATE backtest)

# --- Tests (GoogleTest, fetched automatically) ------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(backtest_tests tests/test_backtester.cpp)
target_link_libraries(backtest_tests PRIVATE backtest GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(backtest_tests)
