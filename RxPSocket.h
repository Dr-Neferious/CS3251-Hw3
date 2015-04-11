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
#include <thread>
#include <mutex>

class RxPSocket
{
public:
    RxPSocket(RxPSocket &&sock);

    static RxPSocket connect(int foreign_port, int local_port = -1);

    static RxPSocket listen(int local_port);

    int recv(char* buffer, int buffer_length);

    int send(char* buffer, int buffer_length, int timeout = 0);

    void close();

private:
    RxPSocket();

    void init();

    std::string receiveFrom(struct sockaddr_in &senderInfo, socklen_t &senderLength);

    void sendTo(const char *buffer, int length, const struct sockaddr_in &receiver, const socklen_t &receiverLength);

    int _handle;

    struct sockaddr_in _destination_info;

    std::vector<char> _in_buffer;
    std::vector<char> _out_buffer;

    std::thread _in_thread;
    std::thread _out_thread;
    std::mutex _in_mutex;
    std::mutex _out_mutex;
};


#endif //CS3251_HW3_RXP_H
