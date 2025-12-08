#ifndef ADDLISTWINDOW_H
#define ADDLISTWINDOW_H

#include <QDialog>

namespace Ui {
class AddListWindow;
}

class AddListWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddListWindow(QWidget *parent = nullptr);
    ~AddListWindow();

private:
    Ui::AddListWindow *ui;
};

#endif // ADDLISTWINDOW_H
