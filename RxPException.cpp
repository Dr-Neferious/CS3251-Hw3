//
// Created by matt on 4/10/15.
//
#include <cstring>

#include "RxPException.h"

using namespace std;

RxPException::RxPException(int error) throw() {
  _error = strerror(error);
}

RxPException::RxPException(const char* error) throw() {
  _error = error;
}

const char* RxPException::what() const throw() {
  return _error;
}

const char* RxPTimeoutException::what() const throw() {
  return "Socket receive call timed out.";
}