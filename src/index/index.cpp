// index.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>

#include "index.h"
#include "../pagefile/buffer_manager.h"

template <typename KT>
void Index<KT>::_readHeader(BufType buffer, unsigned length) {
    // index header is always one page
    int index = BufferManager::bm().getPage(this->table + "." + this->column, 0);
    memcpy(buffer, BufferManager::bm().readBuffer(index), length);
}

template <typename KT>
void Index<KT>::_writeHeader(BufType buffer, unsigned int length) {
    // index header is always one page
    int index = BufferManager::bm().getPage(this->table + "." + this->column, 0);
    memcpy(BufferManager::bm().readBuffer(index), buffer, length);
    BufferManager::bm().markDirty(index);
}