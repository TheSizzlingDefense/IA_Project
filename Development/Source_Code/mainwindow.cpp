#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db("/home/brometheus/IA_Project/data/example.db")
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addWord_clicked() {
    AddCardWindow addCardWindow;
    addCardWindow.setModal(true);
    addCardWindow.exec();
}
