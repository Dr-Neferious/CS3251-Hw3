//
// Created by matt on 4/10/15.
//

#ifndef CS3251_HW3_RXPEXCEPTION_H
#define CS3251_HW3_RXPEXCEPTION_H


#include <exception>

class RxPException : public std::exception
{
public:
  RxPException(int error) throw();

  RxPException(const char *error) throw();

  const char* what() const throw() override;

private:
  const char* _error;
};

class RxPTimeoutException : public std::exception {
public:
  const char* what() const throw() override;
};


#endif //CS3251_HW3_RXPEXCEPTION_H
