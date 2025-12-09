#include "database.h"
#include <stdexcept>



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

        int rc = sqlite3_close(db);

        if (rc != SQLITE_OK) {
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
        "created_date DATETIME DEFAULT CURRENT_TIMESTAMP "
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
        "definition TEXT, "
        "language TEXT DEFAULT 'en', "
        "difficulty_level INTEGER CHECK(difficulty_level BETWEEN 1 AND 5), "
        "date_added DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "tags TEXT "
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
        "order_index INTEGER, "
        "mastery_level INTEGER CHECK(mastery_level BETWEEN 0 AND 5), "
        "is_active BOOLEAN DEFAULT 1, "
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
        "schedule_id INTEGER PRIMARY KEY, "
        "word_id INTEGER NOT NULL UNIQUE, "
        "list_id INTEGER, "
        "next_review_date DATETIME NOT NULL, "
        "ease_factor REAL CHECK (ease_factor BETWEEN 1.3 AND 2.5), "
        "interval_days INTEGER NOT NULL DEFAULT 0, "
        "repetition_count INTEGER NOT NULL DEFAULT 0, "
        "streak_count INTEGER NOT NULL DEFAULT 0, "
        "last_ease_update DATETIME, "
        "scheduling_algorithm TEXT DEFAULT 'sm2', "
        "FOREIGN KEY (word_id) REFERENCES words(word_id), "
        "FOREIGN KEY (list_id) REFERENCES word_lists(list_id) "
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
        "source TEXT, "
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
        "user_id INTEGER, "
        "list_id INTEGER NOT NULL, "
        "words_mastered INTEGER NOT NULL DEFAULT 0, "
        "total_words INTEGER NOT NULL DEFAULT 0, "
        "last_studied DATETIME, "
        "overall_accuracy REAL CHECK (overall_accuracy BETWEEN 0.0 AND 1.0), "
        "total_study_time INTEGER NOT NULL DEFAULT 0, "
        "days_streak INTEGER NOT NULL DEFAULT 0, "
        "current_streak INTEGER NOT NULL DEFAULT 0, "
        "best_streak INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (user_id) REFERENCES users(user_id), "
        "FOREIGN KEY (list_id) REFERENCES vocabulary_lists(list_id), "
        "UNIQUE(user_id, list_id)"
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
        "CREATE INDEX IF NOT EXISTS idx_list_progress_user_id ON progress_list(user_id); "
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
        "strength REAL CHECK (strength BETWEEN 0.0 AND 1.0), "
        "user_confirmed BOOLEAN NOT NULL DEFAULT 0, "
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
        "CREATE INDEX IF NOT EXISTS idx_word_relations_type ON word_relations(relation_type); "
        "CREATE INDEX IF NOT EXISTS idx_word_relations_confirmed ON word_relations(user_confirmed);";

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
    const char* sql = "INSERT INTO vocabulary_lists (list_name, description, language, created_date) VALUES (?, ?, ?, datetime('now'));";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_bind_text(stmt, 1, listName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, targetLanguage.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, description.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Execution failed: " + std::string(sqlite3_errmsg(db)));
    }

    sqlite3_finalize(stmt);

    return true;
}
