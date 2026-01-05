#include "decklistpanel.h"
#include "ui_decklistpanel.h"
#include <algorithm>
#include <QHeaderView>

DeckListPanel::DeckListPanel(DataBase* database, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeckListPanel)
    , db(database)
{
    ui->setupUi(this);
    
    // Configure table widget
    ui->deckList->horizontalHeader()->setStretchLastSection(false);
    ui->deckList->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->deckList->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->deckList->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->deckList->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->deckList->verticalHeader()->setVisible(false);
    
    // Apply styling
    ui->deckList->setStyleSheet(
        "QTableWidget { "
        "   border: none; "
        "   background-color: transparent; "
        "} "
        "QTableWidget::item { "
        "   border: none; "
        "   padding: 8px; "
        "} "
        "QTableWidget::item:selected { "
        "   background-color: palette(highlight); "
        "   color: palette(highlighted-text); "
        "} "
        "QHeaderView::section { "
        "   background-color: transparent; "
        "   border: none; "
        "   border-bottom: 1px solid palette(mid); "
        "   padding: 8px; "
        "   font-weight: bold; "
        "}"
    );
    
    connect(ui->deckList, &QTableWidget::itemDoubleClicked, this, &DeckListPanel::onDeckItemDoubleClicked);
    
    updateDeckList();
}

DeckListPanel::~DeckListPanel()
{
    delete ui;
}

void DeckListPanel::updateDeckList()
{
    ui->deckList->setRowCount(0);
    
    // Get lists with their next review date and sort by earliest review first
    auto lists = db->getVocabListsWithNextReview();
    
    // Sort: empty dates (no reviews) go to the end; otherwise sort by datetime string ascending
    std::sort(lists.begin(), lists.end(), [](const auto &a, const auto &b) {
        const std::string &da = a.second;
        const std::string &dbs = b.second;
        if (da.empty() && dbs.empty()) return a.first < b.first; // both empty -> alphabetical
        if (da.empty()) return false; // a goes after b
        if (dbs.empty()) return true;  // a goes before b
        return da < dbs; // ISO-like datetime textual compare works for 'YYYY-MM-DD HH:MM:SS'
    });
    
    for (const auto &p : lists) {
        QString deckName = QString::fromStdString(p.first);
        
        // Get card counts for this list
        int listID = db->getListId(p.first);
        int newCount = db->getNewCardCount(listID);
        int continuingCount = db->getContinuingCardCount(listID);
        int reviewCount = db->getReviewCardCount(listID);
        
        // Add row to table
        int row = ui->deckList->rowCount();
        ui->deckList->insertRow(row);
        ui->deckList->setItem(row, 0, new QTableWidgetItem(deckName));
        ui->deckList->setItem(row, 1, new QTableWidgetItem(QString::number(newCount)));
        ui->deckList->setItem(row, 2, new QTableWidgetItem(QString::number(continuingCount)));
        ui->deckList->setItem(row, 3, new QTableWidgetItem(QString::number(reviewCount)));
        
        // Center align the numeric columns
        ui->deckList->item(row, 1)->setTextAlignment(Qt::AlignCenter);
        ui->deckList->item(row, 2)->setTextAlignment(Qt::AlignCenter);
        ui->deckList->item(row, 3)->setTextAlignment(Qt::AlignCenter);
    }
}

void DeckListPanel::onDeckItemDoubleClicked(QTableWidgetItem* item)
{
    if (!item) return;
    
    int row = item->row();
    QString deckName = ui->deckList->item(row, 0)->text();
    int listID = db->getListId(deckName.toStdString());
    
    emit deckDoubleClicked(deckName, listID);
}

void DeckListPanel::applyTheme(bool isDarkMode)
{
    // Theme is handled by the parent MainWindow's stylesheet
    // This method is here for future customizations if needed
    Q_UNUSED(isDarkMode);
}
