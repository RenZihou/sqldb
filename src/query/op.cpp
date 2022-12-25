// OpTree.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <functional>

#include "op.h"
#include "op_util.h"
#include "../system/database.h"
#include "../index/int_index.h"

void Op::execute(Printer *printer) {
    throw SqlDBException("not supported syntax");
}

void OpDbCreate::execute(Printer *printer) {
    Database::db().createDb(this->name);
}

void OpDbDrop::execute(Printer *printer) {
    Database::db().dropDb(this->name);
}

void OpDbShow::execute(Printer *printer) {
    Database::db().showDb(printer);
}

void OpDbUse::execute(Printer *printer) {
    Database::db().useDb(this->name);
}

void OpDbShowTables::execute(Printer *printer) {
    Database::db().showTable(printer);
}

void OpTableCreate::execute(Printer *printer) {
    Database::db().createTable(this->name);
    auto table = Table::createTable(this->name);
    for (auto it = this->columns.rbegin(); it != this->columns.rend(); ++it) {
        table->addColumn(*it, "");
    }  // range based inverse loop is not supported by gcc for now
    delete table;
}

void OpTableDrop::execute(Printer *printer) {
    Database::db().dropTable(this->name);
}

void OpTableInsert::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    for (auto &value: this->values) {
        table->insertRecord(value);
    }
    delete table;
}

void OpTableDelete::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    int deleted_count = 0;
    conditionalIterRecord(table, this->conditions, [&](RecordCursor &cursor, bool satisfied) {
        if (satisfied) {
            cursor.del();
            ++deleted_count;
        }
    });
    printer->printMessage("deleted " + std::to_string(deleted_count) + " records");
    delete table;
}

void OpTableUpdate::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    std::vector<std::tuple<int, std::string>> update_columns;
    for (const auto &update: this->updates) {
        int column_index = table->getColumnIndex(std::get<0>(update));
        if (column_index == -1) {
            throw SqlDBException("column not found: " + std::get<0>(update));
        }
        update_columns.emplace_back(column_index, std::get<1>(update));
    }
    int updated_count = 0;
    conditionalIterRecord(table, this->conditions, [&](RecordCursor &cursor, bool satisfied) {
        if (satisfied) {
            cursor.set(update_columns);
            ++updated_count;
        }
    });
    printer->printMessage("updated " + std::to_string(updated_count) + " records");
    delete table;
}

void OpTableSelect::execute(Printer *printer) {
    bool use_index;
    std::string index_column;
    int begin, end;
    switch (this->tables.size()) {
        case 1:
            Database::db().assertTableExists(this->tables[0]);
            checkIndexOnCondition(this->tables[0], this->conditions, use_index, index_column, begin,
                                  end);
            conditionalSelect(this->tables[0], this->selectors, this->conditions,
                              use_index, index_column, begin, end,
                              printer);
            break;
        case 2:
            Database::db().assertTableExists(this->tables[0]);
            Database::db().assertTableExists(this->tables[1]);
            conditionalJoin(this->tables[0], this->tables[1],
                            this->selectors, this->conditions, printer);
            break;
        default:
            throw SqlDBException("invalid table count");
    }
}

void OpTableAlterAddIndex::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    if (this->columns.size() != 1) {
        throw SqlDBException("only support single column index");
    }
    auto table = new Table(this->name);
    int column_index = table->getColumnIndex(this->columns[0]);
    if (column_index == -1) {
        throw SqlDBException("column not found: " + this->columns[0]);
    }
    if (table->getColumnType(column_index) != ColumnType::INT) {
        throw SqlDBException("only support int column index");
    }
    if (table->hasIndex(column_index)) {
        throw SqlDBException("index already exists");
    }
    FileManager::fm().createFile(this->name + "." + this->columns[0]);
    auto index = IntIndex::createIndex(this->name, this->columns[0]);
    conditionalIterRecord(table, {}, [&](RecordCursor &cursor, bool satisfied) {
        index->insert(dynamic_cast<Int *>(cursor.get(column_index))->getValue(),
                      cursor.getOffset());
    });
    table->addIndex(column_index);
    delete index;
    delete table;
}
