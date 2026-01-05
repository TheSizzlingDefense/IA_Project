#ifndef MODESELECTORPANEL_H
#define MODESELECTORPANEL_H

#include <QWidget>

namespace Ui {
class ModeSelectorPanel;
}

class ModeSelectorPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ModeSelectorPanel(QWidget *parent = nullptr);
    ~ModeSelectorPanel();

    void setDeckInfo(const QString& deckName, int listID);
    int getSelectedMode() const; // 0=Flashcard, 1=MultipleChoice, 2=Typing
    int getCurrentListID() const { return currentListID; }
    QString getCurrentDeckName() const { return currentDeckName; }

signals:
    void startStudyClicked(int listID, int mode);
    void viewAllClicked(int listID);
    void deleteListClicked(int listID);

private slots:
    void onStartStudyClicked();
    void onViewAllClicked();
    void onDeleteListClicked();

private:
    Ui::ModeSelectorPanel *ui;
    int currentListID;
    QString currentDeckName;
};

#endif // MODESELECTORPANEL_H
