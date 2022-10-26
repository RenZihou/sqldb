// buffer_manager.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef BUFFER_MANAGER_H_
#define BUFFER_MANAGER_H_

#include <string>
#include "../util/hashmap.h"
#include "../../filesystem/bufmanager/FindReplace.h"
#include "./file_manager.h"
#include "../util/linklist.h"


class BufferManager {
private:
    int last;
    PageHashMap *hash;
    FindReplace *replace;
    //MyLinkList* bpl;
//	bool* dirty;
    uint *dirty;
    /*
     * 缓存页面数组
     */
    BufType *addr;

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
            if (dirty[index]) {
                std::string oldFilename;
                int oldPageID;
                hash->getKeys(index, oldFilename, oldPageID);
                FileManager::fm().writePage(oldFilename, oldPageID, buf);
                dirty[index] = false;
            }
        }
        hash->replace(index, typeID, pageID);
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
    BufType allocPage(const std::string &filename, int pageID, int &index, bool ifRead = false) {
        BufType b = fetchPage(fileID, pageID, index);
        if (ifRead) {
            fileManager->readPage(fileID, pageID, b);
        }
        return b;
    }

    /**
     * @param filename filename
     * @param pageID page id
     * @return index of page buffer
     * @description get buffer index of specified page, fetch the page if not in buffer
     */
    int getPage(const std::string &filename, int pageID) {
        int index = this->hash->findIndex(filename, pageID);
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
        dirty[index >> 5] |= (1 << (index & 31));
        readBuffer(index);
    }

    /*
     * @函数名release
     * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
     * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据不标记写回
     */
    void release(int index) {
        dirty[index] = false;
        replace->free(index);
        hash->remove(index);
    }

    /*
     * @函数名writeBack
     * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
     * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据需要根据脏页标记决定是否写到对应的文件页面中
     */
    void writeBack(int index) {
        if (dirty[index]) {
            int f, p;
            hash->getKeys(index, f, p);
            fileManager->writePage(f, p, addr[index]);
            dirty[index] = false;
        }
        replace->free(index);
        hash->remove(index);
    }

    /*
     * @函数名close
     * 功能:将所有缓存页面归还给缓存管理器，归还前需要根据脏页标记决定是否写到对应的文件页面中
     */
    void close() {
        for (int i = 0; i < CAP; ++i) {
            writeBack(i);
        }
    }

    /*
     * @函数名getKey
     * @参数index:缓存页面数组中的下标，用来指定一个缓存页面
     * @参数fileID:函数返回时，用于存储指定缓存页面所属的文件号
     * @参数pageID:函数返回时，用于存储指定缓存页面对应的文件页号
     */
    void getKey(int index, int &fileID, int &pageID) {
        hash->getKeys(index, fileID, pageID);
    }

    /*
     * 构造函数
     * @参数fm:文件管理器，缓存管理器需要利用文件管理器与磁盘进行交互
     */
    BufferManager() {
        int c = CAP;
        int m = MOD;
        last = -1;
        //bpl = new MyLinkList(CAP, MAX_FILE_NUM);
        dirty = new bool[CAP];
        addr = new BufType[CAP];
        hash = new MyHashMap(c, m);
        replace = new FindReplace(c);
        for (int i = 0; i < CAP; ++i) {
            dirty[i] = false;
            addr[i] = nullptr;
        }
    }
};

#endif  // BUFFER_MANAGER_H_
