#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"
#include "addlistwindow.h"
#include "aicreatewindow.h"
#include "themeutils.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStandardPaths>
#include <QDir>
#include <sstream>
#include <random>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db(getApplicationDataPath("vocabulary.db").toStdString())
{
    ui->setupUi(this);
    
    // Create panel widgets
    deckListPanel = new DeckListPanel(&db, this);
    modeSelectorPanel = new ModeSelectorPanel(this);
    studyPanel = new StudyPanel(&db, this);
    
    // Add panels to the main layout
    ui->deckListContainer->layout()->addWidget(deckListPanel);
    ui->modeSelectorContainer->layout()->addWidget(modeSelectorPanel);
    ui->studyPanelContainer->layout()->addWidget(studyPanel);
    
    // Connect panel signals
    connect(deckListPanel, &DeckListPanel::deckDoubleClicked, this, &MainWindow::onDeckDoubleClicked);
    connect(modeSelectorPanel, &ModeSelectorPanel::startStudyClicked, this, &MainWindow::onStartStudy);
    connect(modeSelectorPanel, &ModeSelectorPanel::viewAllClicked, this, &MainWindow::onViewAll);
    connect(modeSelectorPanel, &ModeSelectorPanel::deleteListClicked, this, &MainWindow::onDeleteList);
    connect(studyPanel, &StudyPanel::studyCompleted, this, &MainWindow::onStudyCompleted);
    
    // Initial visibility - show only deck list on startup
    showDeckList();
    
    // Apply initial theme (light mode by default)
    applyLightTheme();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addWord_clicked() {
    AddCardWindow addCardWindow{nullptr, &db};
    addCardWindow.applyTheme(isDarkMode);
    addCardWindow.setModal(true);
    addCardWindow.exec();
}

void MainWindow::on_createDeck_clicked() {
    AddListWindow addListWindow{nullptr, &db};
    QObject::connect(&addListWindow, &AddListWindow::newAddedList, this, [this]() {
        deckListPanel->updateDeckList();
    });
    addListWindow.applyTheme(isDarkMode);
    addListWindow.setModal(true);
    addListWindow.exec();
}

DataBase* MainWindow::getDB() {
    return &db;
}

void MainWindow::onDeckDoubleClicked(const QString& deckName, int listID) {
    modeSelectorPanel->setDeckInfo(deckName, listID);
    showModePanel();
}

void MainWindow::onStartStudy(int listID, int mode) {
    // Load cards for study
    auto cards = db.getDueCards(listID);
    
    // If no due cards, offer random practice mode
    if (cards.empty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "No Due Cards", 
                                       "No cards are due for review right now.\n\n"
                                       "Would you like to practice random words from this deck?",
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            // Load random practice cards
            auto allWords = db.getWordsInList(listID);
            
            if (allWords.empty()) {
                QMessageBox::information(this, "No Words", "This deck has no words to practice.");
                return;
            }
            
            // Create DueCard objects from all words (shuffle them)
            std::vector<DataBase::DueCard> allCards;
            for (const auto &wordTuple : allWords) {
                DataBase::DueCard card;
                card.word_id = std::get<0>(wordTuple);
                card.word = std::get<1>(wordTuple);
                card.definition = std::get<2>(wordTuple);
                card.list_id = listID;
                card.ease_factor = 2.5;
                card.interval_days = 0;
                card.repetition_count = 0;
                card.next_review_date = "";
                card.schedule_id = -1;
                allCards.push_back(card);
            }
            
            // Shuffle the cards
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(allCards.begin(), allCards.end(), g);
            
            // Take up to 20 cards for practice session
            size_t practiceSize = std::min(size_t(20), allCards.size());
            cards.clear();
            for (size_t i = 0; i < practiceSize; ++i) {
                cards.push_back(allCards[i]);
            }
            
            studyPanel->setRandomPracticeMode(true);
        } else {
            return;
        }
    } else {
        studyPanel->setRandomPracticeMode(false);
        
        // Shuffle the due cards
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(cards.begin(), cards.end(), g);
    }
    
    // Set study mode
    StudyPanel::StudyMode studyMode;
    if (mode == 1) {
        studyMode = StudyPanel::StudyMode::MultipleChoice;
    } else if (mode == 2) {
        studyMode = StudyPanel::StudyMode::Typing;
    } else {
        studyMode = StudyPanel::StudyMode::Flashcard;
    }
    
    studyPanel->setStudyCards(cards, studyMode);
    showStudyPanel();
    studyPanel->showCurrentCard();
}

void MainWindow::onViewAll(int listID) {
    // Get list name from mode selector panel
    QString listName = modeSelectorPanel->getCurrentDeckName();
    
    // Fetch all words for the list and display them in a table
    auto entries = db.getWordsInList(listID);
    
    // Create a dialog with a table widget
    QDialog dlg(this);
    dlg.setWindowTitle("All Words - " + listName);
    dlg.setStyleSheet(isDarkMode ? ThemeUtils::getDarkTheme() : ThemeUtils::getLightTheme());
    dlg.resize(900, 500);
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    
    // Create table widget
    QTableWidget* table = new QTableWidget(&dlg);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"#", "Word", "Definition"});
    table->setRowCount(entries.size());
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    
    // Populate table
    int rowNum = 0;
    for (const auto &t : entries) {
        int wid;
        std::string word, def;
        std::tie(wid, word, def) = t;
        
        // Word number (1-indexed)
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(rowNum + 1));
        numItem->setFlags(numItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(rowNum, 0, numItem);
        
        // Word
        QTableWidgetItem* wordItem = new QTableWidgetItem(QString::fromStdString(word));
        wordItem->setFlags(wordItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(rowNum, 1, wordItem);
        
        // Definition
        QTableWidgetItem* defItem = new QTableWidgetItem(QString::fromStdString(def));
        defItem->setFlags(defItem->flags() & ~Qt::ItemIsEditable);
        table->setItem(rowNum, 2, defItem);
        
        rowNum++;
    }
    
    // Resize columns to content
    table->resizeColumnsToContents();
    table->setColumnWidth(0, 50);
    
    layout->addWidget(table);
    
    // Add close button
    QPushButton* closeBtn = new QPushButton("Close", &dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    dlg.exec();
}

void MainWindow::onDeleteList(int listID) {
    QString listName = modeSelectorPanel->getCurrentDeckName();
    
    // Ask for confirmation before deleting
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete List", 
                                   "Are you sure you want to delete \"" + listName + "\"?\n\n"
                                   "This will permanently delete the list and all associated data.",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        try {
            db.deleteList(listID);
            QMessageBox::information(this, "Success", "List \"" + listName + "\" has been deleted.");
            
            // Go back to deck list view and refresh
            showDeckList();
            deckListPanel->updateDeckList();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", "Failed to delete list: " + QString::fromStdString(e.what()));
        }
    }
}

void MainWindow::onStudyCompleted() {
    showDeckList();
}

void MainWindow::on_listDecks_clicked() {
    showDeckList();
}

void MainWindow::on_aiCreate_clicked() {
    AICreateWindow dlg(this, &db);
    dlg.applyTheme(isDarkMode);
    dlg.setModal(true);
    dlg.exec();
    // after creation, refresh list view in case a new list was created
    deckListPanel->updateDeckList();
}

void MainWindow::on_showStats_clicked() {
    std::string summary = db.getStudySessionSummary();
    showTextDialog("Study Sessions Summary", QString::fromStdString(summary));
}

void MainWindow::showDeckList() {
    ui->deckListContainer->setVisible(true);
    ui->modeSelectorContainer->setVisible(false);
    ui->studyPanelContainer->setVisible(false);
    ui->createDeck->setVisible(true);
    deckListPanel->updateDeckList();
}

void MainWindow::showModePanel() {
    ui->deckListContainer->setVisible(false);
    ui->modeSelectorContainer->setVisible(true);
    ui->studyPanelContainer->setVisible(false);
    ui->createDeck->setVisible(true);
}

void MainWindow::showStudyPanel() {
    ui->deckListContainer->setVisible(false);
    ui->modeSelectorContainer->setVisible(false);
    ui->studyPanelContainer->setVisible(true);
    ui->createDeck->setVisible(false);
}

void MainWindow::on_actionToggleDarkMode_triggered(bool checked) {
    isDarkMode = checked;
    if (isDarkMode) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
    
    // Apply theme to panels
    deckListPanel->applyTheme(isDarkMode);
}

void MainWindow::applyLightTheme() {
    this->setStyleSheet(ThemeUtils::getLightTheme());
}

void MainWindow::applyDarkTheme() {
    this->setStyleSheet(ThemeUtils::getDarkTheme());
}

void MainWindow::showTextDialog(const QString& title, const QString& text, int width, int height) {
    QDialog dlg(this);
    dlg.setWindowTitle(title);
    dlg.setStyleSheet(isDarkMode ? ThemeUtils::getDarkTheme() : ThemeUtils::getLightTheme());
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QTextEdit* view = new QTextEdit(&dlg);
    view->setReadOnly(true);
    view->setPlainText(text);
    layout->addWidget(view);
    QPushButton* closeBtn = new QPushButton("Close", &dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    dlg.resize(width, height);
    dlg.exec();
}
void MainWindow::on_actionImport_triggered() {
    // First, ask user to select a list or create a new one
    auto listNames = db.getVocabLists();
    QStringList lists;
    for (const auto& name : listNames) {
        lists.append(QString::fromStdString(name));
    }
    lists.append("[Create New List]");
    
    bool ok;
    QString selectedList = QInputDialog::getItem(this, "Select List",
                                                  "Import into which list?", lists, 0, false, &ok);
    
    if (!ok) return;
    
    int listID = -1;
    if (selectedList == "[Create New List]") {
        // Prompt for new list details
        QString listName = QInputDialog::getText(this, "Create New List", "List name:", QLineEdit::Normal, "", &ok);
        if (!ok || listName.isEmpty()) return;
        
        QString language = QInputDialog::getText(this, "Create New List", "Target language:", QLineEdit::Normal, "", &ok);
        if (!ok) return;
        
        // Create the new list in database
        try {
            db.createNewList(listName.toStdString(), language.toStdString(), "Imported list");
            listID = db.getListId(listName.toStdString());
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", "Failed to create list: " + QString::fromStdString(e.what()));
            return;
        }
    } else {
        // Get the ID of the selected list
        listID = db.getListId(selectedList.toStdString());
    }
    
    if (listID == -1) {
        QMessageBox::critical(this, "Error", "Failed to determine list ID.");
        return;
    }
    
    // Open file dialog
    QString fileName = QFileDialog::getOpenFileName(this, "Import Words",
        QDir::homePath(),
        "CSV Files (*.csv);;SQLite Database (*.db);;All Files (*)");
    
    if (fileName.isEmpty()) return;
    
    try {
        if (fileName.endsWith(".csv", Qt::CaseInsensitive)) {
            importFromCSV(fileName, listID);
        } else if (fileName.endsWith(".db", Qt::CaseInsensitive)) {
            importFromDB(fileName, listID);
        } else {
            // Try CSV first, then DB
            QFileInfo info(fileName);
            if (info.suffix().isEmpty()) {
                QMessageBox::warning(this, "Warning", "Could not determine file type. Attempting to import as CSV.");
                importFromCSV(fileName, listID);
            } else {
                QMessageBox::critical(this, "Error", "Unsupported file format. Please use CSV or SQLite DB files.");
            }
        }
        
        // Refresh the deck list and show success message
        deckListPanel->updateDeckList();
        QMessageBox::information(this, "Success", "Words have been imported successfully.");
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Import Error", QString::fromStdString(e.what()));
    }
}

void MainWindow::importFromCSV(const QString& filePath, int listID) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Cannot open file: " + filePath.toStdString());
    }
    
    QTextStream in(&file);
    int importedCount = 0;
    int lineNumber = 0;
    
    // Read the entire file to handle multiline quoted fields
    QString fullContent = in.readAll();
    file.close();
    
    // Split into lines but we'll need to handle multiline quoted fields carefully
    QStringList allLines = fullContent.split('\n');
    
    if (allLines.isEmpty()) {
        throw std::runtime_error("CSV file is empty.");
    }
    
    // Detect delimiter from first line (header)
    QString headerLine = allLines[0];
    
    // Detect delimiter by checking which is most common: comma, tab, semicolon, pipe
    QChar delimiter = ',';
    if (!headerLine.isEmpty()) {
        int commaCount = headerLine.count(',');
        int tabCount = headerLine.count('\t');
        int semicolonCount = headerLine.count(';');
        int pipeCount = headerLine.count('|');
        
        int maxCount = commaCount;
        delimiter = ',';
        
        if (tabCount > maxCount) {
            maxCount = tabCount;
            delimiter = '\t';
        }
        if (semicolonCount > maxCount) {
            maxCount = semicolonCount;
            delimiter = ';';
        }
        if (pipeCount > maxCount) {
            maxCount = pipeCount;
            delimiter = '|';
        }
    }
    
    // Process data lines, handling multiline quoted fields
    QString currentRecord;
    int recordLineNumber = 1;  // Line number of the record (for error reporting)
    
    for (int i = 1; i < allLines.size(); ++i) {
        QString line = allLines[i];
        
        if (currentRecord.isEmpty()) {
            recordLineNumber = i + 1;  // +1 because line numbers are 1-based
        }
        
        currentRecord += line;
        
        // Check if we have a complete record (even number of quotes outside of escaped quotes)
        int quoteCount = 0;
        bool escaped = false;
        for (int j = 0; j < currentRecord.length(); ++j) {
            QChar c = currentRecord[j];
            if (c == '"' && !escaped) {
                quoteCount++;
            }
            escaped = (c == '"') && !escaped;  // Handle escaped quotes
        }
        
        // If quote count is even, we have a complete record
        if (quoteCount % 2 == 0 && !currentRecord.trimmed().isEmpty()) {
            try {
                processCSVLine(currentRecord, delimiter, listID, recordLineNumber);
                importedCount++;
            } catch (const std::exception& e) {
                qWarning() << "Error importing word at line " << recordLineNumber << ": " << QString::fromStdString(e.what());
                // Continue importing other words
            }
            currentRecord.clear();
        } else if (quoteCount % 2 == 1) {
            // Odd number of quotes - we're inside a multiline field, add newline back
            currentRecord += '\n';
        }
    }
    
    // Handle any remaining record
    if (!currentRecord.trimmed().isEmpty()) {
        try {
            processCSVLine(currentRecord, delimiter, listID, recordLineNumber);
            importedCount++;
        } catch (const std::exception& e) {
            qWarning() << "Error importing word at line " << recordLineNumber << ": " << QString::fromStdString(e.what());
        }
    }
    
    if (importedCount == 0) {
        throw std::runtime_error("No words were imported. Check CSV format.");
    }
}

void MainWindow::processCSVLine(const QString& line, QChar delimiter, int listID, int lineNumber) {
    // Parse CSV line with proper quoted field handling (including multiline fields)
    QStringList fields;
    QString currentField;
    bool inQuotes = false;
    
    for (int i = 0; i < line.length(); ++i) {
        QChar c = line[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == delimiter && !inQuotes) {
            fields.append(currentField.trimmed());
            currentField.clear();
        } else {
            // Preserve newlines and other characters within quoted fields
            currentField += c;
        }
    }
    fields.append(currentField.trimmed());
    
    if (fields.isEmpty() || fields[0].isEmpty()) return;
    
    // Remove quotes from fields and trim whitespace
    QString word = fields[0].trimmed();
    word.remove('"');
    word = word.trimmed();
    
    QString definition = "";
    if (fields.size() > 1) {
        definition = fields[1].trimmed();
        definition.remove('"');
        definition = definition.trimmed();
    }
    
    QString partOfSpeech = "";
    if (fields.size() > 2) {
        partOfSpeech = fields[2].trimmed();
        partOfSpeech.remove('"');
        partOfSpeech = partOfSpeech.trimmed();
    }
    
    if (word.isEmpty()) return;
    
    try {
        db.addWordAndSetup(listID, word.toStdString(), 
                          partOfSpeech.toStdString(), 
                          definition.toStdString(), "");
    } catch (const std::exception& e) {
        throw e;
    }
}

void MainWindow::importFromDB(const QString& filePath, int listID) {
    // Open the source database
    sqlite3* sourceDb = nullptr;
    int rc = sqlite3_open(filePath.toStdString().c_str(), &sourceDb);
    
    if (rc != SQLITE_OK) {
        throw std::runtime_error("Cannot open database file: " + filePath.toStdString());
    }
    
    try {
        // Query the words table from source database
        sqlite3_stmt* stmt = nullptr;
        const char* query = "SELECT word, definition, part_of_speech FROM words";
        
        rc = sqlite3_prepare_v2(sourceDb, query, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            throw std::runtime_error("Cannot query source database: " + std::string(sqlite3_errmsg(sourceDb)));
        }
        
        int importedCount = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* word = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* definition = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* partOfSpeech = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            
            if (word) {
                try {
                    db.addWordAndSetup(listID, word,
                                      partOfSpeech ? partOfSpeech : "",
                                      definition ? definition : "",
                                      "");
                    importedCount++;
                } catch (const std::exception& e) {
                    qWarning() << "Error importing word: " << e.what();
                    // Continue importing other words
                }
            }
        }
        
        sqlite3_finalize(stmt);
        
        if (importedCount == 0) {
            throw std::runtime_error("No words found in source database.");
        }
    } catch (...) {
        sqlite3_close(sourceDb);
        throw;
    }
}

QString MainWindow::getApplicationDataPath(const QString& filename) {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataPath);
    return appDataPath + "/" + filename;
}