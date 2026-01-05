#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "database.h"
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QString>
#include <QPushButton>
#include <vector>
#include "spacedrepetitioncalculator.h"
#include "decklistpanel.h"
#include "modeselectorpanel.h"
#include "studypanel.h"

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
    void on_createDeck_clicked();
    void on_aiCreate_clicked();
    void on_listDecks_clicked();
    void on_showStats_clicked();
    void on_actionToggleDarkMode_triggered(bool checked);
    
    // Panel slots
    void onDeckDoubleClicked(const QString& deckName, int listID);
    void onStartStudy(int listID, int mode);
    void onViewAll(int listID);
    void onDeleteList(int listID);
    void onStudyCompleted();

private:
    void showDeckList();
    void showModePanel();
    void showStudyPanel();
    void applyLightTheme();
    void applyDarkTheme();
    
    Ui::MainWindow *ui;

    DataBase db;
    
    // Panel widgets
    DeckListPanel* deckListPanel;
    ModeSelectorPanel* modeSelectorPanel;
    StudyPanel* studyPanel;
    
    bool isDarkMode = false;
};
#endif // MAINWINDOW_H
