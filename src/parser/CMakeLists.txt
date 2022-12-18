set(parser_source
        SQLLexer.h SQLLexer.cpp
        SQLParser.h SQLParser.cpp
        SQLVisitor.h SQLVisitor.cpp
        SQLBaseVisitor.h SQLBaseVisitor.cpp
        visitor.h visitor.cpp
        parser.h parser.cpp
        op.h op.cpp
        compare.h compare.cpp )

add_library(parser ${parser_source})