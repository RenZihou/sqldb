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
#include "printer.h"
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
    TABLE_ALTER_ADD_INDEX,
    TABLE_ALTER_DROP_INDEX,
    TABLE_ALTER_ADD_PK,
    TABLE_ALTER_DROP_PK,
    UNKNOWN
};

class Op {
protected:
    Op *_next = nullptr;

public:
    Op() = default;

    virtual ~Op() { delete _next; }

    [[nodiscard]] virtual OpType getType() const { return OpType::UNKNOWN; }

    Op *setNext(Op *next) {
        this->_next = next;
        return this->_next;
    }

    [[nodiscard]] Op *getNext() const { return this->_next; }

    virtual void execute(Printer *printer);
};

class OpDbCreate : public Op {
private:
    std::string name;

public:
    explicit OpDbCreate(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::DB_CREATE; }

    void execute(Printer *printer) override;
};

class OpDbDrop : public Op {
private:
    std::string name;

public:
    explicit OpDbDrop(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::DB_DROP; }

    void execute(Printer *printer) override;
};

class OpDbShow : public Op {
public:
    [[nodiscard]] OpType getType() const override { return OpType::DB_SHOW; }

    void execute(Printer *printer) override;
};

class OpDbUse : public Op {
private:
    std::string name;

public:
    explicit OpDbUse(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::DB_USE; }

    void execute(Printer *printer) override;
};

class OpDbShowTables : public Op {
public:
    [[nodiscard]] OpType getType() const override { return OpType::DB_SHOW_TABLES; }

    void execute(Printer *printer) override;
};

class OpTableCreate : public Op {
private:
    std::string name;
    std::vector<Column> columns;
    std::vector<std::tuple<std::string, std::vector<std::string>>> primary_keys;

public:
    OpTableCreate(std::string name, std::vector<Column> columns, std::vector<std::tuple<std::string, std::vector<std::string>>> primary_keys)
            : Op(), name(std::move(name)), columns(std::move(columns)), primary_keys(std::move(primary_keys)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_CREATE; }

    void execute(Printer *printer) override;
};

class OpTableDrop : public Op {
private:
    std::string name;

public:
    explicit OpTableDrop(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_DROP; }

    void execute(Printer *printer) override;
};

class OpTableDescribe : public Op {
private:
    std::string name;

public:
    explicit OpTableDescribe(std::string name) : Op(), name(std::move(name)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_DESC; }

    void execute(Printer *printer) override;
};

class OpTableInsert : public Op {
private:
    std::string name;
    std::vector<std::vector<std::string>> values;

public:
    explicit OpTableInsert(std::string name, std::vector<std::vector<std::string>> values)
            : Op(), name(std::move(name)), values(std::move(values)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_INSERT; }

    void execute(Printer *printer) override;
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

    void execute(Printer *printer) override;
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

    void execute(Printer *printer) override;
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

    void execute(Printer *printer) override;
};

class OpTableAlterAddIndex : public Op {
private:
    std::string name;
    std::vector<std::string> columns;

public:
    OpTableAlterAddIndex(std::string name, std::vector<std::string> columns)
            : Op(), name(std::move(name)), columns(std::move(columns)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_ALTER_ADD_INDEX; }

    void execute(Printer *printer) override;
};

class OpTableAlterDropIndex : public Op {
private:
    std::string name;
    std::vector<std::string> columns;

public:
    OpTableAlterDropIndex(std::string name, std::vector<std::string> columns)
            : Op(), name(std::move(name)), columns(std::move(columns)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_ALTER_DROP_INDEX; }

    void execute(Printer *printer) override;
};

class OpTableAlterAddPk : public Op {
private:
    std::string name;
    std::string pk;
    std::vector<std::string> columns;

public:
    OpTableAlterAddPk(std::string name, std::string pk, std::vector<std::string> columns)
            : Op(), name(std::move(name)), pk(std::move(pk)), columns(std::move(columns)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_ALTER_ADD_PK; }

    void execute(Printer *printer) override;
};

class OpTableAlterDropPk : public Op {
private:
    std::string name;
    std::string pk;

public:
    OpTableAlterDropPk(std::string name, std::string pk)
            : Op(), name(std::move(name)), pk(std::move(pk)) {}

    [[nodiscard]] OpType getType() const override { return OpType::TABLE_ALTER_DROP_PK; }

    void execute(Printer *printer) override;
};

#endif  // OP_H_
