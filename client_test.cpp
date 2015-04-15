//
// Created by matt on 4/14/15.
//

#include "RxPSocket.h"

using namespace std;

int main(int argc, char** argv)
{
  RxPSocket socket = RxPSocket::connect("127.0.0.1", 8000, 8001);

  string msg = "hello!";

  socket.send((char*)msg.c_str(), msg.size());

  while(true);
}