// buffer_manager.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "buffer_manager.h"
//#include <iostream>

BufType BufferManager::_allocMem() {
    return new unsigned char[PAGE_SIZE];
}

int BufferManager::_fetchPage(const std::string &filename, unsigned pageID) {
    BufType buf;
    int index = this->lru->getNextReplace();
    this->lru->access(index);
    buf = this->addr[index];
    if (buf == nullptr) {
        buf = BufferManager::_allocMem();
        this->addr[index] = buf;
        this->hash->push(Page{.filename=filename, .pageID=pageID}, index);
//        std::cout << "alloc index " << index << " for " << filename << " " << pageID << std::endl;
    } else {
        if (this->dirty[index >> 5] & (1 << (index & 31))) {
            Page oldPage;
            this->hash->getKey(index, oldPage);
            FileManager::fm().writePage(oldPage.filename, oldPage.pageID, buf);
            this->dirty[index >> 5] &= (~(1 << (index & 31)));
        }
        this->hash->replace(index, Page{.filename=filename, .pageID=pageID});
//        std::cout << "replace index " << index << " for " << filename << " " << pageID << std::endl;
    }
    FileManager::fm().readPage(filename, pageID, buf);
    return index;
}

int BufferManager::allocPage(const std::string &filename, unsigned pageID) {
    int index = this->_fetchPage(filename, pageID);
    return index;
}

int BufferManager::getPage(const std::string &filename, unsigned pageID) {
    int index = this->hash->get(Page{.filename=filename, .pageID=pageID});
    if (index == -1) {
        index = this->_fetchPage(filename, pageID);
    }
    return index;
}

BufType BufferManager::readBuffer(int index) {
    this->lru->access(index);
    return this->addr[index];
}

void BufferManager::markDirty(int index) {
    this->dirty[index >> 5] |= (1 << (index & 31));
//    readBuffer(index);
}

void BufferManager::release(int index) {
    this->dirty[index >> 5] &= (~(1 << (index & 31)));
    this->lru->release(index);
    this->hash->remove(index);
    delete[] this->addr[index];
    this->addr[index] = nullptr;
}

void BufferManager::writeBack(int index) {
    if (this->dirty[index >> 5] & (1 << (index & 31))) {
        Page p;
        this->hash->getKey(index, p);
        FileManager::fm().writePage(p.filename, p.pageID, addr[index]);
        dirty[index >> 5] &= (~(1 << (index & 31)));
    }
    this->release(index);
}

void BufferManager::writeBackAll() {
    for (int i = MAX_BUF_PAGE - 1; i >= 0; --i) {
        this->writeBack(i);
    }
//     std::cout << "release all" << std::endl;
}