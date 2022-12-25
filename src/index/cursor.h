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
    unsigned pos;
    IntIndexNode cached_index{};

public:
    explicit IntIndexCursor(IntIndex *index) : index(index), pos(0) {};

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
        if (pos == (cached_index.size & ~(1 << 31))) {  // cache new index node
            if (this->cached_index.children[cached_index.size & ~(1 << 31)] == 0) return false;
            int i = BufferManager::bm().getPage(this->index->table + "." + this->index->column,
                                                this->cached_index.children[cached_index.size]
                                                        >> PAGE_SIZE_IDX);
            memcpy(&this->cached_index,
                   BufferManager::bm().readBuffer(i) +
                   (this->cached_index.children[cached_index.size] & PAGE_SIZE_MASK),
                   sizeof(IntIndexNode));
            pos = 0;
        }
        key = cached_index.keys[pos];
        record_offset = cached_index.children[pos];
        ++pos;
        return true;
    }
};

#endif  // INDEX_CURSOR_H_
