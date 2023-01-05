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

struct ForeignKey {
    std::string name;
    int column;
    std::string ref_table;
};

struct ForeignKeyInfo {
    char name[MAX_KEY_LEN];
    int column;
    char ref_table[MAX_TABLE_NAME_LEN];
};

struct Reference {
    std::string fk_table;
    int fk_column;
};

struct ReferenceInfo {
    char fk_table[MAX_TABLE_NAME_LEN];
    int fk_column;
};

struct TableHeader {
    unsigned columns;  // number of columns
    unsigned rows;  // number of rows
    unsigned pages;  // number of pages (includes header)
    unsigned next_empty;  // next empty slot offset in bytes
    ColumnInfo column_info[MAX_COLUMN];
    unsigned char defaults[MAX_RECORD_SIZE + sizeof(unsigned)];
    char primary_key[MAX_KEY_LEN];
    unsigned foreign_keys;
    ForeignKeyInfo foreign_key_info[MAX_FOREIGN_KEY];
    unsigned references;
    ReferenceInfo reference_info[MAX_REFERENCED];
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
     * @return inserted record offset
     */
    unsigned _insertRecord(void *data);

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
     * @param index column index, caller should ensure index is valid
     * @return whether column can be null
     */
    [[nodiscard]] bool getColumnNullable(int index) const;

    /**
     * @param index column index, caller should ensure index is valid
     * @return column default value
     */
    [[nodiscard]] Type *getColumnDefault(int index) const;

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
     * @brief erase `has index` flag on given column
     * @param column
     */
    void dropIndex(int column);

    /**
     * @brief add primary key to table
     * @param column primary key column
     * @param key primary key name (can be empty)
     */
    void addPrimaryKey(int column, const std::string &key);

    /**
     * @param column
     * @return whether the column is primary key
     */
    bool isPrimaryKey(int column);

    /**
     * @return whether has primary key
     */
    bool getPrimaryKey(int &column, std::string &key);

    /**
     * @brief drop (the only) primary key
     */
    void dropPrimaryKey();

    /**
     * @brief add foreign key to table
     * @param column foreign key column
     * @param key foreign key name (should not be empty)
     * @param ref_table which table ('s primary key) the foreign key references
     */
    void addForeignKey(int column, const std::string &key, const std::string &ref_table);

    /**
     * @return whether has foreign key with given name
     */
    bool getForeignKey(const std::string &key, int &column, std::string &ref_table);

    std::vector<ForeignKey> getForeignKeys();

    /**
     * @brief drop foreign key with given name
     * @param key foreign key name
     */
    void dropForeignKey(const std::string &key);

    /**
     * @brief add reference to primary key, allows multiple same references
     * @param fk_table foreign key table
     * @param fk_key foreign key column
     */
    void addReferenced(const std::string &fk_table, int fk_column);

    std::vector<Reference> getReferences();

    /**
     * @brief drop reference to primary key, only drop one reference
     * @param fk_table foreign key table
     * @param fk_column foreign key column
     */
    void dropReferenced(const std::string &fk_table, int fk_column);
};

#endif  // TABLE_H_
