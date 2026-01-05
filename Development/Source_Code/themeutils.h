#ifndef THEMEUTILS_H
#define THEMEUTILS_H

#include <QString>

class ThemeUtils
{
public:
    static QString getLightTheme() {
        return R"(
            QDialog, QMainWindow {
                background-color: #ffffff;
            }
            
            QWidget {
                background-color: #ffffff;
                color: #2c3e50;
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
            
            QListWidget {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                selection-background-color: #3498db;
                selection-color: #ffffff;
            }
            
            QListWidget::item {
                padding: 8px;
            }
            
            QListWidget::item:selected {
                background-color: #3498db;
                color: #ffffff;
            }
            
            QListWidget::item:hover {
                background-color: #ecf0f1;
            }
            
            QComboBox {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QComboBox:hover {
                border-color: #95a5a6;
            }
            
            QComboBox::drop-down {
                border: none;
            }
            
            QComboBox QAbstractItemView {
                background-color: #ffffff;
                color: #2c3e50;
                selection-background-color: #3498db;
                selection-color: #ffffff;
            }
            
            QLabel {
                color: #2c3e50;
            }
            
            #studyDefinitionLabel {
                background-color: #f0f0f0;
                border: 1px solid #bdc3c7;
            }
            
            QTextEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QLineEdit {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QSpinBox {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                padding: 6px;
            }
            
            QTableWidget {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                border-radius: 4px;
                gridline-color: #bdc3c7;
                selection-background-color: #3498db;
                selection-color: #ffffff;
            }
            
            QTableWidget::item {
                padding: 8px;
            }
            
            QTableWidget::item:selected {
                background-color: #3498db;
                color: #ffffff;
            }
            
            QTableWidget::item:hover {
                background-color: #ecf0f1;
            }
            
            QHeaderView::section {
                background-color: #ecf0f1;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
                padding: 6px;
                font-weight: bold;
            }
            
            QGroupBox {
                color: #2c3e50;
                border: 2px solid #bdc3c7;
                border-radius: 6px;
                margin-top: 12px;
                padding-top: 8px;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 8px;
                background-color: #ffffff;
            }
            
            QMenuBar {
                background-color: #ecf0f1;
                color: #2c3e50;
            }
            
            QMenuBar::item:selected {
                background-color: #d5dbdb;
            }
            
            QMenu {
                background-color: #ffffff;
                color: #2c3e50;
                border: 1px solid #bdc3c7;
            }
            
            QMenu::item:selected {
                background-color: #3498db;
                color: #ffffff;
            }
        )";
    }

    static QString getDarkTheme() {
        return R"(
            QDialog, QMainWindow {
                background-color: #1e1e1e;
            }
            
            QWidget {
                background-color: #1e1e1e;
                color: #e0e0e0;
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
            
            QListWidget {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                selection-background-color: #007acc;
                selection-color: #ffffff;
            }
            
            QListWidget::item {
                padding: 8px;
            }
            
            QListWidget::item:selected {
                background-color: #007acc;
                color: #ffffff;
            }
            
            QListWidget::item:hover {
                background-color: #2d2d30;
            }
            
            QComboBox {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                padding: 5px;
                min-height: 20px;
                margin: 0px;
                outline: 0;
            }
            
            QComboBox:hover {
                border-color: #007acc;
            }
            
            QComboBox::drop-down {
                border: none;
                margin: 0px;
            }
            
            QComboBox QAbstractItemView {
                background-color: #252526;
                color: #e0e0e0;
                selection-background-color: #007acc;
                selection-color: #ffffff;
            }
            
            QLabel {
                color: #e0e0e0;
            }
            
            #studyDefinitionLabel {
                background-color: #2d2d30;
                border: 1px solid #3e3e42;
            }
            
            QTextEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QLineEdit {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QSpinBox {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                padding: 6px;
            }
            
            QTableWidget {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                border-radius: 4px;
                gridline-color: #3e3e42;
                selection-background-color: #007acc;
                selection-color: #ffffff;
            }
            
            QTableWidget::item {
                padding: 8px;
            }
            
            QTableWidget::item:selected {
                background-color: #007acc;
                color: #ffffff;
            }
            
            QTableWidget::item:hover {
                background-color: #2d2d30;
            }
            
            QHeaderView::section {
                background-color: #2d2d30;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
                padding: 6px;
                font-weight: bold;
            }
            
            QGroupBox {
                color: #e0e0e0;
                border: 2px solid #3e3e42;
                border-radius: 6px;
                margin-top: 12px;
                padding-top: 8px;
            }
            
            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;
                padding: 0 8px;
                background-color: #1e1e1e;
            }
            
            QMenuBar {
                background-color: #2d2d30;
                color: #e0e0e0;
            }
            
            QMenuBar::item:selected {
                background-color: #3e3e42;
            }
            
            QMenu {
                background-color: #252526;
                color: #e0e0e0;
                border: 1px solid #3e3e42;
            }
            
            QMenu::item:selected {
                background-color: #007acc;
                color: #ffffff;
            }
        )";
    }
};

#endif // THEMEUTILS_H
