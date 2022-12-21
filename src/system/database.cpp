// database.cpp
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#include <algorithm>

#include "database.h"
#include "../query/op.h"
#include "../util/exception.h"

void Database::boost() {
    if (FileManager::fm().fileExists(".dbs") == -1) {
        FileManager::fm().createFile(".dbs");
        OpTableCreate(".dbs", {Column{
                .name="db_name",
                .type=ColumnType::VARCHAR,
                .length=MAX_DATABASE_NAME_LEN,
                .flags=0,
                .default_value=""}}).execute(nullptr);
    } else {
        MemoryStringPrinter printer;
        OpTableSelect({std::make_tuple("", "*")}, {".dbs"}, {}).execute(&printer);
        this->databases = printer.getLines();
    }
}

const std::string &Database::getCurrentDb() const { return this->current_db; }

void Database::assertTableExists(const std::string &name) const {
    if (name[0] == '.') return;  // do not check meta-tables as we always trust them
    if (std::find(this->tables.begin(), this->tables.end(), name) == this->tables.end()) {
        throw SqlDBException("table does not exist: " + name);
    }
}

void Database::createDb(const std::string &name) {
    if (std::find(this->databases.begin(), this->databases.end(), name) != this->databases.end()) {
        throw SqlDBException("database already exists: " + name);
    }
    this->databases.emplace_back(name);
}

void Database::dropDb(const std::string &name) {
    if (std::find(this->databases.begin(), this->databases.end(), name) == this->databases.end()) {
        throw SqlDBException("database does not exist: " + name);
    }
    this->databases.erase(std::remove(this->databases.begin(), this->databases.end(), name),
                          this->databases.end());
    FileManager::fm().rmFile(name);
}

void Database::useDb(const std::string &name) {
    if (std::find(this->databases.begin(), this->databases.end(), name) == this->databases.end()) {
        throw SqlDBException("database does not exist: " + name);
    }
    BufferManager::bm().writeBackAll();
    for (auto &table: this->tables) {
        FileManager::fm().closeFile(table);
    }
    FileManager::fm().closeFile(".table");
    FileManager::fm().setWd(name);
    current_db = name;
    if (FileManager::fm().fileExists(".tables") == -1) {
        FileManager::fm().createFile(".tables");
        OpTableCreate(".tables", {Column{
                .name="table_name",
                .type=ColumnType::VARCHAR,
                .length=MAX_TABLE_NAME_LEN,
                .flags=0,
                .default_value=""}}).execute(nullptr);
        this->tables.clear();
    } else {
        MemoryStringPrinter printer;
        std::vector<std::tuple<std::string, std::string>> selectors;
        selectors.emplace_back("", "table_name");
        OpTableSelect(selectors, {".tables"}, {}).execute(&printer);
        this->tables = printer.getLines();
    }
}

void Database::showDb(Printer *printer) {
    printer->printHeader({"db_name"});
    for (const auto &db: this->databases) {
        VarChar value((BufType) db.c_str(), db.length());
        printer->printLine({&value});
    }
    printer->printEnd();
}

void Database::createTable(const std::string &name) {
    if (name[0] == '.') return;  // do not create meta-tables
    if (this->current_db.empty()) {
        throw SqlDBException("no database selected");
    }
    if (std::find(this->tables.begin(), this->tables.end(), name) != this->tables.end()) {
        throw SqlDBException("table already exists: " + name);
    }
    this->tables.emplace_back(name);
    OpTableInsert(".tables", {{name}}).execute(nullptr);
}

void Database::dropTable(const std::string &name) {
    if (std::find(this->tables.begin(), this->tables.end(), name) == this->tables.end()) {
        throw SqlDBException("table does not exist: " + name);
    }
    this->tables.erase(std::remove(this->tables.begin(), this->tables.end(), name),
                       this->tables.end());
    ConditionCmp condition(new ExprColumn("", name), new ExprColumn("", name), new Equal);
    OpTableDelete(".tables", {&condition}).execute(nullptr);
    FileManager::fm().rmFile(name);
}

void Database::showTable(Printer *printer) {
    printer->printHeader({"table_name"});
    for (const auto &table: this->tables) {
        VarChar value((BufType) table.c_str(), table.length());
        printer->printLine({&value});
    }
    printer->printEnd();
}

void Database::shutdown() {
    BufferManager::bm().writeBackAll();
    for (auto &table: this->tables) {
        FileManager::fm().closeFile(table);
    }
    FileManager::fm().closeFile(".table");
    FileManager::fm().setWd(".");
    MemoryStringPrinter printer;
    OpTableDelete(".dbs", {}).execute(&printer);
    std::vector<std::vector<std::string>> values;
    for (const auto &db: this->databases) {
        values.emplace_back(std::vector<std::string>{db});
    }
    OpTableInsert(".dbs", values).execute(&printer);
    BufferManager::bm().writeBackAll();
    FileManager::fm().closeFile(".dbs");
}