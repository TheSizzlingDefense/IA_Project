#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , db("/home/brometheus/IA_Project/data/example.db")
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_access_decks_clicked() {

}


void MainWindow::on_add_word_clicked() {
    AddCardWindow addCardWindow;
    addCardWindow.setModal(true);
    addCardWindow.exec();
}


void MainWindow::on_create_deck_clicked() {

}

