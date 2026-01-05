#include "modeselectorpanel.h"
#include "ui_modeselectorpanel.h"

ModeSelectorPanel::ModeSelectorPanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModeSelectorPanel)
    , currentListID(-1)
{
    ui->setupUi(this);
    
    // Connect signals
    connect(ui->startStudyButton, &QPushButton::clicked, this, &ModeSelectorPanel::onStartStudyClicked);
    connect(ui->viewAllButton, &QPushButton::clicked, this, &ModeSelectorPanel::onViewAllClicked);
    connect(ui->deleteListButton, &QPushButton::clicked, this, &ModeSelectorPanel::onDeleteListClicked);
}

ModeSelectorPanel::~ModeSelectorPanel()
{
    delete ui;
}

void ModeSelectorPanel::setDeckInfo(const QString& deckName, int listID)
{
    currentDeckName = deckName;
    currentListID = listID;
    ui->selectedDeckLabel->setText(deckName);
}

int ModeSelectorPanel::getSelectedMode() const
{
    return ui->studyModeComboBox->currentIndex();
}

void ModeSelectorPanel::onStartStudyClicked()
{
    emit startStudyClicked(currentListID, ui->studyModeComboBox->currentIndex());
}

void ModeSelectorPanel::onViewAllClicked()
{
    emit viewAllClicked(currentListID);
}

void ModeSelectorPanel::onDeleteListClicked()
{
    emit deleteListClicked(currentListID);
}
