//
// Created by matt on 4/12/15.
//

#include "RxPMessage.h"

using namespace std;

RxPMessage::RxPMessage()
  : data(0)
{
}

RxPMessage::RxPMessage(const vector<char> &buffer) {
  parseFromBuffer(buffer);
}

void RxPMessage::parseFromBuffer(const vector<char> &buffer) {
  int i = 0;
  // Unpack headers
  sequence_number = (buffer[i++] << 24) | (buffer[i++] << 16) | (buffer[i++] << 8) | buffer[i++];
  ACK_number = (buffer[i++] << 24) | (buffer[i++] << 16) | (buffer[i++] << 8) | buffer[i++];
  checksum = (buffer[i++] << 8) | buffer[i++];
  window_size = (buffer[i++] << 8) | buffer[i++];
  dest_port = (buffer[i++] << 8) | buffer[i++];
  src_port = (buffer[i++] << 8) | buffer[i++];
  char flags = buffer[i];
  ACK_flag = flags & 0x80;
  SYN_flag = flags & 0x40;
  FIN_flag = flags & 0x20;
  RST_flag = flags & 0x10;
  // Unpack data
  data.resize(buffer.size() - 17);
  copy(buffer.begin() + 17, buffer.end(), data.begin());
}

vector<char> RxPMessage::toBuffer() {
  vector<char> result;
  // Pack headers
  result.push_back((char)((sequence_number & 0xFF000000) >> 24));
  result.push_back((char)((sequence_number & 0x00FF0000) >> 16));
  result.push_back((char)((sequence_number & 0x0000FF00) >>  8));
  result.push_back((char)((sequence_number & 0x000000FF)));
  result.push_back((char)((ACK_number & 0xFF000000) >> 24));
  result.push_back((char)((ACK_number & 0x00FF0000) >> 16));
  result.push_back((char)((ACK_number & 0x0000FF00) >>  8));
  result.push_back((char)((ACK_number & 0x000000FF)));
  result.push_back((char)((checksum & 0x0000FF00) >> 8));
  result.push_back((char)((checksum & 0x000000FF)));
  result.push_back((char)((window_size & 0x0000FF00) >> 8));
  result.push_back((char)((window_size & 0x000000FF)));
  result.push_back((char)((dest_port & 0x0000FF00) >> 8));
  result.push_back((char)((dest_port & 0x000000FF)));
  result.push_back((char)((src_port & 0x0000FF00) >> 8));
  result.push_back((char)((src_port & 0x000000FF)));
  result.push_back((char)((ACK_flag << 7) | (SYN_flag << 6) | (FIN_flag << 5) | (RST_flag << 4)));
  // Pack data
  result.resize(17 + data.size());
  result.insert(result.begin() + 17, data.begin(), data.end());
  return result;
}