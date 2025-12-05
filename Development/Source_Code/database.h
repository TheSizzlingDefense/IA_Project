#ifndef DATABASE_H
#define DATABASE_H

#include "sqlite3.h"
#include <string>

class dataBase
{
public:
    dataBase(const std::string& dbPath);
    ~dataBase();

private:
    sqlite3* db;
};

#endif // DATABASE_H
