// buffer_manager.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef BUFFER_MANAGER_H_
#define BUFFER_MANAGER_H_

#include <string>
#include "./file_manager.h"
#include "../util/hashmap.h"
#include "../util/find_replace.h"
#include "../util/linklist.h"


class BufferManager {
private:
    int last;
    PageHashMap *hash;
    FindReplace *replace;
    //MyLinkList* bpl;
    unsigned *dirty;
    BufType *addr;  // page buffer address

    static BufType _allocMem() {
        return new unsigned int[(PAGE_SIZE >> 2)];
    }

    int fetchPage(const std::string &filename, int pageID) {
        BufType buf;
        int index = replace->find();
        buf = addr[index];
        if (buf == nullptr) {
            buf = BufferManager::_allocMem();
            addr[index] = buf;
        } else {
            if (dirty[index >> 5] & (1 << (index & 31))) {
                Page oldPage;
                hash->getKey(index, oldPage);
                FileManager::fm().writePage(oldPage.filename, oldPage.pageID, buf);
                dirty[index >> 5] &= (~(1 << (index & 31)));
            }
        }
        this->hash->replace(index, Page{.filename=filename, .pageID=pageID});
        return index;
    }

public:
    /**
     * @param filename filename relative to working directory
     * @return 0 for success, -1 for error
     * @description create file
     */
    static int createFile(const std::string &filename) {
        return FileManager::fm().createFile(filename);
    }

    /*
     * @函数名allocPage
     * @参数fileID:文件id，数据库程序在运行时，用文件id来区分正在打开的不同的文件
     * @参数pageID:文件页号，表示在fileID指定的文件中，第几个文件页
     * @参数index:函数返回时，用来记录缓存页面数组中的下标
     * @参数ifRead:是否要将文件页中的内容读到缓存中
     * 返回:缓存页面的首地址
     * 功能:为文件中的某一个页面获取一个缓存中的页面
     *           缓存中的页面在缓存页面数组中的下标记录在index中
     *           并根据ifRead是否为true决定是否将文件中的内容写到获取的缓存页面中
     * 注意:在调用函数allocPage之前，调用者必须确信(fileID,pageID)指定的文件页面不存在缓存中
     *           如果确信指定的文件页面不在缓存中，那么就不用在hash表中进行查找，直接调用替换算法，节省时间
     */
    int allocPage(const std::string &filename, int pageID, bool ifRead = false) {
        int index = fetchPage(filename, pageID);
        if (ifRead) {
            FileManager::fm().readPage(filename, pageID, this->addr[index]);
        }
        return index;
    }

    /**
     * @param filename filename
     * @param pageID page id
     * @return index of page buffer
     * @description get buffer index of specified page, fetch the page if not in buffer
     */
    int getPage(const std::string &filename, int pageID) {
        int index = this->hash->get(Page{.filename=filename, .pageID=pageID});
        if (index == -1) index = this->fetchPage(filename, pageID);
        return index;
    }

    /**
     * @param index index of page buffer
     * @return buffer address
     * @description access buffer through index, update LRU
     */
    BufType readBuffer(int index) {
        if (index != this->last) {  // TODO last?
            this->replace->access(index);
            this->last = index;
        }
        return this->addr[index];
    }

    /**
     * @param index index of page buffer
     * @description mark buffer as dirty
     */
    void markDirty(int index) {
        this->dirty[index >> 5] |= (1 << (index & 31));
        readBuffer(index);
    }

    /**
     * @param index index of buffer
     * @description release buffer without writing back
     */
    void release(int index) {
        this->dirty[index >> 5] &= (~(1 << (index & 31)));
        this->replace->free(index);
        this->hash->remove(index);
    }

    /**
     * @param index index of buffer
     * @description release buffer and write back if dirty
     */
    void writeBack(int index) {
        if (this->dirty[index >> 5] & (1 << (index & 31))) {
            Page p;
            this->hash->getKey(index, p);
            FileManager::fm().writePage(p.filename, p.pageID, addr[index]);
        }
        this->release(index);
    }

    /*
     * @函数名close
     * 功能:将所有缓存页面归还给缓存管理器，归还前需要根据脏页标记决定是否写到对应的文件页面中
     */
    [[deprecated]] void closeFile(std::string filename) {
        for (int i = 0; i < CAP; ++i) {
            writeBack(i);
        }
    }

    BufferManager() : hash(new PageHashMap(CAP)), dirty(new unsigned [(CAP >> 5) + 1] {}), addr(new BufType[CAP] {}) {
        last = -1;
        replace = new FindReplace(CAP);
    }
};

#endif  // BUFFER_MANAGER_H_
