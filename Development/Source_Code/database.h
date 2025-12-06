#ifndef DATABASE_H
#define DATABASE_H

extern "C" {
    #include "sqlite3.h"
}
#include <string>
#include <stdexcept>

class dataBase
{
public:
    dataBase(const std::string& dbPath);
    ~dataBase();

    void enableForeignKeys();

    bool createListTable();

    bool createWordsTable();

    bool createListWordTable();

private:
    sqlite3* db;
};

#endif // DATABASE_H
