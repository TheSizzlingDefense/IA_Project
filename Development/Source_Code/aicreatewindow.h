#ifndef AICREATEWINDOW_H
#define AICREATEWINDOW_H

#include <QDialog>
#include "database.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Ui {
class AICreateWindow;
}

class AICreateWindow : public QDialog {
    Q_OBJECT
public:
    explicit AICreateWindow(QWidget* parent = nullptr, DataBase* db = nullptr);
    ~AICreateWindow();

    void applyTheme(bool isDark);

private slots:
    void on_generateButton_clicked();
    void onNetworkReplyFinished(QNetworkReply* reply);

    void on_pushButton_clicked();

private:
    Ui::AICreateWindow *ui;

    DataBase* db;
    QNetworkAccessManager* manager;
};

#endif // AICREATEWINDOW_H
