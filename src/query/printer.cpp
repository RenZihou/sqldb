// printer.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstdio>  // using printf instead of cout for better performance

#include "printer.h"

//void Printer::setup(const std::vector<ColumnType> &types_) {
//    this->types = types_;
//}

void Printer::printHeader(const std::vector<std::string> &header) {
    this->columns = header.size();
    this->printed_lines = 0;
    char sep = '\0';
    for (const auto &column: header) {
        printf("%c%s", sep, column.c_str());
        sep = '\t';
    }
    printf("\n");
}

void Printer::printLine(const std::vector<Type *> &line) {
    ++this->printed_lines;
    char sep = '\0';
//    unsigned columns = this->types.size();
    for (unsigned i = 0; i < this->columns; ++i) {
        printf("%c%s", sep, line[i]->toString().c_str());
        sep = '\t';
    }
    printf("\n");
}

void Printer::printEnd() {
    printf("%d row in set\n", this->printed_lines);
}

void Printer::printMessage(const std::string &message) {
    printf("%s\n", message.c_str());
}

void MemoryStringPrinter::printHeader(const std::vector<std::string> &header) {
    this->lines.clear();
}

void MemoryStringPrinter::printLine(const std::vector<Type *> &line) {
    for (const auto &column: line) {
        this->lines.push_back(column->toString());
    }
}

void MemoryStringPrinter::printEnd() {}

void MemoryStringPrinter::printMessage(const std::string &message) {}

std::vector<std::string> MemoryStringPrinter::getLines() const { return this->lines; }
