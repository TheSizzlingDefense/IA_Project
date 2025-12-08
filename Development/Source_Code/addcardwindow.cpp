#include "addcardwindow.h"
#include "ui_addcardwindow.h"

AddCardWindow::AddCardWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddCardWindow)
{
    ui->setupUi(this);
}

AddCardWindow::~AddCardWindow()
{
    delete ui;
}

void AddCardWindow::on_cancelButton_clicked()
{
    reject();
}
