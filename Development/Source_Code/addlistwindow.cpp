#include "addlistwindow.h"
#include "ui_addlistwindow.h"

AddListWindow::AddListWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddListWindow)
{
    ui->setupUi(this);
}

AddListWindow::~AddListWindow()
{
    delete ui;
}
