#ifndef ADDCARDWINDOW_H
#define ADDCARDWINDOW_H

#include <QDialog>

namespace Ui {
class AddCardWindow;
}

class AddCardWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AddCardWindow(QWidget *parent = nullptr);
    ~AddCardWindow();

private slots:
    void on_cancelButton_clicked();

private:
    Ui::AddCardWindow *ui;
};

#endif // ADDCARDWINDOW_H
