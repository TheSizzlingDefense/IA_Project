#ifndef STUDYWINDOW_H
#define STUDYWINDOW_H

#include <QDialog>
#include <QLabel>
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

private slots:
    void onReveal();
    void onRateAgain();
    void onRateHard();
    void onRateGood();
    void onRateEasy();

private:
    void loadDueCards();
    void showCurrentCard();
    void applyRating(int quality);

    Ui::StudyWindow *ui;

    DataBase* db;
    int listID;
    std::vector<DataBase::DueCard> cards;
    size_t currentIndex;
};

#endif // STUDYWINDOW_H
