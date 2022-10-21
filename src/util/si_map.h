// si_map.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

// a simple map from string to int
// used to map filename to file descriptor
// since the number of total open files should be quite small (< hundreds)
// we use linear search to do the mapping

#ifndef SI_MAP_H_
#define SI_MAP_H_

#include <string>
#include <cstring>

struct SIMapNode {
    std::string key;
    int value = -1;
    SIMapNode* next = nullptr;
};

class SIMap {
private:
    SIMapNode *head;
public:
    SIMap() {
        head = new SIMapNode;
    }
    ~SIMap() {
        SIMapNode *p = head;
        while (p != nullptr) {
            SIMapNode *q = p;
            p = p->next;
            delete q;
        }
    }
    /**
     * @param key key
     * @return value if key exists, -1 otherwise
     * @description get value by key
     */
    int get(const std::string& key) {
        SIMapNode *p = head->next;
        while (p != nullptr) {
            if (p->key == key) {
                return p->value;
            }
            p = p->next;
        }
        return -1;
    }
    /**
     * @param key key
     * @param value value
     * @description insert key-value pair
     */
    void push(const std::string& key, int value) {
        auto *q = new SIMapNode{.key = key, .value=value, .next=head->next};
        head->next = q;
    }
    /**
     * @param key key
     * @return value if key exists, -1 otherwise
     * @description delete by key and return value
     */
    int remove(const std::string &key) {
        SIMapNode *p = head;
        while (p->next != nullptr) {
            if (p->next->key == key) {
                SIMapNode *q = p->next;
                p->next = q->next;
                delete q;
                return 0;
            }
            p = p->next;
        }
        return -1;
    }
};

#endif  // SI_MAP_H_
