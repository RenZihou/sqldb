// record.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "record.h"

//Record::Record() {}

unsigned Record::getPageId() const {
    return this->_page_id;
}

unsigned int Record::getSlotId() const {
    return this->_slot_id;
}

char *Record::getData() const {
    return this->_data;
}

void Record::setData(char *data) {
    this->_data = data;
}


