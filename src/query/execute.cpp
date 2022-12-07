// execute.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>

#include "execute.h"

void execute(Op *op) {
    if (op->getType() == OpType::DB_CREATE) {
        std::cout << "DB_CREATE " << dynamic_cast<OpDbCreate *>(op)->getDbName() << std::endl;
    }
}
