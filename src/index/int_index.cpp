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
    return new IntIndexNode;
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
    node = this->_readNode(this->header->root);
    unsigned offset = this->header->root;
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
    if (size < BTREE_ORDER) {  // can insert directly
        unsigned i = 0;
        while (key > node->keys[i] && i < size) ++i;
        for (unsigned j = size; j > i; j--) {
            node->keys[j] = node->keys[j - 1];
            node->children[j + 1] = node->children[j];
        }
        node->keys[i] = key;
        node->children[i] = record_offset;
        ++node->size;
        this->_writeNode(node, offset);
        delete node;
    } else {  // split needed
        unsigned new_leaf_offset;
        auto new_leaf = this->_newNode(new_leaf_offset);
        int virtual_node_keys[BTREE_ORDER + 1];  // force insert into a larger virtual node
        unsigned virtual_node_children[BTREE_ORDER + 2];
        unsigned i = 0, j;
        while (key > node->keys[i] && i < BTREE_ORDER) {
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
/*
    if (this->header->root == 0) return;  // although this should not happen
    this->_search_path.clear();
    unsigned offset = this->header->root;
    auto node = this->_readNode(offset);
    unsigned left_sibling, right_sibling;
    while (!(node->size & (1 << 31))) {  // find leaf node to remove
        this->_search_path.push_back(offset);
        bool found = false;
        for (int i = 0; i < node->size; i++) {
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
    int pos;
    for (pos = 0; pos < (node->size & ~(1 << 31)); pos++) {
        if (node->keys[pos] == key) {
            found = true;
            break;
        }
    }
    if (!found) return;
    // search for the certain record offset

    for (int i = pos; i < cursor->size; i++) {
        cursor->key[i] = cursor->key[i + 1];
    }
    cursor->size--;
    if (cursor == root) {
        for (auto &i: cursor->ptr) {
            i = nullptr;
        }
        if (cursor->size == 0) {
            std::cout << "Tree died\n";
            delete[] cursor->key;
            delete[] cursor->ptr;
            delete cursor;
            root = nullptr;
        }
        return;
    }
    cursor->ptr[cursor->size] = cursor->ptr[cursor->size + 1];
    cursor->ptr[cursor->size + 1] = nullptr;
    if (cursor->size >= (BTREE_ORDER + 1) / 2) {
        return;
    }
    if (leftSibling >= 0) {
        Node *leftNode = parent->ptr[leftSibling];
        if (leftNode->size >= (BTREE_ORDER + 1) / 2 + 1) {
            for (int i = cursor->size; i > 0; i--) {
                cursor->key[i] = cursor->key[i - 1];
            }
            cursor->size++;
            cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
            cursor->ptr[cursor->size - 1] = nullptr;
            cursor->key[0] = leftNode->key[leftNode->size - 1];
            leftNode->size--;
            leftNode->ptr[leftNode->size] = cursor;
            leftNode->ptr[leftNode->size + 1] = nullptr;
            parent->key[leftSibling] = cursor->key[0];
            return;
        }
    }
    if (rightSibling <= parent->size) {
        Node *rightNode = parent->ptr[rightSibling];
        if (rightNode->size >= (BTREE_ORDER + 1) / 2 + 1) {
            cursor->size++;
            cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
            cursor->ptr[cursor->size - 1] = nullptr;
            cursor->key[cursor->size - 1] = rightNode->key[0];
            rightNode->size--;
            rightNode->ptr[rightNode->size] = rightNode->ptr[rightNode->size + 1];
            rightNode->ptr[rightNode->size + 1] = nullptr;
            for (int i = 0; i < rightNode->size; i++) {
                rightNode->key[i] = rightNode->key[i + 1];
            }
            parent->key[rightSibling - 1] = rightNode->key[0];
            return;
        }
    }
    if (leftSibling >= 0) {
        Node *leftNode = parent->ptr[leftSibling];
        for (int i = leftNode->size, j = 0; j < cursor->size; i++, j++) {
            leftNode->key[i] = cursor->key[j];
        }
        leftNode->ptr[leftNode->size] = nullptr;
        leftNode->size += cursor->size;
        leftNode->ptr[leftNode->size] = cursor->ptr[cursor->size];
        removeInternal(parent->key[leftSibling], parent, cursor);
        delete[] cursor->key;
        delete[] cursor->ptr;
        delete cursor;
    } else if (rightSibling <= parent->size) {
        Node *rightNode = parent->ptr[rightSibling];
        for (int i = cursor->size, j = 0; j < rightNode->size; i++, j++) {
            cursor->key[i] = rightNode->key[j];
        }
        cursor->ptr[cursor->size] = nullptr;
        cursor->size += rightNode->size;
        cursor->ptr[cursor->size] = rightNode->ptr[rightNode->size];
        std::cout << "Merging two leaf nodes\n";
        removeInternal(parent->key[rightSibling - 1], parent, rightNode);
        delete[] rightNode->key;
        delete[] rightNode->ptr;
        delete rightNode;
    }
*/
}

unsigned IntIndex::search(int key) {
    unsigned pos, offset;
    return this->search(key, pos, offset);
}

unsigned IntIndex::search(int key, unsigned &pos, unsigned &offset) {
    if (this->header->root == 0) {
        return 0;
    }
    auto node = this->_readNode(this->header->root);
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
    for (pos = 0; pos < (node->size & ~(1 << 31)); pos++) {
        if (node->keys[pos] == key) {
            return node->children[pos];
        }
    }
    return 0;
}