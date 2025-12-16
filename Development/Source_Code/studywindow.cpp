#include "studywindow.h"
#include "ui_studywindow.h"
#include <QMessageBox>
#include <ctime>
#include <algorithm>
#include <random>

StudyWindow::StudyWindow(DataBase* database, int listID_, QWidget* parent)
    : QDialog(parent), ui(new Ui::StudyWindow), db(database), listID(listID_), currentIndex(0)
{
    ui->setupUi(this);
    ui->wordLabel->setStyleSheet("font-size: 20pt; font-weight: bold;");

    connect(ui->revealButton, &QPushButton::clicked, this, &StudyWindow::onReveal);
    connect(ui->againButton, &QPushButton::clicked, this, &StudyWindow::onRateAgain);
    connect(ui->hardButton, &QPushButton::clicked, this, &StudyWindow::onRateHard);
    connect(ui->goodButton, &QPushButton::clicked, this, &StudyWindow::onRateGood);
    connect(ui->easyButton, &QPushButton::clicked, this, &StudyWindow::onRateEasy);
    // mode combo
    connect(ui->modeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &StudyWindow::onModeChanged);

    // wire choice buttons
    choiceButtons[0] = ui->choiceButton1;
    choiceButtons[1] = ui->choiceButton2;
    choiceButtons[2] = ui->choiceButton3;
    choiceButtons[3] = ui->choiceButton4;
    for (int i = 0; i < 4; ++i) {
        connect(choiceButtons[i], &QPushButton::clicked, this, &StudyWindow::onChoiceSelected);
    }

    // default mode
    mode = StudyMode::Flashcard;

    loadDueCards();
    showCurrentCard();
}

void StudyWindow::loadDueCards() {
    cards = db->getDueCards(listID);
    currentIndex = 0;
}

void StudyWindow::showCurrentCard() {
    if (currentIndex >= cards.size()) {
        QMessageBox::information(this, "Study", "No more due cards.");
        close();
        return;
    }

    const auto &c = cards[currentIndex];
    ui->wordLabel->setText(QString::fromStdString(c.word));
    ui->definitionLabel->setText(QString::fromStdString(c.definition));

    if (mode == StudyMode::Flashcard) {
        ui->definitionLabel->setVisible(false);
        ui->revealButton->setVisible(true);
        ui->revealButton->setEnabled(true);
        // hide choice buttons
        for (int i = 0; i < 4; ++i) choiceButtons[i]->setVisible(false);
    } else {
        // Multiple choice mode
        ui->definitionLabel->setVisible(false);
        ui->revealButton->setVisible(false);

        // prepare choices: correct + 3 distractors from DB
        std::vector<std::string> options;
        std::string correctText = c.definition.empty() ? c.word : c.definition;
        options.push_back(correctText);

        try {
            auto distractors = db->getRandomWordsInList(listID, c.word_id, 3);
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

        // find index of correct
        correctChoiceIndex = -1;
        for (int i = 0; i < 4; ++i) {
            choiceButtons[i]->setText(QString::fromStdString(options[i]));
            choiceButtons[i]->setVisible(true);
            choiceButtons[i]->setEnabled(true);
            if (options[i] == correctText) correctChoiceIndex = i;
        }
        if (correctChoiceIndex < 0) {
            // ensure one correct answer present (fallback)
            choiceButtons[0]->setText(QString::fromStdString(correctText));
            correctChoiceIndex = 0;
        }
    }
}

void StudyWindow::onReveal() {
    ui->definitionLabel->setVisible(true);
    ui->revealButton->setEnabled(false);
}

void StudyWindow::applyRating(int quality) {
    if (currentIndex >= cards.size()) return;
    const auto &c = cards[currentIndex];

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
        std::string modeStr = (mode == StudyMode::Flashcard) ? "flashcard" : "multiple_choice";
        db->recordStudySession(c.word_id, c.list_id, was_correct, quality, modeStr);
    } catch (const std::exception &ex) {
        QMessageBox::critical(this, "DB Error", QString::fromStdString(ex.what()));
    }

    // move to next
    currentIndex++;
    showCurrentCard();
}

void StudyWindow::onRateAgain() { applyRating(0); }
void StudyWindow::onRateHard() { applyRating(3); }
void StudyWindow::onRateGood() { applyRating(4); }
void StudyWindow::onRateEasy() { applyRating(5); }

void StudyWindow::onModeChanged(int index) {
    mode = (index == 1) ? StudyMode::MultipleChoice : StudyMode::Flashcard;
    // refresh current card UI
    showCurrentCard();
}

void StudyWindow::onChoiceSelected() {
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

void StudyWindow::setStudyModeFromString(const QString& modeStr) {
    if (modeStr == "Multiple Choice") mode = StudyMode::MultipleChoice;
    else mode = StudyMode::Flashcard;
    // refresh display for current card
    showCurrentCard();
}
