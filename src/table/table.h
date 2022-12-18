// Table.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TABLE_H_
#define TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "../util/constants.h"
#include "type.h"

class RecordCursor;

struct Column {
    std::string name;
    ColumnType type;
    unsigned length;
    char flags;
    std::string default_value;
};

struct ColumnInfo {
    char name[MAX_COLUMN_NAME_LEN];
    char flags;
    ColumnType type;
    unsigned length;
    unsigned offset;
};

struct TableHeader {
    unsigned columns;  // number of columns
    unsigned rows;  // number of rows
    unsigned pages;  // number of pages (includes header)
    unsigned next_empty;  // next empty slot offset in bytes
    ColumnInfo column_info[MAX_COLUMN];
    char defaults[MAX_RECORD_SIZE];
    // TODO constraints
};

class Table {
    friend RecordCursor;
private:
    TableHeader *header;
    const std::string name;

    /**
     * @return size of a record (excluding null flag) in bytes
     */
    [[nodiscard]] unsigned _getRecordSize() const;

    /**
     * @return size of a record (including null flag) in bytes
     */
    [[nodiscard]] unsigned _getRecordSizeWithFlag() const;

    /**
     * @return number of table-header pages
     */
    [[nodiscard]] static unsigned _getHeaderPageNum();

    /**
     * @return number of slots in a page
     */
    [[nodiscard]] unsigned _getSlotNum() const;

    /**
     * @param offset record offset
     * @param page page number (to be filled)
     * @param slot slot number (to be filled)
     * @description convert record offset to (page, slot) pair
     */
     void _offset_to_slot(unsigned offset, unsigned &page, unsigned &slot) const;

    /**
     * @param data serialized data including null flag
     * @description insert a record into table
     */
    void _insertRecord(void *data);

    /**
     * @param record_offset record offset in bytes
     * @description delete a record from table
     */
    void _deleteRecord(unsigned record_offset);

    /**
     * @param record_offset record offset in bytes
     * @param data new serialized data including null flag
     */
    void _updateRecord(unsigned record_offset, void *data);

    /**
     * @param record_offset record offset in bytes
     * @return record
     */
    void *_selectRecord(unsigned record_offset);

public:
    explicit Table(std::string table_name);

    ~Table();

    [[nodiscard]] unsigned getRows() const;

    /**
     * @param column column name
     * @return column index, -1 if not found
     */
    [[nodiscard]] int getColumnIndex(const std::string &column) const;

    /**
     * @param index column index, caller should ensure index is valid
     * @return column type
     */
    [[nodiscard]] ColumnType getColumnType(int index) const;

    /**
     * @param index column index, caller should ensure index is valid
     * @return column length
     */
    [[nodiscard]] unsigned getColumnLength(int index) const;

    /**
     * @param table_name table name
     * @return created table pointer, nullptr if created failed
     * @description create new table, this will create the table file only
     */
    static Table *createTable(const std::string &table_name);

    /**
     * @param column new column info
     * @param after insert after which column, set "" to insert at the beginning
     * @return 0 for success, -1 for error
     * @description (ONLY CALL ON CREATE TABLE FOR NOW) add a new column to the table
     */
    int addColumn(const Column &column, const std::string &after);

    /**
     * @param values list of values in plain text sorted by column index
     * @description insert a new record into table
     */
    void insertRecord(const std::vector<std::string> &values);

};

#endif  // TABLE_H_
