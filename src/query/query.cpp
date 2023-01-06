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
#include "../util/stopwatch.h"

void print_prompt(bool multi_line = false) {
    std::string prompt = Database::db().getCurrentDb().empty()
            ? "SqlDB" : "SqlDB (" + Database::db().getCurrentDb() + ")";
    std::cout << (multi_line ? std::string(prompt.size(), '.') + ".. " : prompt + " > ");
}

int start_loop() {
    std::string command;
    std::string line;
    Op *op;
    Printer printer;
    Stopwatch stopwatch;
    while (true) {
        // read
        bool multi_line = false;
        command.clear();
        do {
            print_prompt(multi_line);
            getline(std::cin, line);
            if (!multi_line && line == "QUIT") return 0;
            command += line.empty() ? "" : " " + line;
            multi_line = true;
        } while (!command.empty() && command[command.size() - 1] != ';');
        // parse
        op = parse(command);  // sql commands
        // execute
        try {
            while (op) {
                stopwatch.start();
                op->optimize();
                op->execute(&printer);
                stopwatch.stop();
                printf("Used Time: %.3lf seconds\n\n", stopwatch.getElapsedSeconds());
                op = op->getNext();
            }
        } catch (SqlDBException &e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
        }
        delete op;
    }
}