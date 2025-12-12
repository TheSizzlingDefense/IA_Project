#ifndef DATABASE_H
#define DATABASE_H

extern "C" {
    #include "sqlite3.h"
}
#include <string>
#include <vector>

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

    bool createNewList(std::string listName, std::string targetLanguage, std::string description);

    std::vector<std::string> getVocabLists();

    bool createNewWord(std::string word, std::string partOfSpeech = "", std::string definition = "", std::string targetLanguage = "");

    bool createNewExample(int wordID, std::string exampleText = "", std::string contextNotes = "");

private:
    sqlite3* db;
};

#endif // DATABASE_H
