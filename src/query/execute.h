// execute.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef EXECUTE_H_
#define EXECUTE_H_

#include "../parser/optree.h"

/**
 * @param op operator tree
 * @description execute the given operator tree
 */
void execute(Op *op);

#endif  // EXECUTE_H_