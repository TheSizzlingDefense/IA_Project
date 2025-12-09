#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"
#include "addlistwindow.h"

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

void MainWindow::on_creatDeck_clicked() {
    AddListWindow addListWindow{nullptr, &db};
    QObject::connect(&addListWindow, &AddListWindow::newAddedList, this, &MainWindow::updatingList);
    addListWindow.setModal(true);
    addListWindow.exec();
}

DataBase MainWindow::getDB() {
    return db;
}

void MainWindow::updatingList() {

}
