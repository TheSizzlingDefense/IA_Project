#include "studypanel.h"
#include "ui_studypanel.h"
#include "spacedrepetitioncalculator.h"
#include <QMessageBox>
#include <QTimer>
#include <QStyle>
#include <random>
#include <algorithm>
#include <ctime>

StudyPanel::StudyPanel(DataBase* database, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::StudyPanel)
    , db(database)
    , correctChoiceIndex(-1)
    , currentCardIndex(0)
    , studyMode(StudyMode::Flashcard)
    , isRandomPractice(false)
    , typingAttempts(0)
    , showingExample(false)
    , currentStudyListID(-1)
{
    ui->setupUi(this);
    
    // Set up choice buttons array
    choiceButtons[0] = ui->choiceButton1;
    choiceButtons[1] = ui->choiceButton2;
    choiceButtons[2] = ui->choiceButton3;
    choiceButtons[3] = ui->choiceButton4;
    
    // Connect signals
    connect(ui->revealButton, &QPushButton::clicked, this, &StudyPanel::onReveal);
    connect(ui->againButton, &QPushButton::clicked, this, &StudyPanel::onRateAgain);
    connect(ui->hardButton, &QPushButton::clicked, this, &StudyPanel::onRateHard);
    connect(ui->goodButton, &QPushButton::clicked, this, &StudyPanel::onRateGood);
    connect(ui->easyButton, &QPushButton::clicked, this, &StudyPanel::onRateEasy);
    connect(ui->submitTypingButton, &QPushButton::clicked, this, &StudyPanel::onSubmitTypedAnswer);
    connect(ui->typingInput, &QLineEdit::returnPressed, this, &StudyPanel::onSubmitTypedAnswer);
    
    // Connect choice buttons
    for (int i = 0; i < 4; ++i) {
        connect(choiceButtons[i], &QPushButton::clicked, this, &StudyPanel::onChoiceSelected);
    }
}

StudyPanel::~StudyPanel()
{
    delete ui;
}

void StudyPanel::setStudyCards(const std::vector<DataBase::DueCard>& cards, StudyMode mode)
{
    studyCards = cards;
    studyMode = mode;
    currentCardIndex = 0;
    recentlySeenWordIds.clear();
    
    if (!cards.empty()) {
        currentStudyListID = cards[0].list_id;
    }
}

void StudyPanel::showCurrentCard()
{
    if (currentCardIndex >= studyCards.size()) {
        QMessageBox::information(this, "Study Complete", 
            "You've completed all cards in this session!");
        emit studyCompleted();
        return;
    }

    const auto &c = studyCards[currentCardIndex];
    ui->studyWordLabel->setText(QString::fromStdString(c.word));
    ui->studyDefinitionLabel->setText(QString::fromStdString(c.definition));
    
    // Update additional information
    updateAdditionalInfo(c.word_id);
    
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
        ui->typingPromptLabel->setVisible(false);
        ui->typingInput->setVisible(false);
        ui->submitTypingButton->setVisible(false);
        ui->typingFeedbackLabel->setVisible(false);
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
        ui->typingPromptLabel->setVisible(false);
        ui->typingInput->setVisible(false);
        ui->submitTypingButton->setVisible(false);
        ui->typingFeedbackLabel->setVisible(false);
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
            auto distractors = db->getRandomWordsInList(currentStudyListID, c.word_id, 3);
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
            choiceButtons[i]->setStyleSheet("");
            if (options[i] == correctText) correctChoiceIndex = i;
        }
        // Force style update to ensure colors are reset
        for (int i = 0; i < 4; ++i) {
            choiceButtons[i]->style()->unpolish(choiceButtons[i]);
            choiceButtons[i]->style()->polish(choiceButtons[i]);
        }
    } else if (studyMode == StudyMode::Typing) {
        // Typing mode
        ui->studyDefinitionLabel->setVisible(false);
        ui->revealButton->setVisible(false);
        ui->typingPromptLabel->setVisible(true);
        ui->typingInput->setVisible(true);
        ui->submitTypingButton->setVisible(true);
        ui->typingFeedbackLabel->setVisible(true);
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

void StudyPanel::updateAdditionalInfo(int word_id)
{
    try {
        // Get examples and notes
        auto examples = db->getWordExamples(word_id);
        QString examplesTextStr;
        for (const auto& ex : examples) {
            if (!ex.example_text.empty()) {
                examplesTextStr += QString::fromStdString(ex.example_text);
                if (!ex.context_notes.empty()) {
                    examplesTextStr += "\n[Note: " + QString::fromStdString(ex.context_notes) + "]";
                }
                examplesTextStr += "\n\n";
            }
        }
        ui->examplesText->setText(examplesTextStr.trimmed().isEmpty() ? "No examples available" : examplesTextStr.trimmed());
        
        // Get word relations
        auto relations = db->getWordRelations(word_id);
        QString relationsTextStr;
        for (const auto& rel : relations) {
            relationsTextStr += QString::fromStdString(rel.relation_type) + ": " + 
                             QString::fromStdString(rel.related_word) + "\n";
        }
        ui->relationsText->setText(relationsTextStr.trimmed().isEmpty() ? "No word relations available" : relationsTextStr.trimmed());
    } catch (const std::exception& ex) {
        // If there's an error fetching additional info, just clear the fields
        ui->examplesText->setText("No examples available");
        ui->relationsText->setText("No word relations available");
    }
}

void StudyPanel::applyRating(int quality)
{
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
            db->updateReviewScheduleForWord(c.word_id, c.list_id, calc.getRepetitions(), calc.getInterval(), calc.getEasinessFactor(), nextReviewStr);
            std::string modeStr = (studyMode == StudyMode::Flashcard) ? "flashcard" : "multiple_choice";
            db->recordStudySession(c.word_id, c.list_id, was_correct, quality, modeStr);
        } catch (const std::exception &ex) {
            QMessageBox::critical(this, "DB Error", QString::fromStdString(ex.what()));
        }
    } else {
        // In random practice mode, just record the session without updating schedule
        bool was_correct = quality >= 3;
        try {
            std::string modeStr = (studyMode == StudyMode::Flashcard) ? "flashcard" : "multiple_choice";
            db->recordStudySession(c.word_id, c.list_id, was_correct, quality, modeStr);
        } catch (const std::exception &ex) {
            QMessageBox::critical(this, "DB Error", QString::fromStdString(ex.what()));
        }
    }

    // move to next
    currentCardIndex++;
    showCurrentCard();
}

void StudyPanel::onReveal()
{
    ui->studyDefinitionLabel->setVisible(true);
    ui->additionalInfoBox->setVisible(true);
    ui->revealButton->setEnabled(false);
    // Enable rating buttons after revealing the card
    ui->againButton->setEnabled(true);
    ui->hardButton->setEnabled(true);
    ui->goodButton->setEnabled(true);
    ui->easyButton->setEnabled(true);
}

void StudyPanel::onRateAgain() { applyRating(0); }
void StudyPanel::onRateHard() { applyRating(3); }
void StudyPanel::onRateGood() { applyRating(4); }
void StudyPanel::onRateEasy() { applyRating(5); }

void StudyPanel::onChoiceSelected()
{
    QObject* s = sender();
    int chosen = -1;
    for (int i = 0; i < 4; ++i) {
        if (s == choiceButtons[i]) { chosen = i; break; }
    }
    if (chosen < 0) return;

    // disable buttons to avoid double clicks
    for (int i = 0; i < 4; ++i) choiceButtons[i]->setEnabled(false);

    if (chosen == correctChoiceIndex) {
        // Highlight correct answer in green
        choiceButtons[chosen]->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
        
        // Move to next card after a brief delay
        QTimer::singleShot(1000, this, [this]() {
            applyRating(5);
        });
    } else {
        // Highlight wrong answer in red and correct answer in green
        choiceButtons[chosen]->setStyleSheet("background-color: #e74c3c; color: white; font-weight: bold;");
        choiceButtons[correctChoiceIndex]->setStyleSheet("background-color: #2ecc71; color: white; font-weight: bold;");
        
        // Move to next card after showing feedback for a moment
        QTimer::singleShot(1500, this, [this]() {
            applyRating(0);
        });
    }
}

void StudyPanel::onSubmitTypedAnswer()
{
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
        
        // Rate as good (quality 4) and move to next card after a brief delay
        QTimer::singleShot(1000, this, [this]() {
            applyRating(4);
        });
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
            
            // Rate as again (quality 0) after a brief delay to show the correct answer
            QTimer::singleShot(1500, this, [this]() {
                applyRating(0);
            });
        }
    }
}
