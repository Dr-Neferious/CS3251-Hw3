//
// Created by alchaussee on 4/8/15.
//

#ifndef CS3251_HW3_RXP_H
#define CS3251_HW3_RXP_H


class RxP
{
public:
    typedef int Socket;

    void listen(int port);
    Socket connect(int port, int local_port);
    int recv(Socket sock, void* buf, int buf_len);
    int send(Socket sock, void* buf, int buf_len, int timeout);
    void close(Socket sock);
};


#endif //CS3251_HW3_RXP_H
