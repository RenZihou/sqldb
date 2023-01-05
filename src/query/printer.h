// printer.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef PRINTER_H_
#define PRINTER_H_

#include <string>
#include <vector>

#include "../table/type.h"

class Printer {
protected:
    unsigned printed_lines;
    unsigned columns;

public:
    explicit Printer() : printed_lines(0), columns(0) {}

    ~Printer() = default;

    virtual void printHeader(const std::vector<std::string> &header);

    virtual void printLine(const std::vector<Type *> &line);

    virtual void printEnd();

    virtual void printMessage(const std::string &message);
};

class MemoryStringPrinter : public Printer {
private:
    std::vector<std::string> lines;

public:
    void printHeader(const std::vector<std::string> &header) override;

    void printLine(const std::vector<Type *> &line) override;

    void printEnd() override;

    void printMessage(const std::string &message) override;

    [[nodiscard]] std::vector<std::string> getLines() const;
};

class CounterPrinter : public Printer {
public:
    void printHeader(const std::vector<std::string> &header) override;

    void printLine(const std::vector<Type *> &line) override;

    void printEnd() override;

    void printMessage(const std::string &message) override;

    [[nodiscard]] unsigned getCount() const;
};

class DummyPrinter : public Printer {
public:
    void printHeader(const std::vector<std::string> &header) override;

    void printLine(const std::vector<Type *> &line) override;

    void printEnd() override;

    void printMessage(const std::string &message) override;
};

#endif  // PRINTER_H_
