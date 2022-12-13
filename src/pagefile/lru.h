// lru.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef LRU_H_
#define LRU_H_

class LRU {
private:
    int *prev;
    int *next;
    int top;  // most recently used index, next[top] is the last-recently-used index

    void _remove(int index) {
        this->prev[this->next[index]] = this->prev[index];
        this->next[this->prev[index]] = this->next[index];
    }

    void _insertBetween(int index, int prev_index, int next_index) {
        this->prev[index] = prev_index;
        this->next[prev_index] = index;
        this->next[index] = next_index;
        this->prev[next_index] = index;
    }

public:
    explicit LRU(int capacity) : prev(new int[capacity]), next(new int[capacity]),
                                 top(capacity - 1) {
        for (int i = 0; i < capacity; ++i) {
            this->next[i] = i + 1;
            this->prev[i] = i - 1;
        }
        this->prev[0] = capacity - 1;
        this->next[capacity - 1] = 0;
    }

    ~LRU() {
        delete[] this->prev;
        delete[] this->next;
    }

    /**
     * @return index of the least recently used page (to be replaced)
     */
    int getNextReplace() {
        return this->next[this->top];
    }

    /**
     * @param index index
     * @description move index to the top (just used)
     */
    void access(int index) {
        if (index == this->top) return;
        this->_remove(index);
        this->_insertBetween(index, this->top, this->next[this->top]);
        this->top = index;
    }

    /**
     * @param index index
     * @description set index as the next to be replaced
     */
    void release(int index) {
        this->_remove(index);
        this->_insertBetween(index, this->top, this->next[this->top]);
    }
};

#endif  // LRU_H_
