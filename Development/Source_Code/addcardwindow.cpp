#include "addcardwindow.h"
#include "ui_addcardwindow.h"

AddCardWindow::AddCardWindow(QWidget *parent, DataBase* dataBase)
    : QDialog(parent)
    , ui(new Ui::AddCardWindow)
    , db(dataBase)
{
    ui->setupUi(this);
    ui->vocabListDropdown->addItem("Select Option");
    std::vector<std::string> allVocabLists = db->getVocabLists();
    for (size_t i = 0; i < allVocabLists.size(); i++) {
        ui->vocabListDropdown->addItem(QString::fromStdString(allVocabLists.at(i)));
    }
}

AddCardWindow::~AddCardWindow() {
    delete ui;
}

void AddCardWindow::on_cancelAdding_clicked() {
    reject();
}

void AddCardWindow::on_addButton_clicked() {

}

