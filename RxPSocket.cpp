//
// Created by alchaussee on 4/8/15.
//

#include <cstring>
#include <netinet/in.h>
#include <unistd.h>

#include "RxPException.h"

#include "RxPSocket.h"

using namespace std;

RxPSocket::RxPSocket(RxPSocket &&sock) {
  _out_buffer = move(sock._out_buffer);
  _in_buffer = move(sock._in_buffer);
  _in_thread = move(sock._in_thread);
  _out_thread = move(sock._out_thread);
  _handle = sock._handle;
  sock._handle = 0;
  _destination_info = sock._destination_info;
}

RxPSocket RxPSocket::listen(int local_port) {
  RxPSocket sock;

  // bind to local port
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(local_port);

  if(bind(sock._handle, (struct sockaddr *)&address, sizeof(address)) < 0)
    throw RxPException(errno);

  // use recvFrom to listen for connection requests

  // save sender info and initiate synchronization handshake

  // initialize buffers / resources
  sock.init();

  return sock;
}

RxPSocket RxPSocket::connect(int foreign_port, int local_port) {
  RxPSocket sock;

  // bind to local port, if set

  // send connection request to specified server

  // complete synchronization handshake

  // initialize buffers / resources
  sock.init();

  return sock;
}

int RxPSocket::recv(char *buffer, int buffer_length) {
  int numBytesToCopy = min(buffer_length, (int)_in_buffer.size());
  copy(_in_buffer.begin(), _in_buffer.begin() + numBytesToCopy, buffer);
  return numBytesToCopy;
}

int RxPSocket::send(char *buffer, int buffer_length, int timeout) {
  int numBytesToCopy = min(buffer_length, (int)(_out_buffer.capacity() - _out_buffer.size()));
  copy(buffer, buffer + numBytesToCopy, _in_buffer.begin());
  return numBytesToCopy;
}

void RxPSocket::close() {
  ::close(_handle);
}

RxPSocket::RxPSocket() {
  _handle = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(_handle < 0)
    throw RxPException(errno);
}

void RxPSocket::init() {
  _in_buffer.reserve(100);
  _out_buffer.reserve(100);

}

string RxPSocket::receiveFrom(struct sockaddr_in &senderInfo, socklen_t &senderLength) {
  char buffer[111];
  senderLength = sizeof(senderInfo);
  auto bytesrecvd = recvfrom(_handle, buffer, 110, 0, (struct sockaddr *)&senderInfo, &senderLength);
  if(bytesrecvd < 0)
    if(errno == 11)
      throw RxPTimeoutException();
    else
      throw RxPException(errno);
  buffer[bytesrecvd] = 0;
  string result(buffer);
  return result;
}

void RxPSocket::sendTo(const char *buffer, int length, const struct sockaddr_in &receiver,
                       const socklen_t &receiverLength) {
  if(sendto(_handle, buffer, length, 0, (struct sockaddr *)&receiver, receiverLength) < 0)
    throw RxPException(errno);
}