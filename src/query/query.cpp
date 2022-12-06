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
    Op *op;
    while (true) {
        // read
        op = nullptr;
        print_prompt();
        getline(std::cin, command);
        // parse
        if (command == "QUIT") break;  // handle meta-commands
        else op = parse(command);  // sql commands
        if (op == nullptr) {
            std::cerr << "SyntaxError raised when parsing `" << command << "`" << std::endl;
            continue;
        }
        // execute
        if (op->getType() == OpType::DB_CREATE) {
            std::cout << "DB_CREATE " << dynamic_cast<OpDbCreate *>(op)->getDbName() << std::endl;
        }
        // print
    }
    return 0;
}