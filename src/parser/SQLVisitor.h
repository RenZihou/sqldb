
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
    std::any visitProgram(SQLParser::ProgramContext *context) {
        return visitChildren(context);
    }

    std::any visitStatement(SQLParser::StatementContext *context) {
        return visitChildren(context);
    }

    std::any visitCreateDb(SQLParser::CreateDbContext *context) {
        return visitChildren(context);
    }

    std::any visitDropDb(SQLParser::DropDbContext *context) {
        return visitChildren(context);
    }

    std::any visitShowDbs(SQLParser::ShowDbsContext *context) {
        return visitChildren(context);
    }

    std::any visitUseDb(SQLParser::UseDbContext *context) {
        return visitChildren(context);
    }

    std::any visitShowTables(SQLParser::ShowTablesContext *context) {
        return visitChildren(context);
    }

    std::any visitShowIndexes(SQLParser::ShowIndexesContext *context) {
        return visitChildren(context);
    }

    std::any visitLoadData(SQLParser::LoadDataContext *context) {
        return visitChildren(context);
    }

    std::any visitDumpData(SQLParser::DumpDataContext *context) {
        return visitChildren(context);
    }

    std::any visitCreateTable(SQLParser::CreateTableContext *context) {
        return visitChildren(context);
    }

    std::any visitDropTable(SQLParser::DropTableContext *context) {
        return visitChildren(context);
    }

    std::any visitDescribeTable(SQLParser::DescribeTableContext *context) {
        return visitChildren(context);
    }

    std::any visitInsertIntoTable(SQLParser::InsertIntoTableContext *context) {
        return visitChildren(context);
    }

    std::any visitDeleteFromTable(SQLParser::DeleteFromTableContext *context) {
        return visitChildren(context);
    }

    std::any visitUpdateTable(SQLParser::UpdateTableContext *context) {
        return visitChildren(context);
    }

    std::any visitSelectTable_(SQLParser::SelectTable_Context *context) {
        return visitChildren(context);
    }

    std::any visitSelectTable(SQLParser::SelectTableContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterAddIndex(SQLParser::AlterAddIndexContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterDropIndex(SQLParser::AlterDropIndexContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterTableDropPk(SQLParser::AlterTableDropPkContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterTableDropForeignKey(SQLParser::AlterTableDropForeignKeyContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterTableAddPk(SQLParser::AlterTableAddPkContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterTableAddForeignKey(SQLParser::AlterTableAddForeignKeyContext *context) {
        return visitChildren(context);
    }

    std::any visitAlterTableAddUnique(SQLParser::AlterTableAddUniqueContext *context) {
        return visitChildren(context);
    }

    std::any visitFieldList(SQLParser::FieldListContext *context) {
        return visitChildren(context);
    }

    std::any visitNormalField(SQLParser::NormalFieldContext *context) {
        return visitChildren(context);
    }

    std::any visitPrimaryKeyField(SQLParser::PrimaryKeyFieldContext *context) {
        return visitChildren(context);
    }

    std::any visitForeignKeyField(SQLParser::ForeignKeyFieldContext *context) {
        return visitChildren(context);
    }

    std::any visitType(SQLParser::TypeContext *context) {
        return visitChildren(context);
    }

    std::any visitValueLists(SQLParser::ValueListsContext *context) {
        return visitChildren(context);
    }

    std::any visitValueList(SQLParser::ValueListContext *context) {
        return visitChildren(context);
    }

    std::any visitValue(SQLParser::ValueContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereAndClause(SQLParser::WhereAndClauseContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereOperatorExpression(SQLParser::WhereOperatorExpressionContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereOperatorSelect(SQLParser::WhereOperatorSelectContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereNull(SQLParser::WhereNullContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereInList(SQLParser::WhereInListContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereInSelect(SQLParser::WhereInSelectContext *context) {
        return visitChildren(context);
    }

    std::any visitWhereLikeString(SQLParser::WhereLikeStringContext *context) {
        return visitChildren(context);
    }

    std::any visitColumn(SQLParser::ColumnContext *context) {
        return visitChildren(context);
    }

    std::any visitExpression(SQLParser::ExpressionContext *context) {
        return visitChildren(context);
    }

    std::any visitSetClause(SQLParser::SetClauseContext *context) {
        return visitChildren(context);
    }

    std::any visitSelectors(SQLParser::SelectorsContext *context) {
        return visitChildren(context);
    }

    std::any visitSelector(SQLParser::SelectorContext *context) {
        return visitChildren(context);
    }

    std::any visitIdentifiers(SQLParser::IdentifiersContext *context) {
        return visitChildren(context);
    }

    std::any visitOperator_(SQLParser::Operator_Context *context) {
        return visitChildren(context);
    }

    std::any visitAggregator(SQLParser::AggregatorContext *context) {
        return visitChildren(context);
    }


};

