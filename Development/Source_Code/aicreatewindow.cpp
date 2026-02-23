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
#include <QRegularExpression>
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
}

AICreateWindow::~AICreateWindow() {
    delete ui;
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
    systemMsg["content"] = "You are a JSON-only assistant. You must respond with ONLY valid JSON and nothing else. "
                         "No markdown, no code blocks, no explanations. Just the raw JSON array.";
    messages.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    QString userRequest = QString("Generate %1 vocabulary entries for a list named '%2'. "
                                  "Return ONLY a valid JSON array with no markdown or code blocks: [{\"word\": \"...\", \"definition\": \"...\"}, ...]\n")
                          .arg(count).arg(listName);
    if (!customPrompt.isEmpty()) {
        userRequest += "Additional instructions: " + customPrompt + "\n";
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

    // Log the raw response for debugging
    qDebug() << "Raw AI response:" << textContent;

    // Clean up the response: remove markdown code blocks and extra whitespace
    QString cleanedContent = textContent;
    
    // Remove markdown code block markers (handles variations like ```json, ``` etc)
    cleanedContent.replace(QRegularExpression("```[a-zA-Z]*\\s*"), "");
    cleanedContent.replace("```", "");
    
    // Remove any leading/trailing whitespace
    cleanedContent = cleanedContent.trimmed();

    // Try to parse as JSON array directly
    QJsonDocument listDoc = QJsonDocument::fromJson(cleanedContent.toUtf8());
    
    if (listDoc.isNull()) {
        // Attempt to extract a JSON substring from the response
        // Find the first '[' and last ']' to extract the JSON array
        QByteArray b = cleanedContent.toUtf8();
        int start = b.indexOf('[');
        int end = b.lastIndexOf(']');
        
        if (start >= 0 && end > start) {
            QByteArray sub = b.mid(start, end - start + 1);
            listDoc = QJsonDocument::fromJson(sub);
            
            if (!listDoc.isNull()) {
                qDebug() << "Successfully extracted JSON array from response";
            }
        }
    }

    if (listDoc.isNull() || !listDoc.isArray()) {
        qCritical() << "Could not parse OpenAI model output as JSON array.";
        qCritical() << "Cleaned response:" << cleanedContent;
        QMessageBox::critical(this, "Parse Error", "Could not parse the model output as a JSON array.\n\nResponse:\n" + cleanedContent.left(500));
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
        db->createNewList(listName.toStdString(), std::string(""), std::string("Created by AI"));
        int listID = db->getListId(listName.toStdString());

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
