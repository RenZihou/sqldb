// hashmap.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <string>

#include "../util/constants.h"

template<typename KT>
class HashMap {
private:
    const int capacity;

    virtual unsigned _hash(const KT &key) = 0;

protected:
    KT *keys;
//    unsigned *hkeys;
    int *values;
    unsigned *removed;
    int hot = 0;  // last used index

public:
    /**
     * @param key key
     * @return value if key exists, -1 otherwise
     */
    virtual int get(const KT &key) {
        int counter = 0;
        unsigned h = this->_hash(key);
        this->hot = h % this->capacity;
        while ((this->values[this->hot] != -1 && this->keys[this->hot] != key)
               || (this->values[this->hot] == -1 &&
                   (this->removed[this->hot >> 5] & (1 << (this->hot & 31))))) {  // linear probe
            ++this->hot;
            ++counter;
            this->hot %= this->capacity;
            if (counter == this->capacity) {  // full
                return -1;
            }
        }
        return this->values[this->hot];  // -1 when not hit
//        for (hot = h % (2 * this->capacity); hot < this->capacity; ++hot) {
//            if (this->hkeys[hot] == h && this->keys[hot] == key) return this->values[hot];
//        }
//        return -1;
    }

    /**
     * @brief remove by key
     * @param key key
     * @return value of the key
     */
    virtual int remove(const KT &key) {
        this->get(key);
        if (this->values[this->hot] == -1) return -1;
        this->removed[this->hot >> 5] |= 1 << (this->hot & 31);
        return this->values[this->hot];
//        unsigned h = this->_hash(key);
//        int value;
//        for (hot = 0; hot < this->capacity; ++hot) {
//            if (this->hkeys[hot] == h && this->keys[hot] == key) {
//                this->hkeys[hot] = 0;
//                value = this->values[hot];
//                this->values[hot] = -1;
//                return value;
//            }
//        }
//        return -1;
    }

    /**
     * @param key key
     * @param value value
     * @return 0 for success, -1 for error
     */
    virtual int push(const KT &key, int value) {
        // TODO error when reach half capacity
        unsigned h = this->_hash(key);
        this->hot = h % this->capacity;
        while (this->values[this->hot] != -1 && this->keys[this->hot] != key) {
            ++this->hot;
            this->hot %= this->capacity;
        }
        if (this->values[this->hot] != -1) return -1;
        this->keys[this->hot] = key;
//        this->hkeys[this->hot] = h;
        this->values[this->hot] = value;
        return 0;
//        for (hot = 0; hot < this->capacity; ++hot) {
//            if (this->values[hot] == -1) {
//                this->hkeys[hot] = h;
//                this->keys[hot] = key;
//                this->values[hot] = value;
//                return 0;
//            }
//        }
//        return -1;
    }

    explicit HashMap(int c) : capacity(2 * c), keys(new KT[2 * c]), // hkeys(new unsigned[c]),
                              values(new int[2 * c]), removed(new unsigned[(c >> 4) + 1]) {
        for (int i = 0; i < 2 * c; ++i) {
            this->values[i] = -1;
        }
        for (int i = 0; i < (c >> 4) + 1; ++i) {
            this->removed[i] = 0;
        }
    }

    virtual ~HashMap() {
        delete[] keys;
//        delete[] hkeys;
        delete[] values;
        delete[] removed;
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
    explicit StringHashMap(int c) : HashMap(c) {}
//    using HashMap::HashMap;
};

struct Page {
    std::string filename;
    unsigned pageID = -1;

    inline bool operator==(const Page &rhs) const {
        return filename == rhs.filename && pageID == rhs.pageID;
    }

    inline bool operator!=(const Page &rhs) const {
        return !(rhs == *this);
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

    ~PageHashMap() override {  // this will call HashMap::~HashMap() automatically
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
     * @brief remove by key
     * @param key key
     * @return value of the key
     */
    int remove(const Page &key) override {
        int value = HashMap::remove(key);
        if (value != -1) {
            this->reverse[value] = -1;
        }
        return value;
    }

    /**
     * @brief remove by value, slightly faster than remove by key
     * @param value value
     */
    void remove(int value) {
        int index = this->reverse[value];
        if (index != -1) {
//            this->hkeys[index] = 0;
            this->values[index] = -1;
            this->reverse[value] = -1;
            this->removed[index >> 5] |= 1 << (index & 31);
        }
    }

    /**
     * @brief update key by value
     * @param value value
     * @param key new key
     */
    void replace(int value, const Page &key) {
        this->remove(value);
        this->push(key, value);
//        int index = this->reverse[value];
//        if (index != -1) {
//            this->hkeys[index] = this->_hash(key);
//            this->keys[index] = key;
//        }
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
