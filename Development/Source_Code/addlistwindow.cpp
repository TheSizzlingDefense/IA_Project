#include "addlistwindow.h"
#include "ui_addlistwindow.h"
#include "themeutils.h"

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

void AddListWindow::applyTheme(bool isDark) {
    setStyleSheet(isDark ? ThemeUtils::getDarkTheme() : ThemeUtils::getLightTheme());
}

void AddListWindow::on_cancelCreation_clicked() {
    reject();
}


void AddListWindow::on_createList_clicked() {
    db->createNewList(ui->listNameInput->text().toStdString(), ui->languageInput->text().toStdString(), ui->descriptionInput->toPlainText().toStdString());
    emit newAddedList();
    reject();
}

