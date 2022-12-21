// Query.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>
#include <string>

#include "printer.h"
#include "query.h"
#include "../system/database.h"
#include "../parser/parser.h"
#include "../util/exception.h"

void print_prompt() {
    if (Database::db().getCurrentDb().empty()) {
        std::cout << "SqlDB > ";
    } else {
        std::cout << "SqlDB (" << Database::db().getCurrentDb() << ") > ";
    }
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
        // TODO logical optimization here
        // execute
        try {
            while (op) {
                op->execute(&printer);
                op = op->getNext();
            }
//            execute(op, &printer);
        } catch (SqlDBException &e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }
        // print
        delete op;
    }
    return 0;
}