// op_util.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <functional>

#include "compare.h"
#include "../util/exception.h"

void setupCondition(Table *table, ExprColumn *expr,
                    int *table_used_columns, int &table_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    if (col_index == -1) {
        throw SqlDBException("column does not exist: " + expr->column);
    }
    expr->value_index = table_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    ++table_used_columns_count;
}

void setupCondition(Table *table, ExprColumn *expr,
                    int *table_used_columns, int *table_condition_columns,
                    int &table_used_columns_count, int &total_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    if (col_index == -1) {
        throw SqlDBException("column does not exist: " + expr->column);
    }
    expr->value_index = total_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    table_condition_columns[table_used_columns_count] = total_used_columns_count;
    ++table_used_columns_count;
    ++total_used_columns_count;
}

void setupCondition(Table *table, ExprColumn *expr, ColumnType &type, unsigned &length,
                    int *table_used_columns, int &table_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    if (col_index == -1) {
        throw SqlDBException("column does not exist: " + expr->column);
    }
    type = table->getColumnType(col_index);
    length = table->getColumnLength(col_index);
    expr->value_index = table_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    ++table_used_columns_count;
}

void setupCondition(Table *table, ExprColumn *expr, ColumnType &type, unsigned &length,
                    int *table_used_columns, int *table_condition_columns,
                    int &table_used_columns_count, int &total_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    if (col_index == -1) {
        throw SqlDBException("column does not exist: " + expr->column);
    }
    type = table->getColumnType(col_index);
    length = table->getColumnLength(col_index);
    expr->value_index = total_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    table_condition_columns[table_used_columns_count] = total_used_columns_count;
    ++table_used_columns_count;
    ++total_used_columns_count;
}

void searchTableColumn(Table *primary, Table *secondary, const std::string &column,
                       int &primary_index, int &secondary_index) {
    primary_index = primary->getColumnIndex(column);
    if (primary_index == -1) {
        secondary_index = secondary->getColumnIndex(column);
        if (secondary_index == -1) {
            throw SqlDBException("column not found: " + column);
        }
    }
}

void conditionalIterRecord(Table *table, const std::vector<Condition *> &conditions,
                           const std::function<void(RecordCursor &, bool satisfied)> &callback) {
    RecordCursor cursor(table);
    std::vector<Type *> condition_values;
    int used_columns[MAX_COLUMN];
    int used_columns_count = 0;
    // bind condition values to condition columns
    for (auto condition: conditions) {
        if (condition->getType() == ConditionType::Cmp) {
            auto condition_ = dynamic_cast<ConditionCmp *>(condition);
            // prepare lhs column expression
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            setupCondition(table, lhs, lhs_type, lhs_length,
                           used_columns, used_columns_count);
            if (condition_->rhs->getType() == ExpressionType::COLUMN) {
                // prepare lhs column expression
                auto rhs = dynamic_cast<ExprColumn *>(condition_->rhs);
                setupCondition(table, rhs, used_columns, used_columns_count);
            } else if (condition_->rhs->getType() == ExpressionType::VALUE) {
                auto rhs = dynamic_cast<ExprValue *>(condition_->rhs);
                auto buffer = new unsigned char[lhs_length];
                serializeFromString(rhs->value_s, lhs_type, buffer, lhs_length);
                rhs->value = deserialize(buffer, lhs_type, lhs_length);
                delete[] buffer;
            } else {
                throw SqlDBException("invalid expression type");
            }
        } else {
            // TODO
        }
    }
    condition_values.resize(used_columns_count);
    while (cursor.next()) {
        for (int i = 0; i < used_columns_count; ++i) {
            condition_values[i] = cursor.get(used_columns[i]);
        }
        bool satisfied = true;
        for (auto condition: conditions) {
            if (!condition->satisfy(condition_values)) {
                satisfied = false;
                break;
            }
        }
        callback(cursor, satisfied);
        for (auto &condition: condition_values) {
            delete condition;
        }
    }
}
