// Table.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <iostream>
#include <utility>

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
    int unread = sizeof(TableHeader);
    int pid = 0;
    int index;
    while (unread > PAGE_SIZE) {
        index = BufferManager::bm().getPage(this->name, pid);
        memcpy(BufferManager::bm().readBuffer(index), (char *) this->header + PAGE_SIZE * pid,
               PAGE_SIZE);
        BufferManager::bm().markDirty(index);
        unread -= PAGE_SIZE;
        ++pid;
    }
    index = BufferManager::bm().getPage(this->name, pid);
    memcpy(BufferManager::bm().readBuffer(index), (char *) this->header + PAGE_SIZE * pid, unread);
    BufferManager::bm().markDirty(index);
    delete this->header;
}

int Table::createTable(const std::string &name) {
    return BufferManager::createFile(name);
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
        int value_i;
        float value_f;
        switch (column.type) {
            case ColumnType::INT:
                value_i = std::stoi(column.default_value);
                memcpy(this->header->defaults + offset_begin, &value_i, sizeof(int));
                break;
            case ColumnType::FLOAT:
                value_f = std::stof(column.default_value);
                memcpy(this->header->defaults + offset_begin, &value_f, sizeof(float));
                break;
            case ColumnType::VARCHAR:
                memcpy(this->header->defaults + offset_begin, column.default_value.c_str(),
                       column.length);
                break;
            default:
                break;
        }
    }
    // TODO modify data (this function should only be called when creating table for now)
    return 0;
}

void Table::insertRecord(void *data) {
    int index;  // index of buffer to write
    BufType buf;  // buffer to write
    if (this->header->next_empty == 0) {  // all pages are occupied
        index = BufferManager::bm().allocPage(this->name, ++this->header->pages);
        buf = BufferManager::bm().readBuffer(index);
        this->header->next_empty = this->header->pages << PAGE_SIZE_IDX;
    } else {
        index = BufferManager::bm().getPage(this->name, this->header->next_empty >> PAGE_SIZE_IDX);
        buf = BufferManager::bm().readBuffer(index) + (this->header->next_empty & PAGE_SIZE_MASK);
        this->header->next_empty = ((unsigned *) buf)[0];
    }
    memcpy(buf, data, this->header->columns);
    BufferManager::bm().markDirty(index);
}

void Table::deleteRecord(unsigned int record_offset) {
    unsigned page_id = record_offset >> PAGE_SIZE_IDX;
    int index = BufferManager::bm().getPage(this->name, page_id);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    ((unsigned *) buf)[0] = this->header->next_empty;
    this->header->next_empty = record_offset;
    BufferManager::bm().markDirty(index);
}

void Table::updateRecord(unsigned int record_offset, void *data) {
    unsigned page_id = record_offset >> PAGE_SIZE_IDX;
    int index = BufferManager::bm().getPage(this->name, page_id);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    memcpy(buf, data, this->header->columns);
    BufferManager::bm().markDirty(index);
}

void *Table::selectRecord(unsigned int record_offset) {
    unsigned page_id = record_offset >> PAGE_SIZE_IDX;
    int index = BufferManager::bm().getPage(this->name, page_id);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    return (char *) buf;
}