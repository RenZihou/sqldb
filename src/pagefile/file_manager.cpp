// file_manager.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include "file_manager.h"

int FileManager::_openFile(const std::string &filename) {
    int f = open((this->wd + "/" + filename).c_str(), O_RDWR);
    if (f == -1) {
        std::cerr << "failed to open file " << filename << std::endl;
        return -1;
    }
    fd->push(filename, f);
    return 0;
}

void FileManager::setWd(const std::string &wd_) {
    this->wd = wd_;
}

int FileManager::createFile(const std::string &filename) {
    int f = creat((this->wd + "/" + filename).c_str(), S_IRWXU);
    if (f == -1) {
        std::cerr << "failed to create file " << filename << std::endl;
        return -1;
    }
    close(f);
    return 0;
}

int FileManager::writePage(const std::string &filename, unsigned pageID, BufType buf) {
    int f = this->fd->get(filename);  // file descriptor
    if (f == -1) {
        f = this->_openFile(filename);
        this->fd->push(filename, f);
    }
    off_t offset = pageID << PAGE_SIZE_IDX;  // byte offset of page in file
    off_t error = lseek(f, offset, SEEK_SET);
    if (error == -1) {
        std::cerr << "failed to seek to " << offset << " in file " << filename << std::endl;
        return -1;
    }
    error = write(f, (void *) buf, PAGE_SIZE);
    if (error == -1) {
        std::cerr << "failed to write to file " << filename << std::endl;
        return -1;
    }
    return 0;
}

int FileManager::readPage(const std::string &filename, unsigned pageID, BufType buf) {
    int f = this->fd->get(filename);
    if (f == -1) {
        f = this->_openFile(filename);
        this->fd->push(filename, f);
    }
    off_t offset = pageID << PAGE_SIZE_IDX;
    off_t error = lseek(f, offset, SEEK_SET);
    if (error == -1) {
//        std::cerr << "failed to seek to " << offset << " in file " << filename << std::endl;
        return -1;
    }
    error = read(f, (void *) buf, PAGE_SIZE);
    if (error == -1) {
        std::cerr << "failed to read from file " << filename << std::endl;
        return -1;
    }
    return 0;
}

int FileManager::closeFile(const std::string &filename) {
    int f = this->fd->remove(filename);
    close(f);
    return 0;
}
