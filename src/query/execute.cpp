// execute.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "execute.h"

inline void setupCondition(Table *table, ExprColumn *expr,
                           int *table_used_columns, int &table_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    expr->value_index = table_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    ++table_used_columns_count;
}

inline void setupCondition(Table *table, ExprColumn *expr,
                           int *table_used_columns, int *table_condition_columns,
                           int &table_used_columns_count, int &total_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    expr->value_index = total_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    table_condition_columns[table_used_columns_count] = total_used_columns_count;
    ++table_used_columns_count;
    ++total_used_columns_count;
}

inline void setupCondition(Table *table, ExprColumn *expr, ColumnType &type, unsigned &length,
                           int *table_used_columns, int &table_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
    type = table->getColumnType(col_index);
    length = table->getColumnLength(col_index);
    expr->value_index = table_used_columns_count;
    table_used_columns[table_used_columns_count] = col_index;
    ++table_used_columns_count;
}

inline void setupCondition(Table *table, ExprColumn *expr, ColumnType &type, unsigned &length,
                           int *table_used_columns, int *table_condition_columns,
                           int &table_used_columns_count, int &total_used_columns_count) {
    int col_index = table->getColumnIndex(expr->column);
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
            throw std::runtime_error("column `" + column + "` not found");
        }
    }
}

void conditionalSelect(const std::string &table_name,
                       const std::vector<std::tuple<std::string, std::string>> &columns,
                       const std::vector<Condition *> &conditions,
                       Printer &printer) {
    auto table = new Table(table_name);
    RecordCursor cursor(table);
    std::vector<Type *> condition_values;
    int used_columns[MAX_COLUMN];
    int used_columns_count = 0;
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
                throw std::runtime_error("column `" + std::get<1>(column) + "` not found");
            }
            printer_headers.push_back(std::get<1>(column));
            selected_columns[selected_columns_count++] = column_index;
        }
    }
    printer.printHeader(printer_headers);
    printer_line.reserve(selected_columns_count);
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
                throw std::runtime_error("invalid expression type");  // TODO custom error
            }
        } else {
            // TODO
        }
    }
    condition_values.reserve(used_columns_count);
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
        if (satisfied) {
            for (int i = 0; i < selected_columns_count; ++i) {
                printer_line[i] = cursor.get(selected_columns[i]);
            }
            printer.printLine(printer_line);
        }
        for (auto &condition: condition_values) {
            delete condition;
        }
        for (auto &line: printer_line) {
            delete line;
        }
    }
    printer.printEnd();
    delete table;
}

void conditionalJoin(const std::string &primary_table, const std::string &secondary_table,
                     const std::vector<std::tuple<std::string, std::string>> &columns,
                     const std::vector<Condition *> &conditions,
                     Printer &printer) {
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
                    throw std::runtime_error("column `" + std::get<1>(column) + "` not found");
                }
                printer_headers.push_back(std::get<0>(column) + "." + std::get<1>(column));
                selected_columns[selected_columns_count] = column_index;
                selected_columns_is_primary[selected_columns_count] = true;
            } else if (std::get<0>(column) == secondary_table) {
                column_index = secondary->getColumnIndex(std::get<1>(column));
                if (column_index == -1) {
                    throw std::runtime_error("column `" + std::get<1>(column) + "` not found");
                }
                printer_headers.push_back(std::get<0>(column) + "." + std::get<1>(column));
                selected_columns[selected_columns_count] = column_index;
                selected_columns_is_primary[selected_columns_count] = false;
            } else {
                throw std::runtime_error("table `" + std::get<0>(column) + "` not found");
            }
            ++selected_columns_count;
        }
    }
    printer.printHeader(printer_headers);
    printer_line.reserve(printer_headers.size());
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
                auto buffer = new unsigned char[lhs_length];
                serializeFromString(rhs->value_s, lhs_type, buffer, lhs_length);
                rhs->value = deserialize(buffer, lhs_type, lhs_length);
                delete[] buffer;
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
                printer.printLine(printer_line);
            }
        }
        secondary_cursor.reset();
        for (auto &condition: condition_values) {
            delete condition;
        }
        for (auto &line: printer_line) {
            delete line;
        }
    }
    printer.printEnd();
    delete primary;
    delete secondary;
}

void execute(Op *op, Printer &printer) {
    if (op->getType() == OpType::DB_CREATE) {
        // TODO
    } else if (op->getType() == OpType::TABLE_CREATE) {
        auto *op_ = dynamic_cast<OpTableCreate *>(op);
        auto table = Table::createTable(op_->getTableName());
//        auto table = new Table(op_->getTableName());
        auto columns = op_->getTableColumns();
        for (auto it = columns.rbegin(); it != columns.rend(); ++it) {
            table->addColumn(*it, "");
        }  // range based inverse loop is not supported by gcc for now
        delete table;
    } else if (op->getType() == OpType::TABLE_INSERT) {
        auto *op_ = dynamic_cast<OpTableInsert *>(op);
        auto table = new Table(op_->getTableName());
        auto values = op_->getValues();
        for (auto &value: values) {
            table->insertRecord(value);
        }
        delete table;
    } else if (op->getType() == OpType::TABLE_SELECT) {
        auto *op_ = dynamic_cast<OpTableSelect *>(op);
        switch (op_->getTableNames().size()) {
            case 1:
                conditionalSelect(op_->getTableNames()[0], op_->getSelectors(),
                                  op_->getConditions(), printer);
                break;
            case 2:
                conditionalJoin(op_->getTableNames()[0], op_->getTableNames()[1],
                                op_->getSelectors(), op_->getConditions(), printer);
                break;
            default:
                throw std::runtime_error("invalid table count");  // TODO multi-table select
        }
    }
}
