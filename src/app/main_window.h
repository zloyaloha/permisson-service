#pragma once
#include "ui_app_win.h"
#include "command_handler.h"
#include <QApplication>
#include <QWidget>
#include <QMainWindow>
#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardItemModel>
#include <QTreeView>

class JsonTreeHandler {
public:
    JsonTreeHandler();
    ~JsonTreeHandler();

    // Метод для загрузки JSON в QTreeView
    void LoadJsonToTreeView(QTreeView* treeView, const QJsonDocument& jsonDocument);

private:
    QStandardItemModel* model;

    // Рекурсивная функция для заполнения модели данными из JSON
    void PopulateTree(QStandardItem* parentItem, const QJsonObject& jsonObject);
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *pwgt = nullptr);
        ~MainWindow();
        void SetupWindow(const QString& username, const QString& token);
    private slots:
        void OnRoleMessageReceived(const QString& role);
        void OnUpdateFileList(const QString& role);
    public slots:
        void NeedUpdateFileList();
        void CreateFileButtonClicked();
        void ShowContextMenu(QPoint);
    private:
        std::shared_ptr<CommandHandler> _commandHandler;
        std::shared_ptr<JsonTreeHandler> _treeHandler;
        Ui::MainWindow *ui;
        std::string _username;
        std::string _token;
};
