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
    OpType _type;
    Op *_next = nullptr;

public:
    explicit Op(OpType type) : _type(type) {}

    virtual ~Op() { delete _next; }

    [[nodiscard]] OpType getType() const { return this->_type; }

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
    explicit OpDbCreate(std::string name) : Op(OpType::DB_CREATE), name(std::move(name)) {}

    std::string getDbName() { return this->name; }
};

class OpDbUse : public Op {
private:
    std::string name;

public:
    explicit OpDbUse(std::string name) : Op(OpType::DB_USE), name(std::move(name)) {}

    std::string getDbName() { return this->name; }
};

class OpTableCreate : public Op {
private:
    std::string name;
    std::vector<Column> columns;

public:
    OpTableCreate(std::string name, std::vector<Column> columns)
            : Op(OpType::TABLE_CREATE), name(std::move(name)), columns(std::move(columns)) {}

    std::string getTableName() { return this->name; }

    std::vector<Column> getTableColumns() { return this->columns; }
};

class OpTableInsert : public Op {
private:
    std::string name;
    std::vector<std::vector<std::string>> values;

public:
    explicit OpTableInsert(std::string name, std::vector<std::vector<std::string>> values)
            : Op(OpType::TABLE_INSERT), name(std::move(name)), values(std::move(values)) {}

    std::string getTableName() { return this->name; }

    std::vector<std::vector<std::string>> getValues() { return this->values; }
};

class OpTableSelect : public Op {
private:
    std::vector<std::string> selectors;
    std::vector<std::string> tables;
    std::vector<Condition *> conditions;

public:
    OpTableSelect(std::vector<std::string> selectors, std::vector<std::string> tables,
                  std::vector<Condition *> conditions)
            : Op(OpType::TABLE_SELECT), selectors(std::move(selectors)),
              tables(std::move(tables)), conditions(std::move(conditions)) {}

    ~OpTableSelect() override {
        for (auto condition: this->conditions) {
            delete condition;
        }
    }

    std::vector<std::string> getSelectors() { return this->selectors; }

    std::vector<std::string> getTableNames() { return this->tables; }

    std::vector<Condition *> getConditions() { return this->conditions; }
};

class OpUnknown : public Op {
public:
    explicit OpUnknown() : Op(OpType::UNKNOWN) {}
};


#endif  // OP_H_
