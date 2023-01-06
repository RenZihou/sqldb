// cursor.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TABLE_CURSOR_H_
#define TABLE_CURSOR_H_

#include <cstring>

#include "table.h"

class RecordCursor {
private:
    Table *table;
    unsigned page;
    unsigned slot;
    BufType valid;
    BufType cached_record;

    bool _step();

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

    bool next();

    [[nodiscard]] Type *get(int column) const {
        return deserialize(this->cached_record +
                           this->table->header->column_info[column].offset,
                           this->table->header->column_info[column].type);
    }

    [[nodiscard]] unsigned getOffset() const {
        return (this->page << PAGE_SIZE_IDX) + PAGE_HEADER_SIZE +
               this->slot * this->table->_getRecordSizeWithFlag();
    }

    void del();

    void set(const std::vector<std::tuple<int, Type *>> &updates);

    /**
     * @brief move cursor to given offset in table
     * @param offset offset of a record
     * @param do_validate [NO USE NOW: WILL ALWAYS SKIP VALIDATION, AND CANNOT FURTHER `next()`]
     */
    void moveTo(unsigned offset, bool do_validate = true);

    void reset();
};

#endif  // TABLE_CURSOR_H_
