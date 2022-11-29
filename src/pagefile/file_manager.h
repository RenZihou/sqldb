// file_manager.h
// -*- coding: utf-8 -*-
// @Author: RenZihou

#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include <string>
#include <iostream>
#include <utility>
#include <unistd.h>
#include <fcntl.h>
#include "../util/constants.h"
#include "../util/hashmap.h"

class FileManager {
private:
    std::string wd = ".";  // working directory (table directory)
//    HashMap<std::string> *fd;  // <filename, file descriptor> map
    StringHashMap *fd;

    int _openFile(const std::string &filename) {
        int f = open((this->wd + "/" + filename).c_str(), O_RDWR);
        if (f == -1) {
            std::cerr << "failed to open file" << f << std::endl;
            return -1;
        }
        fd->push(filename, f);
        return 0;
    }

    FileManager() : fd(new StringHashMap(MAX_OPEN_FILE)) {}

public:
//    explicit FileManager(std::string wd) : wd(std::move(wd)), fd(new SIMap) {}

    FileManager(const FileManager &) = delete;

    FileManager(FileManager &&) = delete;

    static FileManager &fm() {
        static FileManager fm;
        return fm;
    }

    /**
     * @param wd_ new working directory
     * @description set working directory
     */
    void setWd(const std::string &wd_) {
        this->wd = wd_;
    }

    /**
     * @param filename filename relative to working directory
     * @return 0 for success, -1 for error
     * @description create file
     */
    int createFile(const std::string &filename) {
        int f = creat((this->wd + "/" + filename).c_str(), S_IRWXU);
        if (f == -1) {
            std::cerr << "failed to create file " << f << std::endl;
            return -1;
        }
        close(f);
        return 0;
    }

    /**
     * @param filename filename
     * @param pageID page id in file
     * @param buf data to write
     * @return 0 for success, -1 for error
     * @description write data (start form buf, length is PAGE_SIZE) to the page
     */
    int writePage(const std::string &filename, unsigned pageID, BufType buf) {
        int f = this->fd->get(filename);  // file descriptor
        if (f == -1) {
            f = this->_openFile(filename);
            this->fd->push(filename, f);
        }
        off_t offset = pageID << PAGE_SIZE_IDX;  // byte offset of page in file
        off_t error = lseek(f, offset, SEEK_SET);
        if (error == -1) {
            std::cerr << "failed to seek to " << offset << " in file " << f << std::endl;
            return -1;
        }
        error = write(f, (void *) buf, PAGE_SIZE);
        if (error == -1) {
            std::cerr << "failed to write to file " << f << std::endl;
            return -1;
        }
        return 0;
    }

    /**
     * @param filename filename
     * @param pageID page id in file
     * @param buf address to store data
     * @return 0 for success, -1 for error
     * @description copy whole page to buf
     */
    int readPage(const std::string &filename, unsigned pageID, BufType buf) {
        int f = this->fd->get(filename);
        if (f == -1) {
            f = this->_openFile(filename);
            this->fd->push(filename, f);
        }
        off_t offset = pageID << PAGE_SIZE_IDX;
        off_t error = lseek(f, offset, SEEK_SET);
        if (error == -1) {
            std::cerr << "failed to seek to " << offset << " in file " << f << std::endl;
            return -1;
        }
        error = read(f, (void *) buf, PAGE_SIZE);
        if (error == -1) {
            std::cerr << "failed to read from file " << f << std::endl;
            return -1;
        }
        return 0;
    }

    /**
     * @param filename filename
     * @return 0 for success
     * @description close file
     */
    int closeFile(const std::string &filename) {
        int f = this->fd->remove(filename);
        close(f);
        return 0;
    }

    ~FileManager() {
        delete fd;
    }
};

#endif  // FILE_MANAGER_H_
