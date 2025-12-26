#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"
#include "addlistwindow.h"
#include "aicreatewindow.h"
#include <algorithm>
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <sstream>
#include <random>
#include <ctime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db("/home/brometheus/IA_Project/data/example.db")
{
    ui->setupUi(this);
    
    // Configure table widget
    ui->deckList->setColumnCount(4);
    ui->deckList->setHorizontalHeaderLabels({"Deck Name", "New", "Due", "Review"});
    ui->deckList->horizontalHeader()->setStretchLastSection(false);
    ui->deckList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->deckList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->deckList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->deckList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->deckList->verticalHeader()->setVisible(false);
    ui->deckList->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->deckList->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->deckList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // Remove borders and make the table look more fluid
    ui->deckList->setShowGrid(false);
    ui->deckList->setStyleSheet(
        "QTableWidget { "
        "   border: none; "
        "   background-color: transparent; "
        "} "
        "QTableWidget::item { "
        "   border: none; "
        "   padding: 8px; "
        "} "
        "QTableWidget::item:selected { "
        "   background-color: palette(highlight); "
        "   color: palette(highlighted-text); "
        "} "
        "QHeaderView::section { "
        "   background-color: transparent; "
        "   border: none; "
        "   border-bottom: 1px solid palette(mid); "
        "   padding: 8px; "
        "   font-weight: bold; "
        "}"
    );
    
    // Initialize study mode
    studyMode = StudyMode::Flashcard;
    
    // Set up choice buttons array
    choiceButtons[0] = ui->choiceButton1;
    choiceButtons[1] = ui->choiceButton2;
    choiceButtons[2] = ui->choiceButton3;
    choiceButtons[3] = ui->choiceButton4;
    
    // Connect study panel signals
    connect(ui->revealButton, &QPushButton::clicked, this, &MainWindow::onReveal);
    connect(ui->againButton, &QPushButton::clicked, this, &MainWindow::onRateAgain);
    connect(ui->hardButton, &QPushButton::clicked, this, &MainWindow::onRateHard);
    connect(ui->goodButton, &QPushButton::clicked, this, &MainWindow::onRateGood);
    connect(ui->easyButton, &QPushButton::clicked, this, &MainWindow::onRateEasy);
    
    // Connect choice buttons
    for (int i = 0; i < 4; ++i) {
        connect(choiceButtons[i], &QPushButton::clicked, this, &MainWindow::onChoiceSelected);
    }
    
    // Connect typing mode submit button
    connect(ui->submitTypingButton, &QPushButton::clicked, this, &MainWindow::onSubmitTypedAnswer);
    connect(ui->typingInput, &QLineEdit::returnPressed, this, &MainWindow::onSubmitTypedAnswer);
    
    ui->deckList->setRowCount(0);
    // Get lists with their next review date and sort by earliest review first
    auto lists = db.getVocabListsWithNextReview();

    // sort: empty dates (no reviews) go to the end; otherwise sort by datetime string ascending
    std::sort(lists.begin(), lists.end(), [](const auto &a, const auto &b) {
        const std::string &da = a.second;
        const std::string &dbs = b.second;
        if (da.empty() && dbs.empty()) return a.first < b.first; // both empty -> alphabetical
        if (da.empty()) return false; // a goes after b
        if (dbs.empty()) return true;  // a goes before b
        return da < dbs; // ISO-like datetime textual compare works for 'YYYY-MM-DD HH:MM:SS'
    });

    for (const auto &p : lists) {
        QString deckName = QString::fromStdString(p.first);
        
        // Get card counts for this list
        int listID = db.getListId(p.first);
        int newCount = db.getNewCardCount(listID);
        int continuingCount = db.getContinuingCardCount(listID);
        int reviewCount = db.getReviewCardCount(listID);
        
        // Add row to table
        int row = ui->deckList->rowCount();
        ui->deckList->insertRow(row);
        ui->deckList->setItem(row, 0, new QTableWidgetItem(deckName));
        ui->deckList->setItem(row, 1, new QTableWidgetItem(QString::number(newCount)));
        ui->deckList->setItem(row, 2, new QTableWidgetItem(QString::number(continuingCount)));
        ui->deckList->setItem(row, 3, new QTableWidgetItem(QString::number(reviewCount)));
        
        // Center align the numeric columns
        ui->deckList->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        ui->deckList->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        ui->deckList->item(row, 3)->setTextAlignment(Qt::AlignCenter);
    }

    // connect double-click to show mode selection panel
    connect(ui->deckList, &QTableWidget::itemDoubleClicked, this, &MainWindow::deckListDoubleClicked);
    // Mode panel buttons use Qt auto-connect (on_<objectName>_<signalName> pattern)
    // Don't manually connect startStudyButton - it uses auto-connect via on_startStudyButton_clicked
    // Don't manually connect the top 'Decks' button — use Qt auto-connect by naming the slot `on_listDecks_clicked`
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

void MainWindow::on_creatDeck_clicked() {
    AddListWindow addListWindow{nullptr, &db};
    QObject::connect(&addListWindow, &AddListWindow::newAddedList, this, &MainWindow::updatingList);
    addListWindow.applyTheme(isDarkMode);
    addListWindow.setModal(true);
    addListWindow.exec();
}

DataBase* MainWindow::getDB() {
    return &db;
}

void MainWindow::updatingList() {
    ui->deckList->setRowCount(0);
    auto lists = db.getVocabListsWithNextReview();
    std::sort(lists.begin(), lists.end(), [](const auto &a, const auto &b) {
        const std::string &da = a.second;
        const std::string &dbs = b.second;
        if (da.empty() && dbs.empty()) return a.first < b.first;
        if (da.empty()) return false;
        if (dbs.empty()) return true;
        return da < dbs;
    });

    for (const auto &p : lists) {
        QString deckName = QString::fromStdString(p.first);
        
        // Get card counts for this list
        int listID = db.getListId(p.first);
        int newCount = db.getNewCardCount(listID);
        int continuingCount = db.getContinuingCardCount(listID);
        int reviewCount = db.getReviewCardCount(listID);
        
        // Add row to table
        int row = ui->deckList->rowCount();
        ui->deckList->insertRow(row);
        ui->deckList->setItem(row, 0, new QTableWidgetItem(deckName));
        ui->deckList->setItem(row, 1, new QTableWidgetItem(QString::number(newCount)));
        ui->deckList->setItem(row, 2, new QTableWidgetItem(QString::number(continuingCount)));
        ui->deckList->setItem(row, 3, new QTableWidgetItem(QString::number(reviewCount)));
        
        // Center align the numeric columns
        ui->deckList->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        ui->deckList->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        ui->deckList->item(row, 3)->setTextAlignment(Qt::AlignCenter);
    }
}

void MainWindow::startStudy() {
    int currentRow = ui->deckList->currentRow();
    if (currentRow < 0) return;
    // Get deck name from first column
    QTableWidgetItem* item = ui->deckList->item(currentRow, 0);
    if (!item) return;
    QString listName = item->text();
    int listID = db.getListId(listName.toStdString());
    
    currentStudyListID = listID;
    studyMode = StudyMode::Flashcard;
    
    loadDueCards();
    // Only show study panel if we have cards to study (loadDueCards might have redirected to deck list)
    if (!studyCards.empty()) {
        showStudyPanel();
    }
}

void MainWindow::deckListDoubleClicked(QTableWidgetItem* item) {
    if (!item) return;
    // Get the deck name from the first column of the clicked row
    int row = item->row();
    QTableWidgetItem* nameItem = ui->deckList->item(row, 0);
    if (!nameItem) return;
    pendingListName = nameItem->text();
    pendingListID = db.getListId(pendingListName.toStdString());

    // update mode panel label and show the panel
    ui->selectedDeckLabel->setText(pendingListName);
    showModePanel();
}

void MainWindow::on_startStudyButton_clicked() {
    if (pendingListID < 0) return;
    
    currentStudyListID = pendingListID;
    
    // Set mode based on combo box
    QString mode = ui->studyModeComboBox->currentText();
    if (mode == "Multiple Choice") {
        studyMode = StudyMode::MultipleChoice;
    } else if (mode == "Typing") {
        studyMode = StudyMode::Typing;
    } else {
        studyMode = StudyMode::Flashcard;
    }
    
    loadDueCards();
    // Only show study panel if we have cards to study (loadDueCards might have redirected to deck list)
    if (!studyCards.empty()) {
        showStudyPanel();
    }
}

void MainWindow::on_listDecks_clicked() {
    // hide all panels and show deck list (invoked by top 'Decks' button)
    showDeckList();
    ui->deckList->setVisible(true);
    pendingListID = -1;
    pendingListName.clear();
}

void MainWindow::on_aiCreate_clicked() {
    AICreateWindow dlg(this, &db);
    dlg.applyTheme(isDarkMode);
    dlg.setModal(true);
    dlg.exec();
    // after creation, refresh list view in case a new list was created
    updatingList();
}

void MainWindow::on_showStats_clicked() {
    // Use the database helper to build a summary string and show it in a dialog
    std::string summary = db.getStudySessionSummary();

    QDialog dlg(this);
    dlg.setWindowTitle("Study Sessions Summary");
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

void MainWindow::on_viewAllButton_clicked() {
    if (pendingListID < 0) {
        QMessageBox::information(this, "No Deck Selected", "Please select a deck first (double-click a deck).");
        return;
    }

    // Fetch all words for the pending list and display them
    auto entries = db.getWordsInList(pendingListID);
    std::ostringstream out;
    out << "Words in list: " << pendingListName.toStdString() << "\n\n";
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

void MainWindow::on_deleteListButton_clicked() {
    if (pendingListID < 0) {
        QMessageBox::information(this, "No Deck Selected", "Please select a deck first (double-click a deck).");
        return;
    }

    // Ask for confirmation before deleting
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Delete List", 
                                   "Are you sure you want to delete \"" + pendingListName + "\"?\n\n"
                                   "This will permanently delete the list and all associated data.",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        try {
            db.deleteList(pendingListID);
            QMessageBox::information(this, "Success", "List \"" + pendingListName + "\" has been deleted.");
            
            // Reset pending state and go back to deck list view
            pendingListID = -1;
            pendingListName.clear();
            ui->modePanel->setVisible(false);
            ui->deckList->setVisible(true);
            
            // Refresh the list
            updatingList();
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error", "Failed to delete list: " + QString::fromStdString(e.what()));
        }
    }
}

// Study panel methods
void MainWindow::loadDueCards() {
    studyCards = db.getDueCards(currentStudyListID);
    currentCardIndex = 0;
    isRandomPractice = false;
    recentlySeenWordIds.clear();
    
    // If no due cards, offer random practice mode
    if (studyCards.empty()) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "No Due Cards", 
                                       "No cards are due for review right now.\n\n"
                                       "Would you like to practice random words from this deck?",
                                       QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            loadRandomPracticeCards();
        } else {
            showDeckList();
        }
    }
}

void MainWindow::loadRandomPracticeCards() {
    isRandomPractice = true;
    recentlySeenWordIds.clear();
    
    // Get all words from the list
    auto allWords = db.getWordsInList(currentStudyListID);
    
    if (allWords.empty()) {
        QMessageBox::information(this, "No Words", "This deck has no words to practice.");
        showDeckList();
        return;
    }
    
    // Create DueCard objects from all words (shuffle them)
    std::vector<DataBase::DueCard> allCards;
    for (const auto &wordTuple : allWords) {
        DataBase::DueCard card;
        card.word_id = std::get<0>(wordTuple);
        card.word = std::get<1>(wordTuple);
        card.definition = std::get<2>(wordTuple);
        card.list_id = currentStudyListID;
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
    studyCards.clear();
    size_t practiceSize = std::min(size_t(20), allCards.size());
    for (size_t i = 0; i < practiceSize; ++i) {
        studyCards.push_back(allCards[i]);
    }
    
    currentCardIndex = 0;
}

void MainWindow::showCurrentCard() {
    if (currentCardIndex >= studyCards.size()) {
        if (isRandomPractice) {
            // In random practice mode, keep cycling with new random cards
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Practice Session Complete", 
                                           "You've completed this practice session.\n\n"
                                           "Would you like to continue practicing?",
                                           QMessageBox::Yes | QMessageBox::No);
            
            if (reply == QMessageBox::Yes) {
                loadRandomPracticeCards();
                showCurrentCard();
                return;
            }
        } else {
            QMessageBox::information(this, "Study Complete", "No more cards in this session.");
        }
        showDeckList();
        return;
    }

    const auto &c = studyCards[currentCardIndex];
    ui->studyWordLabel->setText(QString::fromStdString(c.word));
    ui->studyDefinitionLabel->setText(QString::fromStdString(c.definition));
    
    // Fetch and display additional information (examples, notes, relations)
    try {
        // Get examples and notes
        auto examples = db.getWordExamples(c.word_id);
        QString examplesText;
        for (const auto& ex : examples) {
            if (!ex.example_text.empty()) {
                examplesText += QString::fromStdString(ex.example_text);
                if (!ex.context_notes.empty()) {
                    examplesText += "\n[Note: " + QString::fromStdString(ex.context_notes) + "]";
                }
                examplesText += "\n\n";
            }
        }
        ui->examplesText->setText(examplesText.trimmed());
        
        // Get word relations
        auto relations = db.getWordRelations(c.word_id);
        QString relationsText;
        for (const auto& rel : relations) {
            relationsText += QString::fromStdString(rel.relation_type) + ": " + 
                             QString::fromStdString(rel.related_word) + "\n";
        }
        ui->relationsText->setText(relationsText.trimmed());
    } catch (const std::exception& ex) {
        // If there's an error fetching additional info, just clear the fields
        ui->examplesText->setText("No examples available");
        ui->relationsText->setText("No word relations available");
    }
    
    // Add to recently seen list (keep last 5 words to avoid immediate repetition)
    recentlySeenWordIds.push_back(c.word_id);
    if (recentlySeenWordIds.size() > 5) {
        recentlySeenWordIds.erase(recentlySeenWordIds.begin());
    }

    if (studyMode == StudyMode::Flashcard) {
        ui->studyDefinitionLabel->setVisible(false);
        ui->additionalInfoBox->setVisible(false);
        ui->revealButton->setVisible(true);
        ui->revealButton->setEnabled(true);
        ui->typingWidget->setVisible(false);
        // hide choice buttons
        for (int i = 0; i < 4; ++i) choiceButtons[i]->setVisible(false);
        // show rating buttons in flashcard mode but disable them until card is revealed
        ui->againButton->setVisible(true);
        ui->againButton->setEnabled(false);
        ui->hardButton->setVisible(true);
        ui->hardButton->setEnabled(false);
        ui->goodButton->setVisible(true);
        ui->goodButton->setEnabled(false);
        ui->easyButton->setVisible(true);
        ui->easyButton->setEnabled(false);
    } else if (studyMode == StudyMode::MultipleChoice) {
        // Multiple choice mode
        ui->studyDefinitionLabel->setVisible(false);
        ui->additionalInfoBox->setVisible(true);
        ui->revealButton->setVisible(false);
        ui->typingWidget->setVisible(false);
        // hide rating buttons in multiple choice mode
        ui->againButton->setVisible(false);
        ui->hardButton->setVisible(false);
        ui->goodButton->setVisible(false);
        ui->easyButton->setVisible(false);

        // prepare choices: correct + 3 distractors from DB
        std::vector<std::string> options;
        std::string correctText = c.definition.empty() ? c.word : c.definition;
        options.push_back(correctText);

        try {
            auto distractors = db.getRandomWordsInList(currentStudyListID, c.word_id, 3);
            for (auto &p : distractors) {
                std::string d = p.second.empty() ? std::string("") : p.second;
                if (d.empty()) d = std::to_string(p.first);
                options.push_back(d);
            }
        } catch (...) {
            // ignore DB errors, continue with whatever options we have
        }

        // if not enough distractors, pad with empty strings
        while (options.size() < 4) options.push_back(std::string(""));

        // shuffle options and assign to buttons
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(options.begin(), options.end(), g);
        
        correctChoiceIndex = -1;
        for (int i = 0; i < 4; ++i) {
            choiceButtons[i]->setText(QString::fromStdString(options[i]));
            choiceButtons[i]->setVisible(true);
            choiceButtons[i]->setEnabled(true);
            if (options[i] == correctText) correctChoiceIndex = i;
        }
    } else if (studyMode == StudyMode::Typing) {
        // Typing mode
        ui->studyDefinitionLabel->setVisible(false);
        ui->revealButton->setVisible(false);
        ui->typingWidget->setVisible(true);
        ui->typingInput->clear();
        ui->typingInput->setEnabled(true);
        ui->submitTypingButton->setEnabled(true);
        ui->typingFeedbackLabel->clear();
        typingAttempts = 0;
        showingExample = false;
        
        // Initially hide additional info (will show example on first wrong answer)
        ui->additionalInfoBox->setVisible(false);
        
        // hide choice buttons
        for (int i = 0; i < 4; ++i) choiceButtons[i]->setVisible(false);
        // hide rating buttons
        ui->againButton->setVisible(false);
        ui->hardButton->setVisible(false);
        ui->goodButton->setVisible(false);
        ui->easyButton->setVisible(false);
        
        // Focus on the input field
        ui->typingInput->setFocus();
    }
}

void MainWindow::applyRating(int quality) {
    if (currentCardIndex >= studyCards.size()) return;
    const auto &c = studyCards[currentCardIndex];

    // In random practice mode, don't update review schedule
    if (!isRandomPractice) {
        // prepare calculator with current schedule
        SpacedRepetitionCalculator calc;
        calc.setEasinessFactor(c.ease_factor);
        calc.setRepetitions(c.repetition_count);
        calc.setInterval(c.interval_days);

        calc.calculateNextReview(quality);

        // build next_review datetime string in format YYYY-MM-DD HH:MM:SS
        time_t nextT = calc.getNextReview();
        struct tm tm;
        gmtime_r(&nextT, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        std::string nextReviewStr(buf);

        // update DB
        bool was_correct = quality >= 3;
        try {
            db.updateReviewScheduleForWord(c.word_id, c.list_id, calc.getRepetitions(), calc.getInterval(), calc.getEasinessFactor(), nextReviewStr);
            std::string modeStr = (studyMode == StudyMode::Flashcard) ? "flashcard" : "multiple_choice";
            db.recordStudySession(c.word_id, c.list_id, was_correct, quality, modeStr);
        } catch (const std::exception &ex) {
            QMessageBox::critical(this, "DB Error", QString::fromStdString(ex.what()));
        }
    } else {
        // In random practice mode, just record the session without updating schedule
        bool was_correct = quality >= 3;
        try {
            std::string modeStr = (studyMode == StudyMode::Flashcard) ? "flashcard" : "multiple_choice";
            db.recordStudySession(c.word_id, c.list_id, was_correct, quality, modeStr);
        } catch (const std::exception &ex) {
            QMessageBox::critical(this, "DB Error", QString::fromStdString(ex.what()));
        }
    }

    // move to next
    currentCardIndex++;
    showCurrentCard();
}

void MainWindow::onReveal() {
    ui->studyDefinitionLabel->setVisible(true);
    ui->additionalInfoBox->setVisible(true);
    ui->revealButton->setEnabled(false);
    // Enable rating buttons after revealing the card
    ui->againButton->setEnabled(true);
    ui->hardButton->setEnabled(true);
    ui->goodButton->setEnabled(true);
    ui->easyButton->setEnabled(true);
}

void MainWindow::onRateAgain() { applyRating(0); }
void MainWindow::onRateHard() { applyRating(3); }
void MainWindow::onRateGood() { applyRating(4); }
void MainWindow::onRateEasy() { applyRating(5); }

void MainWindow::onChoiceSelected() {
    QObject* s = sender();
    int chosen = -1;
    for (int i = 0; i < 4; ++i) {
        if (s == choiceButtons[i]) { chosen = i; break; }
    }
    if (chosen < 0) return;

    // disable buttons to avoid double clicks
    for (int i = 0; i < 4; ++i) choiceButtons[i]->setEnabled(false);

    if (chosen == correctChoiceIndex) {
        QMessageBox::information(this, "Correct", "Correct!");
        // treat as easy
        applyRating(5);
    } else {
        QMessageBox::information(this, "Incorrect", "Incorrect. The correct answer will be used to update scheduling.");
        // treat as again
        applyRating(0);
    }
}

void MainWindow::onSubmitTypedAnswer() {
    if (currentCardIndex >= studyCards.size()) return;
    
    const auto &c = studyCards[currentCardIndex];
    QString typedAnswer = ui->typingInput->text().trimmed();
    QString correctAnswer = QString::fromStdString(c.definition).trimmed();
    
    // Simple comparison - case insensitive
    bool isCorrect = typedAnswer.toLower() == correctAnswer.toLower();
    
    if (isCorrect) {
        // Correct answer
        ui->typingFeedbackLabel->setText("✓ Correct!");
        ui->typingFeedbackLabel->setStyleSheet("color: green;");
        ui->typingInput->setEnabled(false);
        ui->submitTypingButton->setEnabled(false);
        
        // Rate as good (quality 4)
        applyRating(4);
    } else {
        typingAttempts++;
        
        if (typingAttempts == 1) {
            // First wrong attempt - show example if available
            ui->typingFeedbackLabel->setText("✗ Incorrect. Here's an example to help:");
            ui->typingFeedbackLabel->setStyleSheet("color: orange;");
            
            // Show additional info with examples
            ui->additionalInfoBox->setVisible(true);
            showingExample = true;
            
            // Clear the input for second attempt
            ui->typingInput->clear();
            ui->typingInput->setFocus();
        } else {
            // Second wrong attempt - mark as wrong
            ui->typingFeedbackLabel->setText("✗ Incorrect. The correct answer was: " + correctAnswer);
            ui->typingFeedbackLabel->setStyleSheet("color: red;");
            ui->typingInput->setEnabled(false);
            ui->submitTypingButton->setEnabled(false);
            
            // Show the full definition
            ui->studyDefinitionLabel->setText(correctAnswer);
            ui->studyDefinitionLabel->setVisible(true);
            
            // Rate as again (quality 0)
            applyRating(0);
        }
    }
}

void MainWindow::showDeckList() {
    ui->deckList->setVisible(true);
    ui->modePanel->setVisible(false);
    ui->studyPanel->setVisible(false);
    pendingListID = -1;
    pendingListName.clear();
    updatingList();
}

void MainWindow::showModePanel() {
    ui->deckList->setVisible(false);
    ui->modePanel->setVisible(true);
    ui->studyPanel->setVisible(false);
}

void MainWindow::showStudyPanel() {
    ui->deckList->setVisible(false);
    ui->modePanel->setVisible(false);
    ui->studyPanel->setVisible(true);
    // Don't call showCurrentCard here - it's called by loadDueCards or loadRandomPracticeCards
    // Calling it here causes the card to be displayed before the panel is fully set up
    if (currentCardIndex < studyCards.size()) {
        showCurrentCard();
    }
}

void MainWindow::on_actionToggleDarkMode_triggered(bool checked) {
    isDarkMode = checked;
    if (isDarkMode) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
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
            border-radius: 4px;
            padding: 6px;
        }
        
        QComboBox:hover {
            border-color: #007acc;
        }
        
        QComboBox::drop-down {
            border: none;
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
