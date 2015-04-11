cmake_minimum_required(VERSION 2.8)
project(CS3251_Hw3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

<<<<<<< HEAD
set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.54 COMPONENTS system thread REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(FxA_Server FxA_Server.cpp RxPException.cpp RxPSocket.cpp)
add_executable(FxA_Client FxA_Client.cpp RxPException.cpp RxPSocket.cpp)
target_link_libraries(FxA_Server ${Boost_LIBRARIES})
target_link_libraries(FxA_Client ${Boost_LIBRARIES})

