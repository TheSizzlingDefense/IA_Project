#include "database.h"
#include <stdexcept>
#include <vector>


DataBase::DataBase(const std::string& dbPath) {
    int result = sqlite3_open(dbPath.c_str(), &db);    // Opening the sqlite3 DataBase
    if (result != SQLITE_OK) {
        throw std::runtime_error("Can't open databse: " + std::string(sqlite3_errmsg(db)));
    }

    enableForeignKeys();
    createVocabListTable();
    createWordsTable();
    createListWordTable();
    createStudySessionTable();
    createReviewScheduleTable();
    createExampleTable();
    createListProgressTable();
    createWordRelationTable();
}

DataBase::~DataBase() {
    if (db != nullptr) {

        int result = sqlite3_close(db);

        if (result != SQLITE_OK) {
            sqlite3_close_v2(db);
        }

        db = nullptr;
    }
}

void DataBase::enableForeignKeys() {
    char* errorMessage = nullptr;

    int result = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to enable foreign keys";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}

bool DataBase::createVocabListTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS vocabulary_lists ("
        "list_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "list_name TEXT NOT NULL, "
        "description TEXT, "
        "language TEXT, "
        "date_created DATETIME DEFAULT CURRENT_TIMESTAMP "
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create list table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createWordsTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS words ("
        "word_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "word TEXT NOT NULL, "
        "part_of_speech TEXT, "
        "definition TEXT NOT NULL, "
        "language TEXT DEFAULT 'en', "
        "date_added DATETIME DEFAULT CURRENT_TIMESTAMP "
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createListWordTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS list_words ("
        "list_word_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "list_id INTEGER NOT NULL, "
        "word_id INTEGER NOT NULL, "
        "added_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (list_id) REFERENCES vocabulary_lists(list_id) ON DELETE CASCADE, "
        "FOREIGN KEY (word_id) REFERENCES words(word_id) ON DELETE CASCADE, "
        "UNIQUE(list_id, word_id)"
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_list_words_list_id ON list_words(list_id); "
        "CREATE INDEX IF NOT EXISTS idx_list_words_word_id ON list_words(word_id);";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create index: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createStudySessionTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS study_sessions ( "
        "session_id INTEGER PRIMARY KEY, "
        "word_id INTEGER NOT NULL, "
        "review_date DATETIME NOT NULL, "
        "was_correct BOOLEAN NOT NULL, "
        "response_time INTEGER, "
        "confidence_score INTEGER CHECK (confidence_score BETWEEN 1 AND 5), "
        "study_mode TEXT CHECK (study_mode IN ('flashcard', 'multiple_choice', 'typing', 'listen')), "
        "list_id INTEGER, "
        "notes TEXT, "
        "device_info TEXT, "
        "FOREIGN KEY (word_id) REFERENCES words(word_id) "
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createReviewScheduleTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS review_schedule ( "
        "schedule_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "word_id INTEGER NOT NULL UNIQUE, "
        "list_id INTEGER, "
        "next_review_date DATETIME NOT NULL, "
        "ease_factor REAL CHECK (ease_factor BETWEEN 1.3 AND 2.5), "
        "interval_days INTEGER NOT NULL DEFAULT 0, "
        "repetition_count INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (word_id) REFERENCES words(word_id), "
        "FOREIGN KEY (list_id) REFERENCES vocabulary_lists(list_id) "
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

     const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_next_review_date ON review_schedule(next_review_date); "
        "CREATE INDEX IF NOT EXISTS idx_list_id ON review_schedule(list_id);";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create index: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createExampleTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS word_examples ( "
        "example_id INTEGER PRIMARY KEY, "
        "word_id INTEGER NOT NULL, "
        "example_text TEXT NOT NULL, "
        "context_notes TEXT, "
        "date_added DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY (word_id) REFERENCES words(word_id) "
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_word_examples_word_id ON word_examples(word_id); ";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create index: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createListProgressTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS progress_list ( "
        "progress_id INTEGER PRIMARY KEY, "
        "list_id INTEGER NOT NULL, "
        "words_mastered INTEGER NOT NULL DEFAULT 0, "
        "total_words INTEGER NOT NULL DEFAULT 0, "
        "last_studied DATETIME, "
        "overall_accuracy REAL CHECK (overall_accuracy BETWEEN 0.0 AND 1.0), "
        "total_study_time INTEGER NOT NULL DEFAULT 0, "
        "days_streak INTEGER NOT NULL DEFAULT 0, "
        "current_streak INTEGER NOT NULL DEFAULT 0, "
        "best_streak INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (list_id) REFERENCES vocabulary_lists(list_id)"
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_list_progress_list_id ON progress_list(list_id); "
        "CREATE INDEX IF NOT EXISTS idx_list_progress_last_studied ON progress_list(last_studied);";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create index: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createWordRelationTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS word_relations ( "
        "relation_id INTEGER PRIMARY KEY, "
        "word1_id INTEGER NOT NULL, "
        "word2_id INTEGER NOT NULL, "
        "relation_type TEXT NOT NULL CHECK (relation_type IN ('synonym', 'antonym', 'related', 'derived_from')), "
        "FOREIGN KEY (word1_id) REFERENCES words(word_id), "
        "FOREIGN KEY (word2_id) REFERENCES words(word_id), "
        "UNIQUE(word1_id, word2_id, relation_type) "
        ");";

    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create word table: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_word_relations_word1 ON word_relations(word1_id); "
        "CREATE INDEX IF NOT EXISTS idx_word_relations_word2 ON word_relations(word2_id); "
        "CREATE INDEX IF NOT EXISTS idx_word_relations_type ON word_relations(relation_type); ";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string error = "Failed to create index: ";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }

    return true;
}

bool DataBase::createNewList(std::string listName, std::string targetLanguage = "", std::string description = "") {
    const char* sql = "INSERT INTO vocabulary_lists (list_name, description, language, date_created) VALUES (?, ?, ?, datetime('now'));";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, listName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, targetLanguage.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    return true;
}

std::vector<std::string> DataBase::getVocabLists() {
    std::vector<std::string> allVocabLists;
    const char* sql = "SELECT DISTINCT list_name FROM vocabulary_lists ORDER BY list_name";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* listName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        if (listName) {
            allVocabLists.push_back(std::string(listName));
        }
    }

    sqlite3_finalize(stmt);

    return allVocabLists;
}

std::vector<std::pair<std::string, std::string>> DataBase::getVocabListsWithNextReview() {
    std::vector<std::pair<std::string, std::string>> results;

    const char* sql = "SELECT list_id, list_name FROM vocabulary_lists";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int listID = sqlite3_column_int(stmt, 0);
        const unsigned char* txt = sqlite3_column_text(stmt, 1);
        std::string listName = txt ? reinterpret_cast<const char*>(txt) : std::string("");

        // Query earliest next_review_date for this list
        const char* dateSql = "SELECT MIN(next_review_date) FROM review_schedule WHERE list_id = ?";
        sqlite3_stmt* dstmt = nullptr;
        int drc = sqlite3_prepare_v2(db, dateSql, -1, &dstmt, nullptr);
        if (drc != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_bind_int(dstmt, 1, listID);
        drc = sqlite3_step(dstmt);
        std::string nextReview;
        if (drc == SQLITE_ROW) {
            const unsigned char* dtext = sqlite3_column_text(dstmt, 0);
            if (dtext) nextReview = reinterpret_cast<const char*>(dtext);
        }

        sqlite3_finalize(dstmt);

        results.emplace_back(listName, nextReview);
    }

    sqlite3_finalize(stmt);
    return results;
}

bool DataBase::createNewExample(int wordID, std::string exampleText, std::string contextNotes) {
    const char* sql = "INSERT INTO word_examples (word_id, example_text, context_notes, date_added) VALUES (?, ? , ?, datetime('now'));";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, wordID);
    sqlite3_bind_text(stmt, 2, exampleText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contextNotes.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
        }

    sqlite3_finalize(stmt);

    return true;
}

bool DataBase::createNewRelation(int word1ID, int word2ID, std::string relationType) {
    const char* sql = "INSERT INTO word_relations (word1_id, word2_id, relation_type) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, word1ID);
    sqlite3_bind_int(stmt, 2, word2ID);
    sqlite3_bind_text(stmt, 3, relationType.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
    
    return true;
}

bool DataBase::beginTransaction() {
    char* err = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string e = "Failed to begin transaction: ";
        if (err) { e += err; sqlite3_free(err); }
        throw std::runtime_error(e);
    }
    return true;
}

bool DataBase::commitTransaction() {
    char* err = nullptr;
    int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string e = "Failed to commit transaction: ";
        if (err) { e += err; sqlite3_free(err); }
        throw std::runtime_error(e);
    }
    return true;
}

bool DataBase::rollbackTransaction() {
    char* err = nullptr;
    int rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        std::string e = "Failed to rollback transaction: ";
        if (err) { e += err; sqlite3_free(err); }
        throw std::runtime_error(e);
    }
    return true;
}

int DataBase::getWordId(const std::string& word, const std::string& language) {
    const char* sql = "SELECT word_id FROM words WHERE word = ? AND language = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, language.c_str(), -1, SQLITE_TRANSIENT);

    int wordID = -1;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        wordID = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return wordID;
}

int DataBase::getListId(const std::string& listName) {
    const char* sql = "SELECT list_id FROM vocabulary_lists WHERE list_name = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, listName.c_str(), -1, SQLITE_TRANSIENT);

    int listID = -1;
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        listID = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return listID;
}

int DataBase::addOrGetWord(const std::string& word, const std::string& partOfSpeech, const std::string& definition, const std::string& language) {
    int existing = getWordId(word, language);
    if (existing != -1) return existing;

    const char* sql = "INSERT INTO words (word, part_of_speech, definition, language, date_added) VALUES (?, ?, ?, ?, datetime('now'));";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, partOfSpeech.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, definition.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, language.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
    sqlite3_int64 id64 = sqlite3_last_insert_rowid(db);
    return static_cast<int>(id64);
}

bool DataBase::addWordToList(int listID, int wordID) {
    const char* sql = "INSERT OR IGNORE INTO list_words (list_id, word_id, added_date) VALUES (?, ?, datetime('now'));";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, listID);
    sqlite3_bind_int(stmt, 2, wordID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    // sqlite3_changes returns number of rows modified by the most recent operation on the connection
    int changes = sqlite3_changes(db);
    return changes > 0; // true if inserted, false if ignored
}

bool DataBase::initReviewSchedule(int wordID, int listID) {
    const char* sql = "INSERT OR IGNORE INTO review_schedule (word_id, list_id, next_review_date, ease_factor, interval_days, repetition_count) VALUES (?, ?, datetime('now'), 2.5, 0, 0);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, wordID);
    sqlite3_bind_int(stmt, 2, listID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
    return true;
}

bool DataBase::incrementListProgress(int listID, int delta) {
    const char* updateSql = "UPDATE progress_list SET total_words = total_words + ? WHERE list_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, delta);
    sqlite3_bind_int(stmt, 2, listID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    int changes = sqlite3_changes(db);
    if (changes == 0) {
        // No existing row; insert a new progress row
        const char* insertSql = "INSERT INTO progress_list (list_id, total_words, words_mastered) VALUES (?, ?, 0);";
        sqlite3_stmt* istmt = nullptr;
        rc = sqlite3_prepare_v2(db, insertSql, -1, &istmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_bind_int(istmt, 1, listID);
        sqlite3_bind_int(istmt, 2, delta);

        rc = sqlite3_step(istmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(istmt);
            throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_finalize(istmt);
    }

    return true;
}

int DataBase::addWordAndSetup(int listID, const std::string& word, const std::string& partOfSpeech, const std::string& definition, const std::string& language) {
    try {
        beginTransaction();

        int wordID = addOrGetWord(word, partOfSpeech, definition, language);
        if (wordID < 0) {
            rollbackTransaction();
            throw std::runtime_error("Failed to obtain or create word id");
        }

        bool inserted = addWordToList(listID, wordID);

        // Initialize review schedule regardless (INSERT OR IGNORE inside)
        initReviewSchedule(wordID, listID);

        // Only increment progress when a new membership row was inserted
        if (inserted) {
            incrementListProgress(listID, 1);
        }

        commitTransaction();
        return wordID;
    } catch (...) {
        try { rollbackTransaction(); } catch (...) {}
        throw; // rethrow original exception
    }
}

std::vector<DataBase::DueCard> DataBase::getDueCards(int listID) {
    std::vector<DueCard> out;
    const char* sqlAll =
        "SELECT rs.schedule_id, rs.word_id, rs.list_id, w.word, w.definition, rs.ease_factor, rs.interval_days, rs.repetition_count, rs.next_review_date "
        "FROM review_schedule rs JOIN words w ON rs.word_id = w.word_id "
        "WHERE rs.next_review_date <= datetime('now') "
        "ORDER BY rs.next_review_date ASC;";

    const char* sqlList =
        "SELECT rs.schedule_id, rs.word_id, rs.list_id, w.word, w.definition, rs.ease_factor, rs.interval_days, rs.repetition_count, rs.next_review_date "
        "FROM review_schedule rs JOIN words w ON rs.word_id = w.word_id "
        "WHERE rs.list_id = ? AND rs.next_review_date <= datetime('now') "
        "ORDER BY rs.next_review_date ASC;";

    const char* sql = (listID < 0) ? sqlAll : sqlList;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    if (listID >= 0) sqlite3_bind_int(stmt, 1, listID);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        DueCard c;
        c.schedule_id = sqlite3_column_int(stmt, 0);
        c.word_id = sqlite3_column_int(stmt, 1);
        c.list_id = sqlite3_column_int(stmt, 2);
        const unsigned char* wtxt = sqlite3_column_text(stmt, 3);
        c.word = wtxt ? reinterpret_cast<const char*>(wtxt) : std::string("");
        const unsigned char* dtxt = sqlite3_column_text(stmt, 4);
        c.definition = dtxt ? reinterpret_cast<const char*>(dtxt) : std::string("");
        c.ease_factor = sqlite3_column_double(stmt, 5);
        c.interval_days = sqlite3_column_int(stmt, 6);
        c.repetition_count = sqlite3_column_int(stmt, 7);
        const unsigned char* ndtxt = sqlite3_column_text(stmt, 8);
        c.next_review_date = ndtxt ? reinterpret_cast<const char*>(ndtxt) : std::string("");
        out.push_back(std::move(c));
    }

    sqlite3_finalize(stmt);
    return out;
}

bool DataBase::updateReviewScheduleForWord(int wordID, int listID, int repetition_count, int interval_days, double ease_factor, const std::string& next_review_date) {
    const char* sql = "UPDATE review_schedule SET repetition_count = ?, interval_days = ?, ease_factor = ?, next_review_date = ? WHERE word_id = ? AND list_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, repetition_count);
    sqlite3_bind_int(stmt, 2, interval_days);
    sqlite3_bind_double(stmt, 3, ease_factor);
    sqlite3_bind_text(stmt, 4, next_review_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, wordID);
    sqlite3_bind_int(stmt, 6, listID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
    return true;
}

bool DataBase::recordStudySession(int wordID, int listID, bool was_correct, int quality, const std::string& study_mode) {
    const char* sql = "INSERT INTO study_sessions (word_id, review_date, was_correct, response_time, confidence_score, study_mode, list_id, notes, device_info) VALUES (?, datetime('now'), ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_int(stmt, 1, wordID);
    sqlite3_bind_int(stmt, 2, was_correct ? 1 : 0);
    sqlite3_bind_int(stmt, 3, 0); // response_time unknown
    // confidence_score must be between 1 and 5 (CHECK constraint). Clamp the provided quality.
    int confScore = quality;
    if (confScore < 1) confScore = 1;
    if (confScore > 5) confScore = 5;
    sqlite3_bind_int(stmt, 4, confScore);
    sqlite3_bind_text(stmt, 5, study_mode.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 6, listID);
    sqlite3_bind_text(stmt, 7, "", -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, "", -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);
    return true;
}

// Backwards-compatible wrapper that records as flashcard mode
bool DataBase::recordStudySession(int wordID, int listID, bool was_correct, int quality) {
    return recordStudySession(wordID, listID, was_correct, quality, std::string("flashcard"));
}

std::vector<std::pair<int, std::string>> DataBase::getRandomWordsInList(int listID, int excludeWordID, int count) {
    std::vector<std::pair<int, std::string>> out;
    const char* sql_with_exclude =
        "SELECT w.word_id, w.definition FROM list_words lw JOIN words w ON lw.word_id = w.word_id WHERE lw.list_id = ? AND w.word_id != ? ORDER BY RANDOM() LIMIT ?;";
    const char* sql_no_exclude =
        "SELECT w.word_id, w.definition FROM list_words lw JOIN words w ON lw.word_id = w.word_id WHERE lw.list_id = ? ORDER BY RANDOM() LIMIT ?;";
    const char* sql_global_with_exclude =
        "SELECT w.word_id, w.definition FROM words w WHERE w.word_id != ? ORDER BY RANDOM() LIMIT ?;";
    const char* sql_global_no_exclude =
        "SELECT w.word_id, w.definition FROM words w ORDER BY RANDOM() LIMIT ?;";

    sqlite3_stmt* stmt = nullptr;
    int rc;
    if (listID >= 0) {
        if (excludeWordID >= 0) {
            rc = sqlite3_prepare_v2(db, sql_with_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            sqlite3_bind_int(stmt, 1, listID);
            sqlite3_bind_int(stmt, 2, excludeWordID);
            sqlite3_bind_int(stmt, 3, count);
        } else {
            rc = sqlite3_prepare_v2(db, sql_no_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            sqlite3_bind_int(stmt, 1, listID);
            sqlite3_bind_int(stmt, 2, count);
        }
    } else {
        // global across all words
        if (excludeWordID >= 0) {
            rc = sqlite3_prepare_v2(db, sql_global_with_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            sqlite3_bind_int(stmt, 1, excludeWordID);
            sqlite3_bind_int(stmt, 2, count);
        } else {
            rc = sqlite3_prepare_v2(db, sql_global_no_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
            sqlite3_bind_int(stmt, 1, count);
        }
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int wid = sqlite3_column_int(stmt, 0);
        const unsigned char* dtxt = sqlite3_column_text(stmt, 1);
        std::string def = dtxt ? reinterpret_cast<const char*>(dtxt) : std::string("");
        out.emplace_back(wid, def);
    }

    sqlite3_finalize(stmt);
    return out;
}
