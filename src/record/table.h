// Table.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef TABLE_H_
#define TABLE_H_

#include <string>
#include "../util/constants.h"


enum ColumnType {
    INT,
    FLOAT,
    VARCHAR,
//    DATE,
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
    TableHeader header;
    std::string name;

public:
    Table();

    ~Table();

    int create(std::string table_name);

    /**
     * @param data serialized data
     * @description insert a record into table
     */
    void insertRecord(char *data);

    /**
     * @param record_offset record offset in bytes
     * @description delete a record from table
     */
    void deleteRecord(unsigned record_offset);

    /**
     * @param record_offset record offset in bytes
     * @param data new serialized data
     */
    void updateRecord(unsigned record_offset, char *data);

    /**
     * @param record_offset record offset in bytes
     * @return record
     */
    char *selectRecord(unsigned record_offset);
};


#endif  // TABLE_H_
