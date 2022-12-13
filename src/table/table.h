// Table.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TABLE_H_
#define TABLE_H_

#include <memory>
#include <string>
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
    bool not_null;
    std::string default_value;

};

struct ColumnInfo {
    char name[MAX_COLUMN_NAME_LEN];
    ColumnType type;
    unsigned length;
    unsigned offset;
};

struct TableHeader {
    unsigned columns;
    unsigned pages;
    unsigned next_empty;  // next empty slot offset in bytes
    ColumnInfo column_info[MAX_COLUMN];
    // TODO constraints
};


class Table {
private:
//    std::unique_ptr<TableHeader> header;
    TableHeader *header;
    const std::string name;
    const int header_pages = sizeof(TableHeader) / PAGE_SIZE + 1;

public:
    explicit Table(std::string table_name);

    ~Table();

    /**
     * @param table_name table name
     * @return 0 for success, -1 for error
     * @description create new table, this will create the table file only
     */
    static int createTable(const std::string &table_name);

    /**
     * @param column new column info
     * @param after insert after which column, set "" to insert at the beginning
     * @return 0 for success, -1 for error
     * @description (ONLY CALL ON CREATE TABLE FOR NOW) add a new column to the table
     */
    int addColumn(const Column &column, const std::string &after);

    /**
     * @param data serialized data
     * @description insert a record into table
     */
    void insertRecord(void *data);

    /**
     * @param record_offset record offset in bytes
     * @description delete a record from table
     */
    void deleteRecord(unsigned record_offset);

    /**
     * @param record_offset record offset in bytes
     * @param data new serialized data
     */
    void updateRecord(unsigned record_offset, void *data);

    /**
     * @param record_offset record offset in bytes
     * @return record
     */
    void *selectRecord(unsigned record_offset);
};


#endif  // TABLE_H_
