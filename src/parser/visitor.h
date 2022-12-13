// visitors.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef VISITOR_H_
#define VISITOR_H_

#include "SQLBaseVisitor.h"
#include "optree.h"

class Visitor : public SQLBaseVisitor {
public:
    std::any visitProgram(SQLParser::ProgramContext *ctx) override;

    std::any visitStatement(SQLParser::StatementContext *ctx) override;

    std::any visitCreateDb(SQLParser::CreateDbContext *ctx) override;

    std::any visitCreateTable(SQLParser::CreateTableContext *ctx) override;

    std::any visitSelectTable(SQLParser::SelectTableContext *ctx) override;

    std::any visitFieldList(SQLParser::FieldListContext *ctx) override;

    std::any visitNormalField(SQLParser::NormalFieldContext *ctx) override;

    std::any visitType(SQLParser::TypeContext *ctx) override;

    std::any visitValue(SQLParser::ValueContext *ctx) override;

    std::any visitSelectors(SQLParser::SelectorsContext *ctx) override;

    std::any visitSelector(SQLParser::SelectorContext *ctx) override;

    std::any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override;
};

#endif  // VISITOR_H_