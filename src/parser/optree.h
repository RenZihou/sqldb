// optree.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef OPTREE_H_
#define OPTREE_H_

#include <string>
#include <utility>

enum OpType {
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
    OpType _type = OpType::UNKNOWN;
    Op *_next = nullptr;
public:
    Op() = default;
    virtual ~Op() = default;
    OpType getType() const { return this->_type; }
    Op *setNext(Op *next) { this->_next = next; return this->_next; }
    Op *getNext() const { return this->_next; }
};

class OpDbCreate : public Op {
private:
    std::string name;
public:
    explicit OpDbCreate(std::string name) : Op(), name(std::move(name)) { this->_type = OpType::DB_CREATE; }
    std::string getDbName() { return this->name; }
};

class OpDbUse : public Op {
private:
    std::string name;
public:
    explicit OpDbUse(std::string name) : Op(), name(std::move(name)) { this->_type = OpType::DB_USE; }
    std::string getDbName() { return this->name; }
};

class OpUnknown : public Op {
public:
    explicit OpUnknown() : Op() { this->_type = OpType::UNKNOWN; }
};


#endif  // OPTREE_H_
