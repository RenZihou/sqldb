// cursor.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef INDEX_CURSOR_H_
#define INDEX_CURSOR_H_

#include <cstring>

#include "int_index.h"
#include "../pagefile/buffer_manager.h"

class IntIndexCursor {
private:
    IntIndex *index;
    unsigned pos;  // next position to read in node.children
    unsigned overflow_pos;
    IntIndexNode cached_index{};
    IntIndexOverflowNode cached_overflow{};

public:
    explicit IntIndexCursor(IntIndex *index) : index(index), pos(0), overflow_pos(0) {};

    bool find(int key) {
        unsigned offset;
        unsigned record_offset = this->index->search(key, this->pos, offset);
        int i = BufferManager::bm().getPage(this->index->table + "." + this->index->column,
                                            offset >> PAGE_SIZE_IDX);
        memcpy(&this->cached_index,
               BufferManager::bm().readBuffer(i) + (offset & PAGE_SIZE_MASK),
               sizeof(IntIndexNode));
        return record_offset != 0;
    }

    bool next(int &key, unsigned &record_offset) {
        if (this->pos == (this->cached_index.size & ~(1 << 31))) {  // cache new index node
            if (this->cached_index.children[this->cached_index.size & ~(1 << 31)] == 0) return false;
            int i = BufferManager::bm().getPage(this->index->table + "." + this->index->column,
                                                this->cached_index.children[this->cached_index.size & ~(1 << 31)] >> PAGE_SIZE_IDX);
            memcpy(&this->cached_index,
                   BufferManager::bm().readBuffer(i) +
                   (this->cached_index.children[this->cached_index.size & ~(1 << 31)] & PAGE_SIZE_MASK),
                   sizeof(IntIndexNode));
            this->pos = 0;
        }
        if (this->cached_index.children[this->pos] & 1) {  // links to overflow node
            if (this->overflow_pos == 0) {  // cache new overflow node
                int i = BufferManager::bm().getPage(this->index->table + "." + this->index->column,
                                                    this->cached_index.children[this->pos] >> PAGE_SIZE_IDX);
                memcpy(&this->cached_overflow,
                       BufferManager::bm().readBuffer(i) +
                       (this->cached_index.children[this->pos] & PAGE_SIZE_MASK & ~1),
                       sizeof(IntIndexOverflowNode));
                record_offset = this->cached_overflow.children[0];
                this->overflow_pos = 1;
            } else if (overflow_pos == this->cached_overflow.size) {
                if (this->cached_overflow.children[this->cached_overflow.size] == 0) {  // overflow end
                    ++this->pos;
                    this->overflow_pos = 0;
                    return this->next(key, record_offset);
                } else {  // succeeding overflow node
                    int i = BufferManager::bm().getPage(this->index->table + "." + this->index->column,
                                                        this->cached_overflow.children[this->cached_overflow.size] >> PAGE_SIZE_IDX);
                    memcpy(&this->cached_overflow,
                            BufferManager::bm().readBuffer(i) +
                            (this->cached_overflow.children[this->cached_overflow.size] & PAGE_SIZE_MASK),
                            sizeof(IntIndexOverflowNode));
                    record_offset = this->cached_overflow.children[0];
                    this->overflow_pos = 1;
                }
            } else {  // overflow not end
                record_offset = this->cached_overflow.children[this->overflow_pos];
                ++this->overflow_pos;
            }
            key = this->cached_index.keys[this->pos];
        } else {  // normal node
            record_offset = this->cached_index.children[this->pos];
            key = this->cached_index.keys[this->pos];
            ++this->pos;
        }
        return true;
    }
};

#endif  // INDEX_CURSOR_H_
