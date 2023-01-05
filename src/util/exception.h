// exception.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef EXCEPTION_H_
#define EXCEPTION_H_

#include <exception>
#include <string>

class SqlDBException : public std::exception {
private:
    std::string message;

public:
    explicit SqlDBException(std::string message) : message(std::move(message)) {}

    SqlDBException(const SqlDBException &other) : message(other.message) {}

    [[nodiscard]] const char *what() const noexcept override { return this->message.c_str(); }
};

#endif  // EXCEPTION_H_
