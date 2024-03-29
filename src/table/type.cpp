// type.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <iostream>

#include "type.h"

void serializeFromString(const std::string &value, ColumnType type, BufType buffer, unsigned length) {
    switch (type) {
        case ColumnType::INT:
            *(int *) buffer = std::stoi(value);
            break;
        case ColumnType::FLOAT:
            *(float *) buffer = std::stof(value);
            break;
        case ColumnType::VARCHAR:
            if (value.length() > length - 1) {
                std::cerr << "WARNING: value too long (" << value.length() << ") for column" << std::endl;
            } else {
                length = value.length() + 1;
            }
            memcpy(buffer, value.c_str(), length);
            break;
        default:
            break;
    }
}

Type *deserialize(BufType buf, ColumnType type) {
    switch (type) {
        case ColumnType::INT:
            return new Int(buf);
        case ColumnType::FLOAT:
            return new Float(buf);
        case ColumnType::VARCHAR:
            return new VarChar(buf);
        default:
            return nullptr;
    }
}
