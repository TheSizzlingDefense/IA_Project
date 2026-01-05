#ifndef STUDYPANEL_H
#define STUDYPANEL_H

#include <QWidget>
#include <QPushButton>
#include <vector>
#include "database.h"

namespace Ui {
class StudyPanel;
}

class StudyPanel : public QWidget
{
    Q_OBJECT

public:
    enum class StudyMode {
        Flashcard,
        MultipleChoice,
        Typing
    };

    explicit StudyPanel(DataBase* database, QWidget *parent = nullptr);
    ~StudyPanel();

    void setStudyCards(const std::vector<DataBase::DueCard>& cards, StudyMode mode);
    void showCurrentCard();
    void setRandomPracticeMode(bool isRandom) { isRandomPractice = isRandom; }

signals:
    void studyCompleted();

private slots:
    void onReveal();
    void onRateAgain();
    void onRateHard();
    void onRateGood();
    void onRateEasy();
    void onChoiceSelected();
    void onSubmitTypedAnswer();

private:
    void updateAdditionalInfo(int word_id);
    void applyRating(int quality);

    Ui::StudyPanel *ui;
    DataBase* db;
    std::vector<DataBase::DueCard> studyCards;
    std::vector<int> recentlySeenWordIds;
    QPushButton* choiceButtons[4];
    int correctChoiceIndex;
    size_t currentCardIndex;
    StudyMode studyMode;
    bool isRandomPractice;
    int typingAttempts;
    bool showingExample;
    int currentStudyListID;
};

#endif // STUDYPANEL_H
