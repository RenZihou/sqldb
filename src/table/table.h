// Table.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TABLE_H_
#define TABLE_H_

#include <memory>
#include <string>
#include <vector>

#include "../util/constants.h"


enum class ColumnType {
    INT,
    FLOAT,
    VARCHAR,
//    DATE,
    UNKNOWN,
};

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
    unsigned pages;  // number of pages (includes header)
    unsigned next_empty;  // next empty slot offset in bytes
    ColumnInfo column_info[MAX_COLUMN];
    char defaults[MAX_RECORD_SIZE];
    // TODO constraints
};


class Table {
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
     * @param value value in string format
     * @param type value type (column type)
     * @param buffer buffer to write serialized data
     * @param length data length (number of bytes to be written in buffer)
     * @description serialize value to byte-form buffer
     */
    static void _serialize(const std::string &value, ColumnType type, char *buffer, unsigned length);

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

    void insertRecord(const std::vector<std::string> &values);

};


#endif  // TABLE_H_
