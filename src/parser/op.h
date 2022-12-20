// optree.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef OP_H_
#define OP_H_

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "compare.h"
#include "../table/table.h"

enum class OpType {
    DB_CREATE,
    DB_DROP,
    DB_SHOW,
    DB_USE,
    DB_SHOW_TABLES,
    DB_SHOW_INDEX,
    TABLE_CREATE,
    TABLE_DROP,
    TABLE_DESC,
    TABLE_INSERT,
    TABLE_DELETE,
    TABLE_UPDATE,
    TABLE_SELECT,
    UNKNOWN
};

class Op {
protected:
    Op *_next = nullptr;

public:
    Op() = default;

    virtual ~Op() { delete _next; }

    [[nodiscard]] virtual OpType getType() const = 0;

    Op *setNext(Op *next) {
        this->_next = next;
        return this->_next;
    }

    [[nodiscard]] Op *getNext() const { return this->_next; }
};

class OpDbCreate : public Op {
private:
    std::string name;

public:
    explicit OpDbCreate(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::DB_CREATE; }

    std::string getDbName() { return this->name; }
};

class OpDbUse : public Op {
private:
    std::string name;

public:
    explicit OpDbUse(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::DB_USE; }

    std::string getDbName() { return this->name; }
};

class OpTableCreate : public Op {
private:
    std::string name;
    std::vector<Column> columns;

public:
    OpTableCreate(std::string name, std::vector<Column> columns)
            : Op(), name(std::move(name)), columns(std::move(columns)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_CREATE; }

    std::string getTableName() { return this->name; }

    std::vector<Column> getTableColumns() { return this->columns; }
};

class OpTableInsert : public Op {
private:
    std::string name;
    std::vector<std::vector<std::string>> values;

public:
    explicit OpTableInsert(std::string name, std::vector<std::vector<std::string>> values)
            : Op(), name(std::move(name)), values(std::move(values)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_INSERT; }

    std::string getTableName() { return this->name; }

    std::vector<std::vector<std::string>> getValues() { return this->values; }
};

class OpTableDelete : public Op {
private:
    std::string name;
    std::vector<Condition *> conditions;

public:
    explicit OpTableDelete(std::string name, std::vector<Condition *> conditions)
            : Op(), name(std::move(name)), conditions(std::move(conditions)) {}

    ~OpTableDelete() override {
        for (auto &condition: conditions) {
            delete condition;
        }
    }

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_DELETE; }

    std::string getTableName() { return this->name; }

    std::vector<Condition *> getConditions() { return this->conditions; }
};

class OpTableUpdate : public Op {
private:
    std::string name;
    std::vector<std::tuple<std::string, std::string>> updates;
    std::vector<Condition *> conditions;

public:
    explicit OpTableUpdate(std::string name,
                           std::vector<std::tuple<std::string, std::string>> updates,
                           std::vector<Condition *> conditions)
            : Op(), name(std::move(name)), updates(std::move(updates)),
              conditions(std::move(conditions)) {}

    ~OpTableUpdate() override {
        for (auto &condition: conditions) {
            delete condition;
        }
    }

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_UPDATE; }

    std::string getTableName() { return this->name; }

    std::vector<std::tuple<std::string, std::string>> getUpdates() { return this->updates; }

    std::vector<Condition *> getConditions() { return this->conditions; }
};

class OpTableSelect : public Op {
private:
    std::vector<std::tuple<std::string, std::string>> selectors;
    std::vector<std::string> tables;
    std::vector<Condition *> conditions;

public:
    OpTableSelect(std::vector<std::tuple<std::string, std::string>> selectors,
                  std::vector<std::string> tables,
                  std::vector<Condition *> conditions)
            : Op(), selectors(std::move(selectors)),
              tables(std::move(tables)), conditions(std::move(conditions)) {}

    ~OpTableSelect() override {
        for (auto condition: this->conditions) {
            delete condition;
        }
    }

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_SELECT; }

    std::vector<std::tuple<std::string, std::string>> getSelectors() { return this->selectors; }

    std::vector<std::string> getTableNames() { return this->tables; }

    std::vector<Condition *> getConditions() { return this->conditions; }
};

class OpUnknown : public Op {
public:
    explicit OpUnknown() : Op() {}

    [[nodiscard]] OpType getType() const override { return OpType::UNKNOWN; }
};


#endif  // OP_H_
