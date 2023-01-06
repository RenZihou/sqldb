// int_index.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <cstring>
#include <utility>

#include "int_index.h"
#include "../pagefile/buffer_manager.h"

IntIndex::IntIndex(std::string table, std::string column)
        : Index(std::move(table), std::move(column)),
          header(new IntIndexHeader) {
//    Index::_readHeader((BufType) header, sizeof(IntIndexHeader));
    // index header is always one page
    int index = BufferManager::bm().getPage(this->table + "." + this->column, 0);
    memcpy(header, BufferManager::bm().readBuffer(index), sizeof(IntIndexHeader));
}

IntIndex::~IntIndex() {
//    Index::_writeHeader((BufType) header, sizeof(IntIndexHeader));
    // index header is always one page
    int index = BufferManager::bm().getPage(this->table + "." + this->column, 0);
    memcpy(BufferManager::bm().readBuffer(index), header, sizeof(IntIndexHeader));
    BufferManager::bm().markDirty(index);
    delete header;
}

void IntIndex::_offset_to_slot(unsigned int offset, unsigned int &page, unsigned int &slot) {
    page = offset >> PAGE_SIZE_IDX;
    slot = (offset & PAGE_SIZE_MASK) / sizeof(IntIndexNode);
}

IntIndexNode *IntIndex::_newNode(unsigned &offset) {
    int index;  // index of buffer to write
    BufType buf;  // buffer to write
    if (this->header->next_empty == 0) {  // all pages are occupied
        offset = (this->header->pages << PAGE_SIZE_IDX);
        index = BufferManager::bm().allocPage(this->table + "." + this->column,
                                              this->header->pages);
        buf = BufferManager::bm().readBuffer(index);
        // write next empty on this page
        unsigned next_empty_offset = sizeof(IntIndexNode);
        int available_in_page = PAGE_SIZE / sizeof(IntIndexNode) - 1;  // # of tot slots - 1
        if (available_in_page > 0) {
            this->header->next_empty = (this->header->pages << PAGE_SIZE_IDX) + next_empty_offset;
            while (--available_in_page) {
                *(unsigned *) (buf + next_empty_offset) =
                        (this->header->pages << PAGE_SIZE_IDX)
                        + (next_empty_offset + sizeof(IntIndexNode));
                next_empty_offset += sizeof(IntIndexNode);
            }
            *(unsigned *) (buf + next_empty_offset) = 0;
        } else {
            this->header->next_empty = 0;
        }
        ++this->header->pages;
    } else {
        offset = this->header->next_empty;
        unsigned page, slot;
        IntIndex::_offset_to_slot(this->header->next_empty, page, slot);
        index = BufferManager::bm().getPage(this->table + "." + this->column, page);
        buf = BufferManager::bm().readBuffer(index);
        buf += (this->header->next_empty & PAGE_SIZE_MASK);
        this->header->next_empty = ((unsigned *) buf)[0];
    }
    auto node = new IntIndexNode;
    memset(node, 0, sizeof(IntIndexNode));
    return node;
}

IntIndexNode *IntIndex::_readNode(unsigned offset) {
    unsigned page, slot;
    this->_offset_to_slot(offset, page, slot);
    int index = BufferManager::bm().getPage(this->table + "." + this->column, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK);
    auto *node = new IntIndexNode;
    memcpy(node, buf, sizeof(IntIndexNode));
    return node;
}

void IntIndex::_writeNode(IntIndexNode *node, unsigned offset) {
    unsigned page, slot;
    this->_offset_to_slot(offset, page, slot);
    int index = BufferManager::bm().getPage(this->table + "." + this->column, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK);
    memcpy(buf, node, sizeof(IntIndexNode));
    BufferManager::bm().markDirty(index);
}

void IntIndex::_deleteNode(unsigned int offset) {
    unsigned page, slot;
    this->_offset_to_slot(offset, page, slot);
    int index = BufferManager::bm().getPage(this->table + "." + this->column, page);
    BufType buf = BufferManager::bm().readBuffer(index) + (offset & PAGE_SIZE_MASK);
    ((unsigned *) buf)[0] = this->header->next_empty;
    this->header->next_empty = offset;
    BufferManager::bm().markDirty(index);
}

void IntIndex::_insertInternal(int key, unsigned int new_child_offset) {
    unsigned offset = this->_search_path.back();
    this->_search_path.pop_back();
    auto node = this->_readNode(offset);
    if (node->size < BTREE_ORDER) {  // can insert directly
        unsigned i = 0;
        while (key > node->keys[i] && i < node->size) ++i;  // internal node, no need to mask size
        for (unsigned j = node->size; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
            node->children[j + 1] = node->children[j];
        }
        node->keys[i] = key;
        node->children[i + 1] = new_child_offset;
        ++node->size;
        this->_writeNode(node, offset);
    } else {  // keep splitting
        unsigned new_internal_offset;
        auto new_internal = this->_newNode(new_internal_offset);
        int virtual_node_keys[BTREE_ORDER + 1];
        unsigned virtual_node_children[BTREE_ORDER + 2];
        unsigned i = 0, j;
        while (key > node->keys[i] && i < BTREE_ORDER) {
            virtual_node_keys[i] = node->keys[i];
            virtual_node_children[i] = node->children[i];
            ++i;
        }
        virtual_node_keys[i] = key;
        virtual_node_children[i] = node->children[i];
        virtual_node_children[i + 1] = new_child_offset;
        ++i;
        while (i < BTREE_ORDER + 1) {
            virtual_node_keys[i] = node->keys[i - 1];
            virtual_node_children[i + 1] = node->children[i];
            ++i;
        }
        node->size = (BTREE_ORDER + 1) / 2;
        new_internal->size = BTREE_ORDER - (BTREE_ORDER + 1) / 2;
        for (i = 0, j = node->size + 1; i < new_internal->size; ++i, ++j) {
            new_internal->keys[i] = virtual_node_keys[j];
            new_internal->children[i] = virtual_node_children[j];
        }
        new_internal->children[i] = virtual_node_children[j];
        if (offset == this->header->root) {
            auto new_root = this->_newNode(this->header->root);
            new_root->keys[0] = node->keys[node->size];
            new_root->children[0] = offset;
            new_root->children[1] = new_internal_offset;
            new_root->size = 1;  // flag = 0, is internal node
            this->_writeNode(new_root, this->header->root);
            delete new_root;
        } else {
            this->_insertInternal(node->keys[node->size], new_internal_offset);
        }
        this->_writeNode(node, offset);
        this->_writeNode(new_internal, new_internal_offset);
        delete new_internal;
    }
}

void IntIndex::_removeInternal(int key, unsigned removed_child_offset) {
    unsigned offset = this->_search_path.back();
    this->_search_path.pop_back();
    auto node = this->_readNode(offset);
    if (offset == this->header->root) {
        if (node->size == 1) {
            if (node->children[1] == removed_child_offset) {
                // only left child remains, then it becomes root
                this->header->root = node->children[0];
                this->_deleteNode(offset);
                return;
            } else /* if (node->children[0] == removed_child_offset) */ {
                // only right child remains, then it becomes root
                this->header->root = node->children[1];
                this->_deleteNode(offset);
                return;
            }
        }
    }
    int pos = 0;
    while (pos < static_cast<int>(node->size) && key > node->keys[pos]) ++pos;  // now we reach the key
    for (unsigned i = pos; i < node->size; ++i) {
        node->keys[i] = node->keys[i + 1];
    }
    pos = 0;
    while (pos < static_cast<int>(node->size) + 1 && removed_child_offset != node->children[pos]) ++pos;  // now we reach the child
    for (unsigned i = pos; i < node->size + 1; ++i) {
        node->children[i] = node->children[i + 1];
    }
    --node->size;
    if (node->size >= (BTREE_ORDER + 1) / 2 - 1) return;  // no need to merge
    if (offset == this->header->root) return;  // no need to merge
    auto parent = this->_readNode(this->_search_path.back());
    unsigned left_sibling, right_sibling;
    for (pos = 0; pos < static_cast<int>(parent->size) + 1; ++pos) {
        if (parent->children[pos] == offset) {
            left_sibling = pos - 1;
            right_sibling = pos + 1;
            break;
        }
    }
    if (left_sibling != -1u) {
        auto left_node = this->_readNode(parent->children[left_sibling]);
        if (left_node->size >= (BTREE_ORDER + 1) / 2) {  // rotate: borrow from left sibling
            for (unsigned i = node->size; i > 0; --i) {
                node->keys[i] = node->keys[i - 1];
            }
            node->keys[0] = parent->keys[left_sibling];
            parent->keys[left_sibling] = left_node->keys[left_node->size - 1];
            for (unsigned i = node->size + 1; i > 0; --i) {
                node->children[i] = node->children[i - 1];
            }
            node->children[0] = left_node->children[left_node->size];
            ++node->size;
            --left_node->size;
            this->_writeNode(node, offset);
            this->_writeNode(left_node, parent->children[left_sibling]);
            this->_writeNode(parent, this->_search_path.back());
            delete node;
            delete left_node;
            delete parent;
            return;
        }
    }
    if (right_sibling <= parent->size) {
        auto right_node = this->_readNode(parent->children[right_sibling]);
        if (right_node->size >= (BTREE_ORDER + 1) / 2) {  // rotate: borrow from right sibling
            node->keys[node->size] = parent->keys[pos];
            parent->keys[pos] = right_node->keys[0];
            for (unsigned i = 0; i < right_node->size - 1; i++) {
                right_node->keys[i] = right_node->keys[i + 1];
            }
            node->children[node->size + 1] = right_node->children[0];
            for (unsigned i = 0; i < right_node->size; ++i) {
                right_node->children[i] = right_node->children[i + 1];
            }
            ++node->size;
            --right_node->size;
            this->_writeNode(node, offset);
            this->_writeNode(right_node, parent->children[right_sibling]);
            this->_writeNode(parent, this->_search_path.back());
            delete node;
            delete right_node;
            delete parent;
            return;
        }
    }
    if (left_sibling != -1u) {
        auto left_node = this->_readNode(parent->children[left_sibling]);
        left_node->keys[left_node->size] = parent->keys[left_sibling];
        for (unsigned i = left_node->size + 1, j = 0; j < node->size; ++i, ++j) {
            left_node->keys[i] = node->keys[j];
            left_node->children[i] = node->children[j];
        }
        left_node->size += node->size + 1;
        left_node->children[left_node->size] = node->children[node->size];
        node->size = 0;
        this->_removeInternal(parent->keys[left_sibling], offset);
        this->_writeNode(left_node, parent->children[left_sibling]);
        this->_deleteNode(offset);
        delete left_node;
        delete node;
        delete parent;
    } else if (right_sibling <= parent->size) {
        auto right_node = this->_readNode(parent->children[right_sibling]);
        node->keys[node->size] = parent->keys[right_sibling - 1];
        for (unsigned i = node->size + 1, j = 0; j < right_node->size; ++i, ++j) {
            node->keys[i] = right_node->keys[j];
            node->children[i] = right_node->children[j];
        }
        node->size += right_node->size + 1;
        node->children[node->size] = right_node->children[right_node->size];
        right_node->size = 0;
        this->_removeInternal(parent->keys[right_sibling - 1], parent->children[right_sibling]);
        this->_writeNode(node, offset);
        this->_deleteNode(parent->children[right_sibling]);
        delete right_node;
        delete node;
        delete parent;
    }
}

IntIndex *IntIndex::createIndex(std::string table, std::string column) {
    auto *index = new IntIndex(std::move(table), std::move(column));
    index->header->root = 0;
    index->header->pages = 1;
    index->header->next_empty = 0;
    return index;
}

void IntIndex::insert(int key, unsigned record_offset) {
    this->_search_path.clear();
    IntIndexNode *node;
    if (this->header->root == 0) {
        node = this->_newNode(this->header->root);
        node->keys[0] = key;
        node->children[0] = record_offset;
        node->children[1] = 0;  // TODO record offset of next leaf node
        node->size = (1 << 31) + 1;
        this->_writeNode(node, this->header->root);
        delete node;
        return;
    }
    unsigned offset = this->header->root;
    node = this->_readNode(offset);
    while (!(node->size & (1 << 31))) {  // find leaf node to insert
        this->_search_path.push_back(offset);
        bool found = false;
        for (unsigned i = 0; i < node->size; i++) {  // flag = 0, no need to mask
            if (key < node->keys[i]) {
                offset = node->children[i];
                delete node;
                node = this->_readNode(offset);
                found = true;
                break;
            }
        }
        if (!found) {
            offset = node->children[node->size];
            delete node;
            node = this->_readNode(offset);
        }
    }
    unsigned size = node->size & ~(1 << 31);
    unsigned i = 0;
    while (i < size && key > node->keys[i]) ++i;
    if (i < size && key == node->keys[i]) {  // insert into overflow node
        if (node->children[i] & 1) {  // already have an overflow node
            unsigned new_overflow_offset = node->children[i] & ~1;
            auto overflow_node = (IntIndexOverflowNode *)this->_readNode(new_overflow_offset);
            while (overflow_node->size == 2 * BTREE_ORDER) {  // find the last one to insert
                new_overflow_offset = overflow_node->children[2 * BTREE_ORDER];
                if (new_overflow_offset == 0) {
                    delete overflow_node;
                    overflow_node = (IntIndexOverflowNode *)this->_newNode(new_overflow_offset);
                    break;
                } else {
                    delete overflow_node;
                    overflow_node = (IntIndexOverflowNode *)this->_readNode(new_overflow_offset);
                }
            }
            overflow_node->children[overflow_node->size] = record_offset;
            ++overflow_node->size;
            this->_writeNode((IntIndexNode *)overflow_node, new_overflow_offset);
            delete overflow_node;
            delete node;
        } else {  // create a new overflow node and insert two records
            unsigned old_record_offset = node->children[i];
            auto overflow_node = (IntIndexOverflowNode *)this->_newNode(old_record_offset);
            overflow_node->children[0] = old_record_offset;
            overflow_node->children[1] = record_offset;
            overflow_node->size = 2;
            this->_writeNode((IntIndexNode *)overflow_node, node->children[i]);
            node->children[i] |= 1;
            this->_writeNode(node, offset);
            delete overflow_node;
            delete node;
        }
    } else if (size < BTREE_ORDER) {  // can insert directly
        for (unsigned j = size; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
            node->children[j + 1] = node->children[j];
        }
        node->keys[i] = key;
        node->children[i + 1] = node->children[i];
        node->children[i] = record_offset;
        ++node->size;
        this->_writeNode(node, offset);
        delete node;
    } else {  // split needed
        unsigned new_leaf_offset;
        auto new_leaf = this->_newNode(new_leaf_offset);
        int virtual_node_keys[BTREE_ORDER + 1];  // force insert into a larger virtual node
        unsigned virtual_node_children[BTREE_ORDER + 2];
        unsigned j;
        i = 0;
        while (i < BTREE_ORDER && key > node->keys[i]) {
            virtual_node_keys[i] = node->keys[i];
            virtual_node_children[i] = node->children[i];
            ++i;
        }
        virtual_node_keys[i] = key;
        virtual_node_children[i] = record_offset;
        ++i;
        while (i < BTREE_ORDER + 1) {
            virtual_node_keys[i] = node->keys[i - 1];
            virtual_node_children[i] = node->children[i - 1];
            ++i;
        }
        virtual_node_children[i] = node->children[i - 1];
        // split node and copy data from virtual node
        unsigned node_size = (BTREE_ORDER + 1) / 2;
        unsigned new_leaf_size = BTREE_ORDER + 1 - node_size;
        node->size = (1 << 31) + node_size;
        new_leaf->size = (1 << 31) + new_leaf_size;
        node->children[node_size] = new_leaf_offset;
        new_leaf->children[new_leaf_size] = node->children[BTREE_ORDER];
        for (i = 0; i < node_size; ++i) {
            node->keys[i] = virtual_node_keys[i];
            node->children[i] = virtual_node_children[i];
        }
        for (i = 0, j = node_size; i < new_leaf_size; ++i, ++j) {
            new_leaf->keys[i] = virtual_node_keys[j];
            new_leaf->children[i] = virtual_node_children[j];
        }
        // modify parent node
        if (offset == this->header->root) {
            auto new_root = this->_newNode(this->header->root);
            new_root->keys[0] = new_leaf->keys[0];
            new_root->children[0] = offset;
            new_root->children[1] = new_leaf_offset;
            new_root->size = 1;  // flag = 0, is internal node
            this->_writeNode(new_root, this->header->root);
            delete new_root;
        } else {
            this->_insertInternal(new_leaf->keys[0], new_leaf_offset);
        }
        this->_writeNode(node, offset);
        this->_writeNode(new_leaf, new_leaf_offset);
        delete node;
        delete new_leaf;
    }
}

void IntIndex::remove(int key, unsigned int record_offset) {
    if (this->header->root == 0) return;  // although this should not happen
    this->_search_path.clear();
    unsigned offset = this->header->root;
    auto node = this->_readNode(offset);
    unsigned left_sibling, right_sibling;
    while (!(node->size & (1 << 31))) {  // find leaf node to remove
        this->_search_path.push_back(offset);
        bool found = false;
        for (unsigned i = 0; i < node->size; i++) {
            if (key < node->keys[i]) {
                left_sibling = i - 1;
                right_sibling = i + 1;
                offset = node->children[i];
                delete node;
                node = this->_readNode(offset);
                found = true;
                break;
            }
        }
        if (!found) {
            left_sibling = node->size - 1;
            right_sibling = node->size + 1;
            offset = node->children[node->size];
            delete node;
            node = this->_readNode(offset);
        }
    }
    bool found = false;
    unsigned pos;
    for (pos = 0; pos < (node->size & ~(1 << 31)); pos++) {
        if (node->keys[pos] == key) {
            found = true;
            break;
        }
    }
    if (!found) return;
    if (node->children[pos] & 1) {  // links to overflow node
        unsigned overflow_offset = node->children[pos] & ~1;
        auto overflow_node = (IntIndexOverflowNode *)this->_readNode(overflow_offset);
        unsigned hit_offset;
        unsigned hit_pos = 2 * BTREE_ORDER + 1;
        unsigned hit_replace;
        unsigned last_offset = 0;
        while (overflow_node->size == 2 * BTREE_ORDER && overflow_node->children[2 * BTREE_ORDER] != 0) {
            if (hit_pos != 2 * BTREE_ORDER + 1) continue;
            for (unsigned i = 0; i < overflow_node->size; ++i) {
                if (overflow_node->children[i] == record_offset) {
                    hit_offset = overflow_offset;
                    hit_pos = i;
                    break;
                }
            }
            last_offset = overflow_offset;
            overflow_offset = overflow_node->children[2 * BTREE_ORDER];
            delete overflow_node;
            overflow_node = (IntIndexOverflowNode *)this->_readNode(overflow_offset);
        }
        // now we are at the last overflow node
        if (hit_pos == 2 * BTREE_ORDER + 1) {
            for (unsigned i = 0; i < overflow_node->size; ++i) {
                if (overflow_node->children[i] == record_offset) {
                    hit_offset = overflow_offset;
                    hit_pos = i;
                    break;
                }
            }
        }
        hit_replace = overflow_node->children[overflow_node->size - 1];
        // remove last node
        if (overflow_node->size == 1) {
            if (last_offset == 0) {  // only one overflow node, need to delete real leaf node
                delete overflow_node;
                this->_deleteNode(overflow_offset);
                node->children[pos] = record_offset;
                this->_writeNode(node, offset);
                delete node;
                this->remove(key, record_offset);  // call again to delete real leaf node
                return;
            } else {  // delete last overflow node
                delete overflow_node;
                this->_deleteNode(overflow_offset);
                overflow_node = (IntIndexOverflowNode *)this->_readNode(last_offset);
                overflow_node->children[2 * BTREE_ORDER] = 0;
                this->_writeNode((IntIndexNode *)overflow_node, last_offset);
                delete overflow_node;
            }
        } else {
            --overflow_node->size;
            overflow_node->children[overflow_node->size] = 0;
            this->_writeNode((IntIndexNode *)overflow_node, overflow_offset);
            delete overflow_node;
        }
        // set hit pos to last record
        overflow_node = (IntIndexOverflowNode *)this->_readNode(hit_offset);
        if (hit_pos != overflow_node->size) {  // don't write if hit_pos is last record (already written 0)
            overflow_node->children[hit_pos] = hit_replace;
            this->_writeNode((IntIndexNode *)overflow_node, hit_offset);
        }
//        overflow_node->children[hit_pos] = hit_replace;
//        this->_writeNode((IntIndexNode *)overflow_node, hit_offset);
        delete overflow_node;
    } else {  // links to record
        if (node->children[pos] != record_offset) return;  // although this should not happen either
        for (unsigned i = pos; i < (node->size & ~(1 << 31)) - 1; i++) {  // remove from leaf
            node->keys[i] = node->keys[i + 1];
            node->children[i] = node->children[i + 1];
        }
        node->children[(node->size & ~(1 << 31)) - 1] = node->children[node->size & ~(1 << 31)];
        --node->size;
        if (offset == this->header->root) {
            if (node->size == 0) {
                this->_deleteNode(offset);
                this->header->root = 0;
            } else {
                this->_writeNode(node, offset);
            }
            delete node;
            return;
        }
        if (node->size >= (BTREE_ORDER + 1) / 2) return;  // no need to merge
        auto parent = this->_readNode(this->_search_path.back());
        if (left_sibling != -1u) {
            auto left_node = this->_readNode(parent->children[left_sibling]);
            if ((left_node->size & ~(1 << 31)) >= (BTREE_ORDER + 1) / 2 + 1) {  // rotate: borrow from left sibling
                node->children[(node->size & ~(1 << 31)) + 1] = node->children[node->size & ~(1 << 31)];
                for (unsigned i = (node->size & ~(1 << 31)); i > 0; --i) {
                    node->keys[i] = node->keys[i - 1];
                    node->children[i] = node->children[i - 1];
                }
                ++node->size;
                node->keys[0] = left_node->keys[(left_node->size & ~(1 << 31)) - 1];
                left_node->children[(left_node->size & ~(1 << 31)) - 1] = left_node->children[left_node->size & ~(1 << 31)];
                --left_node->size;
                parent->keys[left_sibling] = node->keys[0];
                this->_writeNode(node, offset);
                this->_writeNode(left_node, parent->children[left_sibling]);
                this->_writeNode(parent, this->_search_path.back());
                delete node;
                delete left_node;
                delete parent;
                return;
            }
        }
        if (right_sibling <= parent->size) {
            auto right_node = this->_readNode(parent->children[right_sibling]);
            if ((right_node->size & ~(1 << 31)) >= (BTREE_ORDER + 1) / 2 + 1) {  // rotate: borrow from right sibling
                node->children[(node->size & ~(1 << 31)) + 1] = node->children[node->size & ~(1 << 31)];
                node->children[node->size & ~(1 << 31)] = right_node->children[0];
                node->keys[(node->size & ~(1 << 31))] = right_node->keys[0];
                ++node->size;
                --right_node->size;
                for (unsigned i = 0; i < (right_node->size & ~(1 << 31)); i++) {
                    right_node->keys[i] = right_node->keys[i + 1];
                    right_node->children[i] = right_node->children[i + 1];
                }
                right_node->children[right_node->size & ~(1 << 31)] = right_node->children[(right_node->size & ~(1 << 31)) + 1];
                parent->keys[right_sibling - 1] = right_node->keys[0];
                this->_writeNode(node, offset);
                this->_writeNode(right_node, parent->children[right_sibling]);
                this->_writeNode(parent, this->_search_path.back());
                delete node;
                delete right_node;
                delete parent;
                return;
            }
        }
        if (left_sibling != -1u) {  // merge with left sibling
            auto left_node = this->_readNode(parent->children[left_sibling]);
            for (unsigned i = (left_node->size & ~(1 << 31)), j = 0; j < (node->size & ~(1 << 31)); ++i, ++j) {
                left_node->keys[i] = node->keys[j];
                left_node->children[i] = node->children[j];
            }
            left_node->size += (node->size & ~(1 << 31));
            left_node->children[left_node->size & ~(1 << 31)] = node->children[node->size & ~(1 << 31)];
            this->_removeInternal(parent->keys[left_sibling], offset);
            this->_writeNode(left_node, parent->children[left_sibling]);
            this->_deleteNode(offset);
            delete node;
            delete left_node;
            delete parent;
        } else /* if (right_sibling <= parent->size) */ {  // merge with right sibling
            auto right_node = this->_readNode(parent->children[right_sibling]);
            for (unsigned i = (node->size & ~(1 << 31)), j = 0; j < (right_node->size & ~(1 << 31)); ++i, ++j) {
                node->keys[i] = right_node->keys[j];
                node->children[i] = right_node->children[j];
            }
            node->size += (right_node->size & ~(1 << 31));
            node->children[node->size & ~(1 << 31)] = right_node->children[right_node->size & ~(1 << 31)];
            this->_removeInternal(parent->keys[right_sibling - 1], parent->children[right_sibling]);
            this->_writeNode(node, offset);
            this->_deleteNode(parent->children[right_sibling]);
            delete node;
            delete right_node;
            delete parent;
        }
    }
}

unsigned IntIndex::search(int key) {
    unsigned pos, offset;
    bool match;
    return this->search(key, pos, offset, match);
}

unsigned IntIndex::search(int key, unsigned &pos, unsigned &offset, bool &match) {
    if (this->header->root == 0) {
        return 0;
    }
    offset = this->header->root;
    auto node = this->_readNode(offset);
    while (!(node->size & (1 << 31))) {  // find leaf node to insert
        bool found = false;
        for (unsigned i = 0; i < node->size; i++) {  // flag = 0, no need to mask
            if (key < node->keys[i]) {
                offset = node->children[i];
                delete node;
                node = this->_readNode(offset);
                found = true;
                break;
            }
        }
        if (!found) {
            offset = node->children[node->size];
            delete node;
            node = this->_readNode(offset);
        }
    }
    unsigned ret = 0;
    for (pos = 0; pos < (node->size & ~(1 << 31)); pos++) {
        if (node->keys[pos] >= key) {
            ret = node->children[pos];
            match = node->keys[pos] == key;
            delete node;
            break;
        }
    }
    return ret;
}