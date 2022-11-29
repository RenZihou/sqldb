// hashmap.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <string>
#include "./constants.h"

template<typename KT>
class HashMap {
private:
    const int capacity;

    virtual unsigned _hash(const KT &key) = 0;

protected:
    KT *keys;
    unsigned *hkeys;
    int *values;
    int hot = 0;  // last used index

public:
    /**
     * @param key key
     * @return value if key exists, -1 otherwise
     */
    int get(const KT &key) {
        unsigned h = this->_hash(key);
        for (hot = 0; hot < this->capacity; ++hot) {
            if (this->hkeys[hot] == h && this->keys[hot] == key) return this->values[hot];
        }
        return -1;
    }

    /**
     * @param key key
     * @return value of the key
     * @description remove by key
     */
    virtual int remove(const KT &key) {
        unsigned h = this->_hash(key);
        int value;
        for (hot = 0; hot < this->capacity; ++hot) {
            if (this->hkeys[hot] == h && this->keys[hot] == key) {
                this->hkeys[hot] = 0;
                value = this->values[hot];
                this->values[hot] = -1;
                return value;
            }
        }
        return -1;
    }

    /**
     * @param key key
     * @param value value
     * @return 0 for success, -1 for error
     */
    virtual int push(const KT &key, int value) {
        unsigned h = this->_hash(key);
        for (hot = 0; hot < this->capacity; ++hot) {
            if (this->values[hot] == -1) {
                this->hkeys[hot] = h;
                this->keys[hot] = key;
                this->values[hot] = value;
                return 0;
            }
        }
        return -1;
    }

    explicit HashMap(int c) : capacity(c), keys(new KT[c]), hkeys(new unsigned[c]),
                              values(new int[c]) {
        for (int i = 0; i < c; ++i) {
            this->values[i] = -1;
        }
    }

    ~HashMap() {
        delete[] keys;
        delete[] hkeys;
        delete[] values;
    }
};

class StringHashMap : public HashMap<std::string> {
private:
    unsigned _hash(const std::string &key) override {  // FNV-1a hash
        unsigned hash = HASH_OFFSET;
        for (auto c: key) {
            hash = hash ^ c;
            hash = hash * HASH_PRIME;
        }
        return hash;
    }

public:
    using HashMap::HashMap;
};

struct Page {
    std::string filename;
    unsigned pageID = -1;

    inline bool operator==(const Page &rhs) const {
        return filename == rhs.filename && pageID == rhs.pageID;
    }
};

class PageHashMap : public HashMap<Page> {
private:
    int *reverse;  // map value to index
    unsigned _hash(const Page &key) override {  // FNV-1a hash
        unsigned hash = HASH_OFFSET;
        for (auto c: key.filename) {
            hash = hash ^ c;
            hash = hash * HASH_PRIME;
        }
        hash = hash ^ key.pageID;
//        hash = hash * HASH_PRIME;
        return hash;
    }

public:
    explicit PageHashMap(int c) : HashMap(c), reverse(new int[c]) {}

    ~PageHashMap() {  // this will call HashMap::~HashMap() automatically
        delete[] reverse;
    }

    /**
     * @param key key
     * @param value value
     * @return 0 for success, -1 for error
     */
    int push(const Page &key, int value) override {
        int code = HashMap::push(key, value);
        if (code == 0) {
            this->reverse[value] = this->hot;
        }
        return code;
    }

    /**
     * @param key key
     * @return value of the key
     * @description remove by key
     */
    int remove(const Page &key) override {
        int value = HashMap::remove(key);
        if (value != -1) {
            this->reverse[value] = -1;
        }
        return value;
    }

    /**
     * @param value value
     * @description remove by value, faster than remove by key
     */
    void remove(int value) {
        int index = this->reverse[value];
        if (index != -1) {
            this->hkeys[index] = 0;
            this->values[index] = -1;
            this->reverse[value] = -1;
        }
    }

    /**
     * @param value value
     * @param key new key
     * @description update key by value
     */
    void replace(int value, const Page &key) {
        int index = this->reverse[value];
        if (index != -1) {
            this->hkeys[index] = this->_hash(key);
            this->keys[index] = key;
        }
    }

    /**
     * @param value value
     * @param key where to store the key
     * @return 0 for success (hit), -1 for error (not hit)
     * @description get key by value (first hit)
     */
    int getKey(int value, Page &key) {
        int index = this->reverse[value];
        if (index == -1) return -1;
        key = this->keys[index];
        return 0;
    }
};

#endif  // HASHMAP_H_
