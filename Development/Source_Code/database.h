#ifndef DATABASE_H
#define DATABASE_H

extern "C" {
    #include "sqlite3.h"
}
#include <string>

class DataBase
{
public:
    DataBase(const std::string& dbPath);
    ~DataBase();

    void enableForeignKeys();

    bool createVocabListTable();

    bool createWordsTable();

    bool createListWordTable();

    bool createStudySessionTable();

    bool createReviewScheduleTable();

    bool createExampleTable();

    bool createListProgressTable();

    bool createWordRelationTable();

private:
    sqlite3* db;
};

#endif // DATABASE_H
