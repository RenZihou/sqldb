// parser.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "antlr4-runtime.h"
#include "parser.h"
#include "SQLLexer.h"
#include "SQLParser.h"
#include "SQLVisitor.h"

void parse(const std::string& command) {
    antlr4::ANTLRInputStream input(command);
    SQLLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLParser parser(&tokens);
    auto tree = parser.program();
    std::cout << tree->getText() << std::endl;
}
