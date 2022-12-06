
// Generated from .\SQL.g4 by ANTLR 4.11.1

#pragma once


#include "antlr4-runtime.h"
#include "SQLParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SQLParser.
 */
class  SQLVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SQLParser.
   */
    virtual std::any visitProgram(SQLParser::ProgramContext *context) = 0;

    virtual std::any visitStatement(SQLParser::StatementContext *context) = 0;

    virtual std::any visitCreateDb(SQLParser::CreateDbContext *context) = 0;

    virtual std::any visitDropDb(SQLParser::DropDbContext *context) = 0;

    virtual std::any visitShowDbs(SQLParser::ShowDbsContext *context) = 0;

    virtual std::any visitUseDb(SQLParser::UseDbContext *context) = 0;

    virtual std::any visitShowTables(SQLParser::ShowTablesContext *context) = 0;

    virtual std::any visitShowIndexes(SQLParser::ShowIndexesContext *context) = 0;

    virtual std::any visitLoadData(SQLParser::LoadDataContext *context) = 0;

    virtual std::any visitDumpData(SQLParser::DumpDataContext *context) = 0;

    virtual std::any visitCreateTable(SQLParser::CreateTableContext *context) = 0;

    virtual std::any visitDropTable(SQLParser::DropTableContext *context) = 0;

    virtual std::any visitDescribeTable(SQLParser::DescribeTableContext *context) = 0;

    virtual std::any visitInsertIntoTable(SQLParser::InsertIntoTableContext *context) = 0;

    virtual std::any visitDeleteFromTable(SQLParser::DeleteFromTableContext *context) = 0;

    virtual std::any visitUpdateTable(SQLParser::UpdateTableContext *context) = 0;

    virtual std::any visitSelectTable_(SQLParser::SelectTable_Context *context) = 0;

    virtual std::any visitSelectTable(SQLParser::SelectTableContext *context) = 0;

    virtual std::any visitAlterAddIndex(SQLParser::AlterAddIndexContext *context) = 0;

    virtual std::any visitAlterDropIndex(SQLParser::AlterDropIndexContext *context) = 0;

    virtual std::any visitAlterTableDropPk(SQLParser::AlterTableDropPkContext *context) = 0;

    virtual std::any visitAlterTableDropForeignKey(SQLParser::AlterTableDropForeignKeyContext *context) = 0;

    virtual std::any visitAlterTableAddPk(SQLParser::AlterTableAddPkContext *context) = 0;

    virtual std::any visitAlterTableAddForeignKey(SQLParser::AlterTableAddForeignKeyContext *context) = 0;

    virtual std::any visitAlterTableAddUnique(SQLParser::AlterTableAddUniqueContext *context) = 0;

    virtual std::any visitFieldList(SQLParser::FieldListContext *context) = 0;

    virtual std::any visitNormalField(SQLParser::NormalFieldContext *context) = 0;

    virtual std::any visitPrimaryKeyField(SQLParser::PrimaryKeyFieldContext *context) = 0;

    virtual std::any visitForeignKeyField(SQLParser::ForeignKeyFieldContext *context) = 0;

    virtual std::any visitType(SQLParser::TypeContext *context) = 0;

    virtual std::any visitValueLists(SQLParser::ValueListsContext *context) = 0;

    virtual std::any visitValueList(SQLParser::ValueListContext *context) = 0;

    virtual std::any visitValue(SQLParser::ValueContext *context) = 0;

    virtual std::any visitWhereAndClause(SQLParser::WhereAndClauseContext *context) = 0;

    virtual std::any visitWhereOperatorExpression(SQLParser::WhereOperatorExpressionContext *context) = 0;

    virtual std::any visitWhereOperatorSelect(SQLParser::WhereOperatorSelectContext *context) = 0;

    virtual std::any visitWhereNull(SQLParser::WhereNullContext *context) = 0;

    virtual std::any visitWhereInList(SQLParser::WhereInListContext *context) = 0;

    virtual std::any visitWhereInSelect(SQLParser::WhereInSelectContext *context) = 0;

    virtual std::any visitWhereLikeString(SQLParser::WhereLikeStringContext *context) = 0;

    virtual std::any visitColumn(SQLParser::ColumnContext *context) = 0;

    virtual std::any visitExpression(SQLParser::ExpressionContext *context) = 0;

    virtual std::any visitSetClause(SQLParser::SetClauseContext *context) = 0;

    virtual std::any visitSelectors(SQLParser::SelectorsContext *context) = 0;

    virtual std::any visitSelector(SQLParser::SelectorContext *context) = 0;

    virtual std::any visitIdentifiers(SQLParser::IdentifiersContext *context) = 0;

    virtual std::any visitOperator_(SQLParser::Operator_Context *context) = 0;

    virtual std::any visitAggregator(SQLParser::AggregatorContext *context) = 0;


};

