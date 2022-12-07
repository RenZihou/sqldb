// Table.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <utility>
#include "table.h"
#include "../pagefile/buffer_manager.h"

Table::Table(std::string table_name) : header(new TableHeader), name(std::move(table_name)) {
    // read table header
    for (int i = 0; i < this->header_pages; ++i) {
        int index = BufferManager::bm().getPage(this->name, i);
        memcpy(this->header + PAGE_SIZE * i, BufferManager::bm().readBuffer(index), PAGE_SIZE);
    }
}

// TODO write back header (no matter whether changed)
Table::~Table() {
    for (int i = 0; i < this->header_pages; ++i) {
        int index = BufferManager::bm().getPage(this->name, i);
        memcpy(BufferManager::bm().readBuffer(index), this->header + PAGE_SIZE * i, PAGE_SIZE);
        BufferManager::bm().markDirty(index);
    }
    delete header;
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