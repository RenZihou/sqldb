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
    unsigned char defaults[MAX_RECORD_SIZE + sizeof(unsigned)];
    // TODO constraints
};

class Table {
    friend RecordCursor;
private:
    TableHeader *header;
    const std::string name;

//    /**
//     * @return size of a record (excluding null flag) in bytes
//     */
//    [[nodiscard]] unsigned _getRecordSize() const;

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
     * @brief convert record offset to (page, slot) pair
     * @param offset record offset
     * @param page page number (to be filled)
     * @param slot slot number (to be filled)
     */
    void _offset_to_slot(unsigned offset, unsigned &page, unsigned &slot) const;

    void _slot_to_offset(unsigned &offset, unsigned page, unsigned slot) const;

    /**
     * @brief insert a record into table
     * @param data serialized data including null flag
     */
    void _insertRecord(void *data);

    /**
     * @brief delete a record from table
     * @param page record page
     * @param slot record slot in page
     */
    void _deleteRecord(unsigned page, unsigned slot);

    /**
     * @param page record page
     * @param slot record slot in page
     * @param data new serialized data including null flag
     */
    void _updateRecord(unsigned page, unsigned slot, void *data);

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
     * @brief add `has index` flag to a column
     * @param column column that (will) have index
     */
    void addIndex(int column);

    /**
     * @param column
     * @return whether the column has a `has index` flag
     */
    bool hasIndex(int column);

    /**
     * @return column names
     */
    [[nodiscard]] std::vector<std::string> getColumns() const;

    /**
     * @brief create new table, this will create the table file only
     * @param table_name table name
     * @return created table pointer, nullptr if created failed
     */
    static Table *createTable(const std::string &table_name);

    /**
     * @brief (ONLY CALL ON CREATE TABLE FOR NOW) add a new column to the table
     * @param column new column info
     * @param after insert after which column, set "" to insert at the beginning
     */
    void addColumn(const Column &column, const std::string &after);

    /**
     * @brief insert a new record into table
     * @param values list of values in plain text sorted by column index
     */
    void insertRecord(const std::vector<std::string> &values);
};

#endif  // TABLE_H_
