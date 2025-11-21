#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "deckwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_access_decks_clicked()
{
    DeckWindow deckwindow;
    deckwindow.setModal(true);
    deckwindow.exec();
}

