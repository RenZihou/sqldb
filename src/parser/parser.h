// parser.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef SQLDB_PARSER_H
#define SQLDB_PARSER_H

#include "optree.h"

Op *parse(const std::string& command);

#endif //SQLDB_PARSER_H
