//
// Created by alchaussee on 4/8/15.
//

#ifndef CS3251_HW3_RXP_H
#define CS3251_HW3_RXP_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <exception>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class RxPSocket
{
public:
    RxPSocket(RxPSocket &&sock);

    static RxPSocket connect(std::string ip_address, int foreign_port, int local_port = -1);

    static RxPSocket listen(int local_port);

    int recv(char* buffer, int buffer_length);

    int send(char* buffer, int buffer_length, int timeout = 0);

    void close();

    void setWindowSize(int size);

    int getWindowSize();

    void setVerbose(bool val);

private:
    RxPSocket();

    void init();

    std::vector<char> receiveFrom(struct sockaddr_in &senderInfo, socklen_t &senderLength);

    void sendTo(const char *buffer, int length, const struct sockaddr_in &receiver, const socklen_t &receiverLength);

    void out_process();

    void in_process();

    void debug_msg(std::string msg);

    int _handle;

    bool _connected;

    int _destination_seq_num;

    int _seq_num;

    int _window_size;

    bool _ack_received = false;

    struct sockaddr_in _destination_info;

    int _local_port;

    int DATASIZE = 10;

    bool _verbose;

    std::vector<char> _in_buffer;
    std::vector<char> _out_buffer;
    int _out_buffer_start_seq;

    std::thread _in_thread;
    std::thread _out_thread;
    std::mutex _in_mutex;
    std::mutex _out_mutex;
};


#endif //CS3251_HW3_RXP_H
