cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_COMPILER "/usr/bin/g++" CACHE STRING "C++ compiler" FORCE)

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -Ofast")

set(CMAKE_CXX_FLAGS "-fcoroutines ${CMAKE_CXX_FLAGS}")
set(THREADS_PREFER_PTHREAD_FLAG ON)

project(Binance-Trading-Platform)

find_package(Threads REQUIRED)
FIND_PACKAGE(Boost 1.72 REQUIRED system)
FIND_PACKAGE(OpenSSL REQUIRED)

INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIRS} Connection/public)

add_library(
  Connection
  SHARED
    Connection/public/Connection.hpp
    Connection/private/Connection.cpp
)

add_library(
  OrderBook
  SHARED
    OrderBook/public/OrderBook.hpp
    OrderBook/private/OrderBook.cpp
)

target_include_directories(
  Connection
  PUBLIC
    ${PROJECT_SOURCE_DIR}/Connection/public
)

target_include_directories(
  OrderBook
  PUBLIC
    ${PROJECT_SOURCE_DIR}/OrderBook/public
)

add_executable(test_app
  ${BOOST_BEAST_FILES}
  TestApp/main.cpp
)

target_link_libraries(test_app Connection OrderBook ${Boost_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto pthread boost_thread)