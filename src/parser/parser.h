// parser.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef PARSER_H_
#define PARSER_H_

#include "../query/op.h"

/**
 * @param command user-input sql command
 * @return operation list
 * @description parse the given sql command
 */
Op *parse(const std::string &command);

#endif  // PARSER_H_
