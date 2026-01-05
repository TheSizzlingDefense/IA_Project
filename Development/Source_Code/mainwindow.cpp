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

    showTextDialog("All Words", QString::fromStdString(out.str()), 520, 400);
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
