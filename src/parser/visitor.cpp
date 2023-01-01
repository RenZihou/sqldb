// visitor.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "visitor.h"
#include "../query/op.h"

std::any Visitor::visitProgram(SQLParser::ProgramContext *ctx) {
    Op *root = new Op();
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
    return new Op();
}

std::any Visitor::visitCreateDb(SQLParser::CreateDbContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto *op = new OpDbCreate(name);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitDropDb(SQLParser::DropDbContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto *op = new OpDbDrop(name);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitShowDbs(SQLParser::ShowDbsContext *ctx) {
    auto *op = new OpDbShow();
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitUseDb(SQLParser::UseDbContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto *op = new OpDbUse(name);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitShowTables(SQLParser::ShowTablesContext *ctx) {
    auto *op = new OpDbShowTables();
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitCreateTable(SQLParser::CreateTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto [columns, pk_info] = std::any_cast<std::tuple<
                    std::vector<Column>,
                    std::vector<std::tuple<std::string, std::vector<std::string>>>>>(visit(ctx->fieldList()));
    auto *op = new OpTableCreate(name, columns, pk_info);

    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitDropTable(SQLParser::DropTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto *op = new OpTableDrop(name);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitDescribeTable(SQLParser::DescribeTableContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto *op = new OpTableDescribe(name);
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
    auto sets = std::any_cast<std::vector<std::tuple<std::string, std::string>>>(
            visit(ctx->setClause()));
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

std::any Visitor::visitAlterAddIndex(SQLParser::AlterAddIndexContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto columns = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
    auto *op = new OpTableAlterAddIndex(name, columns);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitAlterDropIndex(SQLParser::AlterDropIndexContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto columns = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
    auto *op = new OpTableAlterDropIndex(name, columns);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitAlterTableDropPk(SQLParser::AlterTableDropPkContext *ctx) {
    std::string name = ctx->Identifier(0)->getText();
    std::string pk = ctx->Identifier(1) ? ctx->Identifier(1)->getText() : "";
    auto *op = new OpTableAlterDropPk(name, pk);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitAlterTableAddPk(SQLParser::AlterTableAddPkContext *ctx) {
    std::string name = ctx->Identifier(0)->getText();
    std::string pk = ctx->Identifier(1) ? ctx->Identifier(1)->getText() : "";
    auto columns = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
    auto *op = new OpTableAlterAddPk(name, pk, columns);
    return dynamic_cast<Op *>(op);
}

std::any Visitor::visitFieldList(SQLParser::FieldListContext *ctx) {
    std::vector<Column> fields;
    std::vector<std::tuple<std::string, std::vector<std::string>>> pk_info;
    for (auto &field: ctx->field()) {
        switch (field->getStart()->getType()) {  // TODO this is a dirty hack
            case 34:  // PRIMARY
                pk_info.push_back(std::any_cast<std::tuple<std::string, std::vector<std::string>>>(
                        visit(field)));
                break;
            case 36:  // FOREIGN
                break;
            default:  // other -> normal field
                fields.push_back(std::any_cast<Column>(visit(field)));
        }
    }
    return std::make_tuple(fields, pk_info);
}

std::any Visitor::visitNormalField(SQLParser::NormalFieldContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    char flags = 0;
    flags |= ctx->value() ? FLAG_HAS_DEFAULT : 0;
    flags |= ctx->Null() ? FLAG_NOT_NULL : 0;
    auto [type, length] = std::any_cast<std::tuple<ColumnType, unsigned>>(visit(ctx->type()));
    std::string default_value;
    if (ctx->value() != nullptr) {  // default value
        default_value = std::any_cast<std::string>(visit(ctx->value()));
    }
    return Column{.name=name, .type=type, .length=length,
            .flags=flags, .default_value=default_value};
}

std::any Visitor::visitPrimaryKeyField(SQLParser::PrimaryKeyFieldContext *ctx) {
    std::string name = ctx->Identifier() ? ctx->Identifier()->getText() : "";
    auto columns = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
    return std::make_tuple(name, columns);
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
    auto [lhs_table, lhs_column] = std::any_cast<std::tuple<std::string, std::string>>(visit(ctx->column()));
    auto [rhs_table, rhs_column] = std::any_cast<std::tuple<std::string, std::string>>(visit(ctx->expression()));
    auto op = std::any_cast<CmpOp *>(visit(ctx->operator_()));
    if (rhs_column.empty()) {  // value
        return dynamic_cast<Condition *>(
                new ConditionCmp(new ExprColumn(lhs_table, lhs_column),
                                 new ExprValue(rhs_table), op));
    } else {  // column
        return dynamic_cast<Condition *>(
                new ConditionCmp(new ExprColumn(lhs_table, lhs_column),
                                 new ExprColumn(rhs_table, rhs_column), op));
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
    if (ctx->EqualOrAssign()) {
        return dynamic_cast<CmpOp *>(new Equal());
    } else if (ctx->Greater()) {
        return dynamic_cast<CmpOp *>(new Greater());
    } else if (ctx->Less()) {
        return dynamic_cast<CmpOp *>(new Less());
    } else if (ctx->GreaterEqual()) {
        return dynamic_cast<CmpOp *>(new GreaterEqual());
    } else if (ctx->LessEqual()) {
        return dynamic_cast<CmpOp *>(new LessEqual());
    } else if (ctx->NotEqual()) {
        return dynamic_cast<CmpOp *>(new NotEqual());
    }
    return nullptr;
}