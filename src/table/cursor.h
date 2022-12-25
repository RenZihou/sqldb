// cursor.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TABLE_CURSOR_H_
#define TABLE_CURSOR_H_

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
    explicit RecordCursor(Table *table)
            : table(table), page(Table::_getHeaderPageNum() - 1),
              slot(table->_getSlotNum() - 1),
              valid(new unsigned char[PAGE_HEADER_SIZE]),
              cached_record(new unsigned char[table->_getRecordSizeWithFlag()]) {}

    ~RecordCursor() {
        delete[] valid;
        delete[] cached_record;
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
        return deserialize(this->cached_record +
                           this->table->header->column_info[column].offset,
                           this->table->header->column_info[column].type,
                           this->table->header->column_info[column].length);
    }

    [[nodiscard]] unsigned getOffset() const {
        return (this->page << PAGE_SIZE_IDX) + PAGE_HEADER_SIZE + this->slot * this->table->_getRecordSizeWithFlag();
    }

    void del() {
        this->table->_deleteRecord(this->page, this->slot);
    }

    void set(const std::vector<std::tuple<int, std::string>> &updates) {
        for (auto &update: updates) {
            serializeFromString(std::get<1>(update),
                                this->table->header->column_info[std::get<0>(update)].type,
                                this->cached_record +
                                this->table->header->column_info[std::get<0>(update)].offset,
                                this->table->header->column_info[std::get<0>(update)].length);
        }
        this->table->_updateRecord(this->page, this->slot, this->cached_record);
    }

    /**
     * @brief move cursor to given offset in table
     * @param offset offset of a record
     * @param do_validate [NO USE NOW: WILL ALWAYS SKIP VALIDATION, AND CANNOT FURTHER `next()`]
     */
    void moveTo(unsigned offset, bool do_validate = true) {
        this->table->_offset_to_slot(offset, this->page, this->slot);
        int index = BufferManager::bm().getPage(this->table->name, this->page);
        memcpy(this->cached_record,
               BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK),
               this->table->_getRecordSizeWithFlag());
    }

    void reset() {
        this->page = Table::_getHeaderPageNum() - 1;
        this->slot = table->_getSlotNum() - 1;
    }
};

#endif  // TABLE_CURSOR_H_
