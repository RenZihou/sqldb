// cursor.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef CURSOR_H_
#define CURSOR_H_

#include <cstring>

#include "table.h"
#include "../pagefile/buffer_manager.h"

class RecordCursor {
private:
    Table *table;
    unsigned page;
    unsigned slot;
    BufType valid;
    BufType cached_record;

    bool _step() {
        if (this->slot == this->table->_getSlotNum() - 1) {
            this->slot = 0;
            ++this->page;
            if (this->page == this->table->header->pages) return false;  // end of table
            int index = BufferManager::bm().getPage(this->table->name, this->page);
            memcpy(this->valid, BufferManager::bm().readBuffer(index), PAGE_HEADER_SIZE);
        } else {
            ++this->slot;
        }
        return true;
    }

public:
    explicit RecordCursor(Table *table) : table(table), page(Table::_getHeaderPageNum() - 1),
                                          slot(table->_getSlotNum() - 1),
                                          valid(new unsigned char[(PAGE_HEADER_SIZE >> 3) + 1]),
                                          cached_record(new unsigned char[table->_getRecordSizeWithFlag()]) {}

    ~RecordCursor() {
        delete[] valid;
    }

    bool next() {
        while (this->_step()) {
            if (this->valid[this->slot >> 3] & (1 << (this->slot & 7))) {
                int index = BufferManager::bm().getPage(this->table->name, this->page);
                memcpy(this->cached_record,
                       BufferManager::bm().readBuffer(index) + PAGE_HEADER_SIZE +
                       this->slot * this->table->_getRecordSizeWithFlag(),
                       this->table->_getRecordSizeWithFlag());
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] Type *get(int column) const {
        return deserialize(this->cached_record + sizeof(unsigned) + this->table->header->column_info[column].offset,
                           this->table->header->column_info[column].type,
                           this->table->header->column_info[column].length);
    }

    void reset() {
        this->page = Table::_getHeaderPageNum() - 1;
        this->slot = table->_getSlotNum() - 1;
    }
};

#endif  // CURSOR_H_
