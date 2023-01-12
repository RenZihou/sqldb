// dumb.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

// this file do nothing

#include <cassert>

#include "constants.h"
#include "exception.h"

void check_define() {
    assert(MAX_RECORD_SIZE <= PAGE_SIZE - sizeof(unsigned));
    assert(MAX_COLUMN <= 8 * sizeof(unsigned));
}
