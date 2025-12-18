#include "addcardwindow.h"
#include "ui_addcardwindow.h"
#include <QMessageBox>
#include <stdexcept>

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
    
    // Hide additional options initially
    ui->additionalOptionsBox->setVisible(false);
}

AddCardWindow::~AddCardWindow() {
    delete ui;
}

void AddCardWindow::on_toggleAdditionalOptionsButton_toggled(bool checked) {
    if (checked) {
        ui->toggleAdditionalOptionsButton->setText("▼ Additional Options");
        ui->additionalOptionsBox->setVisible(true);
    } else {
        ui->toggleAdditionalOptionsButton->setText("▶ Additional Options");
        ui->additionalOptionsBox->setVisible(false);
    }
}

void AddCardWindow::applyTheme(bool isDark) {
    if (isDark) {
        // Dark theme
        QString darkStyle = R"(
            QDialog {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            
            QLabel {
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
            
            QLineEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QLineEdit:focus {
                border-color: #007acc;
            }
            
            QTextEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QTextEdit:focus {
                border-color: #007acc;
            }
            
            QComboBox {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QComboBox:hover {
                border-color: #007acc;
            }
            
            QComboBox::drop-down {
                border: none;
            }
            
            QComboBox QAbstractItemView {
                background-color: #252526;
                color: #e0e0e0;
                selection-background-color: #007acc;
                selection-color: #ffffff;
            }
            
            QGroupBox {
                color: #e0e0e0;
                border: 2px solid #3e3e42;
                border-radius: 6px;
                margin-top: 12px;
                padding-top: 8px;
                font-weight: bold;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 8px;
                background-color: #1e1e1e;
            }
        )";
        this->setStyleSheet(darkStyle);
    } else {
        // Light theme
        QString lightStyle = R"(
            QDialog {
                background-color: #ffffff;
                color: #2c3e50;
            }
            
            QLabel {
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
            
            QLineEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QLineEdit:focus {
                border-color: #3498db;
            }
            
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QTextEdit:focus {
                border-color: #3498db;
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
            
            QGroupBox {
                color: #2c3e50;
                border: 2px solid #bdc3c7;
                border-radius: 6px;
                margin-top: 12px;
                padding-top: 8px;
                font-weight: bold;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 8px;
                background-color: #ffffff;
            }
        )";
        this->setStyleSheet(lightStyle);
    }
}

void AddCardWindow::on_cancelAdding_clicked() {
    reject();
}

void AddCardWindow::on_addButton_clicked() {
    // Gather inputs
    QString listNameQ = ui->vocabListDropdown->currentText();
    std::string listName = listNameQ.toStdString();

    if (listNameQ.isEmpty() || listNameQ == "Select Option") {
        QMessageBox::warning(this, "Invalid List", "Please select a vocabulary list.");
        return;
    }

    QString wordQ = ui->wordInput->text();
    std::string word = wordQ.toStdString();
    if (wordQ.trimmed().isEmpty()) {
        QMessageBox::warning(this, "Invalid Word", "Please enter a word.");
        return;
    }

    QString definitionQ = ui->definitionInput->toPlainText();
    std::string definition = definitionQ.toStdString();

    QString posQ = ui->partOfSpeechList->currentText();
    std::string partOfSpeech = (posQ == "Select Option") ? std::string("") : posQ.toStdString();

    // Find list id
    int listID = db->getListId(listName);
    if (listID < 0) {
        QMessageBox::warning(this, "List Not Found", "Selected list was not found in the database.");
        return;
    }

    try {
        int wordID = db->addWordAndSetup(listID, word, partOfSpeech, definition, "");

        // If additional options checked, create example, notes, and/or relations
        if (ui->additionalOptionsBox->isChecked()) {
            // Handle examples and notes
            QString exampleQ = ui->exampleInput->toPlainText();
            QString notesQ = ui->notesInput->toPlainText();
            
            // Create example entry if either example or notes are provided
            if (!exampleQ.trimmed().isEmpty() || !notesQ.trimmed().isEmpty()) {
                db->createNewExample(wordID, exampleQ.toStdString(), notesQ.toStdString());
            }

            // Handle word relations (synonym, antonym, etc.)
            QString relatedWordQ = ui->relatedWordInput->text();
            QString relationTypeQ = ui->relationTypeDropdown->currentText();
            
            if (!relatedWordQ.trimmed().isEmpty() && relationTypeQ != "Select Option") {
                // Try to find the related word in the database
                std::string relatedWordStr = relatedWordQ.toStdString();
                int relatedWordID = db->getWordId(relatedWordStr, ""); // Empty language means any language
                
                if (relatedWordID >= 0) {
                    // Word exists, create the relation
                    db->createNewRelation(wordID, relatedWordID, relationTypeQ.toStdString());
                } else {
                    // Word doesn't exist yet, show a warning but don't fail
                    QMessageBox::warning(this, "Related Word Not Found", 
                        QString("The related word '%1' was not found in the database. The word has been added without this relation.").arg(relatedWordQ));
                }
            }
        }

        accept(); // close dialog with success
    } catch (const std::exception& ex) {
        QMessageBox::critical(this, "Database Error", QString::fromStdString(ex.what()));
    }
}

