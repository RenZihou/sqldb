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

void Op::optimize() {}

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
    for (auto &[fk, ref_table, fk_columns, ref_columns] : this->foreign_keys) {
        OpTableAlterAddFk(this->name, fk, ref_table, fk_columns, ref_columns).execute(printer);
    }
}

void OpTableDrop::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    if (!table->getReferences().empty()) {
        throw SqlDBException("table referenced by foreign key");
    }
    auto fks = table->getForeignKeys();
    delete table;
    for (const auto &fk: fks) {
        OpTableAlterDropFk(this->name, fk.name).execute(printer);
    }
    Database::db().dropTable(this->name);
}

void OpTableDescribe::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
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
                type = new VarChar("VARCHAR(" + std::to_string(table->getColumnLength(i) - 1) + ")");
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
    // foreign key
    auto fks = table->getForeignKeys();
    Table *ref_table;
    int ref_table_pk_column;
    std::string ref_table_pk_key;
    for (const auto &[key, column, ref]: fks) {
        ref_table = new Table(ref);
        ref_table->getPrimaryKey(ref_table_pk_column, ref_table_pk_key);
        printer->printMessage("FOREIGN KEY " + key + "(" + columns[column] + ") REFERENCES " +
                              ref + "(" + ref_table->getColumns()[ref_table_pk_column] + ")");
        delete ref_table;
    }
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
        table->insertRecord(value);  // table will check integrity update index
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
    std::vector<std::tuple<int, Type *>> update_columns;
    std::vector<std::tuple<int, int, IntIndex *>> update_indexes;
    for (const auto &[column, value]: this->updates) {
        int column_index = table->getColumnIndex(column);
        if (column_index == -1) {
            throw SqlDBException("column not found: " + column);
        }
        update_columns.emplace_back(column_index, value);
        if (table->hasIndex(column_index)) {
            update_indexes.emplace_back(column_index, dynamic_cast<Int *>(value)->getValue(),
                                        new IntIndex(this->name, column));
        }
    }
    int updated_count = 0;
    // TODO check unique
    conditionalIterRecord(table, this->conditions, [&](RecordCursor &cursor, bool satisfied) {
        if (satisfied) {
            for (auto &[column, new_value, index]: update_indexes) {
                index->remove(dynamic_cast<Int *>(cursor.get(column))->getValue(),
                              cursor.getOffset());
            }
            cursor.set(update_columns);
            for (auto &[column, new_value, index]: update_indexes) {
                index->insert(new_value, cursor.getOffset());
            }
            ++updated_count;
        }
    });
    printer->printMessage("updated " + std::to_string(updated_count) + " records");
    delete table;
    for (const auto &index: update_indexes) {
        delete std::get<2>(index);
    }
}

void OpTableSelect::optimize() {
    if (this->tables.size() == 2) {
        auto primary = new Table(this->tables[0]);
        auto secondary = new Table(this->tables[1]);
        // add table name in column expression
        for (const auto &condition : this->conditions) {
            ExprColumn *lhs = nullptr;
            ExprColumn *rhs = nullptr;
            if (condition->getType() == ConditionType::Cmp) {
                auto condition_ = dynamic_cast<ConditionCmp *>(condition);
                lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
                if (condition_->rhs->getType() == ExpressionType::COLUMN) {
                    rhs = dynamic_cast<ExprColumn *>(condition_->rhs);
                }
            } else if (condition->getType() == ConditionType::In) {
                auto condition_ = dynamic_cast<ConditionIn *>(condition);
                lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            } else if (condition->getType() == ConditionType::Like) {
                auto condition_ = dynamic_cast<ConditionLike *>(condition);
                lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            }
            if (lhs && lhs->table.empty()) {
                int primary_index, secondary_index;
                searchTableColumn(primary, secondary, lhs->column,
                                  primary_index, secondary_index);
                if (primary_index != -1) {
                    lhs->table = this->tables[0];
                } else if (secondary_index != -1) {
                    lhs->table = this->tables[1];
                } else {
                    throw SqlDBException("column not found: " + lhs->column);
                }
            }
            if (rhs && rhs->table.empty()) {
                int primary_index, secondary_index;
                searchTableColumn(primary, secondary, rhs->column,
                                  primary_index, secondary_index);
                if (primary_index != -1) {
                    rhs->table = this->tables[0];
                } else if (secondary_index != -1) {
                    rhs->table = this->tables[1];
                } else {
                    throw SqlDBException("column not found: " + rhs->column);
                }
            }
        }
        // optimize for t1.c1 = t2.c2 condition

//        for (const auto &condition : this->conditions) {
//            if (condition->getType() == ConditionType::Cmp) {
//                auto condition_ = dynamic_cast<ConditionCmp *>(condition);
//                auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
//                if (condition_->rhs->getType() == ExpressionType::COLUMN) {
//                    auto rhs = dynamic_cast<ExprColumn *>(condition_->rhs);
//                    if (lhs->table != rhs->table && condition_->op->getType() == CompareType::EQ) {
//                        // search for index
//
//                    }
//                }
//            }
//        }
        delete primary;
        delete secondary;
    }
}

void OpTableSelect::execute(Printer *printer) {
    bool use_index;
    std::string index_column;
    int begin, end;
    switch (this->tables.size()) {
        case 1:
            Database::db().assertTableExists(this->tables[0]);
            checkIndexOnCondition(this->tables[0], this->conditions,
                                  use_index, index_column, begin, end);
            conditionalSelect(this->tables[0], this->selectors, this->conditions,
                              use_index, index_column, begin, end, printer);
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
    if (table->getColumnType(pk_column) != ColumnType::INT) {
        throw SqlDBException("primary key must be INT type");
    }
    bool has_index = table->hasIndex(pk_column);
    delete table;
    // add index
    if (!has_index) {
        OpTableAlterAddIndex(this->name, this->columns).execute(printer);
    }
    // check primary key unique (no null support for now)
    auto index = new IntIndex(this->name, this->columns[0]);
    auto index_cursor = new IntIndexCursor(index);
    int key, prev_key;
    unsigned offset;
    index_cursor->find(MIN_INT);
    if (index_cursor->next(key, offset)) {
        prev_key = key;
        while (index_cursor->next(key, offset)) {
            if (key == prev_key) {
                // rollback
                OpTableAlterDropIndex(this->name, this->columns).execute(printer);
                throw SqlDBException("primary key not unique");
            }
            prev_key = key;
        }
    }
    delete index_cursor;
    delete index;
    table = new Table(this->name);
    table->addPrimaryKey(pk_column, this->pk);
    delete table;
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
    if (!table->getReferences().empty()) {
        throw SqlDBException("column referenced by foreign key");
    }
    table->dropPrimaryKey();
    auto column = table->getColumns()[pk_column];
    delete table;
    OpTableAlterDropIndex(this->name, {column}).execute(printer);
}

void OpTableAlterAddFk::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    Database::db().assertTableExists(this->ref_table_name);
    if (this->columns.size() != 1 || this->ref_columns.size() != 1) {
        throw SqlDBException("multiple foreign keys are not supported");
    }
    if (this->name == this->ref_table_name) {
        throw SqlDBException("cannot add foreign key to self");  // TODO really?
    }
    auto table = new Table(this->name);
    auto ref_table = new Table(this->ref_table_name);
    int column_index = table->getColumnIndex(this->columns[0]);
    if (column_index == -1) {
        throw SqlDBException("column not found: " + this->columns[0]);
    }
    if (table->getColumnType(column_index) != ColumnType::INT) {
        throw SqlDBException("foreign key must be INT type");
    }
    int ref_column_index = ref_table->getColumnIndex(this->ref_columns[0]);
    if (ref_column_index == -1) {
        throw SqlDBException("column not found: " + this->ref_columns[0]);
    }
    if (!ref_table->isPrimaryKey(ref_column_index)) {
        throw SqlDBException("foreign key must reference primary key");
    }
    // check foreign key valid
    auto index = new IntIndex(this->ref_table_name, this->ref_columns[0]);
    unsigned index_pos, index_offset;
    bool match;
    conditionalIterRecord(table, {}, [&](RecordCursor &cursor, bool satisfied) {
        int value = dynamic_cast<Int *>(cursor.get(column_index))->getValue();
        if (!(index->search(value, index_pos, index_offset, match) && match)) {
            throw SqlDBException("target column does not have value: " + std::to_string(value));
        }
    });
    std::string fk_ = this->fk.empty() ? this->columns[0] : this->fk;
    table->addForeignKey(column_index, fk_, this->ref_table_name);
    try {
        ref_table->addReferenced(this->name, column_index);
    } catch (SqlDBException &e) {  // rollback when add failed
        table->dropForeignKey(fk_);
        throw SqlDBException(e);
    }
    delete table;
    delete ref_table;
}

void OpTableAlterDropFk::execute(Printer *printer) {
    Database::db().assertTableExists(this->name);
    auto table = new Table(this->name);
    int column_index;
    std::string ref_table_name;
    if (!table->getForeignKey(this->fk, column_index, ref_table_name)) {
        throw SqlDBException("foreign key not exists");
    }
    table->dropForeignKey(this->fk);
    auto ref_table = new Table(ref_table_name);
    ref_table->dropReferenced(this->name, column_index);
    delete table;
    delete ref_table;
}
