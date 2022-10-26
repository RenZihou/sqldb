// hashmap.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef MY_HASH_MAP
#define MY_HASH_MAP

#include <string>
#include "./constants.h"

template<typename KT>
class HashMap {
private:
    const int capacity;
    KT *keys;
    unsigned *hkeys{};
    int *values{};

//    unsigned _hash(const KT &key) {  // FNV-1a hash
//        unsigned hash = HASH_OFFSET;
//        for (auto c: key) {
//            hash = hash ^ c;
//            hash = hash * HASH_PRIME;
//        }
//        return hash;
//    }
    virtual unsigned _hash(const KT &key) = 0;

public:
    /**
     * @param key key
     * @return value if key exists, -1 otherwise
     */
    int get(const KT &key) {
        unsigned h = this->_hash(key);
        for (int i = 0; i < this->capacity; ++i) {
            if (this->hkeys[i] == h && this->keys[i] == key) return this->values[i];
        }
        return -1;
    }

    /**
     * @param key key
     * @return value of the key
     */
    int remove(const KT &key) {
        unsigned h = this->_hash(key);
        int value;
        for (int i = 0; i < this->capacity; ++i) {
            if (this->hkeys[i] == h && this->keys[i] == key) {
                this->hkeys[i] = 0;
                value = this->values[i];
                this->values[i] = -1;
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
    int push(const KT &key, int value) {
        unsigned h = this->_hash(key);
        for (int i = 0; i < this->capacity; ++i) {
            if (this->values[i] == -1) {
                this->hkeys[i] = h;
                this->keys[i] = key;
                this->values[i] = value;
                return 0;
            }
        }
        return -1;
    }

    explicit HashMap(int c) : capacity(c) {
        this->keys = new KT[c];
        this->hkeys = new unsigned[c];
        this->values = new int[c];
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
    int pageID;
    inline bool operator==(const Page &rhs) const {
        return filename == rhs.filename && pageID == rhs.pageID;
    }
};

class PageHashMap : public HashMap<Page> {
private:
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
    using HashMap::HashMap;
};

//class _HashMap {
//private:
//    static const int A = 1;
//    static const int B = 1;
//    int CAP_, MOD_;
//
//public:
//    /*
//     * @函数名replace
//     * @参数index:指定的value
//     * @参数k1:指定的第一个key
//     * @参数k2:指定的第二个key
//     * 功能:在hash表中，将指定value对应的两个key设置为k1和k2
//     */
//    void replace(int index, int k1, int k2) {
//        int h = hash(k1, k2);
//        //cout << h << endl;
//        list->insertFirst(h, index);
//        a[index].key1 = k1;
//        a[index].key2 = k2;
//    }
//
//    /*
//     * @函数名getKeys
//     * @参数index:指定的value
//     * @参数k1:存储指定value对应的第一个key
//     * @参数k2:存储指定value对应的第二个key
//     */
//    void getKeys(int index, int &k1, int &k2) {
//        k1 = a[index].key1;
//        k2 = a[index].key2;
//    }
//};

#endif
