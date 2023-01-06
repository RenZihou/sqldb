// op_util.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef OP_UTIL_H_
#define OP_UTIL_H_

#include <functional>
#include <utility>

#include "compare.h"
#include "../util/exception.h"
#include "../index/cursor.h"

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
                           const std::function<void(RecordCursor &, bool)> &callback) {
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
                if (lhs_type != rhs->value->getType()) {
                    throw SqlDBException("column type mismatch");
                }
            } else {
                throw SqlDBException("invalid expression type");
            }
        } else if (condition->getType() == ConditionType::In) {
            auto condition_ = dynamic_cast<ConditionIn *>(condition);
            // prepare lhs column expression
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            setupCondition(table, lhs, lhs_type, lhs_length,
                           used_columns, used_columns_count);
            for (auto &rhs: condition_->rhs) {
                if (lhs_type != rhs->value->getType()) {
                    throw SqlDBException("column type mismatch");
                }
            }
        } else if (condition->getType() == ConditionType::Like) {
            auto condition_ = dynamic_cast<ConditionLike *>(condition);
            // prepare lhs column expression
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            setupCondition(table, lhs, lhs_type, lhs_length,
                           used_columns, used_columns_count);
            if (lhs_type != ColumnType::VARCHAR) {
                throw SqlDBException("column type mismatch");
            }
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

void conditionalIterRecordWithIndex(Table *table, const std::vector<Condition *> &conditions,
                                    IntIndex *index, int begin, int end,
                                    const std::function<void(RecordCursor &, IntIndexCursor &,
                                                             bool)> &callback) {
    RecordCursor cursor(table);
    IntIndexCursor index_cursor(index);
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
                if (lhs_type != rhs->value->getType()) {
                    throw SqlDBException("column type mismatch");
                }
            } else {
                throw SqlDBException("invalid expression type");
            }
        } else if (condition->getType() == ConditionType::In) {
            auto condition_ = dynamic_cast<ConditionIn *>(condition);
            // prepare lhs column expression
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            setupCondition(table, lhs, lhs_type, lhs_length,
                           used_columns, used_columns_count);
            for (auto &rhs: condition_->rhs) {
                if (lhs_type != rhs->value->getType()) {
                    throw SqlDBException("column type mismatch");
                }
            }
        } else if (condition->getType() == ConditionType::Like) {
            auto condition_ = dynamic_cast<ConditionLike *>(condition);
            // prepare lhs column expression
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            setupCondition(table, lhs, lhs_type, lhs_length,
                           used_columns, used_columns_count);
            if (lhs_type != ColumnType::VARCHAR) {
                throw SqlDBException("column type mismatch");
            }
        }
    }
    condition_values.resize(used_columns_count);
    int key;
    unsigned record_offset;
    bool satisfied;
    index_cursor.find(begin);
    while (index_cursor.next(key, record_offset) && begin <= key && key < end) {
        cursor.moveTo(record_offset);
        for (int i = 0; i < used_columns_count; ++i) {
            condition_values[i] = cursor.get(used_columns[i]);
        }
        satisfied = true;
        for (auto condition: conditions) {
            if (!condition->satisfy(condition_values)) {
                satisfied = false;
                break;
            }
        }
        callback(cursor, index_cursor, satisfied);
        for (auto &condition: condition_values) {
            delete condition;
        }
    }
}

void conditionalSelect(const std::string &table_name,
                       const std::vector<std::tuple<std::string, std::string>> &columns,
                       const std::vector<Condition *> &conditions,
                       bool use_index, const std::string &index_column,
                       const int begin, const int end,
                       Printer *printer) {
    auto table = new Table(table_name);
    std::vector<std::string> printer_headers;
    std::vector<Type *> printer_line;
    int selected_columns[MAX_COLUMN];
    int selected_columns_count = 0;
    // setup printer
    if (std::get<1>(columns[0]) == "*") {
        printer_headers = table->getColumns();
        selected_columns_count = static_cast<int>(printer_headers.size());
        for (int i = 0; i < selected_columns_count; ++i) {
            selected_columns[i] = i;
        }
    } else {
        for (auto &column: columns) {
            int column_index = table->getColumnIndex(std::get<1>(column));
            if (column_index == -1) {
                throw SqlDBException("column not found: " + std::get<1>(column));
            }
            printer_headers.push_back(std::get<1>(column));
            selected_columns[selected_columns_count++] = column_index;
        }
    }
    printer->printHeader(printer_headers);
    printer_line.resize(selected_columns_count);
    if (use_index) {
        auto *index = new IntIndex(table_name, index_column);
        conditionalIterRecordWithIndex(table, conditions, index, begin, end,
                                       [&](RecordCursor &cursor, IntIndexCursor &index_cursor,
                                           bool satisfied) {
                                           if (satisfied) {
                                               for (int i = 0; i < selected_columns_count; ++i) {
                                                   printer_line[i] = cursor.get(
                                                           selected_columns[i]);
                                               }
                                               printer->printLine(printer_line);
                                               for (auto &line: printer_line) {
                                                   delete line;
                                               }
                                           }
                                       });
    } else {
        conditionalIterRecord(table, conditions, [&](RecordCursor &cursor, bool satisfied) {
            if (satisfied) {
                for (int i = 0; i < selected_columns_count; ++i) {
                    printer_line[i] = cursor.get(selected_columns[i]);
                }
                printer->printLine(printer_line);
                for (auto &line: printer_line) {
                    delete line;
                }
            }
        });
    }
    printer->printEnd();
    delete table;
}

void conditionalJoin(const std::string &primary_table, const std::string &secondary_table,
                     const std::vector<std::tuple<std::string, std::string>> &columns,
                     const std::vector<Condition *> &conditions,
                     Printer *printer) {
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
    int primary_condition_columns[MAX_COLUMN];  // map from used column index to condition values index
    int primary_used_columns_count = 0;  // number of used columns
    int secondary_used_columns[MAX_COLUMN];
    int secondary_condition_columns[MAX_COLUMN];
    int secondary_used_columns_count = 0;
    int total_used_columns_count = 0;
    std::vector<std::string> printer_headers;
    std::vector<Type *> printer_line;
    int selected_columns[MAX_COLUMN * 2];  // map from selected column index to table column index
    bool selected_columns_is_primary[MAX_COLUMN * 2];
    int selected_columns_count = 0;
    // setup printer
    if (std::get<1>(columns[0]) == "*") {
        for (auto &column: primary->getColumns()) {
            printer_headers.push_back(primary_table + "." + column);
            selected_columns[selected_columns_count] = primary->getColumnIndex(column);
            selected_columns_is_primary[selected_columns_count++] = true;
        }
        for (auto &column: secondary->getColumns()) {
            printer_headers.push_back(secondary_table + "." + column);
            selected_columns[selected_columns_count] = secondary->getColumnIndex(column);
            selected_columns_is_primary[selected_columns_count++] = false;
        }
    } else {
        for (const auto &column: columns) {
            int column_index;
            if (std::get<0>(column).empty()) {
                int primary_index, secondary_index;
                searchTableColumn(primary, secondary, std::get<1>(column), primary_index,
                                  secondary_index);
                if (primary_index != -1) {
                    printer_headers.push_back(std::get<1>(column) + "." + std::get<1>(column));
                    selected_columns[selected_columns_count] = primary_index;
                    selected_columns_is_primary[selected_columns_count] = true;
                } else {
                    printer_headers.push_back(std::get<1>(column) + "." + std::get<1>(column));
                    selected_columns[selected_columns_count] = secondary_index;
                    selected_columns_is_primary[selected_columns_count] = false;
                }
            } else if (std::get<0>(column) == primary_table) {
                column_index = primary->getColumnIndex(std::get<1>(column));
                if (column_index == -1) {
                    throw SqlDBException("column not found: " + std::get<1>(column));
                }
                printer_headers.push_back(std::get<0>(column) + "." + std::get<1>(column));
                selected_columns[selected_columns_count] = column_index;
                selected_columns_is_primary[selected_columns_count] = true;
            } else if (std::get<0>(column) == secondary_table) {
                column_index = secondary->getColumnIndex(std::get<1>(column));
                if (column_index == -1) {
                    throw SqlDBException("column not found: " + std::get<1>(column));
                }
                printer_headers.push_back(std::get<0>(column) + "." + std::get<1>(column));
                selected_columns[selected_columns_count] = column_index;
                selected_columns_is_primary[selected_columns_count] = false;
            } else {
                throw SqlDBException("table not found: " + std::get<0>(column));
            }
            ++selected_columns_count;
        }
    }
    printer->printHeader(printer_headers);
    printer_line.resize(printer_headers.size());
    // bind condition values to condition expressions
    for (auto condition: conditions) {
        if (condition->getType() == ConditionType::Cmp) {
            auto condition_ = dynamic_cast<ConditionCmp *>(condition);
            // prepare lhs column expression
            ColumnType lhs_type;
            unsigned lhs_length;
            auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
            if (lhs->table == primary_table) {  // TODO combine codes
                setupCondition(primary, lhs, lhs_type, lhs_length,
                               primary_used_columns, primary_condition_columns,
                               primary_used_columns_count, total_used_columns_count);
            } else if (lhs->table == secondary_table) {
                setupCondition(secondary, lhs, lhs_type, lhs_length,
                               secondary_used_columns, secondary_condition_columns,
                               secondary_used_columns_count, total_used_columns_count);
            } else {
                int primary_index, secondary_index;
                searchTableColumn(primary, secondary, lhs->column, primary_index, secondary_index);
                if (primary_index != -1) {
                    setupCondition(primary, lhs, lhs_type, lhs_length,
                                   primary_used_columns, primary_condition_columns,
                                   primary_used_columns_count, total_used_columns_count);
                } else {
                    setupCondition(secondary, lhs, lhs_type, lhs_length,
                                   secondary_used_columns, secondary_condition_columns,
                                   secondary_used_columns_count, total_used_columns_count);
                }
            }
            if (condition_->rhs->getType() == ExpressionType::COLUMN) {
                // prepare lhs column expression
                auto rhs = dynamic_cast<ExprColumn *>(condition_->rhs);
                if (rhs->table == primary_table) {
                    setupCondition(primary, rhs, primary_used_columns, primary_condition_columns,
                                   primary_used_columns_count, total_used_columns_count);
                } else if (rhs->table == secondary_table) {
                    setupCondition(secondary, rhs, secondary_used_columns,
                                   secondary_condition_columns,
                                   secondary_used_columns_count, total_used_columns_count);
                } else {
                    int primary_index, secondary_index;
                    searchTableColumn(primary, secondary, rhs->column, primary_index,
                                      secondary_index);
                    if (primary_index != -1) {
                        setupCondition(primary, rhs, primary_used_columns,
                                       primary_condition_columns,
                                       primary_used_columns_count, total_used_columns_count);
                    } else {
                        setupCondition(secondary, rhs, secondary_used_columns,
                                       secondary_condition_columns,
                                       secondary_used_columns_count, total_used_columns_count);
                    }
                }
            } else if (condition_->rhs->getType() == ExpressionType::VALUE) {
                auto rhs = dynamic_cast<ExprValue *>(condition_->rhs);
                if (lhs_type != rhs->value->getType()) {
                    throw SqlDBException("column type mismatch");
                }
            } else {
                throw SqlDBException("invalid expression type");
            }
        } else if (condition->getType() == ConditionType::In) {
            // TODO condition in
        }
    }
    condition_values.resize(total_used_columns_count);
    while (primary_cursor.next()) {
        // read primary table values
        for (int i = 0; i < primary_used_columns_count; ++i) {
            condition_values[primary_condition_columns[i]] = primary_cursor.get(
                    primary_used_columns[i]);
        }
        while (secondary_cursor.next()) {
            // read secondary table values
            for (int i = 0; i < secondary_used_columns_count; ++i) {
                condition_values[secondary_condition_columns[i]] = secondary_cursor.get(
                        secondary_used_columns[i]);
            }
            bool satisfied = true;
            for (auto condition: conditions) {
                if (!condition->satisfy(condition_values)) {
                    satisfied = false;
                    break;
                }
            }
            if (satisfied) {
                // send to printer or temp table
                for (int i = 0; i < selected_columns_count; ++i) {
                    printer_line[i] = selected_columns_is_primary[i]
                                      ? primary_cursor.get(selected_columns[i])
                                      : secondary_cursor.get(selected_columns[i]);
                }
                printer->printLine(printer_line);
                for (auto &line: printer_line) {
                    delete line;
                }
            }
            for (int i = 0; i < secondary_used_columns_count; ++i) {
                delete condition_values[secondary_condition_columns[i]];
            }
        }
        secondary_cursor.reset();
        for (int i = 0; i < primary_used_columns_count; ++i) {
            delete condition_values[primary_condition_columns[i]];
        }
//        for (auto &condition: condition_values) {
//            delete condition;
//        }
//        for (auto &line: printer_line) {
//            delete line;
//        }
    }
    printer->printEnd();
    delete primary;
    delete secondary;
}

void checkIndexOnCondition(std::string table_name, std::vector<Condition *> &conditions,
                           bool &use_index, std::string &index_column, int &begin, int &end) {
    // check if index can be used
    auto table = new Table(std::move(table_name));
    use_index = false;
    for (auto condition: conditions) {
        if (condition->getType() == ConditionType::Cmp) {
            auto condition_ = dynamic_cast<ConditionCmp *>(condition);
            if (condition_->lhs->getType() == ExpressionType::COLUMN &&
                condition_->rhs->getType() == ExpressionType::VALUE) {
                auto lhs = dynamic_cast<ExprColumn *>(condition_->lhs);
                auto rhs = dynamic_cast<ExprValue *>(condition_->rhs);
                int column_index = table->getColumnIndex(lhs->column);
                if (column_index == -1) {
                    throw SqlDBException("column not found: " + lhs->column);
                }
                if (table->hasIndex(column_index)) {
                    // use index
                    use_index = true;
                    index_column = lhs->column;
                    switch (condition_->op->getType()) {
                        case CompareType::EQ:
                            begin = dynamic_cast<Int *>(rhs->value)->getValue();  // rhs should only be int
                            end = begin + 1;
                            break;
                        case CompareType::LT:
                            begin = MIN_INT;
                            end = dynamic_cast<Int *>(rhs->value)->getValue();
                            break;
                        case CompareType::LE:
                            begin = MIN_INT;
                            end = dynamic_cast<Int *>(rhs->value)->getValue() + 1;
                            break;
                        case CompareType::GT:
                            begin = dynamic_cast<Int *>(rhs->value)->getValue() + 1;
                            end = MAX_INT;
                            break;
                        case CompareType::GE:
                            begin = dynamic_cast<Int *>(rhs->value)->getValue();
                            end = MAX_INT;
                            break;
                        default:
                            use_index = false;
                            break;
                    }
                    if (use_index) {
                        conditions.erase(
                                std::remove(conditions.begin(), conditions.end(), condition),
                                conditions.end());
                        return;
                    }
                }
            }
        }
    }
    delete table;
}

#endif  // OP_UTIL_H_