#pragma once
#include "ui_app_win.h"
#include "command_handler.h"
#include <map>
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
#include <QMessageBox>
#include <QCheckBox>
#include <QFile>
#include <QCloseEvent>
#include <QWindow>

#include <sw/redis++/redis++.h>

class JsonTreeHandler {
public:
    JsonTreeHandler();
    ~JsonTreeHandler();

    void LoadJsonToTreeView(QTreeView* treeView, const QJsonDocument& jsonDocument);
private:
    QStandardItemModel* model;

    void PopulateTree(QStandardItem* parentItem, const QJsonObject& jsonObject);
};

class JsonUserListHandler {
public:
    JsonUserListHandler();
    ~JsonUserListHandler();

    void LoadJsonToTableView(QTableView* tableView, const QJsonDocument& jsonDocument);
private:
    QStandardItemModel* usersModel;

    void AddUser(const QJsonObject& jsonObject);
};

class JsonGroupTreeHandler {
public:
    JsonGroupTreeHandler();
    ~JsonGroupTreeHandler();

    void LoadJsonToTreeView(QTreeView* tableView, const QJsonDocument& jsonDocument);
private:
    QStandardItemModel* groupsModel;
    void AddGroup(const QJsonObject& jsonObject);
};

class RedisSubscriber : public QObject {
    Q_OBJECT
signals:
    void redisMessageReceived(const QString& username, const QString& event, qint64 timestamp);
public:
    RedisSubscriber(std::shared_ptr<sw::redis::Redis> redis, QObject* parent);
    void Subscribe(const std::string& channel);
    ~RedisSubscriber();
private:
    std::shared_ptr<sw::redis::Redis> _redis;
    std::unique_ptr<sw::redis::Subscriber> _subscriber;
    std::thread _workerThread;
    QObject* _parent;
    bool _running;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
    public:
        MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *pwgt = nullptr);
        ~MainWindow();
        void SetupWindow(const QString& username, const QString& token);
        void SendCommand(const Operation op, const std::initializer_list<std::string>& data);
    private slots:
        void OnRoleMessageReceived(const QString& role);
        void OnUpdateFileList(const QString& role);
        void OnGetUsersList(const QString& response);
        void OnGetGroupsList(const QString& response);
        void OnOperationWithFile(const QString& response);
        void OnOperationWithGroup(const QString& response);
        void OnLeave(const QString& response);
        void DeleteFile(const QString& filename);
        void NeedUpdateFileList();
    public slots:
        void CreateFileButtonClicked();
        void CreateDirButtonClicked();
        void MakeCopyButtonClicked();
        void RecoverDBButtonClicked();
        void CreateGroupButtonClicked();
        void ShowContextMenu(const QPoint& pos);
        void ShowContextMenuGroups(const QPoint& pos);
    private:
        std::shared_ptr<CommandHandler> _commandHandler;
        std::shared_ptr<JsonTreeHandler> _treeHandler;
        std::shared_ptr<JsonUserListHandler> _usersListHandler;
        std::shared_ptr<JsonGroupTreeHandler> _groupsTreeHandler;
        std::shared_ptr<sw::redis::Redis> _connectionRedis;
        std::unique_ptr<RedisSubscriber> _redisSubscriber;
        Ui::MainWindow *ui;
        std::string _username;
        std::string _token;
    protected:
        void closeEvent(QCloseEvent *event) override;
};
