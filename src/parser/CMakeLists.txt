set(parser_source
        SQLLexer.h SQLLexer.cpp
        SQLParser.h SQLParser.cpp
        SQLVisitor.h SQLVisitor.cpp
        SQLBaseVisitor.h SQLBaseVisitor.cpp
        parser.cpp parser.h visitor.h optree.cpp optree.h visitor.cpp)

add_library(parser ${parser_source})