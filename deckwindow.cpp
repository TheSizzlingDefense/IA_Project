#include "deckwindow.h"
#include "ui_deckwindow.h"

DeckWindow::DeckWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::deckwindow)
{
    ui->setupUi(this);
}

DeckWindow::~DeckWindow()
{
    delete ui;
}
