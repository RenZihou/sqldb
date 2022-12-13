// execute.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>

#include "execute.h"

void execute(Op *op) {
    if (op->getType() == OpType::DB_CREATE) {
        std::cout << "DB_CREATE " << dynamic_cast<OpDbCreate *>(op)->getDbName() << std::endl;
    } else if (op->getType() == OpType::TABLE_CREATE) {
        auto *op_ = dynamic_cast<OpTableCreate *>(op);
        Table::createTable(op_->getTableName());
        auto table = new Table(op_->getTableName());
        auto columns = std::move(op_->getTableColumns());
        for (auto it = columns.rbegin(); it != columns.rend(); ++it) {
            table->addColumn(*it, "");
        }  // range based loop is not supported by gcc now
        delete table;
        return;
    }
}
