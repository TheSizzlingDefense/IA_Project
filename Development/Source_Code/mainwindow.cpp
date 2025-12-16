#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcardwindow.h"
#include "addlistwindow.h"
#include <algorithm>
#include "studywindow.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , db("/home/brometheus/IA_Project/data/example.db")
{
    ui->setupUi(this);
    ui->deckList->setStyleSheet(
        "QListWidget::item:selected { color: black; }"
        );
    ui->deckList->clear();
    // Get lists with their next review date and sort by earliest review first
    auto lists = db.getVocabListsWithNextReview();

    // sort: empty dates (no reviews) go to the end; otherwise sort by datetime string ascending
    std::sort(lists.begin(), lists.end(), [](const auto &a, const auto &b) {
        const std::string &da = a.second;
        const std::string &dbs = b.second;
        if (da.empty() && dbs.empty()) return a.first < b.first; // both empty -> alphabetical
        if (da.empty()) return false; // a goes after b
        if (dbs.empty()) return true;  // a goes before b
        return da < dbs; // ISO-like datetime textual compare works for 'YYYY-MM-DD HH:MM:SS'
    });

    for (const auto &p : lists) {
        QString label = QString::fromStdString(p.first);
        if (!p.second.empty()) {
            label += "  — Next: ";
            label += QString::fromStdString(p.second);
        } else {
            label += "  — Next: (no scheduled reviews)";
        }
        ui->deckList->addItem(label);
    }

    // connect double-click to show mode selection panel
    connect(ui->deckList, &QListWidget::itemDoubleClicked, this, &MainWindow::deckListDoubleClicked);
    // connect mode panel buttons
    connect(ui->startStudyButton, &QPushButton::clicked, this, &MainWindow::on_startStudyButton_clicked);
    // don't manually connect the top 'Decks' button — use Qt auto-connect by naming the slot `on_listDecks_clicked`
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_addWord_clicked() {
    AddCardWindow addCardWindow{nullptr, &db};
    addCardWindow.setModal(true);
    addCardWindow.exec();
}

void MainWindow::on_creatDeck_clicked() {
    AddListWindow addListWindow{nullptr, &db};
    QObject::connect(&addListWindow, &AddListWindow::newAddedList, this, &MainWindow::updatingList);
    addListWindow.setModal(true);
    addListWindow.exec();
}

DataBase* MainWindow::getDB() {
    return &db;
}

void MainWindow::updatingList() {
    ui->deckList->clear();
    auto lists = db.getVocabListsWithNextReview();
    std::sort(lists.begin(), lists.end(), [](const auto &a, const auto &b) {
        const std::string &da = a.second;
        const std::string &dbs = b.second;
        if (da.empty() && dbs.empty()) return a.first < b.first;
        if (da.empty()) return false;
        if (dbs.empty()) return true;
        return da < dbs;
    });

    for (const auto &p : lists) {
        QString label = QString::fromStdString(p.first);
        if (!p.second.empty()) {
            label += "  — Next: ";
            label += QString::fromStdString(p.second);
        } else {
            label += "  — Next: (no scheduled reviews)";
        }
        ui->deckList->addItem(label);
    }
}

void MainWindow::startStudy() {
    QListWidgetItem* item = ui->deckList->currentItem();
    if (!item) return;
    // extract list name before the separator '  — Next:'
    QString text = item->text();
    QStringList parts = text.split("  — Next:");
    QString listName = parts.at(0).trimmed();
    int listID = db.getListId(listName.toStdString());
    StudyWindow* w = new StudyWindow(&db, listID, this);
    w->setModal(true);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void MainWindow::deckListDoubleClicked(QListWidgetItem* item) {
    if (!item) return;
    QString text = item->text();
    QStringList parts = text.split("  — Next:");
    pendingListName = parts.at(0).trimmed();
    pendingListID = db.getListId(pendingListName.toStdString());

    // update mode panel label and show the panel
    ui->selectedDeckLabel->setText(pendingListName);
    ui->modePanel->setVisible(true);
    ui->deckList->setVisible(false);
}

void MainWindow::on_startStudyButton_clicked() {
    if (pendingListID < 0) return;
    StudyWindow* w = new StudyWindow(&db, pendingListID, this);
    // set mode based on combo box
    QString mode = ui->studyModeComboBox->currentText();
    w->setStudyModeFromString(mode);
    w->setModal(true);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->show();
}

void MainWindow::on_listDecks_clicked() {
    // hide mode panel and show deck list (invoked by top 'Decks' button)
    ui->modePanel->setVisible(false);
    ui->deckList->setVisible(true);
    pendingListID = -1;
    pendingListName.clear();
}

void MainWindow::on_showStats_clicked() {
    // Use the database helper to build a summary string and show it in a dialog
    std::string summary = db.getStudySessionSummary();

    QDialog dlg(this);
    dlg.setWindowTitle("Study Sessions Summary");
    QVBoxLayout* layout = new QVBoxLayout(&dlg);
    QTextEdit* view = new QTextEdit(&dlg);
    view->setReadOnly(true);
    view->setPlainText(QString::fromStdString(summary));
    layout->addWidget(view);
    QPushButton* closeBtn = new QPushButton("Close", &dlg);
    QObject::connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    dlg.resize(480, 320);
    dlg.exec();
}
