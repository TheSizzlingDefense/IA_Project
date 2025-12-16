#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include <QDialog>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <vector>
#include "database.h"
#include "spacedrepetitioncalculator.h"

namespace Ui {
class StudyWindow;
}

class StudyWindow : public QDialog {
    Q_OBJECT
public:
    explicit StudyWindow(DataBase* db, int listID = -1, QWidget* parent = nullptr);
    // allow external code to set the study mode before showing window
    void setStudyModeFromString(const QString& modeStr);

private slots:
    void onReveal();
    void onRateAgain();
    void onRateHard();
    void onRateGood();
    void onRateEasy();
    void onModeChanged(int index);
    void onChoiceSelected();

private:
    void loadDueCards();
    void showCurrentCard();
    void applyRating(int quality);

    Ui::StudyWindow *ui;

    DataBase* db;
    int listID;
    std::vector<DataBase::DueCard> cards;
    size_t currentIndex;
    enum class StudyMode { Flashcard, MultipleChoice } mode;
    // choice buttons (set up from UI)
    QPushButton* choiceButtons[4];
    // helper: index of correct choice when in multiple choice
    int correctChoiceIndex;
};

#endif // STUDYWINDOW_H
