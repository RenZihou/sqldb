// parser.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "antlr4-runtime.h"
#include "parser.h"
#include "SQLLexer.h"
#include "SQLParser.h"
#include "visitor.h"

Op *parse(const std::string& command) {
    antlr4::ANTLRInputStream input(command);
    SQLLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLParser parser(&tokens);
    auto tree = parser.program();
    Visitor visitor{};
    auto result = visitor.visit(tree);
    return std::any_cast<Op *>(result);
}
