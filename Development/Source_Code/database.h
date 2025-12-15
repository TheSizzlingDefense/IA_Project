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

    // Transaction helpers
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // Lookup / CRUD helpers for adding words to lists
    int getWordId(const std::string& word, const std::string& language);
    int getListId(const std::string& listName);
    int addOrGetWord(const std::string& word, const std::string& partOfSpeech, const std::string& definition, const std::string& language);
    // returns true if a new membership row was inserted (false if it already existed)
    bool addWordToList(int listID, int wordID);
    bool initReviewSchedule(int wordID, int listID);
    bool incrementListProgress(int listID, int delta = 1);

    // High-level convenience: add a word to a list and initialize related rows.
    // Returns the word_id on success, or -1 on error (throws on DB errors).
    int addWordAndSetup(int listID, const std::string& word, const std::string& partOfSpeech = "", const std::string& definition = "", const std::string& language = "");

    bool createNewList(std::string listName, std::string targetLanguage, std::string description);

    std::vector<std::string> getVocabLists();

    bool createNewExample(int wordID, std::string exampleText = "", std::string contextNotes = "");

    bool createNewRelation(int word1ID, int word2ID, std::string relationType = "");

private:
    sqlite3* db;
};

#endif // DATABASE_H
