// OpTree.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <functional>

#include "op.h"
#include "op_util.h"
#include "../system/database.h"

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
    if (this->primary_keys.size() == 1) {
        OpTableAlterAddPk(this->name, std::get<0>(this->primary_keys[0]),
                          std::get<1>(this->primary_keys[0])).execute(printer);
    } else if (this->primary_keys.size() > 1) {
        throw SqlDBException("multiple primary keys are not supported");
    }
}

void OpTableDrop::execute(Printer *printer) {
    Database::db().dropTable(this->name);
}

void OpTableDescribe::execute(Printer *printer) {
    auto table = new Table(this->name);
    // columns
    printer->printHeader({"field", "type", "null", "default"});
    auto columns = table->getColumns();
    Type *field, *type, *null, *default_;
    for (int i = 0; i < static_cast<int>(columns.size()); ++i) {
        field = new VarChar(columns[i]);
        switch (table->getColumnType(i)) {
            case ColumnType::INT:
                type = new VarChar("INT");
                break;
            case ColumnType::FLOAT:
                type = new VarChar("FLOAT");
                break;
            case ColumnType::VARCHAR:
                type = new VarChar("VARCHAR(" + std::to_string(table->getColumnLength(i)) + ")");
                break;
            default:
                type = new VarChar("UNKNOWN");
                break;
        }
        null = new VarChar(table->getColumnNullable(i) ? "YES" : "NO");
        default_ = table->getColumnDefault(i);
        if (default_ == nullptr) {
            default_ = new VarChar("NULL");
        }
        printer->printLine({field, type, null, default_});
        delete field;
        delete type;
        delete null;
        delete default_;
    }
    printer->printEnd();
    // primary key
    int pk_column;
    std::string pk_key;
    if (table->getPrimaryKey(pk_column, pk_key)) {
        printer->printMessage("PRIMARY KEY " + pk_key + "(" + columns[pk_column] + ")");
    }
    // TODO foreign key
    // TODO unique
    // index
    for (int i = 0; i < static_cast<int>(columns.size()); ++i) {
        if (table->hasIndex(i)) {
            printer->printMessage("INDEX (" + columns[i] + ")");
        }
    }
    delete table;
}

void OpTableInsert::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    for (auto &value: this->values) {
        table->insertRecord(value);  // table will update index
    }
    delete table;
}

void OpTableDelete::execute(Printer *printer) {  // TODO move index maintenance to table cursor
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    std::vector<std::tuple<int, IntIndex *>> indexes;
    int deleted_count = 0;
    auto columns = table->getColumns();
    for (int i = 0; i < static_cast<int>(columns.size()); ++i) {
        if (table->hasIndex(i)) {
            indexes.emplace_back(i, new IntIndex(this->name, columns[i]));
        }
    }
    conditionalIterRecord(table, this->conditions, [&](RecordCursor &cursor, bool satisfied) {
        if (satisfied) {
            for (auto &[column, index]: indexes) {
                index->remove(dynamic_cast<Int *>(cursor.get(column))->getValue(),
                              cursor.getOffset());
            }
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
    std::vector<std::tuple<int, int, IntIndex *>> update_indexes;
    for (const auto &[column, value_s]: this->updates) {
        int column_index = table->getColumnIndex(column);
        if (column_index == -1) {
            throw SqlDBException("column not found: " + column);
        }
        update_columns.emplace_back(column_index, value_s);
        if (table->hasIndex(column_index)) {
            update_indexes.emplace_back(column_index, std::stoi(value_s),
                                        new IntIndex(this->name, column));
        }
    }
    int updated_count = 0;
    conditionalIterRecord(table, this->conditions, [&](RecordCursor &cursor, bool satisfied) {
        if (satisfied) {
            for (auto &[column, new_value, index]: update_indexes) {
                index->remove(dynamic_cast<Int *>(cursor.get(column))->getValue(),
                              cursor.getOffset());
                index->insert(new_value, cursor.getOffset());
            }
            cursor.set(update_columns);
            ++updated_count;
        }
    });
    printer->printMessage("updated " + std::to_string(updated_count) + " records");
    delete table;
    for (const auto &index: update_indexes) {
        delete std::get<2>(index);
    }
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

void OpTableAlterDropIndex::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    if (this->columns.size() != 1) {
        throw SqlDBException("only support single column index");
    }
    auto table = new Table(this->name);
    int column_index = table->getColumnIndex(this->columns[0]);
    if (column_index == -1) {
        throw SqlDBException("column not found: " + this->columns[0]);
    }
    if (table->isPrimaryKey(column_index)) {
        throw SqlDBException("cannot drop primary key index");
    }
    if (!table->hasIndex(column_index)) {
        throw SqlDBException("index not exists");
    }
    FileManager::fm().rmFile(this->name + "." + this->columns[0]);
    table->dropIndex(column_index);
    delete table;
}

void OpTableAlterAddPk::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    int pk_column;
    std::string pk_key;
    if (table->getPrimaryKey(pk_column, pk_key)) {
        throw SqlDBException("primary key already exists");
    }
    if (this->columns.size() != 1) {
        throw SqlDBException("multiple primary keys are not supported");
    }
    pk_column = table->getColumnIndex(this->columns[0]);
    if (pk_column == -1) {
        throw SqlDBException("column not found: " + this->columns[0]);
    }
    table->addPrimaryKey(pk_column, this->pk);
    delete table;
    // add index
    try {
        OpTableAlterAddIndex(this->name, this->columns).execute(printer);
    } catch (SqlDBException &e) {
        // already has index, ignore error
    }
}

void OpTableAlterDropPk::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    int pk_column;
    std::string pk_key;
    if (!table->getPrimaryKey(pk_column, pk_key)) {
        throw SqlDBException("primary key not exists");
    }
    if (!this->pk.empty() && this->pk != pk_key) {
        throw SqlDBException("primary key not match");
    }
    table->dropPrimaryKey();
    auto column = table->getColumns()[pk_column];
    delete table;
    OpTableAlterDropIndex(this->name, {column}).execute(printer);
}