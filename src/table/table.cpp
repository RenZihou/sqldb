// Table.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <iostream>
#include <utility>
#include <cmath>

#include "table.h"
#include "../pagefile/buffer_manager.h"

Table::Table(std::string table_name) : header(new TableHeader()), name(std::move(table_name)) {
    // read table header
    int unread = sizeof(TableHeader);
    int pid = 0;
    int index;
    while (unread > PAGE_SIZE) {
        index = BufferManager::bm().getPage(this->name, pid);
        memcpy((char *) this->header + PAGE_SIZE * pid, BufferManager::bm().readBuffer(index),
               PAGE_SIZE);
        unread -= PAGE_SIZE;
        ++pid;
    }
    index = BufferManager::bm().getPage(this->name, pid);
    memcpy((char *) this->header + PAGE_SIZE * pid, BufferManager::bm().readBuffer(index), unread);
}

Table::~Table() {
    int unwritten = sizeof(TableHeader);
    int pid = 0;
    int index;
    while (unwritten > PAGE_SIZE) {
        index = BufferManager::bm().getPage(this->name, pid);
        memcpy(BufferManager::bm().readBuffer(index), (char *) this->header + PAGE_SIZE * pid,
               PAGE_SIZE);
        BufferManager::bm().markDirty(index);
        unwritten -= PAGE_SIZE;
        ++pid;
    }
    index = BufferManager::bm().getPage(this->name, pid);
    memcpy(BufferManager::bm().readBuffer(index), (
            char *) this->header + PAGE_SIZE * pid, unwritten);
    BufferManager::bm().markDirty(index);
    delete this->header;
}

inline unsigned Table::_getRecordSize() const {
    return this->header->column_info[this->header->columns - 1].offset +
           this->header->column_info[this->header->columns - 1].length;
}

inline unsigned Table::_getRecordSizeWithFlag() const {
    return this->_getRecordSize() + sizeof(unsigned);
}

void Table::_serialize(const std::string &value, ColumnType type, char *buffer, unsigned length) {
    switch (type) {
        case ColumnType::INT:
            *(int *) buffer = std::stoi(value);
            break;
        case ColumnType::FLOAT:
            *(float *) buffer = std::stof(value);
            break;
        case ColumnType::VARCHAR:
            if (value.length() > length - 1) {
                std::cerr << "value too long (" << value.length() << ") for column" << std::endl;
            } else {
                length = value.length() + 1;
            }
            memcpy(buffer, value.c_str(), length);
            break;
        default:
            break;
    }
}

void Table::_insertRecord(void *data) {
    int index;  // index of buffer to write
    BufType buf;  // buffer to write
    if (this->header->next_empty == 0) {  // all pages are occupied
        index = BufferManager::bm().allocPage(this->name, this->header->pages);
        buf = BufferManager::bm().readBuffer(index);
        // write next empty on this page
        unsigned next_empty_offset = this->_getRecordSizeWithFlag();
        int available_in_page = PAGE_SIZE / this->_getRecordSizeWithFlag() - 1;  // # of tot slots
        if (available_in_page > 0) {
            this->header->next_empty =
                    (this->header->pages << PAGE_SIZE_IDX) + this->_getRecordSizeWithFlag();
            while (--available_in_page) {
                *(unsigned *) (buf + next_empty_offset) =
                        (this->header->pages << PAGE_SIZE_IDX)
                        + (next_empty_offset + this->_getRecordSizeWithFlag());
                next_empty_offset += this->_getRecordSizeWithFlag();
            }
            *(unsigned *) (buf + next_empty_offset) = 0;
        } else {
            this->header->next_empty = 0;
        }
        ++this->header->pages;
    } else {
        index = BufferManager::bm().getPage(this->name, this->header->next_empty >> PAGE_SIZE_IDX);
        buf = BufferManager::bm().readBuffer(index) + (this->header->next_empty & PAGE_SIZE_MASK);
        this->header->next_empty = ((unsigned *) buf)[0];
    }
    memcpy(buf, data, this->_getRecordSizeWithFlag());
    BufferManager::bm().markDirty(index);
}

void Table::_deleteRecord(unsigned int record_offset) {
    unsigned page_id = record_offset >> PAGE_SIZE_IDX;
    int index = BufferManager::bm().getPage(this->name, page_id);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    ((unsigned *) buf)[0] = this->header->next_empty;
    this->header->next_empty = record_offset;
    BufferManager::bm().markDirty(index);
}

void Table::_updateRecord(unsigned int record_offset, void *data) {
    unsigned page_id = record_offset >> PAGE_SIZE_IDX;
    int index = BufferManager::bm().getPage(this->name, page_id);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    memcpy(buf + sizeof(unsigned), data, this->_getRecordSizeWithFlag());
    BufferManager::bm().markDirty(index);
}

void *Table::_selectRecord(unsigned int record_offset) {
    unsigned page_id = record_offset >> PAGE_SIZE_IDX;
    int index = BufferManager::bm().getPage(this->name, page_id);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    return (char *) buf;
}

Table *Table::createTable(const std::string &name) {
    if (BufferManager::createFile(name) != 0) return nullptr;
    auto table = new Table(name);
    table->header->pages = static_cast<unsigned >(std::ceil(
            static_cast<double>(sizeof(TableHeader)) / PAGE_SIZE));
    table->header->columns = 0;
    table->header->next_empty = 0;
    return table;
}

int Table::addColumn(const Column &column, const std::string &after) {
    if (this->header->columns >= MAX_COLUMN) {
        std::cerr << "reached max column number (" << MAX_COLUMN << ")" << std::endl;
        return -1;
    }
    int index = -1;
    if (after.empty()) index = 0;
    else {
        for (int i = 0; i < this->header->columns; ++i) {
            if (this->header->column_info[i].name == after) {
                index = i + 1;
                break;
            }
        }
    }
    if (index == -1) {
        std::cerr << "column " << after << " not found" << std::endl;
        return -1;
    }
    // modify header
    for (int i = static_cast<int>(this->header->columns); i > index; --i) {
        this->header->column_info[i] = this->header->column_info[i - 1];
        this->header->column_info[i].offset += column.length;
    }
    memcpy(this->header->column_info[index].name, column.name.c_str(), column.name.length());
    this->header->column_info[index].flags = column.flags;
    this->header->column_info[index].type = column.type;
    this->header->column_info[index].length = column.length;
    this->header->column_info[index].offset =
            index == 0 ? 0 : this->header->column_info[index - 1].offset
                             + this->header->column_info[index - 1].length;
    ++this->header->columns;
    if (column.flags & FLAG_HAS_DEFAULT) {
        // move defaults to the right
        unsigned offset_begin = this->header->column_info[index].offset;
        unsigned offset_end = offset_begin + this->header->column_info[index].length;
        memcpy(this->header->defaults + offset_end,
               this->header->defaults + offset_begin,
               sizeof(this->header->defaults) - offset_end);
        // set new default
//        int value_i;
//        float value_f;
        this->_serialize(column.default_value, column.type, this->header->defaults + offset_begin,
                         column.length);
//        switch (column.type) {
//            case ColumnType::INT:
//                value_i = std::stoi(column.default_value);
//                memcpy(this->header->defaults + offset_begin, &value_i, sizeof(int));
//                break;
//            case ColumnType::FLOAT:
//                value_f = std::stof(column.default_value);
//                memcpy(this->header->defaults + offset_begin, &value_f, sizeof(float));
//                break;
//            case ColumnType::VARCHAR:
//                memcpy(this->header->defaults + offset_begin, column.default_value.c_str(),
//                       column.length);
//                break;
//            default:
//                break;
//        }
    }
    // TODO modify data (this function should only be called when creating table for now)
    return 0;
}

void Table::insertRecord(const std::vector<std::string> &values) {
    if (values.size() != this->header->columns) {
        std::cerr << "column number mismatch (expected: " << this->header->columns <<
                  " got: " << values.size() << ")" << std::endl;
        return;
    }
    char *data = new char[this->_getRecordSizeWithFlag()];
    for (int i = 0; i < this->header->columns; ++i) {
        this->_serialize(values[i], this->header->column_info[i].type,
                         data + sizeof(unsigned) + this->header->column_info[i].offset,
                         this->header->column_info[i].length);
        *(unsigned *) data = 0;  // TODO null flags
    }
    this->_insertRecord(data);
    delete[] data;
}