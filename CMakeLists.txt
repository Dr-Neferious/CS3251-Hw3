cmake_minimum_required(VERSION 2.8)
project(CS3251_Hw3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_executable(FxA_Server FxA_Server.cpp RxPException.cpp RxPSocket.cpp)
add_executable(FxA_Client FxA_Client.cpp RxPException.cpp RxPSocket.cpp)
