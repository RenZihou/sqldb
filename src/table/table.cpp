// Table.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <utility>

#include "table.h"
#include "../index/int_index.h"
#include "../pagefile/buffer_manager.h"
#include "../util/exception.h"

Table::Table(std::string table_name) : header(new TableHeader()), name(std::move(table_name)) {
    // read table header
    int unread = sizeof(TableHeader);
    int pid = 0;
    int index;
    while (unread > PAGE_SIZE) {
        index = BufferManager::bm().getPage(this->name, pid);
        memcpy((char *) this->header + PAGE_SIZE * pid, BufferManager::bm().readBuffer(index),
               PAGE_SIZE);
        unread -= PAGE_SIZE;
        ++pid;
    }
    index = BufferManager::bm().getPage(this->name, pid);
    memcpy((char *) this->header + PAGE_SIZE * pid, BufferManager::bm().readBuffer(index), unread);
}

Table::~Table() {
    int unwritten = sizeof(TableHeader);
    int pid = 0;
    int index;
    while (unwritten > PAGE_SIZE) {
        index = BufferManager::bm().getPage(this->name, pid);
        memcpy(BufferManager::bm().readBuffer(index), (char *) this->header + PAGE_SIZE * pid,
               PAGE_SIZE);
        BufferManager::bm().markDirty(index);
        unwritten -= PAGE_SIZE;
        ++pid;
    }
    index = BufferManager::bm().getPage(this->name, pid);
    memcpy(BufferManager::bm().readBuffer(index), (
                                                          char *) this->header + PAGE_SIZE * pid,
           unwritten);
    BufferManager::bm().markDirty(index);
    delete this->header;
}

inline unsigned Table::_getRecordSizeWithFlag() const {
    return this->header->column_info[this->header->columns - 1].offset +
           this->header->column_info[this->header->columns - 1].length;
}

inline unsigned Table::_getHeaderPageNum() {
    return (sizeof(TableHeader) - 1) / PAGE_SIZE + 1;
}

unsigned Table::_getSlotNum() const {
    return (PAGE_SIZE - PAGE_HEADER_SIZE) / this->_getRecordSizeWithFlag();
}

inline void Table::_offset_to_slot(unsigned int offset, unsigned int &page,
                                   unsigned int &slot) const {
    page = offset >> PAGE_SIZE_IDX;
    slot = ((offset & PAGE_SIZE_MASK) - PAGE_HEADER_SIZE) / this->_getRecordSizeWithFlag();
}

inline void
Table::_slot_to_offset(unsigned int &offset, unsigned int page, unsigned int slot) const {
    offset = (page << PAGE_SIZE_IDX) + PAGE_HEADER_SIZE + slot * this->_getRecordSizeWithFlag();
}

unsigned Table::_insertRecord(void *data) {
    int index;  // index of buffer to write
    BufType buf;  // buffer to write
    unsigned ret;
    if (this->header->next_empty == 0) {  // all pages are occupied
        ret = (this->header->pages << PAGE_SIZE_IDX) + PAGE_HEADER_SIZE;
        index = BufferManager::bm().allocPage(this->name, this->header->pages);
        buf = BufferManager::bm().readBuffer(index);
        for (int i = 0; i < PAGE_HEADER_SIZE; ++i) {
            buf[i] = 0;
        }
        buf[0] = 1;  // set first slot to 1 (occupied)
        // write next empty on this page
        unsigned next_empty_offset = PAGE_HEADER_SIZE + this->_getRecordSizeWithFlag();
        int available_in_page = (PAGE_SIZE - PAGE_HEADER_SIZE)
                                / static_cast<int>(this->_getRecordSizeWithFlag()) -
                                1;  // # of tot slots
        if (available_in_page > 0) {
            this->header->next_empty = (this->header->pages << PAGE_SIZE_IDX) + next_empty_offset;
            while (--available_in_page) {
                *(unsigned *) (buf + next_empty_offset) =
                        (this->header->pages << PAGE_SIZE_IDX)
                        + (next_empty_offset + this->_getRecordSizeWithFlag());
                next_empty_offset += this->_getRecordSizeWithFlag();
            }
            *(unsigned *) (buf + next_empty_offset) = 0;
        } else {
            this->header->next_empty = 0;
        }
        ++this->header->pages;
        buf += PAGE_HEADER_SIZE;
    } else {
        ret = this->header->next_empty;
        unsigned page, slot;
        this->_offset_to_slot(this->header->next_empty, page, slot);
        index = BufferManager::bm().getPage(this->name, page);
        buf = BufferManager::bm().readBuffer(index);
        buf[slot >> 3] |= (1 << (slot & 7));  // set slot to 1 (occupied)
        buf += (this->header->next_empty & PAGE_SIZE_MASK);
        this->header->next_empty = ((unsigned *) buf)[0];
    }
    memcpy(buf, data, this->_getRecordSizeWithFlag());
    BufferManager::bm().markDirty(index);
    ++this->header->rows;
    return ret;
}

void Table::_deleteRecord(unsigned page, unsigned slot) {
    unsigned offset;
    this->_slot_to_offset(offset, page, slot);
    this->_offset_to_slot(offset, page, slot);
    int index = BufferManager::bm().getPage(this->name, page);
    BufType buf = BufferManager::bm().readBuffer(index);
    buf[slot >> 3] &= ~(1 << (slot & 7));  // set slot to 0 (empty)
    buf += (offset & PAGE_SIZE_MASK);
    ((unsigned *) buf)[0] = this->header->next_empty;
    this->header->next_empty = offset;
    BufferManager::bm().markDirty(index);
    --this->header->rows;
}

void Table::_updateRecord(unsigned page, unsigned slot, void *data) {
    unsigned offset;
    this->_slot_to_offset(offset, page, slot);
    int index = BufferManager::bm().getPage(this->name, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK);
    memcpy(buf, data, this->_getRecordSizeWithFlag());
    BufferManager::bm().markDirty(index);
}

void *Table::_selectRecord(unsigned record_offset) {
    unsigned page, slot;
    this->_offset_to_slot(record_offset, page, slot);
    int index = BufferManager::bm().getPage(this->name, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (record_offset & PAGE_SIZE_MASK);
    return buf;
}

unsigned Table::getRows() const {
    return this->header->rows;
}

int Table::getColumnIndex(const std::string &column) const {
    for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
        if (column == this->header->column_info[i].name) {
            return i;
        }
    }
    return -1;
}

ColumnType Table::getColumnType(int index) const {
    return this->header->column_info[index].type;
}

unsigned Table::getColumnLength(int index) const {
    return this->header->column_info[index].length;
}

bool Table::getColumnNullable(int index) const {
    return !(this->header->column_info[index].flags & FLAG_NOT_NULL);
}

Type *Table::getColumnDefault(int index) const {
    if (this->header->column_info[index].flags & FLAG_HAS_DEFAULT) {
        return deserialize(this->header->defaults + this->header->column_info[index].offset,
                           this->header->column_info[index].type,
                           this->header->column_info[index].length);
    } else {
        return nullptr;
    }
}

std::vector<std::string> Table::getColumns() const {
    std::vector<std::string> columns;
    for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
        columns.emplace_back(this->header->column_info[i].name);
    }
    return columns;
}

Table *Table::createTable(const std::string &name) {
    if (FileManager::fm().createFile(name) != 0) return nullptr;
    auto table = new Table(name);
    table->header->pages = Table::_getHeaderPageNum();
    table->header->columns = 0;
    table->header->rows = 0;
    table->header->next_empty = 0;
    table->header->foreign_keys = 0;
    table->header->references = 0;
    return table;
}

void Table::addColumn(const Column &column, const std::string &after) {
    if (this->header->columns >= MAX_COLUMN) {
        throw SqlDBException("reached max column number");
    }
    if (this->getColumnIndex(column.name) != -1) {
        throw SqlDBException("column already exists");
    }
    if (column.name.length() >= MAX_COLUMN_NAME_LEN) {
        throw SqlDBException("column name too long");
    }
    int index = -1;
    if (after.empty()) index = 0;
    else {
        for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
            if (this->header->column_info[i].name == after) {
                index = i + 1;
                break;
            }
        }
    }
    if (index == -1) {
        throw SqlDBException("column not found: " + after);
    }
    // modify header
    for (int i = static_cast<int>(this->header->columns); i > index; --i) {
        this->header->column_info[i] = this->header->column_info[i - 1];
        this->header->column_info[i].offset += column.length;
    }
    memcpy(this->header->column_info[index].name, column.name.c_str(), column.name.length() + 1);
    this->header->column_info[index].flags = column.flags;
    this->header->column_info[index].type = column.type;
    this->header->column_info[index].length = column.length;
    this->header->column_info[index].offset =
            index == 0 ? sizeof(unsigned) : this->header->column_info[index - 1].offset
                                            + this->header->column_info[index - 1].length;
    ++this->header->columns;
    // move defaults to the right
    unsigned offset_begin = this->header->column_info[index].offset;
    unsigned offset_end = offset_begin + this->header->column_info[index].length;
    memcpy(this->header->defaults + offset_end,
           this->header->defaults + offset_begin,
           sizeof(this->header->defaults) - offset_end);
    if (column.flags & FLAG_HAS_DEFAULT) {
        column.default_value->serialize(this->header->defaults + offset_begin, column.length);
    }
    // TODO modify data (this function should only be called when creating table for now)
    // TODO modify fk / ref info
}

void Table::insertRecord(const std::vector<Type *> &values) {
    if (values.size() != this->header->columns) {
        throw SqlDBException("column number mismatch");
    }
    unsigned index_pos, index_offset;
    bool index_match;
    // check for primary key unique
    for (unsigned i = 0; i < this->header->columns; ++i) {
        if (this->header->column_info[i].flags & FLAG_IS_PRIMARY) {
            IntIndex index(this->name, this->header->column_info[i].name);
            if (index.search(dynamic_cast<Int *>(values[i])->getValue(),
                             index_pos, index_offset, index_match) && index_match) {
                throw SqlDBException("primary key constraint violated: duplicate value");
            }
        }
    }
    // check for foreign key
    for (unsigned i = 0; i < this->header->foreign_keys; ++i) {
        Table ref_table(this->header->foreign_key_info[i].ref_table);
        int ref_column;
        std::string ref_pk;
        ref_table.getPrimaryKey(ref_column, ref_pk);
        IntIndex index_(this->header->foreign_key_info[i].ref_table,
                        ref_table.header->column_info[ref_column].name);
        if (!(index_.search(dynamic_cast<Int *>((values[this->header->foreign_key_info[i].column]))->getValue(),
                            index_pos, index_offset, index_match) && index_match)) {
            throw SqlDBException("foreign key constraint violated: no target found");
        }
    }
    auto data = new unsigned char[this->_getRecordSizeWithFlag()];
    for (unsigned i = 0; i < this->header->columns; ++i) {
        if (values[i]->getType() != this->header->column_info[i].type) {
            throw SqlDBException("column type mismatch");
        }
        values[i]->serialize(data + this->header->column_info[i].offset,
                             this->header->column_info[i].length);
        *(unsigned *) data = 0;  // TODO null flags
    }
    unsigned offset = this->_insertRecord(data);
    for (unsigned i = 0; i < this->header->columns; ++i) {
        if (this->header->column_info[i].flags & FLAG_HAS_INDEX) {
            IntIndex index(this->name, this->header->column_info[i].name);
            index.insert(dynamic_cast<Int *>(values[i])->getValue(), offset);
        }
    }
    delete[] data;
}

void Table::addIndex(int column) {
    this->header->column_info[column].flags |= FLAG_HAS_INDEX;
}

bool Table::hasIndex(int column) {
    return this->header->column_info[column].flags & FLAG_HAS_INDEX;
}

void Table::dropIndex(int column) {
    this->header->column_info[column].flags &= ~FLAG_HAS_INDEX;
}

void Table::addPrimaryKey(int column, const std::string &key) {
    if (key.length() >= MAX_KEY_LEN - 1) {
        throw SqlDBException("key too long");
    }
    this->header->column_info[column].flags |= FLAG_IS_PRIMARY;
    memcpy(this->header->primary_key, key.c_str(), key.length() + 1);
}

bool Table::isPrimaryKey(int column) {
    return this->header->column_info[column].flags & FLAG_IS_PRIMARY;
}

bool Table::getPrimaryKey(int &column, std::string &key) {
    for (int i = 0; i < static_cast<int>(this->header->columns); ++i) {
        if (this->header->column_info[i].flags & FLAG_IS_PRIMARY) {
            column = i;
            key = this->header->primary_key;
            return true;
        }
    }
    return false;
}

void Table::dropPrimaryKey() {
    for (unsigned i = 0; i < this->header->columns; ++i) {
        this->header->column_info[i].flags &= ~FLAG_IS_PRIMARY;
    }
    this->header->primary_key[0] = '\0';
}

void Table::addForeignKey(int column, const std::string &key, const std::string &ref_table) {
    if (key.length() >= MAX_KEY_LEN - 1) {
        throw SqlDBException("key too long");
    }
    if (this->header->foreign_keys >= MAX_FOREIGN_KEY) {
        throw SqlDBException("reached max foreign key number");
    }
    for (unsigned i = 0; i < this->header->foreign_keys; ++i) {
        if (this->header->foreign_key_info[i].name == key) {
            throw SqlDBException("foreign key with same name already exists");
        }
    }
    this->header->column_info[column].flags |= FLAG_IS_FOREIGN;
    this->header->foreign_key_info[this->header->foreign_keys].column = column;
    memcpy(this->header->foreign_key_info[this->header->foreign_keys].name, key.c_str(),
           key.length() + 1);
    memcpy(this->header->foreign_key_info[this->header->foreign_keys].ref_table, ref_table.c_str(),
           ref_table.length() + 1);
    ++this->header->foreign_keys;
}

bool Table::getForeignKey(const std::string &key, int &column, std::string &ref_table) {
    for (unsigned i = 0; i < this->header->foreign_keys; ++i) {
        if (this->header->foreign_key_info[i].name == key) {
            column = this->header->foreign_key_info[i].column;
            ref_table = this->header->foreign_key_info[i].ref_table;
            return true;
        }
    }
    return false;
}

std::vector<ForeignKey> Table::getForeignKeys() {
    std::vector<ForeignKey> fks;
    for (unsigned i = 0; i < this->header->foreign_keys; ++i) {
        fks.push_back(ForeignKey{this->header->foreign_key_info[i].name,
                                 this->header->foreign_key_info[i].column,
                                 this->header->foreign_key_info[i].ref_table});
    }
    return fks;
}

void Table::dropForeignKey(const std::string &key) {
    int column = -1;
    for (unsigned i = 0; i < this->header->foreign_keys; ++i) {
        if (this->header->foreign_key_info[i].name == key) {
            column = this->header->foreign_key_info[i].column;
            this->header->column_info[this->header->foreign_key_info[i].column].flags &=
                    ~FLAG_IS_FOREIGN;
            for (unsigned j = i; j < this->header->foreign_keys - 1; ++j) {
                this->header->foreign_key_info[j] = this->header->foreign_key_info[j + 1];
            }
            --this->header->foreign_keys;
            break;
        }
    }
    for (unsigned i = 0; i < this->header->foreign_keys; ++i) {
        if (this->header->foreign_key_info[i].column == column) {
            this->header->column_info[column].flags |= FLAG_IS_FOREIGN;
            break;
        }
    }
}

void Table::addReferenced(const std::string &fk_table, int fk_column) {
    if (this->header->references >= MAX_REFERENCED) {
        throw SqlDBException("reached max reference number");
    }
    this->header->reference_info[this->header->references].fk_column = fk_column;
    memcpy(this->header->reference_info[this->header->references].fk_table, fk_table.c_str(),
           fk_table.length() + 1);
    ++this->header->references;
}

std::vector<Reference> Table::getReferences() {
    std::vector<Reference> refs;
    for (unsigned i = 0; i < this->header->references; ++i) {
        refs.push_back(Reference{this->header->reference_info[i].fk_table,
                                 this->header->reference_info[i].fk_column});
    }
    return refs;
}

void Table::dropReferenced(const std::string &fk_table, int fk_column) {
    for (unsigned i = 0; i < this->header->references; ++i) {
        if (this->header->reference_info[i].fk_column == fk_column &&
            this->header->reference_info[i].fk_table == fk_table) {
            for (unsigned j = i; j < this->header->references - 1; ++j) {
                this->header->reference_info[j] = this->header->reference_info[j + 1];
            }
            --this->header->references;
            break;
        }
    }
}