#ifndef ADDCARDWINDOW_H
#define ADDCARDWINDOW_H

#include <QDialog>
#include "database.h"

namespace Ui {
class AddCardWindow;
}

class AddCardWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddCardWindow(QWidget *parent = nullptr, DataBase* dataBase = nullptr);
    ~AddCardWindow();

private slots:
    void on_cancelAdding_clicked();

    void on_addButton_clicked();

private:
    Ui::AddCardWindow *ui;

    DataBase* db;
};

#endif // ADDCARDWINDOW_H
