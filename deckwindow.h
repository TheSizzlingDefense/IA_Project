#ifndef DECKWINDOW_H
#define DECKWINDOW_H

#include <QDialog>

namespace Ui {
class deckwindow;
}

class DeckWindow : public QDialog
{
    Q_OBJECT

public:
    explicit DeckWindow(QWidget *parent = nullptr);
    ~DeckWindow();

private:
    Ui::deckwindow *ui;
};

#endif // DECKWINDOW_H
