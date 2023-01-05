// int_index.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef INT_INDEX_H_
#define INT_INDEX_H_

#include <vector>

#include "index.h"
#include "../util/constants.h"

class IntIndexCursor;

struct IntIndexHeader {
    unsigned root;
    unsigned pages;
    unsigned next_empty;
};

struct IntIndexNode {
    unsigned size;  // [1 bit identifies whether leaf node | 31 bit for key size]
    int keys[BTREE_ORDER];
    unsigned children[BTREE_ORDER + 1];  // for leaf node, this records the offset of the record, the least significant bit indicates whether links to overflow node
};

struct IntIndexOverflowNode {  // used to store duplicated keys
    unsigned size;
    unsigned children[2 * BTREE_ORDER + 1];  // make sure it has the same size with IntIndexNode
};

/**
 * @brief index for int type implemented by B+ tree
 */
class IntIndex : public Index<int> {
    friend class IntIndexCursor;
private:
    IntIndexHeader *header;

    std::vector<unsigned> _search_path;

    static void _offset_to_slot(unsigned offset, unsigned &page, unsigned &slot);

    IntIndexNode *_newNode(unsigned &offset);

    IntIndexNode *_readNode(unsigned offset);

    void _deleteNode(unsigned offset);

    void _writeNode(IntIndexNode *node, unsigned offset);

    /**
     * @brief insert key and link new_leaf_offset into last (internal) node in _search_path
     * @param key
     * @param new_child_offset
     */
    void _insertInternal(int key, unsigned new_child_offset);

    /**
     * @brief remove
     * @param key
     * @param child_offset
     */
    void _removeInternal(int key, unsigned child_offset);

public:
    IntIndex(std::string table, std::string column);

    ~IntIndex() override;

    static IntIndex *createIndex(std::string table, std::string column);

    void insert(int key, unsigned record_offset) override;

    void remove(int key, unsigned record_offset) override;

    [[deprecated]] unsigned search(int key) override;

    /**
     * @brief search for key
     * @param key key to search
     * @param pos position of key in (leaf) node
     * @param offset offset of (leaf) node
     * @param match whether key is found exactly
     * @return offset of first record whose value is not less than key
     */
    unsigned search(int key, unsigned &pos, unsigned &offset, bool &match);
};

#endif  // INT_INDEX_H_
