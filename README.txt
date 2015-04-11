Programming Assignment 2 - Code
CS3251-A
Matthew Barulic <mbarulic@gatech.edu>
Richard Chaussee <rchaussee3@gatech.edu>
4/12/2015

-FILES-
	FxA_Server.cpp
		Simple file transfer application server implemented using our RxP protocol

	FxA_Client.cpp
		Simple file transfer application client implemented using our RxP protocol

	RxPSocket.cpp
		

	RxPSocket.h
		

	RxPException.cpp
		

	RxPException.h
		

	CMakeLists.txt
		CMake file used to build the file transfer application

	README.txt
		This file

	Sample.txt
		Provides sample output of the file transfer application


-INSTRUCTIONS-
	Written and tested in Ubuntu 14.04
	Requires CMake, C++11, gcc, libboost-threads, and libboost-system.

	1) In a terminal navigate to the folder containing the code
		"cd CS3251-Hw3"
	2) Create a directory called "build"
		"mkdir build"
	3) Navigate into the new build folder
		"cd build"
	4) Run the below command
		"cmake .."
	5) Run the below command to complie the code and create the executables
		"make"
	6) You should now see two executable files in the build folder
		FxA_Server
		FxA_Client


-UPDATED PROTOCOL-




-UPDATED API DESCRIPTION-




-BUGS and LIMITATIONS-
	BUGS
		Occasionally when posting a file the server will crash due to throwing an instance of 'std::invalid_argument'
	
	LIMITATIONS
		Not very reliable when transfering large-ish files (>a few mb)






