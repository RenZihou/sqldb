set(parser_source
        SQLLexer.h SQLLexer.cpp
        SQLParser.h SQLParser.cpp
        SQLVisitor.h SQLVisitor.cpp
        parser.cpp parser.h)

add_library(parser ${parser_source})