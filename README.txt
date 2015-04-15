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
		RxP protocol implemented here. Creates sockets and allows you to call things like recv and send on them

	RxPSocket.h
		RxP protocol implemented here. Creates sockets and allows you to call things like recv and send on them

	RxPException.cpp
		Provides exceptions for RxP sockets to throw

	RxPException.h
		Provides exceptions for RxP sockets to throw

	RxPMessage.cpp
		Outlines the fields of an RxP protocol message

	RxPMessage.h
		Outlines the fields of an RxP protocol message

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
	RxP is a simple, non-pipelined, ARQ, byte-stream transport protocol for providing connection-oriented, reliable inter-process communication.

Connections are established through a three-stage synchronization handshake that allows end hosts to coordinate sequence numbers and port numbers for this connection.

After the synchronization handshake is complete, implementation-specific buffer resources are allocated for both inbound and outbound messages on both endpoint hosts. Both processes are now free to send and receive data to and from the connection as needed. The inbound and outbound buffers at each endpoint allow for bidirectional data transfer limited by the allocated buffer sizes. A sliding window protocol will prevent receiving data that is unable to be copied into the inbound or outbound buffer due to the buffer being full.

Packets transmitted from one host to the other must be answered with an ACK, or acknowledgement to confirm successful delivery, free of loss or corruption. To facilitate this, RxP features sequence numbers and a timeout timer. When an RxP endpoint sends a packet to another RxP endpoint, the timer is started. If the sending endpoint does not receive an ACK packet before the timeout duration elapses, the packet will be resent.

Duplicate or out-of-order packets can be identified using RxP’s sequence number. The current sequence number increments with each packet transmitted. Upon arrival if the sequence numbers of the packets are not in order than all packets after and including the out-of-order one will be dropped. If a packet arrives that has the same or a lower sequence number than the expected sequence number, the packet is either a duplicate or out of order. The offending packet is dropped and once the senders ACK timeout expires will the sender will resend the packet that was dropped.

A checksum in the RxP header allows for detection of packet corruption. When a packet is received a checksum will be calculated and compared to the one in the header. If the two checksums are not the same than the packet is deemed corrupted, the offending packet is dropped, and once the senders ACK timeout expires will the sender will resend the packet that was dropped.

Window-based flow control will be handled using the window size field in the header. The window size will be determined by the amount of free space in the incoming buffer and inform the receiver how much data can be sent.

When data transmission is complete and the connection no longer needed, the connection can be closed.


-UPDATED API DESCRIPTION-
Listen
	Format: LISTEN(local port number) -> connection handle

	LISTEN causes the RxP layer to begin listening connection requests from clients on the given port number. Upon receiving a connection request, the synchronization handshake is executed, inbound and outbound buffers are allocated, and a connection handle is returned to the application for connection identification.

Connect
	Format: CONNECT(ip address, foreign port number [, local port number]) -> connection handle

	CONNECT sends a connection request to a server. If the server accepts the request and completes the synchronization handshake, inbound and outbound buffers are allocated, and a connection handle is returned to the application for connection identification.

	The application may, optionally, specify the port to be used on the local host for this connection. If none is specified, the operating system will be asked to pick any available port.

Recv
	Format: RECV(buffer, buffer length [, timeout]) -> received byte count

	RECV copies in an amount of data from the incoming buffer to the given application buffer. The number of bytes copied will be the minimum of the bytes available in the inbound buffer and the given application buffer length.

	An optional timeout value may be specified, if the desired timeout duration for this application is different than the implementation’s default.

	This command must be preceded by either a LISTEN or CONNECT, as appropriate. Without the execution of one of these commands, the application will not be able to provide a valid connection handle, and an error will be returned.

Send
	Format: SEND(buffer, buffer length) -> sent byte count

	SEND copies an amount of data from the given application buffer to the outgoing buffer. Once in the outbound buffer, the data is queued for transmission, and the command will immediately return control to the application. This command returns the number of bytes successfully copied to the outbound buffer, which may be less than buffer length.

	This command must be preceded by either a LISTEN or CONNECT, as appropriate. Without the execution of one of these commands, the application will not be able to provide a valid connection handle, and an error will be returned.

Close
Format: CLOSE() -> void

	CLOSE marks the connection closed. This means that the application will no longer be able to add to the outbound buffer. The RxP service will continue attempting to transmit all data in the outbound buffer before terminating the connection. Similarly, the application is permitted to continue receiving data while the connection is waiting to terminate. Once the connection is terminated, RECV commands will return an error.

	This command must be preceded by either a LISTEN or CONNECT, as appropriate. Without the execution of one of these commands, the application will not be able to provide a valid connection handle, and an error will be returned.




-BUGS and LIMITATIONS-
Everything






