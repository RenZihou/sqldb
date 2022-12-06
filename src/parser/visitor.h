// visitors.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef VISITOR_H_
#define VISITOR_H_

#include "SQLBaseVisitor.h"
#include "optree.h"

class Visitor : public SQLBaseVisitor {
public:
    std::any visitProgram(SQLParser::ProgramContext *ctx) override {
        Op *root = new Op();
        Op *top = root;
        for (auto &stmt : ctx->statement()) {
            top = top->setNext(std::any_cast<Op *>(visit(stmt)));
        }
        return root->getNext();
    }

    std::any visitStatement(SQLParser::StatementContext *ctx) override {
//        return visit(ctx->)
        if (ctx->dbStatement()) return visit(ctx->dbStatement());
        else if (ctx->tableStatement()) return visit(ctx->tableStatement());
        else if (ctx->alterStatement()) return visit(ctx->alterStatement());
        auto *op = new OpUnknown();
        return op;
    }

    std::any visitCreateDb(SQLParser::CreateDbContext *ctx) override {
        std::string name = ctx->Identifier()->getText();
        auto *op = new OpDbCreate(name);
        return dynamic_cast<Op *>(op);
    }

    std::any visitSelectTable(SQLParser::SelectTableContext *ctx) override {
        auto selectors = std::any_cast<std::vector<std::string>>(visit(ctx->selectors()));
        auto tables = std::any_cast<std::vector<std::string>>(visit(ctx->identifiers()));
//        std::vector<std::string> columns;
//        if (ctx->columns() != nullptr) {
//            columns = std::any_cast<std::vector<std::string>>(visit(ctx->columns()));
//        }
        std::cout << "SELECT <selectors: " << selectors.size() << ", tables: " << tables.size() << ">" << std::endl;
        return std::make_tuple(selectors, tables);
    }

    std::any visitSelectors(SQLParser::SelectorsContext *ctx) override {
        if (ctx->getStart()->getText() == "*") {
            return std::vector<std::string>{"*"};
        }
        std::vector<std::string> selectors;
        for (auto &selector : ctx->selector()) {
            selectors.push_back(std::any_cast<std::string>(visit(selector)));
        }
        return selectors;
    }

    std::any visitSelector(SQLParser::SelectorContext *ctx) override {
        return ctx->getText();
    }

    std::any visitIdentifiers(SQLParser::IdentifiersContext *ctx) override {
//        std::string identifier = ctx->getText();
        std::vector<std::string> identifiers;
        for (auto &ident : ctx->Identifier()) {
            identifiers.push_back(ident->getSymbol()->getText());
//            identifier += ", " + visit(ident).as<std::string>();

        }
        return identifiers;
//        return visitChildren(ctx);
    }
};

#endif  // VISITOR_H_