#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "database.h"

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

    void updatingList();

private:
    Ui::MainWindow *ui;

    DataBase db;
};
#endif // MAINWINDOW_H
