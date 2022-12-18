// execute.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>

#include "execute.h"
//#include "../table/cursor.h"
//#include "../parser/compare.h"

void conditionalSelect(const std::string &table_name, const std::vector<std::string> &columns,
                       const std::vector<Condition *> &conditions) {

}

void conditionalJoin(const std::string &primary_table, const std::string &secondary_table,
                     const std::vector<std::string> &columns,
                     const std::vector<Condition *> &conditions) {
    // TODO choose the table with index as secondary table
    // choose the table with the least number of rows as primary table
    auto primary = new Table(primary_table);
    auto secondary = new Table(secondary_table);
    // TODO move the following code to a function (optimizer?)
//    if (primary->getRows() > secondary->getRows()) {
//        std::swap(primary, secondary);
//    }
    RecordCursor primary_cursor(primary);
    RecordCursor secondary_cursor(secondary);
    std::vector<Type *> condition_values;  // table values that are used in condition expressions
    int primary_used_columns[MAX_COLUMN];  // map from used column index to table column index
    int primary_mapped_columns[MAX_COLUMN];  // map from used column index to condition values index
    int primary_used_columns_count = 0;  // number of used columns
    int secondary_used_columns[MAX_COLUMN];
    int secondary_mapped_columns[MAX_COLUMN];
    int secondary_used_columns_count = 0;
    int total_used_columns_count = 0;
    // bind condition values to condition expressions
    for (auto condition : conditions) {
        if (condition->getType() == ConditionType::Cmp) {
            auto condition_ = dynamic_cast<ConditionCmp *>(condition);
            // prepare lhs column expression
            int lhs_col_index;
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            if (lhs->table == primary_table) {
                lhs_col_index = primary->getColumnIndex(lhs->column);
                lhs_type = primary->getColumnType(lhs_col_index);
                lhs_length = primary->getColumnLength(lhs_col_index);
                lhs->value_index = total_used_columns_count;
                primary_used_columns[primary_used_columns_count] = lhs_col_index;
                primary_mapped_columns[primary_used_columns_count] = total_used_columns_count;
                ++primary_used_columns_count;
                ++total_used_columns_count;
            } else if (lhs->table == secondary_table) {
                lhs_col_index = primary->getColumnIndex(lhs->column);
                lhs_type = primary->getColumnType(lhs_col_index);
                lhs_length = primary->getColumnLength(lhs_col_index);
                lhs->value_index = total_used_columns_count;
                secondary_used_columns[secondary_used_columns_count] = lhs_col_index;
                secondary_mapped_columns[secondary_used_columns_count] = total_used_columns_count;
                ++secondary_used_columns_count;
                ++total_used_columns_count;
            } else {
                throw std::runtime_error("table not found");  // TODO custom error
            }
            if (condition_->rhs->getType() == ExpressionType::COLUMN) {
                // prepare lhs column expression
                int rhs_col_index;
                auto rhs = dynamic_cast<ExprColumn *>(condition_->rhs);
                if (rhs->table == primary_table) {
                    rhs_col_index = primary->getColumnIndex(rhs->column);
                    rhs->value_index = total_used_columns_count;
                    primary_used_columns[primary_used_columns_count] = rhs_col_index;
                    primary_mapped_columns[primary_used_columns_count] = total_used_columns_count;
                    ++primary_used_columns_count;
                    ++total_used_columns_count;
                } else if (rhs->table == secondary_table) {
                    rhs_col_index = secondary->getColumnIndex(rhs->column);
                    rhs->value_index = total_used_columns_count;
                    secondary_used_columns[secondary_used_columns_count] = rhs_col_index;
                    secondary_mapped_columns[secondary_used_columns_count] = total_used_columns_count;
                    ++secondary_used_columns_count;
                    ++total_used_columns_count;
                } else {
                    throw std::runtime_error("table not found");  // TODO custom error
                }
            } else if (condition_->rhs->getType() == ExpressionType::VALUE) {
                auto rhs = dynamic_cast<ExprValue *>(condition_->rhs);
                auto value_s = new unsigned char [rhs->value_s.length() + 1];
                memcpy(value_s, rhs->value_s.c_str(), rhs->value_s.length());
                rhs->value = deserialize(value_s, lhs_type, lhs_length);
                delete[] value_s;
            } else {
                throw std::runtime_error("invalid expression type");  // TODO custom error
            }
        } else {
            // TODO condition in
        }
    }
    condition_values.reserve(total_used_columns_count);
    while (primary_cursor.next()) {
        // read primary table values
        for (int i = 0; i < primary_used_columns_count; ++i) {
            condition_values[primary_mapped_columns[i]] = primary_cursor.get(primary_used_columns[i]);
        }
        while (secondary_cursor.next()) {
            // read secondary table values
            for (int i = 0; i < secondary_used_columns_count; ++i) {
                condition_values[secondary_mapped_columns[i]] = secondary_cursor.get(secondary_used_columns[i]);
            }
            bool satisfied = true;
            for (auto condition: conditions) {
                if (!condition->satisfy(condition_values)) {
                    satisfied = false;
                    break;
                }
            }
            if (satisfied) {
                // TODO send to printer or temp table
                std::cout << "satisfied" << std::endl;
            }
        }
        secondary_cursor.reset();
    }
    delete primary;
    delete secondary;
}

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
    } else if (op->getType() == OpType::TABLE_INSERT) {
        auto *op_ = dynamic_cast<OpTableInsert *>(op);
        auto table = new Table(op_->getTableName());
        auto values = std::move(op_->getValues());
        for (auto &value: values) {
            table->insertRecord(value);
        }
        delete table;
    } else if (op->getType() == OpType::TABLE_SELECT) {
        auto *op_ = dynamic_cast<OpTableSelect *>(op);
        switch (op_->getTableNames().size()) {
            case 1:
                break;
            case 2:
                // TODO * selector
                conditionalJoin(op_->getTableNames()[0], op_->getTableNames()[1], op_->getSelectors(), op_->getConditions());
                break;
            default:
                throw std::runtime_error("invalid table count");  // TODO multi-table select
        }
    }
}
