// index.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef INDEX_H_
#define INDEX_H_

#include <string>
#include <utility>

#include "../util/constants.h"

template<typename KT>
class Index {
protected:
    std::string table;
    std::string column;

    /**
     * @brief read index header from buffer
     * @param buffer target buffer
     * @param length should <= PAGE_SIZE
     */
    void _readHeader(BufType buffer, unsigned length);

    void _writeHeader(BufType buffer, unsigned length);

public:
    Index(std::string table, std::string column) : table(std::move(table)),
                                                   column(std::move(column)) {};

    virtual ~Index() = default;

    virtual void insert(KT key, unsigned record_offset) = 0;

    virtual void remove(KT key, unsigned record_offset) = 0;

    virtual unsigned search(KT key) = 0;
};

#endif  // INDEX_H_
