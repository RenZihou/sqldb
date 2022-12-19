// Query.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>
#include <string>

#include "execute.h"
#include "printer.h"
#include "query.h"
#include "../parser/parser.h"


void print_prompt() {
    std::cout << "SQLDB > ";
}


int start_loop() {
    std::string command;
    Op *op;
    Printer printer;
    while (true) {
        // read
        print_prompt();
        getline(std::cin, command);
        // parse
        if (command == "QUIT") break;  // handle meta-commands
        op = parse(command);  // sql commands
        if (op == nullptr) {
            std::cerr << "Unexpected error occurred when parsing `" << command << "`" << std::endl;
            continue;
        }
        // TODO logical optimization here
        // execute
        execute(op, printer);
        // print
    }
    return 0;
}