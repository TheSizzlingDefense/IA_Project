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
}

AddCardWindow::~AddCardWindow() {
    delete ui;
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

