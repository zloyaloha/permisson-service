#include "main_window.h"

MainWindow::MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *parent): 
    QMainWindow(parent), ui(new Ui::MainWindow), 
    _commandHandler(commandor), _treeHandler(std::make_shared<JsonTreeHandler>()), _usersListHandler(std::make_shared<JsonUserListHandler>()),
    _groupsTreeHandler(std::make_shared<JsonGroupTreeHandler>())
{
    ui->setupUi(this);
    ui->listTree->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->groupsTree->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(_commandHandler.get(), &CommandHandler::GetRoleMessageReceived, this, &MainWindow::OnRoleMessageReceived);
    connect(_commandHandler.get(), &CommandHandler::UpdateFileList, this, &MainWindow::OnUpdateFileList);
    connect(_commandHandler.get(), &CommandHandler::GetUsersList, this, &MainWindow::OnGetUsersList);
    connect(_commandHandler.get(), &CommandHandler::GetGroupsList, this, &MainWindow::OnGetGroupsList);

    connect(_commandHandler.get(), &CommandHandler::OperationWithFile, this, &MainWindow::OnOperationWithFile);
    connect(_commandHandler.get(), &CommandHandler::OperationWithGroup, this, &MainWindow::OnOperationWithGroup);

    connect(ui->createFileButton, &QPushButton::clicked, this, &MainWindow::CreateFileButtonClicked);
    connect(ui->dirCreateButton, &QPushButton::clicked, this, &MainWindow::CreateDirButtonClicked);
    connect(ui->groupCreateButton, &QPushButton::clicked, this, &MainWindow::CreateGroupButtonClicked);
    connect(ui->listTree, SIGNAL(customContextMenuRequested(QPoint)), SLOT(ShowContextMenu(QPoint)));
    connect(ui->groupsTree, &QTreeView::customContextMenuRequested, this, &MainWindow::ShowContextMenuGroups);


    ui->listTree->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QFile file(":/styles/styles.qss");

    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QString styleSheet = QLatin1String(file.readAll());
        ui->listTree->setStyleSheet(styleSheet);
        ui->groupsTree->setStyleSheet(styleSheet);
        file.close();
    } else {
        qWarning() << "Failed to load stylesheet from";
    }

}

MainWindow::~MainWindow() {}

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
    if (ui->createFile->text().isEmpty()) {
        return;
    }
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

void MainWindow::CreateGroupButtonClicked() {
    std::string groupName = ui->groupCreate->text().toStdString();
    if (groupName != "") {
        _commandHandler->SendCommand(Operation::CreateGroup, {_username, _token, groupName});
    }
}

void MainWindow::CreateDirButtonClicked() {
    std::string pathToFile = "root/" + ui->dirCreateLine->text().toStdString();
    if (ui->dirCreateLine->text().isEmpty()) {
        return;
    }
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
    if (message == "common") {
        ui->usersList->hide();
    } else if (message == "admin") {
        _commandHandler->SendCommand(Operation::GetUsersList, {_username, _token});
        _commandHandler->SendCommand(Operation::GetGroupsList, {_username, _token});
    }
}

void MainWindow::OnOperationWithFile(const QString& message) {
    if (message == "Success") {
        _commandHandler->SendCommand(Operation::GetFileList, {_username, _token});
    } else if (message == "No access") {
        ui->statusbar->showMessage("Have not enough rights");
    } else if (message == "File exists") {
        ui->statusbar->showMessage("File with this name already exists");
    }
    ui->createFile->clear();
    ui->dirCreateLine->clear();
}

void MainWindow::OnOperationWithGroup(const QString& message) {
    qDebug() << message;
    if (message == "Success") {
        _commandHandler->SendCommand(Operation::GetGroupsList, {_username, _token});
    } else if (message == "No access") {
        ui->statusbar->showMessage("Have not enough rights");
    } else if (message == "User Not Found") {
        ui->statusbar->showMessage("User with this login isn't exist");
    } else if (message == "Already Exists") {
        ui->statusbar->showMessage("Group alredy exists");
    }
}

void MainWindow::OnUpdateFileList(const QString& message) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDocument.isNull()) {
        qWarning("Failed to parse JSON.");
        return ;
    }
    _treeHandler->LoadJsonToTreeView(ui->listTree, jsonDocument);
}

void MainWindow::OnGetUsersList(const QString& response) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(response.toUtf8());
    if (jsonDocument.isNull()) {
        qWarning("Failed to parse JSON.");
        return ;
    }
    _usersListHandler->LoadJsonToTableView(ui->usersList, jsonDocument);
}


void MainWindow::OnGetGroupsList(const QString& response) {
    QJsonDocument jsonDocument = QJsonDocument::fromJson(response.toUtf8());
    if (jsonDocument.isNull()) {
        qWarning("Failed to parse JSON.");
        return ;
    }
    _groupsTreeHandler->LoadJsonToTreeView(ui->groupsTree, jsonDocument);
}

void JsonUserListHandler::LoadJsonToTableView(QTableView* usersListView, const QJsonDocument& jsonDocument) {
    if (usersModel) {
        delete usersModel;
    }
    usersModel = new QStandardItemModel(usersListView);
    usersModel->setHorizontalHeaderLabels({"Username", "Role", "Active"});
    if (jsonDocument.isObject()) {
        QJsonObject rootObject = jsonDocument.object();
        QJsonValue usersValue = rootObject.value("users");
        if (usersValue.isArray()) {
            QJsonArray usersArray = usersValue.toArray();
            for (const QJsonValue& user : usersArray) {
                if (user.isObject()) {
                    AddUser(user.toObject());
                }
            }
        }
    }
    usersListView->setModel(usersModel);
    usersListView->resizeColumnsToContents();
    usersListView->horizontalHeader()->setStretchLastSection(true);
    usersListView->show();
}

void JsonUserListHandler::AddUser(const QJsonObject& user) {
    QJsonValue nameValue = user.value("username");
    QJsonValue isAdminValue = user.value("is_admin");
    QJsonValue isActiveValue = user.value("is_active");

    QString username = nameValue.toString();
    QString role = isAdminValue.toBool() ? "Admin" : "User";
    QString activeStatus = isActiveValue.toBool() ? "Active" : "Inactive";

    QStandardItem* usernameItem = new QStandardItem(username);
    QStandardItem* isAdminItem = new QStandardItem(role);
    QStandardItem* isActiveItem = new QStandardItem(activeStatus);

    usersModel->appendRow({usernameItem, isAdminItem, isActiveItem});
}

JsonUserListHandler::JsonUserListHandler() : usersModel(nullptr) {}

JsonUserListHandler::~JsonUserListHandler() {
    if (usersModel) {
        delete usersModel;
    }
}

void JsonTreeHandler::LoadJsonToTreeView(QTreeView* treeView, const QJsonDocument& jsonDocument) {
    if (model) {
        delete model;
    }

    model = new QStandardItemModel(treeView);
    model->setHorizontalHeaderLabels({"Name", "Type", "Owner name", "Group name", "Permissions"});

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
    QJsonValue permissionsValue = jsonObject.value("permissions");

    QStandardItem* nameItem = new QStandardItem(nameValue.toString());
    QStandardItem* typeItem = new QStandardItem(typeValue.toString());
    QStandardItem* userNameItem = new QStandardItem(userNameValue.toString());
    QStandardItem* groupNameItem = new QStandardItem(groupNameValue.toString());
    QStandardItem* permissions = new QStandardItem(permissionsValue.toString());

    QIcon folderIcon(":/icons/folder.png");
    QIcon fileIcon(":/icons/file.png");

    if (typeValue.toString() == "DIR") {
        QStandardItem* folderItem = new QStandardItem(nameValue.toString());
        folderItem->setData("DIR", Qt::UserRole);
        folderItem->setIcon(folderIcon);
        parentItem->appendRow({folderItem, typeItem, userNameItem, groupNameItem, permissions});

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
        parentItem->appendRow({nameItem, typeItem, userNameItem, groupNameItem, permissions});
    }
}

void MainWindow::ShowContextMenu(const QPoint& pos) {
    QModelIndex index = ui->listTree->indexAt(pos);
    if (!index.isValid() || index.column() != 0) {
        return;
    }

    int column = 1;
    QModelIndex columnIndex = index.sibling(index.row(), column);

    if (!columnIndex.isValid()) {
        return;
    }

    QVariant data = columnIndex.data();
    qDebug() << "Data from column:" << data.toString();

    std::unordered_map<std::string, QAction *> actions;
    actions.emplace("Update", new QAction(tr("Обновить"), this));
    actions.emplace("Delete", new QAction(tr("Удалить"), this));
    actions.emplace("Add", new QAction(tr("Добавить в группу"), this));
    actions.emplace("Change_rights", new QAction(tr("Изменить права"), this));
    QMenu* menu = new QMenu(this);

    connect(actions["Add"], &QAction::triggered, this, [this, index]() {
        std::string fileName = index.data().toString().toStdString();
        QDialog dialog(this);
        dialog.setWindowTitle("Добавление в группу");
        QVBoxLayout* layout = new QVBoxLayout(&dialog);
        QLabel* label = new QLabel("Введите имя группы", &dialog);
        layout->addWidget(label);
        QLineEdit* lineEdit = new QLineEdit(&dialog);
        layout->addWidget(lineEdit);
        QPushButton* submitButton = new QPushButton("Подтвердить", &dialog);
        layout->addWidget(submitButton);

        connect(submitButton, &QPushButton::clicked, &dialog, [&dialog, lineEdit, fileName, this]() {
            QString enteredGroup = lineEdit->text();
            if (!enteredGroup.isEmpty()) {
                _commandHandler->SendCommand(Operation::AddFileToGroup, {_username, _token, fileName, enteredGroup.toStdString()});
            }
            dialog.accept();
        });

        dialog.setLayout(layout);
        dialog.exec();
    });

    connect(actions["Change_rights"], &QAction::triggered, this, [this, index]() {
        std::string fileName = index.data().toString().toStdString();
        QDialog dialog(this);
        dialog.setWindowTitle("Изменение прав");
        QVBoxLayout* verticalLayout = new QVBoxLayout(&dialog);

        QGridLayout *layout = new QGridLayout(this);
        QLabel *headerUser = new QLabel("Права пользователя");
        QLabel *headerGroup = new QLabel("Права группы");
        QLabel *headerAll = new QLabel("Права всех");

        layout->addWidget(new QLabel(""), 0, 0);
        layout->addWidget(headerUser, 1, 0);
        layout->addWidget(headerGroup, 2, 0);
        layout->addWidget(headerAll, 3, 0);

        QLabel *readLabel = new QLabel("Чтение");
        QLabel *writeLabel = new QLabel("Запись");
        QLabel *execLabel = new QLabel("Выполнение");

        layout->addWidget(readLabel, 0, 1);
        layout->addWidget(writeLabel, 0, 2);
        layout->addWidget(execLabel, 0, 3);

        // Чекбоксы
        QCheckBox *userRead = new QCheckBox();
        QCheckBox *groupRead = new QCheckBox();
        QCheckBox *allRead = new QCheckBox();

        QCheckBox *userWrite = new QCheckBox();
        QCheckBox *groupWrite = new QCheckBox();
        QCheckBox *allWrite = new QCheckBox();

        QCheckBox *userExec = new QCheckBox();
        QCheckBox *groupExec = new QCheckBox();
        QCheckBox *allExec = new QCheckBox();

        layout->addWidget(userRead, 1, 1);
        layout->addWidget(groupRead, 1, 2);
        layout->addWidget(allRead, 1, 3);

        layout->addWidget(userWrite, 2, 1);
        layout->addWidget(groupWrite, 2, 2);
        layout->addWidget(allWrite, 2, 3);

        layout->addWidget(userExec, 3, 1);
        layout->addWidget(groupExec, 3, 2);
        layout->addWidget(allExec, 3, 3);

        QPushButton* submitButton = new QPushButton("Подтвердить", &dialog);
        verticalLayout->addLayout(layout);
        verticalLayout->addWidget(submitButton);

        connect(submitButton, &QPushButton::clicked, &dialog, [&dialog, layout, fileName, this]() {
            QString permissions = "";
            for (int row = 1; row <= 3; ++row) {
                for (int col = 1; col <= 3; ++col) {
                    QCheckBox* checkBox = qobject_cast<QCheckBox*>(layout->itemAtPosition(row, col)->widget());
                    if (checkBox) {
                        if (checkBox->isChecked()) {
                            if (col == 1) permissions += "r";
                            else if (col == 2) permissions += "w";
                            else if (col == 3) permissions += "x";
                        } else {
                            permissions += "-";
                        }
                    }
                }
            }
            _commandHandler->SendCommand(Operation::ChangeRights, {_username, _token, fileName, permissions.toStdString()});
            dialog.accept();
        });

        dialog.setLayout(layout);
        dialog.exec();
    });

    connect(actions["Update"], SIGNAL(triggered()), this, SLOT(NeedUpdateFileList()));
    connect(actions["Delete"], &QAction::triggered, [this, index]() { DeleteFile(index.data(Qt::DisplayRole).toString()); });
    menu->addAction(actions["Add"]);
    menu->addAction(actions["Update"]);
    menu->addAction(actions["Delete"]);
    menu->addAction(actions["Change_rights"]);
    menu->popup(ui->listTree->viewport()->mapToGlobal(pos));
}

void MainWindow::ShowContextMenuGroups(const QPoint& pos) {
    QModelIndex index = ui->groupsTree->indexAt(pos);
    if (!index.isValid()) return;

    if (index.column() == 0 && !index.data().toString().isEmpty()) {
        QMenu contextMenu;
        QAction* addUser = contextMenu.addAction("Добавить пользователя");
        QAction* deleteGroup = contextMenu.addAction("Удалить группу");

        connect(addUser, &QAction::triggered, this, [this, index]() {
            std::string groupName = index.data().toString().toStdString();

            QDialog dialog(this);
            dialog.setWindowTitle("Добавление пользователя");
            QVBoxLayout* layout = new QVBoxLayout(&dialog);
            QLabel* label = new QLabel("Введите имя пользователя", &dialog);
            layout->addWidget(label);
            QLineEdit* lineEdit = new QLineEdit(&dialog);
            layout->addWidget(lineEdit);
            QPushButton* submitButton = new QPushButton("Подтвердить", &dialog);
            layout->addWidget(submitButton);

            connect(submitButton, &QPushButton::clicked, &dialog, [&dialog, lineEdit, groupName, this]() {
                QString enteredUser = lineEdit->text();
                if (!enteredUser.isEmpty()) {
                    _commandHandler->SendCommand(Operation::AddUserToGroup, {_username, _token, groupName, enteredUser.toStdString()});
                }
                dialog.accept();
            });

            dialog.setLayout(layout);
            dialog.exec();
        });

        connect(deleteGroup, &QAction::triggered, this, [this, index]() {
            std::string groupName = index.data().toString().toStdString();
            _commandHandler->SendCommand(Operation::DeleteGroup, {_username, _token, groupName});
        });

        contextMenu.exec(ui->groupsTree->viewport()->mapToGlobal(pos));
    }
}

void MainWindow::NeedUpdateFileList() {
    _commandHandler->SendCommand(Operation::GetFileList, {_username, _token});
}

void MainWindow::DeleteFile(const QString& filename) {
    std::string file = filename.toStdString();
    _commandHandler->SendCommand(Operation::DeleteFile, {_username, _token, file});
}

JsonTreeHandler::JsonTreeHandler() : model(nullptr) {}

JsonTreeHandler::~JsonTreeHandler() {
    if (model) {
        delete model;
    }
}

void JsonGroupTreeHandler::LoadJsonToTreeView(QTreeView* groupTreeView, const QJsonDocument& jsonDocument) {
    if (groupsModel) {
        delete groupsModel;
    }
    groupsModel = new QStandardItemModel(groupTreeView);
    groupsModel->setHorizontalHeaderLabels({"Group Name", "User"});
    if (jsonDocument.isObject()) {
        QJsonObject rootObject = jsonDocument.object();
        QJsonValue groupsValue = rootObject.value("groups");
        if (groupsValue.isArray()) {
            QJsonArray groupsArray = groupsValue.toArray();
            for (const QJsonValue& group: groupsArray) {
                if (group.isObject()) {
                    AddGroup(group.toObject());
                }
            }
        }
    }
    groupTreeView->setModel(groupsModel);
    groupTreeView->expandAll();
    groupTreeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void JsonGroupTreeHandler::AddGroup(const QJsonObject& group) {
    QJsonValue nameValue = group.value("group_name");
    QString groupName = nameValue.toString();

    QStandardItem* groupItem = new QStandardItem(groupName);
    QStandardItem* emptyItem = new QStandardItem();

    groupsModel->appendRow({groupItem, emptyItem});

    QJsonValue usersValue = group.value("users");
    if (usersValue.isArray()) {
        QJsonArray usersArray = usersValue.toArray();
        for (const QJsonValue& user : usersArray) {
            if (user.isObject()) {
                QJsonObject userObject = user.toObject();
                QString username = userObject.value("username").toString();

                QStandardItem* userGroupItem = new QStandardItem();
                QStandardItem* userItem = new QStandardItem(username);

                groupItem->appendRow({userGroupItem, userItem});
            }
        }
    }
}

JsonGroupTreeHandler::JsonGroupTreeHandler() : groupsModel(nullptr) {}

JsonGroupTreeHandler::~JsonGroupTreeHandler() {
    if (groupsModel) {
        delete groupsModel;
    }
}

