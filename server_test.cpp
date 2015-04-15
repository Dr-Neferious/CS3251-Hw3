//
// Created by matt on 4/14/15.
//

#include "RxPSocket.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
  RxPSocket socket = RxPSocket::listen(8000);

  while(true) {
    char buffer[10];
    int r = socket.recv(buffer, 10);
    for(int i = 0; i < r; i++)
      cout << buffer[i];
    if(r > 0)
      cout << endl;
  }
}