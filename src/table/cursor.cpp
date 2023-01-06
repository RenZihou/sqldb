// RecordCursor.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "cursor.h"
#include "../query/op.h"
#include "../index/int_index.h"
#include "../pagefile/buffer_manager.h"
#include "../util/exception.h"

bool RecordCursor::_step() {
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

bool RecordCursor::next() {
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

void RecordCursor::del() {
    if (this->table->header->references > 0) {
        int pk_column;
        std::string pk_name;
        this->table->getPrimaryKey(pk_column, pk_name);
        auto old_type = this->get(pk_column);
        int old_value = dynamic_cast<Int *>(old_type)->getValue();
        delete old_type;
        for (unsigned i = 0; i < this->table->header->references; ++i) {
            auto fk_table = new Table(this->table->header->reference_info[i].fk_table);
            std::string fk_column = fk_table->header->column_info[this->table->header->reference_info[i].fk_column].name;
            std::vector<std::tuple<std::string, std::string>> selectors;
            selectors.emplace_back("", fk_column);
            auto condition = new ConditionCmp(new ExprColumn("", fk_column), new ExprValue(std::to_string(old_value)), new Equal);
            CounterPrinter printer;
            OpTableSelect(selectors, {this->table->header->reference_info[i].fk_table}, {condition}).execute(&printer);
            if (printer.getCount() > 0) {
                throw SqlDBException("foreign key constraint violated: reference exists");
            }
        }

    }
    this->table->_deleteRecord(this->page, this->slot);
}

void RecordCursor::set(const std::vector<std::tuple<int, std::string>> &updates) {
    for (auto &[column, value_s]: updates) {
        unsigned pos, offset;
        bool match;
        if (this->table->header->column_info[column].flags & FLAG_IS_PRIMARY) {
            // check primary key constraint
            IntIndex index(this->table->name, this->table->header->column_info[column].name);
            if (index.search(std::stoi(value_s), pos, offset, match) && match) {
                throw SqlDBException("primary key constraint violated: duplicate value");
            }
            auto old_type = this->get(column);
            int old_value = dynamic_cast<Int *>(old_type)->getValue();
            delete old_type;
            // check reference constraint
            for (unsigned i = 0; i < this->table->header->references; ++i) {
                auto fk_table = new Table(this->table->header->reference_info[i].fk_table);
                std::string fk_column = fk_table->header->column_info[this->table->header->reference_info[i].fk_column].name;
                std::vector<std::tuple<std::string, std::string>> selectors;
                selectors.emplace_back("", fk_column);
                auto condition = new ConditionCmp(new ExprColumn("", fk_column), new ExprValue(std::to_string(old_value)), new Equal);
                CounterPrinter printer;
                OpTableSelect(selectors, {this->table->header->reference_info[i].fk_table}, {condition}).execute(&printer);
                if (printer.getCount() > 0) {
                    throw SqlDBException("foreign key constraint violated: reference exists");
                }
            }
        }
        // check foreign key constraint
        if (this->table->header->column_info[column].flags & FLAG_IS_FOREIGN) {
            for (unsigned i = 0; i < this->table->header->foreign_keys; ++i) {
                if (this->table->header->foreign_key_info[i].column == column) {
                    Table ref_table(this->table->header->foreign_key_info[i].ref_table);
                    int ref_column;
                    std::string ref_pk;
                    ref_table.getPrimaryKey(ref_column, ref_pk);
                    IntIndex index_(this->table->header->foreign_key_info[i].ref_table,
                                    ref_table.header->column_info[ref_column].name);
                    if (!(index_.search(std::stoi(value_s),pos, offset, match) && match)) {
                        throw SqlDBException(
                                "foreign key constraint violated: no target found");
                    }
                }
            }
        }
        serializeFromString(value_s,
                            this->table->header->column_info[column].type,
                            this->cached_record +
                            this->table->header->column_info[column].offset,
                            this->table->header->column_info[column].length);
    }
    this->table->_updateRecord(this->page, this->slot, this->cached_record);
}

void RecordCursor::moveTo(unsigned int offset, bool do_validate) {
    this->table->_offset_to_slot(offset, this->page, this->slot);
    int index = BufferManager::bm().getPage(this->table->name, this->page);
    memcpy(this->cached_record,
           BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK),
           this->table->_getRecordSizeWithFlag());
}

void RecordCursor::reset() {
    this->page = Table::_getHeaderPageNum() - 1;
    this->slot = table->_getSlotNum() - 1;
}