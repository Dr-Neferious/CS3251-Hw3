//
// Created by alchaussee on 4/8/15.
//

#include <cstring>
#include <unistd.h>

#include "RxPException.h"

#include "RxPSocket.h"

#include <iostream>
#include <arpa/inet.h>

#include "RxPMessage.h"

using namespace std;
using namespace std::chrono;

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
  _window_size = 1;
}

RxPSocket RxPSocket::listen(int local_port) {
  RxPSocket sock;

  // bind to local port
  cout << "bound to " << local_port << endl;
  sock._local_port = local_port;
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
  cout << "waiting for syn" << endl;
  do {
    try {
      message.parseFromBuffer(sock.receiveFrom(senderInfo, addrlen));
    } catch(const RxPMessage::ParseException& e) {
      continue;
    }
  } while(!message.SYN_flag);

  // save sender info and initiate synchronization handshake
  sock._destination_info = senderInfo;
  cout << "got syn sending syn ack and waiting for ack" << endl;
  do {
    RxPMessage sendmessage;
    sendmessage.SYN_flag = true;
    sendmessage.ACK_flag = true;
    sendmessage.ACK_number = message.sequence_number + 1;
    sendmessage.sequence_number = sock._seq_num;
    sendmessage.fillChecksum();
    vector<char> buffer = sendmessage.toBuffer();
    sock.sendTo(buffer.data(), buffer.size(), sock._destination_info, sizeof(sock._destination_info));
    try {
      message.parseFromBuffer(sock.receiveFrom(senderInfo, addrlen));
    } catch (const RxPMessage::ParseException &e) {
      cout << e.what() << endl;
      continue;
    }
  } while(!(message.ACK_flag && message.ACK_number == sock._seq_num));

  // initialize buffers / resources
  sock.init();

  return sock;
}

RxPSocket RxPSocket::connect(string ip_address, int foreign_port, int local_port) {
  RxPSocket sock;

  // bind to local port, if set
  cout << "bound to " << local_port << endl;
  cout << "Sending to " << foreign_port << endl;
  if(local_port != -1)
  {
    sock._local_port = local_port;
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(local_port);
    if(bind(sock._handle, (struct sockaddr *)&address, sizeof(address)) < 0)
      throw RxPException(errno);
  } else {
    struct sockaddr_in info;
    socklen_t len = sizeof(info);
    if(getsockname(sock._handle, (struct sockaddr *)&info, &len) < 0)
      throw RxPException(errno);
    sock._local_port = ntohs(info.sin_port);
  }

  // send connection request to specified server
  sock._destination_info.sin_family = AF_INET;
  sock._destination_info.sin_port = htons(foreign_port);
  if(inet_pton(AF_INET, ip_address.c_str(), &(sock._destination_info.sin_addr)) <= 0)
    throw RxPException(errno);
  RxPMessage response_message;
  struct sockaddr_in senderInfo;
  socklen_t addrlen = sizeof(senderInfo);
  cout << "Sending syn" << endl;
  do {
    RxPMessage send_message;
    send_message.SYN_flag = true;
    send_message.sequence_number = sock._seq_num;
    send_message.fillChecksum();
    vector<char> buffer = send_message.toBuffer();
    sock.sendTo(buffer.data(), buffer.size(), sock._destination_info, sizeof(sock._destination_info));
    cout << "Waiting for syn ack" << endl;
    try {
      response_message.parseFromBuffer(sock.receiveFrom(senderInfo, addrlen));
    } catch(const RxPMessage::ParseException &e) {
      cout << "Received invalid message. Resending." << endl;
      continue;
    }
  } while(!(response_message.ACK_flag && response_message.SYN_flag));

  // complete synchronization handshake
  RxPMessage ack_message;
  ack_message.ACK_flag = true;
  ack_message.ACK_number = response_message.sequence_number + 1;
  ack_message.sequence_number = sock._seq_num;
  ack_message.fillChecksum();
  vector<char> buffer = ack_message.toBuffer();
  sock.sendTo(buffer.data(), buffer.size(), sock._destination_info, sizeof(sock._destination_info));

  // initialize buffers / resources
  sock.init();

  return sock;
}

int RxPSocket::recv(char *buffer, int buffer_length) {
  lock_guard<mutex> lock(_in_mutex);
  int numBytesToCopy = min(buffer_length, (int)_in_buffer.size());
  copy(_in_buffer.begin(), _in_buffer.begin() + numBytesToCopy, buffer);
  return numBytesToCopy;
}

int RxPSocket::send(char *buffer, int buffer_length, int timeout) {
  lock_guard<mutex> lock(_out_mutex);
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

vector<char> RxPSocket::receiveFromNonBlocking(struct sockaddr_in &senderInfo, socklen_t &senderLength) {
  char buffer[111];
  senderLength = sizeof(senderInfo);
  //TODO Check errors set by recvfrom using the MSG_DONTWAIT flag http://linux.die.net/man/2/recvfrom
  auto bytesrecvd = recvfrom(_handle, buffer, 110, MSG_DONTWAIT, (struct sockaddr *)&senderInfo, &senderLength);
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

void RxPSocket::setWindowSize(int size)
{
  _window_size = size;
}

int RxPSocket::getWindowSize()
{
  return _window_size;
}

/*
 * If there is space left in the in_buffer, use recvFrom to fill that space
 * Managing window size stuff
 */
void RxPSocket::in_process()
{
  int prev_seq_num = 0;
  while(_connected)
  {
    if(_in_buffer.capacity()-_in_buffer.size() > DATASIZE)
    {
      RxPMessage msg;
      struct sockaddr_in senderInfo;
      socklen_t addrlen = sizeof(senderInfo);
      msg.parseFromBuffer(receiveFromNonBlocking(senderInfo, addrlen));

      //Out of order packet
      if(prev_seq_num > msg.sequence_number)
      {
        _in_buffer.clear();
        continue;
      }

      //Duplicate packet
      if(prev_seq_num == msg.sequence_number)
        continue;

      //TODO Handle message flags
      if(msg.ACK_flag)
      {
        if(msg.ACK_number == _seq_num)
        {
          _ack_received = true;
          //TODO Remove corresponding data from _out_buffer
        }
        else if(msg.ACK_number < _seq_num)
        {

        }
        else
        {

        }
        prev_seq_num = msg.sequence_number;
        setWindowSize(_in_buffer.capacity()-_in_buffer.size()%DATASIZE);
        //Message wont contain any data
        continue;
      }
      else if(msg.RST_flag)
      {
        //TODO Reset the connection or maybe just not use this flag
      }
      else
        copy(msg.data.begin(), msg.data.end(), _in_buffer.begin()+_in_buffer.size());

      prev_seq_num = msg.sequence_number;
      setWindowSize(_in_buffer.capacity()-_in_buffer.size()%DATASIZE);
    }
  }
}

void RxPSocket::out_process()
{
  while(_connected)
  {
    int init_seq_num = _seq_num;
    { // Scope for mutex lock
      lock_guard<mutex> lock(_out_mutex);
      if(_out_buffer.empty())
        continue;
      //Send multiple messages
      auto iter = _out_buffer.begin();
      for (int i = 0; i < _window_size && _seq_num < (_out_buffer_start_seq + _out_buffer.size()); i++) {
        RxPMessage msg;
        msg.dest_port = _destination_info.sin_port;
        msg.src_port = _local_port;
        msg.sequence_number = _seq_num;

        auto numBytesToSend = min(_seq_num - _out_buffer_start_seq, DATASIZE);
        //TODO Wont this just keep sending the same data
        msg.data = vector<char>(_out_buffer.begin(), _out_buffer.begin() + numBytesToSend);

        _seq_num += numBytesToSend;

        msg.fillChecksum();

        vector<char> buffer = msg.toBuffer();
        sendTo(buffer.data(), buffer.size(), _destination_info, sizeof(_destination_info));
      }
    }
    auto start_time = system_clock::now();
    auto timeout_duration = duration<int>(10); // 10 seconds
    while(!_ack_received)
    {
      this_thread::__sleep_for(seconds(1), nanoseconds(0));
      if( (system_clock::now() - start_time) > timeout_duration )
        break;
    }
    if(!_ack_received)
    {
      // Ack never received so must've timed out.
      _seq_num = init_seq_num;
      continue;
    }
  }
}