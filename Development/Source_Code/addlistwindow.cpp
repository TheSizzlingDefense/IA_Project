#include "addlistwindow.h"
#include "ui_addlistwindow.h"
#include <QMessageBox>

AddListWindow::AddListWindow(QWidget *parent, DataBase* dataBase)
    : QDialog(parent)
    , ui(new Ui::AddListWindow)
    , db(dataBase)
{
    ui->setupUi(this);
}

AddListWindow::~AddListWindow() {
    delete ui;
}

void AddListWindow::on_cancelCreation_clicked() {
    reject();
}


void AddListWindow::on_createList_clicked() {
    QString listName = ui->listNameInput->text().trimmed();
    
    if (listName.isEmpty()) {
        QMessageBox::warning(this, "Empty List Name", "Please enter a name for the vocabulary list.");
        return;
    }
    
    try {
        db->createNewList(listName.toStdString(), ui->languageInput->text().toStdString(), ui->descriptionInput->toPlainText().toStdString());
        emit newAddedList();
        reject();
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Error Creating List", QString::fromStdString(ex.what()));
    }
}

