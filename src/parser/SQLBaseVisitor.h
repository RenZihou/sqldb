
// Generated from .\SQL.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "SQLVisitor.h"


/**
 * This class provides an empty implementation of SQLVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SQLBaseVisitor : public SQLVisitor {
public:

  virtual std::any visitProgram(SQLParser::ProgramContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitStatement(SQLParser::StatementContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreateDb(SQLParser::CreateDbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDropDb(SQLParser::DropDbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShowDbs(SQLParser::ShowDbsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUseDb(SQLParser::UseDbContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShowTables(SQLParser::ShowTablesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitShowIndexes(SQLParser::ShowIndexesContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLoadData(SQLParser::LoadDataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDumpData(SQLParser::DumpDataContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreateTable(SQLParser::CreateTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDropTable(SQLParser::DropTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDescribeTable(SQLParser::DescribeTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsertIntoTable(SQLParser::InsertIntoTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDeleteFromTable(SQLParser::DeleteFromTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpdateTable(SQLParser::UpdateTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectTable_(SQLParser::SelectTable_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectTable(SQLParser::SelectTableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterAddIndex(SQLParser::AlterAddIndexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterDropIndex(SQLParser::AlterDropIndexContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterTableDropPk(SQLParser::AlterTableDropPkContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterTableDropForeignKey(SQLParser::AlterTableDropForeignKeyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterTableAddPk(SQLParser::AlterTableAddPkContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterTableAddForeignKey(SQLParser::AlterTableAddForeignKeyContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlterTableAddUnique(SQLParser::AlterTableAddUniqueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFieldList(SQLParser::FieldListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitNormalField(SQLParser::NormalFieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPrimaryKeyField(SQLParser::PrimaryKeyFieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeignKeyField(SQLParser::ForeignKeyFieldContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType(SQLParser::TypeContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueLists(SQLParser::ValueListsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValueList(SQLParser::ValueListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue(SQLParser::ValueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereAndClause(SQLParser::WhereAndClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereOperatorExpression(SQLParser::WhereOperatorExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereOperatorSelect(SQLParser::WhereOperatorSelectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereNull(SQLParser::WhereNullContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereInList(SQLParser::WhereInListContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereInSelect(SQLParser::WhereInSelectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWhereLikeString(SQLParser::WhereLikeStringContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn(SQLParser::ColumnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpression(SQLParser::ExpressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSetClause(SQLParser::SetClauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelectors(SQLParser::SelectorsContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelector(SQLParser::SelectorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOperator_(SQLParser::Operator_Context *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregator(SQLParser::AggregatorContext *ctx) override {
    return visitChildren(ctx);
  }


};

