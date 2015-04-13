//
// Created by alchaussee on 4/8/15.
//

#include <cstring>
#include <unistd.h>

#include "RxPException.h"

#include "RxPSocket.h"

#include <iostream>

#include "RxPMessage.h"

using namespace std;

RxPSocket::RxPSocket(RxPSocket &&sock) {
  _out_buffer = move(sock._out_buffer);
  _in_buffer = move(sock._in_buffer);
  _in_thread = move(sock._in_thread);
  _out_thread = move(sock._out_thread);
  _handle = sock._handle;
  sock._handle = 0;
  _destination_info = sock._destination_info;
  _connected = sock._connected;
  _destination_seq_num = sock._destination_seq_num;
  _seq_num = sock._seq_num;
}

RxPSocket RxPSocket::listen(int local_port) {
  RxPSocket sock;

  // bind to local port
  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(local_port);

  if (bind(sock._handle, (struct sockaddr *) &address, sizeof(address)) < 0)
    throw RxPException(errno);

  // use recvFrom to listen for connection requests
  RxPMessage message;
  struct sockaddr_in senderInfo;
  socklen_t addrlen = sizeof(senderInfo);
  do {
    message.parseFromBuffer(sock.receiveFrom(senderInfo, addrlen));
  } while(!message.SYN_flag);

  // save sender info and initiate synchronization handshake
  sock._destination_info = senderInfo;

  do {
    RxPMessage sendmessage;
    sendmessage.SYN_flag = true;
    sendmessage.ACK_flag = true;
    sendmessage.ACK_number = message.sequence_number + 1;
    sendmessage.sequence_number = sock._seq_num;
    vector<char> buffer = sendmessage.toBuffer();
    sock.sendTo(buffer.data(), buffer.size(), sock._destination_info, sizeof(sock._destination_info));
    //TODO wait for ack
    // NOTE no random backoff nonsense
  } while(!(message.ACK_flag && message.ACK_number == sock._seq_num));

  // initialize buffers / resources
  sock.init();

  return sock;
}

RxPSocket RxPSocket::connect(string ip_address, int foreign_port, int local_port) {
  RxPSocket sock;

  // bind to local port, if set

  // send connection request to specified server


  // complete synchronization handshake

  // initialize buffers / resources
  sock.init();

  return sock;
}

int RxPSocket::recv(char *buffer, int buffer_length) {
  // TODO mutex
  int numBytesToCopy = min(buffer_length, (int)_in_buffer.size());
  copy(_in_buffer.begin(), _in_buffer.begin() + numBytesToCopy, buffer);
  return numBytesToCopy;
}

int RxPSocket::send(char *buffer, int buffer_length, int timeout) {
  //TODO mutex
  int numBytesToCopy = min(buffer_length, (int)(_out_buffer.capacity() - _out_buffer.size()));
  copy(buffer, buffer + numBytesToCopy, _out_buffer.begin());
  return numBytesToCopy;
}

void RxPSocket::close() {
  ::close(_handle);
}

RxPSocket::RxPSocket() {
  _handle = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(_handle < 0)
    throw RxPException(errno);
  _connected = false;
  _destination_seq_num = 0;
  _seq_num = 0;
}

void RxPSocket::init() {
  _connected = true;
  _in_buffer.reserve(100);
  _out_buffer.reserve(100);
  _in_thread = thread(&RxPSocket::in_process, this);
  _out_thread = thread(&RxPSocket::out_process, this);
}

vector<char> RxPSocket::receiveFrom(struct sockaddr_in &senderInfo, socklen_t &senderLength) {
  char buffer[111];
  senderLength = sizeof(senderInfo);
  auto bytesrecvd = recvfrom(_handle, buffer, 110, 0, (struct sockaddr *)&senderInfo, &senderLength);
  if(bytesrecvd < 0)
    if(errno == 11)
      throw RxPTimeoutException();
    else
      throw RxPException(errno);
  buffer[bytesrecvd] = 0;
  vector<char> result(buffer, buffer + bytesrecvd);
  return result;
}

void RxPSocket::sendTo(const char *buffer, int length, const struct sockaddr_in &receiver,
                       const socklen_t &receiverLength) {
  if(sendto(_handle, buffer, length, 0, (struct sockaddr *)&receiver, receiverLength) < 0)
    throw RxPException(errno);
}

/*
 * If there is space left in the in_buffer, use recvFrom to fill that space
 * Managing window size stuff
 */
void RxPSocket::in_process() {
  while(_connected) {
    //TODO look into non-block recvFrom in order to prevent this from locking down the in_buffer indefinitely
  }
}

void RxPSocket::out_process() {
  while(_connected) {

  }
}