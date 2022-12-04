// Query.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>
#include <string>

#include "query.h"
#include "../parser/parser.h"


void print_prompt() {
    std::cout << "SQLDB > ";
}


int start_loop() {
    std::string command;
    while (true) {
        // read
        print_prompt();
        getline(std::cin, command);
        // parse
        if (command == "quit") break;  // handle meta-commands
        else parse(command);  // sql commands
        // execute
        // print
    }
    return 0;
}