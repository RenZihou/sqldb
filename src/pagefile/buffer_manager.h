// buffer_manager.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef BUFFER_MANAGER_H_
#define BUFFER_MANAGER_H_

#include <string>

#include "file_manager.h"
#include "hashmap.h"
#include "lru.h"

class BufferManager {
private:
    PageHashMap *hash;
    LRU *lru;
    unsigned *dirty;
    BufType *addr;  // page buffer address

    /**
     * @return buffer address
     * @description alloc memory for new buffer
     */
    static BufType _allocMem();

    /**
     * @param filename filename relative to working directory
     * @param pageID page id
     * @return buffer index
     * @description read new page from disk to buffer
     */
    int _fetchPage(const std::string &filename, unsigned pageID);

    BufferManager() : hash(new PageHashMap(MAX_BUF_PAGE)),
                      lru(new LRU(MAX_BUF_PAGE)),
                      dirty(new unsigned[(MAX_BUF_PAGE >> 5) + 1]{}),
                      addr(new BufType[MAX_BUF_PAGE]{}) {}

    ~BufferManager() {
        delete hash;
        delete lru;
        delete[] dirty;
        for (int i = 0; i < MAX_BUF_PAGE; ++i) {
            delete[] addr[i];
        }
        delete[] addr;
    }

public:

    BufferManager(const BufferManager &) = delete;

    BufferManager(BufferManager &&) = delete;

    static BufferManager &bm() {
        static BufferManager bm;
        return bm;
    }

    /**
     * @param filename filename relative to working directory
     * @return 0 for success, -1 for error
     * @description create file
     */
    static int createFile(const std::string &filename);

    /**
     * @param filename filename relative to working directory
     * @param pageID page id
     * @return buffer index
     * @description allocate new page, use only when you are sure that the page is not in buffer
     */
    int allocPage(const std::string &filename, unsigned pageID);

    /**
     * @param filename filename relative to working directory
     * @param pageID page id
     * @return buffer index
     * @description get buffer index of specified page, will fetch the page if not in buffer
     */
    int getPage(const std::string &filename, unsigned pageID);

    /**
     * @param index buffer index
     * @return buffer address
     * @description access buffer through index, update LRU
     */
    BufType readBuffer(int index);

    /**
     * @param index buffer index
     * @description mark buffer as dirty
     */
    void markDirty(int index);

    /**
     * @param index buffer index
     * @description release buffer without writing back
     */
    void release(int index);

    /**
     * @param index buffer index
     * @description release buffer and write back if dirty
     */
    void writeBack(int index);

    /**
     * @description write back all dirty pages
     */
    void writeBackAll();
};

#endif  // BUFFER_MANAGER_H_
