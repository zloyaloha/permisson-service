#include "main_window.h"

MainWindow::MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _commandHandler(commandor), _treeHandler(std::make_shared<JsonTreeHandler>())
{
    ui->setupUi(this);
    ui->listTree->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_commandHandler.get(), &CommandHandler::GetRoleMessageReceived, this, &MainWindow::OnRoleMessageReceived);
    connect(_commandHandler.get(), &CommandHandler::UpdateFileList, this, &MainWindow::OnUpdateFileList);
    connect(ui->createFileButton, &QPushButton::clicked, this, &MainWindow::CreateFileButtonClicked);
    connect(ui->dirCreateButton, &QPushButton::clicked, this, &MainWindow::CreateDirButtonClicked);
    connect(ui->listTree, SIGNAL(customContextMenuRequested(QPoint)), SLOT(ShowContextMenu(QPoint)));

    QFile file(":/styles/styles.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        ui->listTree->setStyleSheet(styleSheet);
        file.close();
    } else {
        qWarning() << "Failed to load stylesheet from";
    }

}

MainWindow::~MainWindow() {
    std::cout << "плохо" << std::endl;
}

void MainWindow::SetupWindow(const QString& username, const QString& token) {
    _token = token.toStdString();
    _username = username.toStdString();
    ui->usernameLine->setText(username);
    _commandHandler->SendCommand(Operation::GetRole, {_username, _token});
    _commandHandler->SendCommand(Operation::GetFileList, {_username, _token});
    this->show();
}

void MainWindow::CreateFileButtonClicked() {
    std::string pathToFile = "root/" + ui->createFile->text().toStdString();
    size_t lastSlashPos = pathToFile.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        std::string path = "";
        std::string filename = pathToFile;
        _commandHandler->SendCommand(Operation::CreateFile, {_username, _token, path, filename});
    } else {
        std::string path = pathToFile.substr(0, lastSlashPos);
        std::string filename = pathToFile.substr(lastSlashPos + 1);
        _commandHandler->SendCommand(Operation::CreateFile, {_username, _token, path, filename});
    }
}

void MainWindow::CreateDirButtonClicked() {
    std::string pathToFile = "root/" + ui->dirCreateLine->text().toStdString();
    size_t lastSlashPos = pathToFile.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        std::string path = "";
        std::string filename = pathToFile;
        _commandHandler->SendCommand(Operation::CreateDir, {_username, _token, path, filename});
    } else {
        std::string path = pathToFile.substr(0, lastSlashPos);
        std::string filename = pathToFile.substr(lastSlashPos + 1);
        _commandHandler->SendCommand(Operation::CreateDir, {_username, _token, path, filename});
    }
}


void MainWindow::OnRoleMessageReceived(const QString& message) {
    ui->roleLine->setText(message);
}

void MainWindow::OnUpdateFileList(const QString& message) {
    std::cout << "files updated" << std::endl;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDocument.isNull()) {
        qWarning("Failed to parse JSON.");
        return ;
    }
    _treeHandler->LoadJsonToTreeView(ui->listTree, jsonDocument);
}

void JsonTreeHandler::LoadJsonToTreeView(QTreeView* treeView, const QJsonDocument& jsonDocument) {
    if (model) {
        delete model;
    }

    model = new QStandardItemModel(treeView);
    model->setHorizontalHeaderLabels({"Name", "Type", "Owner name", "Group name", "Can Read", "Can Write", "Can Execute"});

    QStandardItem* rootItem = model->invisibleRootItem();

    if (jsonDocument.isObject()) {
        QJsonObject rootObject = jsonDocument.object();
        QJsonValue fileSystemValue = rootObject.value("file_system");

        if (fileSystemValue.isArray()) {
            QJsonArray fileSystemArray = fileSystemValue.toArray();
            for (const QJsonValue& file : fileSystemArray) {
                if (file.isObject()) {
                    PopulateTree(rootItem, file.toObject());
                }
            }
        }
    }

    treeView->setModel(model);
    treeView->expandAll();
    treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void JsonTreeHandler::PopulateTree(QStandardItem* parentItem, const QJsonObject& jsonObject) {
    QJsonValue nameValue = jsonObject.value("name");
    QJsonValue typeValue = jsonObject.value("type");
    QJsonValue userNameValue = jsonObject.value("userName");
    QJsonValue groupNameValue = jsonObject.value("groupName");
    QJsonValue canReadValue = jsonObject.value("can_read");
    QJsonValue canWriteValue = jsonObject.value("can_write");
    QJsonValue canExecValue = jsonObject.value("can_exec");

    QStandardItem* nameItem = new QStandardItem(nameValue.toString());
    QStandardItem* typeItem = new QStandardItem(typeValue.toString());
    QStandardItem* userNameItem = new QStandardItem(userNameValue.toString());
    QStandardItem* groupNameItem = new QStandardItem(groupNameValue.toString());
    QStandardItem* canReadItem = new QStandardItem(canReadValue.toString());
    QStandardItem* canWriteItem = new QStandardItem(canWriteValue.toString());
    QStandardItem* canExecItem = new QStandardItem(canExecValue.toString());

    QIcon folderIcon(":/icons/folder.png");
    QIcon fileIcon(":/icons/file.png");

    if (typeValue.toString() == "DIR") {
        QStandardItem* folderItem = new QStandardItem(nameValue.toString());
        folderItem->setData("DIR", Qt::UserRole);
        folderItem->setIcon(folderIcon);
        parentItem->appendRow({folderItem, typeItem, userNameItem, groupNameItem, canReadItem, canWriteItem, canExecItem});

        QJsonValue filesValue = jsonObject.value("files");
        if (filesValue.isArray()) {
            QJsonArray filesArray = filesValue.toArray();
            for (const QJsonValue& file : filesArray) {
                if (file.isObject()) {
                    PopulateTree(folderItem, file.toObject());
                }
            }
        }
    } else if (typeValue.toString() == "FILE") {
        nameItem->setIcon(fileIcon);
        parentItem->appendRow({nameItem, typeItem, userNameItem, groupNameItem, canReadItem, canWriteItem, canExecItem});
    }
}

void MainWindow::ShowContextMenu(QPoint pos) {
    QMenu* menu = new QMenu(this);
    QAction* updateFiles = new QAction(tr("Обновить"), this);
    connect(updateFiles, SIGNAL(triggered()), this, SLOT(NeedUpdateFileList()));
    menu->addAction(updateFiles);
    menu->popup(ui->listTree->viewport()->mapToGlobal(pos));
}

void MainWindow::NeedUpdateFileList() {
    _commandHandler->SendCommand(Operation::GetFileList, {_username, _token});
}

JsonTreeHandler::JsonTreeHandler() : model(nullptr) {}

JsonTreeHandler::~JsonTreeHandler() {
    if (model) {
        delete model;
    }
}

