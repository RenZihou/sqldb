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
        auto table = Table::createTable(op_->getTableName());
//        auto table = new Table(op_->getTableName());
        auto columns = std::move(op_->getTableColumns());
        for (auto it = columns.rbegin(); it != columns.rend(); ++it) {
            table->addColumn(*it, "");
        }  // range based inverse loop is not supported by gcc for now
        delete table;
        return;
    } else if (op->getType() == OpType::TABLE_INSERT) {
        auto *op_ = dynamic_cast<OpTableInsert *>(op);
        auto table = new Table(op_->getTableName());
        auto values = std::move(op_->getValues());
        for (auto &value: values) {
            table->insertRecord(value);
        }
        delete table;
    }
}
