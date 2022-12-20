// printer.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef PRINTER_H_
#define PRINTER_H_

#include <string>
#include <vector>

#include "../table/type.h"

class Printer {
private:
    unsigned printed_lines;
    unsigned columns;
//    std::vector<ColumnType> types;

public:
    explicit Printer() : printed_lines(0), columns(0) {}

    ~Printer() = default;

//    virtual void setup(const std::vector<ColumnType> &types_);

    virtual void printHeader(const std::vector<std::string> &header);

    virtual void printLine(const std::vector<Type *> &line);

    virtual void printEnd();

    virtual void printMessage(const std::string &message);
};

#endif  // PRINTER_H_
