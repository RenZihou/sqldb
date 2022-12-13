// file_manager.h
// -*- coding: utf-8 -*-
// @Author: RenZihou

#ifndef FILE_MANAGER_H_
#define FILE_MANAGER_H_

#include <string>

#include "../util/constants.h"
#include "hashmap.h"

class FileManager {
private:
    std::string wd = ".";  // working directory (table directory)
    StringHashMap *fd;

    int _openFile(const std::string &filename);

    FileManager() : fd(new StringHashMap(MAX_OPEN_FILE)) {}

    ~FileManager() { delete fd; }

public:
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
    void setWd(const std::string &wd_);

    /**
     * @param filename filename relative to working directory
     * @return 0 for success, -1 for error
     * @description create file
     */
    int createFile(const std::string &filename);

    /**
     * @param filename filename
     * @param pageID page id in file
     * @param buf data to write
     * @return 0 for success, -1 for error
     * @description write data (start form buf, length is PAGE_SIZE) to the page
     */
    int writePage(const std::string &filename, unsigned pageID, BufType buf);

    /**
     * @param filename filename
     * @param pageID page id in file
     * @param buf address to store data
     * @return 0 for success, -1 for error
     * @description copy whole page to buf
     */
    int readPage(const std::string &filename, unsigned pageID, BufType buf);

    /**
     * @param filename filename
     * @return 0 for success
     * @description close file
     */
    int closeFile(const std::string &filename);
};

#endif  // FILE_MANAGER_H_
