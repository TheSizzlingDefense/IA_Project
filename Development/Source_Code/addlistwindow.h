#ifndef ADDLISTWINDOW_H
#define ADDLISTWINDOW_H

#include <QDialog>
#include "database.h"

namespace Ui {
class AddListWindow;
}

class AddListWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddListWindow(QWidget *parent = nullptr, DataBase* dataBase = nullptr);
    ~AddListWindow();

signals:
    void newAddedList();

private slots:
    void on_cancelCreation_clicked();

    void on_createList_clicked();

private:
    Ui::AddListWindow *ui;

    DataBase* db;
};

#endif // ADDLISTWINDOW_H
