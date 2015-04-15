cmake_minimum_required(VERSION 2.8)
project(CS3251_Hw3)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

set(Boost_USE_STATIC_LIBS        ON) # only find static libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

find_package(Boost 1.54 COMPONENTS system thread REQUIRED)
find_package(Threads REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(FxA_Server FxA_Server.cpp RxPException.cpp RxPSocket.cpp RxPMessage.cpp)
target_link_libraries(FxA_Server ${Boost_LIBRARIES})

add_executable(FxA_Client FxA_Client.cpp RxPException.cpp RxPSocket.cpp RxPMessage.cpp)
target_link_libraries(FxA_Client ${Boost_LIBRARIES})

add_executable(FxA_ServerRxP FxA_ServerRxP.cpp RxPException.cpp RxPSocket.cpp RxPMessage.cpp)
target_link_libraries(FxA_ServerRxP ${Boost_LIBRARIES})

add_executable(FxA_ClientRxP FxA_ClientRxP.cpp RxPException.cpp RxPSocket.cpp RxPMessage.cpp)
target_link_libraries(FxA_ClientRxP ${Boost_LIBRARIES})

add_executable(server_test server_test.cpp RxPException.cpp RxPSocket.cpp RxPMessage.cpp)
target_link_libraries(server_test ${Boost_LIBRARIES})

add_executable(client_test client_test.cpp RxPException.cpp RxPSocket.cpp RxPMessage.cpp)
target_link_libraries(client_test ${Boost_LIBRARIES})