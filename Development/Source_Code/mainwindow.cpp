#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"
#include "database.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
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
    try {
        dataBase db("/home/brometheus/IA_Project/data/example.db");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

