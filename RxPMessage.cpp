//
// Created by matt on 4/12/15.
//

#include "RxPMessage.h"
#include <algorithm>
#include <iostream>

using namespace std;

RxPMessage::RxPMessage()
  : data(0)
{
  sequence_number = 0;
  ACK_number = 0;
  checksum = 0;
  window_size = 1;
  dest_port = 0;
  src_port = 0;
  ACK_flag = false;
  SYN_flag = false;
  FIN_flag = false;
  RST_flag = false;
}

RxPMessage::RxPMessage(const vector<char> &buffer) {
  parseFromBuffer(buffer);
}

void RxPMessage::parseFromBuffer(const vector<char> &buffer) {
  if(accumulate(buffer.begin(), buffer.end(), 0) != 0)
    throw ParseException("Checksum calculation indicated message corruption.");
  if(buffer.size() < 17)
    throw ParseException("Buffer not large enough to contain a valid message.");
  int i = 0;
  // Unpack headers
  sequence_number = (buffer[i++] << 24) | (buffer[i++] << 16) | (buffer[i++] << 8) | buffer[i++];
  ACK_number = (buffer[i++] << 24) | (buffer[i++] << 16) | (buffer[i++] << 8) | buffer[i++];
  checksum = buffer[i++];
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
  result.push_back((char)(checksum));
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

void RxPMessage::fillChecksum() {
  checksum = 0;
  vector<char> buffer = toBuffer();
  checksum = std::accumulate(buffer.begin(), buffer.end(), 0);
  checksum = ~checksum + 1;
}

string RxPMessage::toString() {
  string result = "Message:\n";

  result += "\tSequence Number:" + to_string(sequence_number) + "\n";
  result += "\tACK Number:" + to_string(ACK_number) + "\n";
  result += "\tChecksum: " + to_string((int)checksum) + "\n";
  result += "\tWindow Size: " + to_string(window_size) + "\n";
  result += "\tDestination Port: " + to_string(dest_port) + "\n";
  result += "\tSource Port: " + to_string(src_port) + "\n";
  result += "\tACK Flag: " + string(ACK_flag ? "set" : "not set") + "\n";
  result += "\tSYN Flag: " + string(SYN_flag ? "set" : "not set") + "\n";
  result += "\tFIN Flag: " + string(FIN_flag ? "set" : "not set") + "\n";
  result += "\tRST Flag: " + string(RST_flag ? "set" : "not set") + "\n";
  result += "\tData: ";
  for(auto byte : data)
    result += to_string(byte) + ", ";

  return result;
}