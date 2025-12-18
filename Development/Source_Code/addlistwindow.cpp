#include "addlistwindow.h"
#include "ui_addlistwindow.h"

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
    if (isDark) {
        // Dark theme
        setStyleSheet(R"(
            QDialog {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            QLabel {
                color: #e0e0e0;
                font-size: 10pt;
            }
            QLineEdit {
                background-color: #2d2d2d;
                color: #e0e0e0;
                border: 1px solid #3f3f3f;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QLineEdit:focus {
                border: 1px solid #007acc;
            }
            QTextEdit {
                background-color: #2d2d2d;
                color: #e0e0e0;
                border: 1px solid #3f3f3f;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QTextEdit:focus {
                border: 1px solid #007acc;
            }
            QPushButton {
                background-color: #007acc;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 8px 16px;
                font-size: 10pt;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #005a9e;
            }
            QPushButton:pressed {
                background-color: #004578;
            }
        )");
    } else {
        // Light theme
        setStyleSheet(R"(
            QDialog {
                background-color: #ffffff;
                color: #2c3e50;
            }
            QLabel {
                color: #2c3e50;
                font-size: 10pt;
            }
            QLineEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QLineEdit:focus {
                border: 1px solid #3498db;
            }
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QTextEdit:focus {
                border: 1px solid #3498db;
            }
            QPushButton {
                background-color: #3498db;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 8px 16px;
                font-size: 10pt;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2980b9;
            }
            QPushButton:pressed {
                background-color: #1f618d;
            }
        )");
    }
}

void AddListWindow::on_cancelCreation_clicked() {
    reject();
}


void AddListWindow::on_createList_clicked() {
    db->createNewList(ui->listNameInput->text().toStdString(), ui->languageInput->text().toStdString(), ui->descriptionInput->toPlainText().toStdString());
    emit newAddedList();
    reject();
}

