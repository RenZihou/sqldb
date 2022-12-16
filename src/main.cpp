// main.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>

#include "query/query.h"
#include "pagefile/buffer_manager.h"

int main() {
    std::cout << "SqlDB boosting..." << std::endl;
    start_loop();
    std::cout << "SqlDB shutting down..." << std::endl;
    BufferManager::bm().writeBackAll();
    std::cout << "Bye." << std::endl;
    return 0;
}
