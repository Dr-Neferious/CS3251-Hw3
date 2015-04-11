//
// Created by alchaussee on 4/8/15.
//

#ifndef CS3251_HW3_RXP_H
#define CS3251_HW3_RXP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <exception>
#include <string>
#include <vector>

class RxPSocket
{
public:
    static RxPSocket connect(int foreign_port, int local_port = -1);

    static RxPSocket listen(int local_port);

    int recv(void* buffer, int buffer_length);

    int send(void* buffer, int buffer_length, int timeout = 0);

    void close();

private:
    RxPSocket();

    std::string receiveFrom(struct sockaddr_in &senderInfo, socklen_t &senderLength);

    void sendTo(const char *buffer, int length, const struct sockaddr_in &receiver, const socklen_t &receiverLength);

    int _handle;

    struct sockaddr_in _destination_info;

    std::vector<char> _in_buffer;
    std::vector<char> _out_buffer;
};


#endif //CS3251_HW3_RXP_H
