// visitor.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou


#include "visitor.h"

std::any Visitor::visitProgram(SQLParser::ProgramContext *ctx) {
    Op *root = new Op(OpType::UNKNOWN);
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

std::any Visitor::visitSelectTable(SQLParser::SelectTableContext *ctx) {
    auto selectors = std::any_cast<std::vector<std::string>>(visit(ctx->selectors()));
    auto tables = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
//        std::vector<std::string> columns;
//        if (ctx->columns() != nullptr) {
//            columns = std::any_cast<std::vector<std::string>>(visit(ctx->columns()));
//        }
    std::cout << "SELECT <selectors: " << selectors.size() << ", tables: " << tables.size() << ">"
              << std::endl;
    return std::make_tuple(selectors, tables);
}

std::any Visitor::visitFieldList(SQLParser::FieldListContext *ctx) {
    std::vector<Column> fields;
    for (auto &field : ctx->field()) {
        fields.push_back(std::any_cast<Column>(visit(field)));
    }
    return fields;
}

std::any Visitor::visitNormalField(SQLParser::NormalFieldContext *ctx) {
    std::string name = ctx->Identifier()->getText();
    auto type = std::any_cast<std::tuple<ColumnType, unsigned>>(visit(ctx->type()));
    std::string default_value;
    if (ctx->value() != nullptr) {  // default value
        default_value = std::any_cast<std::string>(visit(ctx->value()));
    } else {
        default_value = "";
    }
    return Column{.name=name, .type=std::get<0>(type), .length=std::get<1>(type),
            .not_null=(ctx->Null() != nullptr), .default_value=default_value};
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

std::any Visitor::visitSelectors(SQLParser::SelectorsContext *ctx) {
    if (ctx->getStart()->getText() == "*") {
        return std::vector<std::string>{"*"};
    }
    std::vector<std::string> selectors;
    for (auto &selector: ctx->selector()) {
        selectors.push_back(std::any_cast<std::string>(visit(selector)));
    }
    return selectors;
}

std::any Visitor::visitSelector(SQLParser::SelectorContext *ctx) {
    return ctx->getText();
}

std::any Visitor::visitIdentifiers(SQLParser::IdentifiersContext *ctx) {
//        std::string identifier = ctx->getText();
    std::vector<std::string> identifiers;
    for (auto &ident: ctx->Identifier()) {
        identifiers.push_back(ident->getSymbol()->getText());
//            identifier += ", " + visit(ident).as<std::string>();

    }
    return identifiers;
//        return visitChildren(ctx);
}
