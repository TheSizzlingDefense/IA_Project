#include "database.h"



dataBase::dataBase(const std::string& dbPath) {
    int result = sqlite3_open(dbPath.c_str(), &db);    // Opening the sqlite3 database
    if (result != SQLITE_OK) {
        throw std::runtime_error("Can't open databse: " + std::string(sqlite3_errmsg(db)));
    }

    enableForeignKeys();
    createListTable();
}

void dataBase::enableForeignKeys() {
    char* errorMessage = nullptr;

    int result = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errorMessage);

    if (result != SQLITE_OK) {
        std::string error = "Failed to enable foreign keys";
        error += errorMessage;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error);
    }
}

bool dataBase::createListTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS lists ("
        "list_id INTEGER PRIMARY KEY AUTOINCREMENT,"    // Unique ID for each list
        "list_name TEXT NOT, NULL"      // Name the user sees when list is displayed
        "description TEXT, "          // Description of what the list contains
        "language TEXT, "             // Language of study
        "created_date DATETIME DEFAULT CURRENT_TIMESTAMP, "   // Date the list was created
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

bool dataBase::createWordsTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS words ("
        "word_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "word TEXT NOT NULL, "
        "part_of_speech TEXT, "
        "definition TEXT, "
        "language TEXT DEFAULT 'en', "
        "difficulty_level INTEGER CHECK(difficulty_level BETWEEN 1 AND 5), "
        "date_added DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "tags TEXT, "
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

bool dataBase::createListWordTable() {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS list_words ("
        "list_word_id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "list_id INTEGER NOT NULL, "
        "word_id INTEGER NOT NULL, "
        "added_date DATETIME DEFAULT CURRENT_TIMESTAMP, "
        "order_index INTEGER, "
        "mastery_level INTEGER CHECK(mastery_level BETWEEN 0 AND 5), "
        "is_active BOOLEAN DEFAULT 1, "
        "FOREIGN KEY (list_id) REFERENCES word_lists(list_id) ON DELETE CASCADE, "
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
