// visitors.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef VISITOR_H_
#define VISITOR_H_

#include "SQLBaseVisitor.h"

class Visitor : public SQLBaseVisitor {
public:
    std::any visitProgram(SQLParser::ProgramContext *ctx) override;

    std::any visitStatement(SQLParser::StatementContext *ctx) override;

    std::any visitCreateDb(SQLParser::CreateDbContext *ctx) override;

    std::any visitCreateTable(SQLParser::CreateTableContext *ctx) override;

    std::any visitInsertIntoTable(SQLParser::InsertIntoTableContext *ctx) override;

    std::any visitDeleteFromTable(SQLParser::DeleteFromTableContext *ctx) override;

    std::any visitUpdateTable(SQLParser::UpdateTableContext *ctx) override;

    std::any visitSelectTable_(SQLParser::SelectTable_Context *ctx) override;

    std::any visitSelectTable(SQLParser::SelectTableContext *ctx) override;

    std::any visitFieldList(SQLParser::FieldListContext *ctx) override;

    std::any visitNormalField(SQLParser::NormalFieldContext *ctx) override;

    std::any visitType(SQLParser::TypeContext *ctx) override;

    std::any visitValueLists(SQLParser::ValueListsContext *ctx) override;

    std::any visitValueList(SQLParser::ValueListContext *ctx) override;

    std::any visitValue(SQLParser::ValueContext *ctx) override;

    std::any visitWhereAndClause(SQLParser::WhereAndClauseContext *ctx) override;

    std::any visitWhereOperatorExpression(SQLParser::WhereOperatorExpressionContext *ctx) override;

//    std::any visitWhereInList(SQLParser::WhereInListContext *ctx) override;

//    std::any visitWhereLikeString(SQLParser::WhereLikeStringContext *ctx) override;

    std::any visitColumn(SQLParser::ColumnContext *ctx) override;

    std::any visitExpression(SQLParser::ExpressionContext *ctx) override;

    std::any visitSetClause(SQLParser::SetClauseContext *ctx) override;

    std::any visitSelectors(SQLParser::SelectorsContext *ctx) override;

    std::any visitSelector(SQLParser::SelectorContext *ctx) override;

    std::any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override;

    std::any visitOperator_(SQLParser::Operator_Context *ctx) override;
};

#endif  // VISITOR_H_