#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "database.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QString>

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

private:
    Ui::MainWindow *ui;

    DataBase db;
    // pending selection when opening mode panel
    int pendingListID = -1;
    QString pendingListName;
};
#endif // MAINWINDOW_H
