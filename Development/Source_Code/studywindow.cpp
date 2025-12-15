#include "studywindow.h"
#include "ui_studywindow.h"
#include <QMessageBox>
#include <ctime>

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
    ui->definitionLabel->setVisible(false);
    ui->revealButton->setEnabled(true);
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
        db->recordStudySession(c.word_id, c.list_id, was_correct, quality);
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
