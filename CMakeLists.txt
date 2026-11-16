cmake_minimum_required(VERSION 3.21)
project(PolynomialLab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/utf-8 /W4)
endif()

# Static library
add_library(PolynomialLib STATIC
  src/Polynomial.cpp
)
target_include_directories(PolynomialLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Demo application
add_executable(PolynomialDemo demo/main.cpp)
target_link_libraries(PolynomialDemo PRIVATE PolynomialLib)

# Misuse of exceptions (int / std::string)
add_executable(AntipatternDemo antipattern/main.cpp)

# Unit tests
find_package(GTest CONFIG REQUIRED)
enable_testing()

add_executable(PolynomialTests tests/PolynomialTests.cpp)
target_link_libraries(PolynomialTests PRIVATE PolynomialLib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(PolynomialTests)
