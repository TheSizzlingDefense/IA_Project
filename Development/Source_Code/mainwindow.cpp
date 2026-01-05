#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"
#include "addlistwindow.h"
#include "aicreatewindow.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <sstream>
#include <random>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db("/home/brometheus/IA_Project/data/example.db")
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
    
    // Fetch all words for the list and display them
    auto entries = db.getWordsInList(listID);
    std::ostringstream out;
    out << "Words in list: " << listName.toStdString() << "\n\n";
    for (const auto &t : entries) {
        int wid;
        std::string word, def;
        std::tie(wid, word, def) = t;
        out << "- " << word;
        if (!def.empty()) out << ": " << def;
        out << "\n";
    }

    QDialog dlg(this);
    dlg.setWindowTitle("All Words");
    
    // Apply current theme to the dialog
    if (isDarkMode) {
        dlg.setStyleSheet(R"(
            QDialog {
                background-color: #1e1e1e;
            }
            QWidget {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            QPushButton {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #3e3e42;
                border-color: #007acc;
            }
            QPushButton:pressed {
                background-color: #007acc;
            }
            QTextEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
        )");
    } else {
        dlg.setStyleSheet(R"(
            QDialog {
                background-color: #ffffff;
            }
            QWidget {
                background-color: #ffffff;
                color: #2c3e50;
            }
            QPushButton {
                background-color: #ecf0f1;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #d5dbdb;
                border-color: #95a5a6;
            }
            QPushButton:pressed {
                background-color: #bdc3c7;
            }
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
        )");
    }
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QTextEdit* view = new QTextEdit(&dlg);
    view->setReadOnly(true);
    view->setPlainText(QString::fromStdString(out.str()));
    layout->addWidget(view);
    QPushButton* closeBtn = new QPushButton("Close", &dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn);
    dlg.resize(520, 400);
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
    // Use the database helper to build a summary string and show it in a dialog
    std::string summary = db.getStudySessionSummary();

    QDialog dlg(this);
    dlg.setWindowTitle("Study Sessions Summary");
    
    // Apply current theme to the dialog
    if (isDarkMode) {
        dlg.setStyleSheet(R"(
            QDialog {
                background-color: #1e1e1e;
            }
            QWidget {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            QPushButton {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #3e3e42;
                border-color: #007acc;
            }
            QPushButton:pressed {
                background-color: #007acc;
            }
            QTextEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
        )");
    } else {
        dlg.setStyleSheet(R"(
            QDialog {
                background-color: #ffffff;
            }
            QWidget {
                background-color: #ffffff;
                color: #2c3e50;
            }
            QPushButton {
                background-color: #ecf0f1;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 8px 16px;
            }
            QPushButton:hover {
                background-color: #d5dbdb;
                border-color: #95a5a6;
            }
            QPushButton:pressed {
                background-color: #bdc3c7;
            }
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
        )");
    }
    
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QTextEdit* view = new QTextEdit(&dlg);
    view->setReadOnly(true);
    view->setPlainText(QString::fromStdString(summary));
    layout->addWidget(view);
    QPushButton* closeBtn = new QPushButton("Close", &dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg.resize(480, 320);
    dlg.exec();
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
    // Light theme - clean and minimal
    QString lightStyle = R"(
        QMainWindow {
            background-color: #ffffff;
        }
        
        QWidget {
            background-color: #ffffff;
            color: #2c3e50;
        }
        
        QPushButton {
            background-color: #ecf0f1;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            padding: 8px 16px;
        }
        
        QPushButton:hover {
            background-color: #d5dbdb;
            border-color: #95a5a6;
        }
        
        QPushButton:pressed {
            background-color: #bdc3c7;
        }
        
        QPushButton:disabled {
            background-color: #ecf0f1;
            color: #95a5a6;
        }
        
        QListWidget {
            background-color: #ffffff;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            selection-background-color: #3498db;
            selection-color: #ffffff;
        }
        
        QListWidget::item {
            padding: 8px;
        }
        
        QListWidget::item:selected {
            background-color: #3498db;
            color: #ffffff;
        }
        
        QListWidget::item:hover {
            background-color: #ecf0f1;
        }
        
        QComboBox {
            background-color: #ffffff;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            padding: 6px;
        }
        
        QComboBox:hover {
            border-color: #95a5a6;
        }
        
        QComboBox::drop-down {
            border: none;
        }
        
        QComboBox QAbstractItemView {
            background-color: #ffffff;
            color: #2c3e50;
            selection-background-color: #3498db;
            selection-color: #ffffff;
        }
        
        QLabel {
            color: #2c3e50;
        }
        
        #studyDefinitionLabel {
            background-color: #f0f0f0;
            border: 1px solid #bdc3c7;
        }
        
        QTextEdit {
            background-color: #ffffff;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            padding: 6px;
        }
        
        QLineEdit {
            background-color: #ffffff;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            padding: 6px;
        }
        
        QTableWidget {
            background-color: #ffffff;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            border-radius: 4px;
            gridline-color: #bdc3c7;
            selection-background-color: #3498db;
            selection-color: #ffffff;
        }
        
        QTableWidget::item {
            padding: 8px;
        }
        
        QTableWidget::item:selected {
            background-color: #3498db;
            color: #ffffff;
        }
        
        QTableWidget::item:hover {
            background-color: #ecf0f1;
        }
        
        QHeaderView::section {
            background-color: #ecf0f1;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
            padding: 6px;
            font-weight: bold;
        }
        
        QGroupBox {
            color: #2c3e50;
            border: 2px solid #bdc3c7;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 8px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            background-color: #ffffff;
        }
        
        QMenuBar {
            background-color: #ecf0f1;
            color: #2c3e50;
        }
        
        QMenuBar::item:selected {
            background-color: #d5dbdb;
        }
        
        QMenu {
            background-color: #ffffff;
            color: #2c3e50;
            border: 1px solid #bdc3c7;
        }
        
        QMenu::item:selected {
            background-color: #3498db;
            color: #ffffff;
        }
    )";
    
    this->setStyleSheet(lightStyle);
}

void MainWindow::applyDarkTheme() {
    // Dark theme - modern and easy on the eyes
    QString darkStyle = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        
        QWidget {
            background-color: #1e1e1e;
            color: #e0e0e0;
        }
        
        QPushButton {
            background-color: #2d2d30;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            border-radius: 4px;
            padding: 8px 16px;
        }
        
        QPushButton:hover {
            background-color: #3e3e42;
            border-color: #007acc;
        }
        
        QPushButton:pressed {
            background-color: #007acc;
        }
        
        QPushButton:disabled {
            background-color: #2d2d30;
            color: #656565;
        }
        
        QListWidget {
            background-color: #252526;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            border-radius: 4px;
            selection-background-color: #007acc;
            selection-color: #ffffff;
        }
        
        QListWidget::item {
            padding: 8px;
        }
        
        QListWidget::item:selected {
            background-color: #007acc;
            color: #ffffff;
        }
        
        QListWidget::item:hover {
            background-color: #2d2d30;
        }
        
        QComboBox {
            background-color: #2d2d30;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            padding: 5px;
            min-height: 20px;
            margin: 0px;
            outline: 0;
        }
        
        QComboBox:hover {
            border-color: #007acc;
        }
        
        QComboBox::drop-down {
            border: none;
            margin: 0px;
        }
        
        QComboBox QAbstractItemView {
            background-color: #252526;
            color: #e0e0e0;
            selection-background-color: #007acc;
            selection-color: #ffffff;
        }
        
        QLabel {
            color: #e0e0e0;
        }
        
        #studyDefinitionLabel {
            background-color: #2d2d30;
            border: 1px solid #3e3e42;
        }
        
        QTextEdit {
            background-color: #252526;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            border-radius: 4px;
            padding: 6px;
        }
        
        QLineEdit {
            background-color: #252526;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            border-radius: 4px;
            padding: 6px;
        }
        
        QTableWidget {
            background-color: #252526;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            border-radius: 4px;
            gridline-color: #3e3e42;
            selection-background-color: #007acc;
            selection-color: #ffffff;
        }
        
        QTableWidget::item {
            padding: 8px;
        }
        
        QTableWidget::item:selected {
            background-color: #007acc;
            color: #ffffff;
        }
        
        QTableWidget::item:hover {
            background-color: #2d2d30;
        }
        
        QHeaderView::section {
            background-color: #2d2d30;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
            padding: 6px;
            font-weight: bold;
        }
        
        QGroupBox {
            color: #e0e0e0;
            border: 2px solid #3e3e42;
            border-radius: 6px;
            margin-top: 12px;
            padding-top: 8px;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 8px;
            background-color: #1e1e1e;
        }
        
        QMenuBar {
            background-color: #2d2d30;
            color: #e0e0e0;
        }
        
        QMenuBar::item:selected {
            background-color: #3e3e42;
        }
        
        QMenu {
            background-color: #252526;
            color: #e0e0e0;
            border: 1px solid #3e3e42;
        }
        
        QMenu::item:selected {
            background-color: #007acc;
            color: #ffffff;
        }
    )";
    
    this->setStyleSheet(darkStyle);
}
