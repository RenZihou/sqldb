// compare.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef COMPARE_H_
#define COMPARE_H_

#include <vector>
#include <algorithm>
#include <regex>

#include "../table/type.h"
#include "../table/type.h"
#include "../table/cursor.h"

enum class CompareType {
    EQ, NE, LT, LE, GT, GE, IN
};

struct CmpOp {
    CmpOp() = default;

    virtual ~CmpOp() = default;

    [[nodiscard]] virtual CompareType getType() const = 0;

    virtual bool operator()(const Type *lhs, const Type *rhs) = 0;
};

struct Equal : public CmpOp {
    [[nodiscard]] CompareType getType() const override { return CompareType::EQ; }

    bool operator()(const Type *lhs, const Type *rhs) override { return *lhs == *rhs; }
};

struct Greater : public CmpOp {
    [[nodiscard]] CompareType getType() const override { return CompareType::GT; }

    bool operator()(const Type *lhs, const Type *rhs) override { return *lhs > *rhs; }
};

struct Less : public CmpOp {
    [[nodiscard]] CompareType getType() const override { return CompareType::LT; }

    bool operator()(const Type *lhs, const Type *rhs) override { return *lhs < *rhs; }
};

struct GreaterEqual : public CmpOp {
    [[nodiscard]] CompareType getType() const override { return CompareType::GE; }

    bool operator()(const Type *lhs, const Type *rhs) override { return *lhs >= *rhs; }
};

struct LessEqual : public CmpOp {
    [[nodiscard]] CompareType getType() const override { return CompareType::LE; }

    bool operator()(const Type *lhs, const Type *rhs) override { return *lhs <= *rhs; }
};

struct NotEqual : public CmpOp {
    [[nodiscard]] CompareType getType() const override { return CompareType::NE; }

    bool operator()(const Type *lhs, const Type *rhs) override { return *lhs != *rhs; }
};

enum class ExpressionType {
    COLUMN,
    VALUE,
};

struct Expression {
    virtual ~Expression() = default;

    [[nodiscard]] virtual ExpressionType getType() const = 0;

    virtual Type *pick(const std::vector<Type *> &values) = 0;
};

struct ExprValue : public Expression {
    Type *value;

    explicit ExprValue(Type *value) : value(value) {}

    ~ExprValue() override { delete value; }

    [[nodiscard]] ExpressionType getType() const override { return ExpressionType::VALUE; }

    Type *pick(const std::vector<Type *> &values) override { return value; }
};

struct ExprColumn : public Expression {
    std::string table;
    std::string column;
    int value_index;

    ExprColumn(std::string table, std::string column) : table(std::move(table)),
                                                        column(std::move(column)),
                                                        value_index(-1) {}

    [[nodiscard]] ExpressionType getType() const override { return ExpressionType::COLUMN; }

    Type *pick(const std::vector<Type *> &values) override { return values[value_index]; }
};

enum class ConditionType {
    Cmp,
    In,
    Like,
};

struct Condition {
    virtual ~Condition() = default;

    virtual ConditionType getType() = 0;

    [[nodiscard]] virtual bool satisfy(const std::vector<Type *> &values) const = 0;
};

struct ConditionCmp : public Condition {
    Expression *lhs;
    Expression *rhs;
    CmpOp *op;

    ConditionCmp(Expression *lhs, Expression *rhs, CmpOp *op) : lhs(lhs), rhs(rhs), op(op) {}

    ~ConditionCmp() override {
        delete lhs;
        delete rhs;
        delete op;
    }

    ConditionType getType() override { return ConditionType::Cmp; }

    [[nodiscard]] bool satisfy(const std::vector<Type *> &values) const override {
        return (*this->op)(this->lhs->pick(values), this->rhs->pick(values));
    }
};

struct ConditionIn : public Condition {
    Expression *lhs;
    std::vector<ExprValue *> rhs;

    ConditionIn(Expression *lhs, std::vector<ExprValue *> rhs) : lhs(lhs), rhs(std::move(rhs)) {}

    ~ConditionIn() override {
        delete lhs;
        for (auto &i : rhs) {
            delete i;
        }
    }

    ConditionType getType() override { return ConditionType::In; }

    [[nodiscard]] bool satisfy(const std::vector<Type *> &values) const override {
        return std::any_of(this->rhs.begin(), this->rhs.end(), [&](ExprValue *r) {
            return Equal()(this->lhs->pick(values), r->value);
        });
    }
};

struct ConditionLike : Condition {
    Expression *lhs;
    std::regex pattern;

    ConditionLike(Expression *lhs, const std::string &like_pattern) : lhs(lhs) {
        std::regex reserved(R"([.*+?^=!:${}()|\[\]\/\\])");
        std::string re_pattern = std::regex_replace(like_pattern, reserved, R"(\$&)");
        re_pattern = std::regex_replace(re_pattern, std::regex(R"(%)"), ".*?");
        re_pattern = std::regex_replace(re_pattern, std::regex(R"(_)"), ".");
        this->pattern = std::regex(re_pattern);
    }

    ~ConditionLike() override {
        delete lhs;
    }

    ConditionType getType() override { return ConditionType::Like; }

    [[nodiscard]] bool satisfy(const std::vector<Type *> &values) const override {
        return std::regex_match(this->lhs->pick(values)->toString(), this->pattern);
    }
};

#endif  // COMPARE_H_
