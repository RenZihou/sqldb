// main.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>

#include "query/query.h"
#include "system/database.h"

int main() {
    std::cout << "SqlDB boosting..." << std::endl;
    Database::db().boost();
    start_loop();
    std::cout << "SqlDB shutting down..." << std::endl;
    Database::db().shutdown();
    std::cout << "Bye." << std::endl;
    return 0;
}
