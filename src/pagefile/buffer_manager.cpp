// buffer_manager.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include "buffer_manager.h"

BufType BufferManager::_allocMem() {
    return new unsigned int[(PAGE_SIZE >> 2)];
}

int BufferManager::_fetchPage(const std::string &filename, unsigned pageID) {
    BufType buf;
    int index = this->lru->getNextReplace();
    buf = this->addr[index];
    if (buf == nullptr) {
        buf = BufferManager::_allocMem();
        this->addr[index] = buf;
    } else {
        if (this->dirty[index >> 5] & (1 << (index & 31))) {
            Page oldPage;
            this->hash->getKey(index, oldPage);
            FileManager::fm().writePage(oldPage.filename, oldPage.pageID, buf);
            this->dirty[index >> 5] &= (~(1 << (index & 31)));
        }
    }
    FileManager::fm().readPage(filename, pageID, buf);
    this->hash->replace(index, Page{.filename=filename, .pageID=pageID});
    return index;
}

int BufferManager::createFile(const std::string &filename) {
    return FileManager::fm().createFile(filename);
}

int BufferManager::allocPage(const std::string &filename, unsigned pageID) {
    int index = this->_fetchPage(filename, pageID);
    return index;
}

int BufferManager::getPage(const std::string &filename, unsigned pageID) {
    int index = this->hash->get(Page{.filename=filename, .pageID=pageID});
    if (index == -1) {
        index = this->_fetchPage(filename, pageID);
        this->hash->push(Page{.filename=filename, .pageID=pageID}, index);
    }
    return index;
}

BufType BufferManager::readBuffer(int index) {
    this->lru->access(index);
    return this->addr[index];
}

void BufferManager::markDirty(int index) {
    this->dirty[index >> 5] |= (1 << (index & 31));
    readBuffer(index);
}

void BufferManager::release(int index) {
    this->dirty[index >> 5] &= (~(1 << (index & 31)));
    this->lru->release(index);
    this->hash->remove(index);
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
