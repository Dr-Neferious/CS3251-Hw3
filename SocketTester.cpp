//
// Created by matt on 4/12/15.
//

#include "RxPSocket.h"

int main(int argc, char** argv)
{
  RxPSocket client = RxPSocket::connect("127.0.0.1", 8800);

  RxPSocket server = RxPSocket::listen(8800);

}