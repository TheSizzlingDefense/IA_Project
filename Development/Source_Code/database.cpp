#include "database.h"
#include <stdexcept>
#include <vector>
#include <sstream>
#include <cmath>
#include <QDebug>


DataBase::DataBase(const std::string& dbPath) {
    int result = sqlite3_open(dbPath.c_str(), &db);    // Opening the sqlite3 DataBase
    if (result != SQLITE_OK) {
        QString errorMsg = "Can't open database: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString error = "Failed to enable foreign keys: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create list table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create word table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create list_words table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_list_words_list_id ON list_words(list_id); "
        "CREATE INDEX IF NOT EXISTS idx_list_words_word_id ON list_words(word_id);";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        QString error = "Failed to create index on list_words: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create study_sessions table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create review_schedule table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

     const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_next_review_date ON review_schedule(next_review_date); "
        "CREATE INDEX IF NOT EXISTS idx_list_id ON review_schedule(list_id);";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        QString error = "Failed to create index on review_schedule: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create word_examples table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_word_examples_word_id ON word_examples(word_id); ";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        QString error = "Failed to create index on word_examples: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

    return true;
}

std::string DataBase::getStudySessionSummary() {
    std::ostringstream out;

    const char* totalSql = "SELECT COUNT(*) FROM study_sessions;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, totalSql, -1, &stmt, nullptr);
    long long total = 0;
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            total = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    const char* correctSql = "SELECT COUNT(*) FROM study_sessions WHERE was_correct = 1;";
    rc = sqlite3_prepare_v2(db, correctSql, -1, &stmt, nullptr);
    long long correct = 0;
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            correct = sqlite3_column_int64(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    const char* avgRespSql = "SELECT AVG(response_time) FROM study_sessions WHERE response_time IS NOT NULL;";
    rc = sqlite3_prepare_v2(db, avgRespSql, -1, &stmt, nullptr);
    double avgResp = 0.0;
    bool hasAvgResp = false;
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt,0) != SQLITE_NULL) {
                avgResp = sqlite3_column_double(stmt, 0);
                hasAvgResp = true;
            }
        }
    }
    sqlite3_finalize(stmt);

    const char* avgConfSql = "SELECT AVG(confidence_score) FROM study_sessions WHERE confidence_score IS NOT NULL;";
    rc = sqlite3_prepare_v2(db, avgConfSql, -1, &stmt, nullptr);
    double avgConf = 0.0;
    bool hasAvgConf = false;
    if (rc == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt,0) != SQLITE_NULL) {
                avgConf = sqlite3_column_double(stmt, 0);
                hasAvgConf = true;
            }
        }
    }
    sqlite3_finalize(stmt);

    out << "Study Sessions Summary:\n";
    out << "Total sessions: " << total << "\n";
    out << "Correct responses: " << correct << "\n";
    if (total > 0) {
        double pct = (100.0 * static_cast<double>(correct)) / static_cast<double>(total);
        out << "Percent correct: " << std::round(pct * 100.0) / 100.0 << "%\n";
    }
    if (hasAvgResp) out << "Average response time (s): " << std::round(avgResp * 100.0) / 100.0 << "\n";
    if (hasAvgConf) out << "Average confidence (1-5): " << std::round(avgConf * 100.0) / 100.0 << "\n";

    // breakdown by study_mode
    const char* modeSql = "SELECT COALESCE(study_mode, 'unknown'), COUNT(*) FROM study_sessions GROUP BY study_mode;";
    rc = sqlite3_prepare_v2(db, modeSql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
        out << "Sessions by mode:\n";
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* mode = sqlite3_column_text(stmt, 0);
            long long cnt = sqlite3_column_int64(stmt, 1);
            out << "  " << (mode ? reinterpret_cast<const char*>(mode) : "unknown") << ": " << cnt << "\n";
        }
    }
    sqlite3_finalize(stmt);

    return out.str();
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
        QString error = "Failed to create progress_list table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_list_progress_list_id ON progress_list(list_id); "
        "CREATE INDEX IF NOT EXISTS idx_list_progress_last_studied ON progress_list(last_studied);";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        QString error = "Failed to create index on progress_list: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
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
        QString error = "Failed to create word_relations table: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

    const char* indexSql =
        "CREATE INDEX IF NOT EXISTS idx_word_relations_word1 ON word_relations(word1_id); "
        "CREATE INDEX IF NOT EXISTS idx_word_relations_word2 ON word_relations(word2_id); "
        "CREATE INDEX IF NOT EXISTS idx_word_relations_type ON word_relations(relation_type); ";

    result = sqlite3_exec(db,indexSql, nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        QString error = "Failed to create index on word_relations: " + QString::fromUtf8(errorMessage ? errorMessage : "");
        qCritical() << error;
        sqlite3_free(errorMessage);
        throw std::runtime_error(error.toStdString());
    }

    return true;
}

bool DataBase::createNewList(std::string listName, std::string targetLanguage = "", std::string description = "") {
    const char* sql = "INSERT INTO vocabulary_lists (list_name, description, language, date_created) VALUES (?, ?, ?, datetime('now'));";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for createNewList: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_text(stmt, 1, listName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, targetLanguage.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for createNewList: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_finalize(stmt);

    return true;
}

bool DataBase::deleteList(int listID) {
    // Begin transaction
    if (!beginTransaction()) {
        QString errorMsg = "Failed to begin transaction for deleteList";
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    try {
        // Delete from review_schedule (no CASCADE)
        const char* delReviewSql = "DELETE FROM review_schedule WHERE list_id = ?;";
        sqlite3_stmt* stmt;
        int result = sqlite3_prepare_v2(db, delReviewSql, -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            rollbackTransaction();
            QString errorMsg = "Failed to prepare review_schedule delete: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }
        sqlite3_bind_int(stmt, 1, listID);
        result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result != SQLITE_DONE) {
            rollbackTransaction();
            QString errorMsg = "Failed to delete from review_schedule: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }

        // Delete from progress_list (no CASCADE)
        const char* delProgressSql = "DELETE FROM progress_list WHERE list_id = ?;";
        result = sqlite3_prepare_v2(db, delProgressSql, -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            rollbackTransaction();
            QString errorMsg = "Failed to prepare progress_list delete: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }
        sqlite3_bind_int(stmt, 1, listID);
        result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result != SQLITE_DONE) {
            rollbackTransaction();
            QString errorMsg = "Failed to delete from progress_list: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }

        // Delete from study_sessions (no CASCADE on list_id)
        const char* delSessionsSql = "DELETE FROM study_sessions WHERE list_id = ?;";
        result = sqlite3_prepare_v2(db, delSessionsSql, -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            rollbackTransaction();
            QString errorMsg = "Failed to prepare study_sessions delete: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }
        sqlite3_bind_int(stmt, 1, listID);
        result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result != SQLITE_DONE) {
            rollbackTransaction();
            QString errorMsg = "Failed to delete from study_sessions: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }

        // Finally, delete the list itself (list_words will CASCADE automatically)
        const char* delListSql = "DELETE FROM vocabulary_lists WHERE list_id = ?;";
        result = sqlite3_prepare_v2(db, delListSql, -1, &stmt, nullptr);
        if (result != SQLITE_OK) {
            rollbackTransaction();
            QString errorMsg = "Failed to prepare vocabulary_lists delete: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }
        sqlite3_bind_int(stmt, 1, listID);
        result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result != SQLITE_DONE) {
            rollbackTransaction();
            QString errorMsg = "Failed to delete vocabulary list: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }

        // Commit transaction
        if (!commitTransaction()) {
            rollbackTransaction();
            QString errorMsg = "Failed to commit transaction for deleteList";
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }

        return true;
    } catch (...) {
        rollbackTransaction();
        throw;
    }
}

std::vector<std::string> DataBase::getVocabLists() {
    std::vector<std::string> allVocabLists;
    const char* sql = "SELECT DISTINCT list_name FROM vocabulary_lists ORDER BY list_name";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getVocabLists: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for getVocabListsWithNextReview: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
            QString errorMsg = "Failed to prepare statement for review_schedule query: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for createNewExample: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_int(stmt, 1, wordID);
    sqlite3_bind_text(stmt, 2, exampleText.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, contextNotes.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for createNewExample: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
        }

    sqlite3_finalize(stmt);

    return true;
}

bool DataBase::createNewRelation(int word1ID, int word2ID, std::string relationType) {
    const char* sql = "INSERT INTO word_relations (word1_id, word2_id, relation_type) VALUES (?, ?, ?);";

    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for createNewRelation: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_int(stmt, 1, word1ID);
    sqlite3_bind_int(stmt, 2, word2ID);
    sqlite3_bind_text(stmt, 3, relationType.c_str(), -1, SQLITE_TRANSIENT);

    result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for createNewRelation: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_finalize(stmt);
    
    return true;
}

std::vector<DataBase::WordExample> DataBase::getWordExamples(int wordID) {
    const char* sql = "SELECT example_id, example_text, context_notes FROM word_examples WHERE word_id = ?;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getWordExamples: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    sqlite3_bind_int(stmt, 1, wordID);
    
    std::vector<WordExample> examples;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        WordExample ex;
        ex.example_id = sqlite3_column_int(stmt, 0);
        
        const unsigned char* example_text = sqlite3_column_text(stmt, 1);
        ex.example_text = example_text ? std::string(reinterpret_cast<const char*>(example_text)) : "";
        
        const unsigned char* context_notes = sqlite3_column_text(stmt, 2);
        ex.context_notes = context_notes ? std::string(reinterpret_cast<const char*>(context_notes)) : "";
        
        examples.push_back(ex);
    }
    
    sqlite3_finalize(stmt);
    return examples;
}

std::vector<DataBase::WordRelation> DataBase::getWordRelations(int wordID) {
    const char* sql = 
        "SELECT w.word_id, w.word, wr.relation_type "
        "FROM word_relations wr "
        "JOIN words w ON wr.word2_id = w.word_id "
        "WHERE wr.word1_id = ?;";
    
    sqlite3_stmt* stmt;
    int result = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getWordRelations: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    sqlite3_bind_int(stmt, 1, wordID);
    
    std::vector<WordRelation> relations;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        WordRelation rel;
        rel.related_word_id = sqlite3_column_int(stmt, 0);
        
        const unsigned char* word = sqlite3_column_text(stmt, 1);
        rel.related_word = word ? std::string(reinterpret_cast<const char*>(word)) : "";
        
        const unsigned char* relation_type = sqlite3_column_text(stmt, 2);
        rel.relation_type = relation_type ? std::string(reinterpret_cast<const char*>(relation_type)) : "";
        
        relations.push_back(rel);
    }
    
    sqlite3_finalize(stmt);
    return relations;
}

bool DataBase::beginTransaction() {
    char* err = nullptr;
    int rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to begin transaction: " + QString::fromUtf8(err ? err : "");
        qCritical() << errorMsg;
        if (err) sqlite3_free(err);
        throw std::runtime_error(errorMsg.toStdString());
    }
    return true;
}

bool DataBase::commitTransaction() {
    char* err = nullptr;
    int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to commit transaction: " + QString::fromUtf8(err ? err : "");
        qCritical() << errorMsg;
        if (err) sqlite3_free(err);
        throw std::runtime_error(errorMsg.toStdString());
    }
    return true;
}

bool DataBase::rollbackTransaction() {
    char* err = nullptr;
    int rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to rollback transaction: " + QString::fromUtf8(err ? err : "");
        qCritical() << errorMsg;
        if (err) sqlite3_free(err);
        throw std::runtime_error(errorMsg.toStdString());
    }
    return true;
}

int DataBase::getWordId(const std::string& word, const std::string& language) {
    std::string sql;
    if (language.empty()) {
        // If no language specified, find any word matching the text
        sql = "SELECT word_id FROM words WHERE word = ? LIMIT 1;";
    } else {
        sql = "SELECT word_id FROM words WHERE word = ? AND language = ? LIMIT 1;";
    }
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getWordId: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_TRANSIENT);
    if (!language.empty()) {
        sqlite3_bind_text(stmt, 2, language.c_str(), -1, SQLITE_TRANSIENT);
    }

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
        QString errorMsg = "Failed to prepare statement for getListId: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for addOrGetWord: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_text(stmt, 1, word.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, partOfSpeech.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, definition.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, language.c_str(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for addOrGetWord: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for addWordToList: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_int(stmt, 1, listID);
    sqlite3_bind_int(stmt, 2, wordID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for addWordToList: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for initReviewSchedule: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_int(stmt, 1, wordID);
    sqlite3_bind_int(stmt, 2, listID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for initReviewSchedule: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_finalize(stmt);
    return true;
}

bool DataBase::incrementListProgress(int listID, int delta) {
    const char* updateSql = "UPDATE progress_list SET total_words = total_words + ? WHERE list_id = ?;";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, updateSql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for incrementListProgress UPDATE: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_int(stmt, 1, delta);
    sqlite3_bind_int(stmt, 2, listID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for incrementListProgress UPDATE: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_finalize(stmt);

    int changes = sqlite3_changes(db);
    if (changes == 0) {
        // No existing row; insert a new progress row
        const char* insertSql = "INSERT INTO progress_list (list_id, total_words, words_mastered) VALUES (?, ?, 0);";
        sqlite3_stmt* istmt = nullptr;
        rc = sqlite3_prepare_v2(db, insertSql, -1, &istmt, nullptr);
        if (rc != SQLITE_OK) {
            QString errorMsg = "Failed to prepare statement for incrementListProgress INSERT: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }

        sqlite3_bind_int(istmt, 1, listID);
        sqlite3_bind_int(istmt, 2, delta);

        rc = sqlite3_step(istmt);
        if (rc != SQLITE_DONE) {
            sqlite3_finalize(istmt);
            QString errorMsg = "Execution failed for incrementListProgress INSERT: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
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
            QString errorMsg = "Failed to obtain or create word id in addWordAndSetup";
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for getDueCards: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Failed to prepare statement for updateReviewScheduleForWord: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_bind_int(stmt, 1, repetition_count);
    sqlite3_bind_int(stmt, 2, interval_days);
    // Ensure we don't violate the CHECK constraint on ease_factor (1.3 - 2.5)
    double ef = ease_factor;
    if (ef < 1.3) ef = 1.3;
    if (ef > 2.5) ef = 2.5;
    sqlite3_bind_double(stmt, 3, ef);
    sqlite3_bind_text(stmt, 4, next_review_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, wordID);
    sqlite3_bind_int(stmt, 6, listID);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        QString errorMsg = "Execution failed for updateReviewScheduleForWord: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }

    sqlite3_finalize(stmt);
    return true;
}

bool DataBase::recordStudySession(int wordID, int listID, bool was_correct, int quality, const std::string& study_mode) {
    const char* sql = "INSERT INTO study_sessions (word_id, review_date, was_correct, response_time, confidence_score, study_mode, list_id, notes, device_info) VALUES (?, datetime('now'), ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for recordStudySession: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
        QString errorMsg = "Execution failed for recordStudySession: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
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
            if (rc != SQLITE_OK) {
                QString errorMsg = "Failed to prepare statement for getRandomWordsInList (with exclude): " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
                qCritical() << errorMsg;
                throw std::runtime_error(errorMsg.toStdString());
            }
            sqlite3_bind_int(stmt, 1, listID);
            sqlite3_bind_int(stmt, 2, excludeWordID);
            sqlite3_bind_int(stmt, 3, count);
        } else {
            rc = sqlite3_prepare_v2(db, sql_no_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                QString errorMsg = "Failed to prepare statement for getRandomWordsInList (no exclude): " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
                qCritical() << errorMsg;
                throw std::runtime_error(errorMsg.toStdString());
            }
            sqlite3_bind_int(stmt, 1, listID);
            sqlite3_bind_int(stmt, 2, count);
        }
    } else {
        // global across all words
        if (excludeWordID >= 0) {
            rc = sqlite3_prepare_v2(db, sql_global_with_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                QString errorMsg = "Failed to prepare statement for getRandomWordsInList (global with exclude): " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
                qCritical() << errorMsg;
                throw std::runtime_error(errorMsg.toStdString());
            }
            sqlite3_bind_int(stmt, 1, excludeWordID);
            sqlite3_bind_int(stmt, 2, count);
        } else {
            rc = sqlite3_prepare_v2(db, sql_global_no_exclude, -1, &stmt, nullptr);
            if (rc != SQLITE_OK) {
                QString errorMsg = "Failed to prepare statement for getRandomWordsInList (global no exclude): " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
                qCritical() << errorMsg;
                throw std::runtime_error(errorMsg.toStdString());
            }
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

std::vector<std::tuple<int, std::string, std::string>> DataBase::getWordsInList(int listID) {
    std::vector<std::tuple<int, std::string, std::string>> out;
    const char* sql_in_list =
        "SELECT w.word_id, w.word, w.definition FROM list_words lw JOIN words w ON lw.word_id = w.word_id WHERE lw.list_id = ? ORDER BY lw.added_date ASC;";
    const char* sql_all =
        "SELECT w.word_id, w.word, w.definition FROM words w ORDER BY w.word ASC;";

    sqlite3_stmt* stmt = nullptr;
    int rc;
    if (listID >= 0) {
        rc = sqlite3_prepare_v2(db, sql_in_list, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            QString errorMsg = "Failed to prepare statement for getWordsInList (in list): " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }
        sqlite3_bind_int(stmt, 1, listID);
    } else {
        rc = sqlite3_prepare_v2(db, sql_all, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            QString errorMsg = "Failed to prepare statement for getWordsInList (all): " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
            qCritical() << errorMsg;
            throw std::runtime_error(errorMsg.toStdString());
        }
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int wid = sqlite3_column_int(stmt, 0);
        const unsigned char* wtxt = sqlite3_column_text(stmt, 1);
        const unsigned char* dtxt = sqlite3_column_text(stmt, 2);
        std::string word = wtxt ? reinterpret_cast<const char*>(wtxt) : std::string("");
        std::string def = dtxt ? reinterpret_cast<const char*>(dtxt) : std::string("");
        out.emplace_back(wid, word, def);
    }

    sqlite3_finalize(stmt);
    return out;
}

// Get count of new cards (never reviewed) for a list
int DataBase::getNewCardCount(int listID) {
    const char* sql = 
        "SELECT COUNT(*) FROM review_schedule "
        "WHERE list_id = ? AND repetition_count = 0;";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getNewCardCount: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    sqlite3_bind_int(stmt, 1, listID);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

// Get count of continuing cards (already started but due for review)
int DataBase::getContinuingCardCount(int listID) {
    const char* sql = 
        "SELECT COUNT(*) FROM review_schedule "
        "WHERE list_id = ? AND repetition_count > 0 AND next_review_date <= datetime('now');";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getContinuingCardCount: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    sqlite3_bind_int(stmt, 1, listID);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}

// Get count of all cards due for review (new + continuing)
int DataBase::getReviewCardCount(int listID) {
    const char* sql = 
        "SELECT COUNT(*) FROM review_schedule "
        "WHERE list_id = ? AND next_review_date <= datetime('now');";
    
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        QString errorMsg = "Failed to prepare statement for getReviewCardCount: " + QString::fromStdString(std::string(sqlite3_errmsg(db)));
        qCritical() << errorMsg;
        throw std::runtime_error(errorMsg.toStdString());
    }
    
    sqlite3_bind_int(stmt, 1, listID);
    
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    
    sqlite3_finalize(stmt);
    return count;
}
