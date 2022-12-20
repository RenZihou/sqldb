// visitor.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "visitor.h"
#include "op.h"
#include "compare.h"

std::any Visitor::visitProgram(SQLParser::ProgramContext *ctx) {
    Op *root = new OpUnknown();
    Op *top = root;
    for (auto &stmt: ctx->statement()) {
        top = top->setNext(std::any_cast<Op *>(visit(stmt)));
    }
    Op *ret = root->getNext();
    root->setNext(nullptr);
    delete root;
    return ret;
}

std::any Visitor::visitStatement(SQLParser::StatementContext *ctx) {
    if (ctx->dbStatement()) return visit(ctx->dbStatement());
    else if (ctx->tableStatement()) return visit(ctx->tableStatement());
    else if (ctx->alterStatement()) return visit(ctx->alterStatement());
    auto *op = new OpUnknown();
    return op;
}

std::any Visitor::visitCreateDb(SQLParser::CreateDbContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto *op = new OpDbCreate(name);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitCreateTable(SQLParser::CreateTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto columns = std::any_cast<std::vector<Column>>(visit(ctx->fieldList()));
    auto *op = new OpTableCreate(name, columns);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitInsertIntoTable(SQLParser::InsertIntoTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto values = std::any_cast<std::vector<std::vector<std::string>>>(visit(ctx->valueLists()));
    auto *op = new OpTableInsert(name, values);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitDeleteFromTable(SQLParser::DeleteFromTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto wheres = std::any_cast<std::vector<Condition *>>(visit(ctx->whereAndClause()));
    auto *op = new OpTableDelete(name, wheres);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitUpdateTable(SQLParser::UpdateTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto sets = std::any_cast<std::vector<std::tuple<std::string, std::string>>>(visit(ctx->setClause()));
    auto wheres = std::any_cast<std::vector<Condition *>>(visit(ctx->whereAndClause()));
    auto *op = new OpTableUpdate(name, sets, wheres);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitSelectTable_(SQLParser::SelectTable_Context *ctx) {
    return visit(ctx->selectTable());
}

std::any Visitor::visitSelectTable(SQLParser::SelectTableContext *ctx) {
    auto selectors = std::any_cast<std::vector<std::tuple<std::string, std::string>>>(
            visit(ctx->selectors()));
    auto tables = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
    std::vector<Condition *> wheres;
    if (ctx->whereAndClause()) {
        wheres = std::any_cast<std::vector<Condition *>>(
                visit(ctx->whereAndClause()));  // TODO where
    }
    auto *op = new OpTableSelect(selectors, tables, wheres);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitFieldList(SQLParser::FieldListContext *ctx) {
    std::vector<Column> fields;
    for (auto &field: ctx->field()) {
        fields.push_back(std::any_cast<Column>(visit(field)));
    }
    return fields;
}

std::any Visitor::visitNormalField(SQLParser::NormalFieldContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    char flags = 0;
    flags |= ctx->value() ? FLAG_HAS_DEFAULT : 0;
    flags |= ctx->Null() ? FLAG_NOT_NULL : 0;
    auto type = std::any_cast<std::tuple<ColumnType, unsigned>>(visit(ctx->type()));
    std::string default_value;
    if (ctx->value() != nullptr) {  // default value
        default_value = std::any_cast<std::string>(visit(ctx->value()));
    }
    return Column{.name=name, .type=std::get<0>(type), .length=std::get<1>(type),
            .flags=flags, .default_value=default_value};
}

std::any Visitor::visitType(SQLParser::TypeContext *ctx) {
    if (ctx->getStart()->getText() == "INT")
        return std::make_tuple(ColumnType::INT, 4u);
    if (ctx->getStart()->getText() == "VARCHAR")
        return std::make_tuple(ColumnType::VARCHAR,
                               static_cast<unsigned>(std::stoi(ctx->Integer()->getText())));
    if (ctx->getStart()->getText() == "FLOAT")
        return std::make_tuple(ColumnType::FLOAT, 4u);
    return std::make_tuple(ColumnType::UNKNOWN, 0u);
}

std::any Visitor::visitValueLists(SQLParser::ValueListsContext *ctx) {
    std::vector<std::vector<std::string>> values;
    for (auto &valueList: ctx->valueList()) {
        values.push_back(std::any_cast<std::vector<std::string>>(visit(valueList)));
    }
    return values;
}

std::any Visitor::visitValueList(SQLParser::ValueListContext *ctx) {
    std::vector<std::string> values;
    for (auto &value: ctx->value()) {
        values.push_back(std::any_cast<std::string>(visit(value)));
    }
    return values;
}

std::any Visitor::visitValue(SQLParser::ValueContext *ctx) {
    if (ctx->Integer() != nullptr) {
        return ctx->Integer()->getText();
    } else if (ctx->Float() != nullptr) {
        return ctx->Float()->getText();
    } else if (ctx->String() != nullptr) {
        std::string with_quote = ctx->String()->getText();
        return with_quote.substr(1, with_quote.length() - 2);
    } else if (ctx->Null() != nullptr) {
        return "";
    }
    return "";
}

std::any Visitor::visitWhereAndClause(SQLParser::WhereAndClauseContext *ctx) {
    std::vector<Condition *> conditions;
    for (auto &where: ctx->whereClause()) {
        conditions.push_back(std::any_cast<Condition *>(visit(where)));
    }
    return conditions;
}

std::any Visitor::visitWhereOperatorExpression(SQLParser::WhereOperatorExpressionContext *ctx) {
    auto lhs = std::any_cast<std::tuple<std::string, std::string>>(visit(ctx->column()));
    auto rhs = std::any_cast<std::tuple<std::string, std::string>>(visit(ctx->expression()));
    auto op = std::any_cast<CmpOp *>(visit(ctx->operator_()));
    if (std::get<1>(rhs).empty()) {  // value
        return dynamic_cast<Condition *>(
                new ConditionCmp(new ExprColumn(std::get<0>(lhs), std::get<1>(lhs)),
                                 new ExprValue(std::get<0>(rhs)), op));
    } else {  // column
        return dynamic_cast<Condition *>(
                new ConditionCmp(new ExprColumn(std::get<0>(lhs), std::get<1>(lhs)),
                                 new ExprColumn(std::get<0>(rhs), std::get<1>(rhs)), op));
    }
}

std::any Visitor::visitColumn(SQLParser::ColumnContext *ctx) {
    if (ctx->Identifier().size() == 1) {  // only column name
        return std::make_tuple<std::string, std::string>("", ctx->Identifier(0)->getText());
    } else {  // table name and column name
        return std::make_tuple<std::string, std::string>(ctx->Identifier(0)->getText(),
                                                         ctx->Identifier(1)->getText());
    }
}

std::any Visitor::visitExpression(SQLParser::ExpressionContext *ctx) {
    if (ctx->value()) {
        return std::make_tuple<std::string, std::string>(
                std::any_cast<std::string>(visit(ctx->value())), "");
    } else {  // column
        return visit(ctx->column());
    }
}

std::any Visitor::visitSetClause(SQLParser::SetClauseContext *ctx) {
    std::vector<std::tuple<std::string, std::string>> sets;
    unsigned num = ctx->value().size();
    for (unsigned i = 0; i < num; ++i) {
        auto column = ctx->Identifier(i)->getText();
        auto value = std::any_cast<std::string>(visit(ctx->value(i)));
        sets.emplace_back(column, value);
    }
    return sets;
}

std::any Visitor::visitSelectors(SQLParser::SelectorsContext *ctx) {
    std::vector<std::tuple<std::string, std::string>> selectors;
    if (ctx->getStart()->getText() == "*") {
        selectors.push_back(std::make_tuple<std::string, std::string>("", "*"));
        return selectors;
    }
    for (auto &selector: ctx->selector()) {
        selectors.push_back(std::any_cast<std::tuple<std::string, std::string>>(visit(selector)));
    }
    return selectors;
}

std::any Visitor::visitSelector(SQLParser::SelectorContext *ctx) {
    return visit(ctx->column());
}

std::any Visitor::visitIdentifiers(SQLParser::IdentifiersContext *ctx) {
    std::vector<std::string> identifiers;
    for (auto &ident: ctx->Identifier()) {
        identifiers.push_back(ident->getSymbol()->getText());

    }
    return identifiers;
}

std::any Visitor::visitOperator_(SQLParser::Operator_Context *ctx) {
    if (ctx->getStart()->getText() == "=") {
        return dynamic_cast<CmpOp *>(new Equal());
    } else if (ctx->getStart()->getText() == ">") {
        return dynamic_cast<CmpOp *>(new Greater());
    } else if (ctx->getStart()->getText() == "<") {
        return dynamic_cast<CmpOp *>(new Less());
    } else if (ctx->getStart()->getText() == ">=") {
        return dynamic_cast<CmpOp *>(new GreaterEqual());
    } else if (ctx->getStart()->getText() == "<=") {
        return dynamic_cast<CmpOp *>(new LessEqual());
    } else if (ctx->getStart()->getText() == "<>") {
        return dynamic_cast<CmpOp *>(new NotEqual());
    }
    return nullptr;
}