#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "database.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QString>
#include <QPushButton>
#include <vector>
#include "spacedrepetitioncalculator.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    DataBase* getDB();

private slots:
    void on_addWord_clicked();

    void on_creatDeck_clicked();
    void on_aiCreate_clicked();

    void updatingList();
    void startStudy();
    void deckListDoubleClicked(QListWidgetItem* item);
    void on_startStudyButton_clicked();
    void on_listDecks_clicked();
    void on_showStats_clicked();
    void on_viewAllButton_clicked();
    void on_deleteListButton_clicked();
    void on_actionToggleDarkMode_triggered(bool checked);
    
    // Study panel slots
    void onReveal();
    void onRateAgain();
    void onRateHard();
    void onRateGood();
    void onRateEasy();
    void onChoiceSelected();

private:
    void loadDueCards();
    void loadRandomPracticeCards();
    void showCurrentCard();
    void applyRating(int quality);
    void showDeckList();
    void showModePanel();
    void showStudyPanel();
    void applyLightTheme();
    void applyDarkTheme();
    
    Ui::MainWindow *ui;

    DataBase db;
    // pending selection when opening mode panel
    int pendingListID = -1;
    QString pendingListName;
    
    // Study session state
    int currentStudyListID = -1;
    std::vector<DataBase::DueCard> studyCards;
    size_t currentCardIndex = 0;
    enum class StudyMode { Flashcard, MultipleChoice } studyMode;
    QPushButton* choiceButtons[4];
    int correctChoiceIndex = -1;
    bool isRandomPractice = false;
    std::vector<int> recentlySeenWordIds; // Track recently seen words to avoid repetition
    bool isDarkMode = false;
};
#endif // MAINWINDOW_H
