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
        // Dark theme - modern and easy on the eyes
        setStyleSheet(R"(
            QDialog, QMainWindow {
                background-color: #1e1e1e;
            }
            
            QWidget {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            
            QPushButton {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 8px 16px;
            }
            
            QPushButton:hover {
                background-color: #3e3e42;
                border-color: #007acc;
            }
            
            QPushButton:pressed {
                background-color: #007acc;
            }
            
            QPushButton:disabled {
                background-color: #2d2d30;
                color: #656565;
            }
            
            QLabel {
                color: #e0e0e0;
            }
            
            QTextEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QLineEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QComboBox {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                padding: 5px;
                min-height: 20px;
                margin: 0px;
                outline: 0;
            }
            
            QComboBox:hover {
                border-color: #007acc;
            }
            
            QComboBox::drop-down {
                border: none;
                margin: 0px;
            }
            
            QComboBox QAbstractItemView {
                background-color: #252526;
                color: #e0e0e0;
                selection-background-color: #007acc;
                selection-color: #ffffff;
            }
        )");
    } else {
        // Light theme - clean and minimal
        setStyleSheet(R"(
            QDialog, QMainWindow {
                background-color: #ffffff;
            }
            
            QWidget {
                background-color: #ffffff;
                color: #2c3e50;
            }
            
            QPushButton {
                background-color: #ecf0f1;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 8px 16px;
            }
            
            QPushButton:hover {
                background-color: #d5dbdb;
                border-color: #95a5a6;
            }
            
            QPushButton:pressed {
                background-color: #bdc3c7;
            }
            
            QPushButton:disabled {
                background-color: #ecf0f1;
                color: #95a5a6;
            }
            
            QLabel {
                color: #2c3e50;
            }
            
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QLineEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QComboBox {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QComboBox:hover {
                border-color: #95a5a6;
            }
            
            QComboBox::drop-down {
                border: none;
            }
            
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                color: #2c3e50;
                selection-background-color: #3498db;
                selection-color: #ffffff;
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

