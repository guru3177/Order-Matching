cmake_minimum_required(VERSION 3.10.2)
set (CMAKE_CXX_STANDARD 14)
project(BlockOne)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.65.1 COMPONENTS thread unit_test_framework)


if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    add_library(BlockOne Order.cpp OrderBook.cpp)
    target_link_libraries(BlockOne ${Boost_LIBRARIES})
    add_executable(order_book BlockOne.cpp)
    add_executable(order_book_test OrderBookTest.cpp)
    target_link_libraries(order_book BlockOne)
    target_link_libraries(order_book_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} BlockOne)
endif()
