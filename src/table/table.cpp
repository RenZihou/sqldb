// Table.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <utility>

#include "table.h"
#include "../pagefile/buffer_manager.h"
#include "../util/exception.h"

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
                                                          char *) this->header + PAGE_SIZE * pid,
           unwritten);
    BufferManager::bm().markDirty(index);
    delete this->header;
}

inline unsigned Table::_getRecordSizeWithFlag() const {
    return this->header->column_info[this->header->columns - 1].offset +
           this->header->column_info[this->header->columns - 1].length;
}

inline unsigned Table::_getHeaderPageNum() {
    return (sizeof(TableHeader) - 1) / PAGE_SIZE + 1;
}

unsigned Table::_getSlotNum() const {
    return (PAGE_SIZE - PAGE_HEADER_SIZE) / this->_getRecordSizeWithFlag();
}

inline void Table::_offset_to_slot(unsigned int offset, unsigned int &page,
                                   unsigned int &slot) const {
    page = offset >> PAGE_SIZE_IDX;
    slot = ((offset & PAGE_SIZE_MASK) - PAGE_HEADER_SIZE) / this->_getRecordSizeWithFlag();
}

inline void Table::_slot_to_offset(unsigned int &offset, unsigned int page, unsigned int slot) const {
    offset = (page << PAGE_SIZE_IDX) + PAGE_HEADER_SIZE + slot * this->_getRecordSizeWithFlag();
}

void Table::_insertRecord(void *data) {
    int index;  // index of buffer to write
    BufType buf;  // buffer to write
    if (this->header->next_empty == 0) {  // all pages are occupied
        index = BufferManager::bm().allocPage(this->name, this->header->pages);
        buf = BufferManager::bm().readBuffer(index);
        for (int i = 0; i <= (PAGE_HEADER_SIZE >> 3); ++i) {  // escaping header does no harm
            buf[i] = 0;
        }
        buf[0] = 1;  // set first slot to 1 (occupied)
        // write next empty on this page
        unsigned next_empty_offset = PAGE_HEADER_SIZE + this->_getRecordSizeWithFlag();
        int available_in_page = (PAGE_SIZE - PAGE_HEADER_SIZE)
                                / static_cast<int>(this->_getRecordSizeWithFlag()) -
                                1;  // # of tot slots
        if (available_in_page > 0) {
            this->header->next_empty = (this->header->pages << PAGE_SIZE_IDX) + next_empty_offset;
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
        buf += PAGE_HEADER_SIZE;
    } else {
        unsigned page, slot;
        this->_offset_to_slot(this->header->next_empty, page, slot);
        index = BufferManager::bm().getPage(this->name, page);
        buf = BufferManager::bm().readBuffer(index);
        buf[slot >> 3] |= (1 << (slot & 7));  // set slot to 1 (occupied)
        buf += (this->header->next_empty & PAGE_SIZE_MASK);
        this->header->next_empty = ((unsigned *) buf)[0];
    }
    memcpy(buf, data, this->_getRecordSizeWithFlag());
    BufferManager::bm().markDirty(index);
    ++this->header->rows;
}

void Table::_deleteRecord(unsigned page, unsigned slot) {
    unsigned offset;
    this->_slot_to_offset(offset, page, slot);
    this->_offset_to_slot(offset, page, slot);
    int index = BufferManager::bm().getPage(this->name, page);
    BufType buf = BufferManager::bm().readBuffer(index);
    buf[slot >> 3] &= ~(1 << (slot & 7));  // set slot to 0 (empty)
    buf += (offset & PAGE_SIZE_MASK);
    ((unsigned *) buf)[0] = this->header->next_empty;
    this->header->next_empty = offset;
    BufferManager::bm().markDirty(index);
    --this->header->rows;
}

void Table::_updateRecord(unsigned page, unsigned slot, void *data) {
    unsigned offset;
    this->_slot_to_offset(offset, page, slot);
    int index = BufferManager::bm().getPage(this->name, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK);
    memcpy(buf, data, this->_getRecordSizeWithFlag());
    BufferManager::bm().markDirty(index);
}

void *Table::_selectRecord(unsigned record_offset) {
    unsigned page, slot;
    this->_offset_to_slot(record_offset, page, slot);
    int index = BufferManager::bm().getPage(this->name, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    return buf;
}

unsigned Table::getRows() const {
    return this->header->rows;
}

int Table::getColumnIndex(const std::string &column) const {
    for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
        if (column == this->header->column_info[i].name) {
            return i;
        }
    }
    return -1;
}

ColumnType Table::getColumnType(int index) const {
    return this->header->column_info[index].type;
}

unsigned Table::getColumnLength(int index) const {
    return this->header->column_info[index].length;
}

std::vector<std::string> Table::getColumns() const {
    std::vector<std::string> columns;
    for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
        columns.emplace_back(this->header->column_info[i].name);
    }
    return columns;
}

Table *Table::createTable(const std::string &name) {
    if (FileManager::fm().createFile(name) != 0) return nullptr;
    auto table = new Table(name);
    table->header->pages = Table::_getHeaderPageNum();
    table->header->columns = 0;
    table->header->rows = 0;
    table->header->next_empty = 0;
    return table;
}

void Table::addColumn(const Column &column, const std::string &after) {
    if (this->header->columns >= MAX_COLUMN) {
        throw SqlDBException("reached max column number");
    }
    int index = -1;
    if (after.empty()) index = 0;
    else {
        for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
            if (this->header->column_info[i].name == after) {
                index = i + 1;
                break;
            }
        }
    }
    if (index == -1) {
        throw SqlDBException("column not found: " + after);
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
            index == 0 ? sizeof(unsigned) : this->header->column_info[index - 1].offset
                             + this->header->column_info[index - 1].length;
    ++this->header->columns;
    if (column.flags & FLAG_HAS_DEFAULT) {
        // move defaults to the right
        unsigned offset_begin = this->header->column_info[index].offset;
        unsigned offset_end = offset_begin + this->header->column_info[index].length;
        memcpy(this->header->defaults + offset_end,
               this->header->defaults + offset_begin,
               sizeof(this->header->defaults) - offset_end);
        serializeFromString(column.default_value, column.type,
                            this->header->defaults + offset_begin,
                            column.length);
    }
    // TODO modify data (this function should only be called when creating table for now)
}

void Table::insertRecord(const std::vector<std::string> &values) {
    if (values.size() != this->header->columns) {
        throw SqlDBException("column number mismatch");
    }
    auto data = new unsigned char[this->_getRecordSizeWithFlag()];
    for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
        serializeFromString(values[i], this->header->column_info[i].type,
                            data + this->header->column_info[i].offset,
                            this->header->column_info[i].length);
        *(unsigned *) data = 0;  // TODO null flags
    }
    this->_insertRecord(data);
    delete[] data;
}
