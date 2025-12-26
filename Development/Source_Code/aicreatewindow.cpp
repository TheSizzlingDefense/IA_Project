#include "aicreatewindow.h"
#include "ui_aicreatewindow.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QInputDialog>
#include <QMessageBox>
#include <QUrl>
#include <QByteArray>
#include <QDebug>
#include <cstdlib>

AICreateWindow::AICreateWindow(QWidget* parent, DataBase* db_)
    : QDialog(parent), db(db_) {
    ui = new Ui::AICreateWindow();
    ui->setupUi(this); // initialize widgets from the .ui

    // populate model choices
    ui->modelCombo->addItem("gpt-3.5-turbo");
    ui->modelCombo->addItem("gpt-4o-mini");

    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &AICreateWindow::onNetworkReplyFinished);

    // explicitly connect generate button to slot
    connect(ui->generateButton, &QPushButton::clicked, this, &AICreateWindow::on_generateButton_clicked);
}

AICreateWindow::~AICreateWindow() {
    delete ui;
}

void AICreateWindow::applyTheme(bool isDark) {
    if (isDark) {
        // Dark theme - buttons use default Qt style
        setStyleSheet(R"(
            QDialog {
                background-color: #1e1e1e;
                color: #e0e0e0;
            }
            QLabel {
                color: #e0e0e0;
                font-size: 10pt;
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
                font-size: 10pt;
            }
            QLineEdit:focus {
                border: 1px solid #007acc;
            }
            QTextEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QTextEdit:focus {
                border: 1px solid #007acc;
            }
            QComboBox {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QComboBox:hover {
                border-color: #007acc;
            }
            QComboBox::drop-down {
                border: none;
                width: 20px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 4px solid transparent;
                border-right: 4px solid transparent;
                border-top: 6px solid #e0e0e0;
                margin-right: 5px;
            }
            QComboBox QAbstractItemView {
                background-color: #252526;
                color: #e0e0e0;
                selection-background-color: #007acc;
                selection-color: #ffffff;
                border: 1px solid #3e3e42;
            }
            QSpinBox {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QSpinBox:focus {
                border: 1px solid #007acc;
            }
        )");
    } else {
        // Light theme - buttons use default Qt style
        setStyleSheet(R"(
            QDialog {
                background-color: #ffffff;
                color: #2c3e50;
            }
            QLabel {
                color: #2c3e50;
                font-size: 10pt;
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
                font-size: 10pt;
            }
            QLineEdit:focus {
                border: 1px solid #3498db;
            }
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QTextEdit:focus {
                border: 1px solid #3498db;
            }
            QComboBox {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QComboBox:hover {
                border-color: #95a5a6;
            }
            QComboBox:focus {
                border: 1px solid #3498db;
            }
            QComboBox::drop-down {
                border: none;
                width: 20px;
            }
            QComboBox::down-arrow {
                image: none;
                border-left: 4px solid transparent;
                border-right: 4px solid transparent;
                border-top: 6px solid #2c3e50;
                margin-right: 5px;
            }
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                color: #2c3e50;
                selection-background-color: #3498db;
                selection-color: #ffffff;
                border: 1px solid #bdc3c7;
            }
            QSpinBox {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
                font-size: 10pt;
            }
            QSpinBox:focus {
                border: 1px solid #3498db;
            }
        )");
    }
}

static QString fetchApiKeyInteractive(QWidget* parent) {
    // Try environment variable first
    const char* env = std::getenv("OPENAI_API_KEY");
    if (env && *env) return QString::fromUtf8(env);

    // Ask user for API key (do not store)
    bool ok = false;
    QString key = QInputDialog::getText(parent, "OpenAI API Key", "Enter your OpenAI API key:", QLineEdit::Password, "", &ok);
    if (ok) return key;
    return QString();
}

void AICreateWindow::on_generateButton_clicked() {
    QString listName = ui->listNameEdit->text().trimmed();
    if (listName.isEmpty()) {
        qWarning() << "Missing list name for AI vocabulary generation";
        QMessageBox::warning(this, "Missing List Name", "Please provide a name for the vocabulary list.");
        return;
    }

    int count = ui->numWordsSpin->value();
    QString customPrompt = ui->promptEdit->toPlainText().trimmed();
    QString model = ui->modelCombo->currentText();

    QString apiKey = fetchApiKeyInteractive(this);
    if (apiKey.isEmpty()) {
        qWarning() << "OpenAI API key not found";
        QMessageBox::warning(this, "No API Key", "OpenAI API key is required (set OPENAI_API_KEY or enter it when prompted).");
        return;
    }

    // Build the system+user messages to request a JSON array of words and definitions.
    QJsonObject root;
    root["model"] = model;

    QJsonArray messages;
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "You are a helpful assistant that outputs JSON only."
                         " When asked, produce a JSON array of objects with keys 'word' and 'definition'.";
    messages.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    QString userRequest = QString("Generate %1 vocabulary entries for a list named '%2'. "
                                  "Return ONLY a JSON array like [{\"word\": \"...\", \"definition\": \"...\"}, ...].\n")
                          .arg(count).arg(listName);
    if (!customPrompt.isEmpty()) {
        userRequest += "Additional instructions: \n" + customPrompt + "\n";
    }
    userMsg["content"] = userRequest;
    messages.append(userMsg);

    QJsonObject body;
    body["model"] = model;
    body["messages"] = messages;
    body["max_tokens"] = 1500;
    body["temperature"] = 0.8;

    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonDocument doc(body);
    QByteArray payload = doc.toJson();
    ui->generateButton->setEnabled(false);
    ui->statusLabel->setText("Generating...");

    manager->post(request, payload);
}

void AICreateWindow::onNetworkReplyFinished(QNetworkReply* reply) {
    ui->generateButton->setEnabled(true);
    QByteArray resp = reply->readAll();
    reply->deleteLater();
    ui->statusLabel->clear();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Network error in AI vocabulary generation:" << reply->errorString();
        QMessageBox::critical(this, "Network Error", reply->errorString());
        return;
    }

    // parse the response JSON and attempt to extract model output
    QJsonDocument doc = QJsonDocument::fromJson(resp);
    if (doc.isNull() || !doc.isObject()) {
        qCritical() << "Invalid JSON response from OpenAI API";
        QMessageBox::critical(this, "API Error", "Invalid JSON response from API");
        return;
    }

    QJsonObject obj = doc.object();
    QJsonValue choicesVal = obj.value("choices");
    QString textContent;
    if (choicesVal.isArray()) {
        QJsonArray choices = choicesVal.toArray();
        if (!choices.isEmpty()) {
            QJsonObject first = choices.at(0).toObject();
            // chat completions: message.content
            if (first.contains("message") && first.value("message").isObject()) {
                textContent = first.value("message").toObject().value("content").toString();
            } else if (first.contains("text")) {
                textContent = first.value("text").toString();
            }
        }
    }

    if (textContent.isEmpty()) {
        qCritical() << "OpenAI API returned no content";
        QMessageBox::critical(this, "API Error", "API returned no content.");
        return;
    }

    // Try to parse textContent as JSON array
    QJsonDocument listDoc = QJsonDocument::fromJson(textContent.toUtf8());
    if (listDoc.isNull()) {
        // Attempt to extract a JSON substring from the response
        QByteArray b = textContent.toUtf8();
        int start = b.indexOf('[');
        int end = b.lastIndexOf(']');
        if (start >= 0 && end > start) {
            QByteArray sub = b.mid(start, end - start + 1);
            listDoc = QJsonDocument::fromJson(sub);
        }
    }

    if (listDoc.isNull() || !listDoc.isArray()) {
        qCritical() << "Could not parse OpenAI model output as JSON array. Response:" << textContent;
        QMessageBox::critical(this, "Parse Error", "Could not parse the model output as a JSON array.\nResponse:\n" + textContent);
        return;
    }

    QJsonArray arr = listDoc.array();
    if (arr.isEmpty()) {
        QMessageBox::information(this, "No Results", "Model returned empty list.");
        return;
    }

    // Create the new list in the DB
    QString listName = ui->listNameEdit->text().trimmed();
    try {
        bool ok = db->createNewList(listName.toStdString(), std::string(""), std::string("Created by AI"));
        int listID = db->getListId(listName.toStdString());
        if (listID < 0) {
            qCritical() << "Failed to create or retrieve new list id for AI-generated list:" << listName;
            throw std::runtime_error("Failed to create or retrieve new list id.");
        }

        int added = 0;
        for (const QJsonValue &v : arr) {
            if (!v.isObject()) continue;
            QJsonObject o = v.toObject();
            QString w = o.value("word").toString().trimmed();
            QString def = o.value("definition").toString().trimmed();
            if (w.isEmpty()) continue;
            int wid = db->addWordAndSetup(listID, w.toStdString(), std::string(""), def.toStdString(), std::string(""));
            if (wid >= 0) added++;
        }

        QMessageBox::information(this, "Done", QString("Added %1 entries to list '%2'.").arg(added).arg(listName));
        accept();
    } catch (const std::exception &ex) {
        qCritical() << "Database error during AI vocabulary list creation:" << ex.what();
        QMessageBox::critical(this, "DB Error", QString::fromStdString(ex.what()));
    }
}

void AICreateWindow::on_pushButton_clicked() {
    reject();
}
