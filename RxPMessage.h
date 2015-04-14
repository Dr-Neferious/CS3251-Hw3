//
// Created by matt on 4/12/15.
//

#ifndef CS3251_HW3_RXPMESSAGE_H
#define CS3251_HW3_RXPMESSAGE_H

#include <vector>

class RxPMessage {
public:
  RxPMessage();

  RxPMessage(const std::vector<char> &buffer);

  void parseFromBuffer(const std::vector<char> &buffer);

  std::vector<char> toBuffer();

  void fillChecksum();

  int calcChecksum();

  int sequence_number;
  int ACK_number;
  int checksum;
  int window_size;
  int dest_port;
  int src_port;
  bool ACK_flag;
  bool SYN_flag;
  bool FIN_flag;
  bool RST_flag;
  std::vector<char> data;

  class ParseException : public std::exception {
  public:
    const char *what() const throw() override {
      return "Checksum calculation indicated message corruption.";
    }
  };
};

#endif //CS3251_HW3_RXPMESSAGE_H
