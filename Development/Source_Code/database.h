#ifndef DATABASE_H
#define DATABASE_H

extern "C" {
    #include "sqlite3.h"
}
#include <string>
#include <vector>
#include <utility>
#include <tuple>

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

    bool deleteList(int listID);

    std::vector<std::string> getVocabLists();
    // Returns a vector of pairs (list_name, next_review_date_string or empty if none)
    std::vector<std::pair<std::string, std::string>> getVocabListsWithNextReview();

    struct DueCard {
        int schedule_id;
        int word_id;
        int list_id;
        std::string word;
        std::string definition;
        double ease_factor;
        int interval_days;
        int repetition_count;
        std::string next_review_date;
    };

    // Get due cards (next_review_date <= now). If listID < 0, return for all lists.
    std::vector<DueCard> getDueCards(int listID = -1);

    // Update review schedule for a given word/list
    bool updateReviewScheduleForWord(int wordID, int listID, int repetition_count, int interval_days, double ease_factor, const std::string& next_review_date);

    // Record a study session entry
    bool recordStudySession(int wordID, int listID, bool was_correct, int quality);

    // Record a study session with an explicit study mode (e.g. "flashcard", "multiple_choice")
    bool recordStudySession(int wordID, int listID, bool was_correct, int quality, const std::string& study_mode);

    // Returns a human-readable summary string of study sessions (counts, averages, breakdowns)
    std::string getStudySessionSummary();

    // Get random words (id and definition) from a list to be used as distractors.
    // excludeWordID may be -1 to not exclude anything.
    std::vector<std::pair<int, std::string>> getRandomWordsInList(int listID, int excludeWordID, int count);

    // Return all words in a list (word_id, word_text, definition). If listID < 0 return all words.
    std::vector<std::tuple<int, std::string, std::string>> getWordsInList(int listID);

    bool createNewExample(int wordID, std::string exampleText = "", std::string contextNotes = "");

    bool createNewRelation(int word1ID, int word2ID, std::string relationType = "");

private:
    sqlite3* db;
};

#endif // DATABASE_H
