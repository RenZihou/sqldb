// database.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef DATABASE_H_
#define DATABASE_H_

#include <string>
#include <vector>

#include "../query/printer.h"

class Database {
private:
    std::vector<std::string> databases;
    std::vector<std::string> tables;
    std::string current_db;

    Database() = default;

    ~Database() = default;

public:
    static Database &db() {
        static Database instance;
        return instance;
    }

    void boost();

    [[nodiscard]] const std::string &getCurrentDb() const;

    void assertTableExists(const std::string &name) const;

    void createDb(const std::string &name);

    void dropDb(const std::string &name);

    void useDb(const std::string &name);

    void showDb(Printer *printer);

    void createTable(const std::string &name);

    void dropTable(const std::string &name);

    void showTable(Printer *printer);

    void showIndex();

    void shutdown();
};

#endif  // DATABASE_H_
