// parser.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef PARSER_H_
#define PARSER_H_

#include "op.h"

/**
 * @param command user-input sql command
 * @return operator tree
 * @description parse the given sql command
 */
Op *parse(const std::string& command);

#endif  // PARSER_H_
