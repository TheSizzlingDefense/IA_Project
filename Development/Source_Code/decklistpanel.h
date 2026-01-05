#ifndef DECKLISTPANEL_H
#define DECKLISTPANEL_H

#include <QWidget>
#include <QTableWidgetItem>
#include "database.h"

namespace Ui {
class DeckListPanel;
}

class DeckListPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DeckListPanel(DataBase* database, QWidget *parent = nullptr);
    ~DeckListPanel();

    void updateDeckList();
    void applyTheme(bool isDarkMode);

signals:
    void deckDoubleClicked(const QString& deckName, int listID);

private slots:
    void onDeckItemDoubleClicked(QTableWidgetItem* item);

private:
    Ui::DeckListPanel *ui;
    DataBase* db;
};

#endif // DECKLISTPANEL_H
