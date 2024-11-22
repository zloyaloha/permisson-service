#include "main_window.h"

MainWindow::MainWindow(std::shared_ptr<CommandHandler> commandor, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), _commandHandler(commandor), _treeHandler(std::make_shared<JsonTreeHandler>())
{
    ui->setupUi(this);
    connect(_commandHandler.get(), &CommandHandler::GetRoleMessageReceived, this, &MainWindow::OnRoleMessageReceived);
    connect(_commandHandler.get(), &CommandHandler::UpdateFileList, this, &MainWindow::OnUpdateFileList);
    connect(ui->createFileButton, &QPushButton::clicked, this, &MainWindow::CreateFileButtonClicked);
}

MainWindow::~MainWindow() {
    std::cout << "плохо" << std::endl;
}

void MainWindow::SetupWindow(const QString& username, const QString& token) {
    _token = token.toStdString();
    _username = username.toStdString();
    ui->usernameLine->setText(username);
    _commandHandler->StartAsyncReading();
    _commandHandler->SendCommand(Operation::GetRole, {_username, _token});
    _commandHandler->SendCommand(Operation::GetFileList, {_username, _token});
    this->show();
}

void MainWindow::CreateFileButtonClicked() {
    std::string pathToFile = ui->createFile->text().toStdString();
    size_t lastSlashPos = pathToFile.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        std::string path = "";
        std::string filename = pathToFile;
        std::cout << "Path: " << path << ", Filename: " << filename << std::endl;
        _commandHandler->SendCommand(Operation::CreateFile, {_username, _token, path, filename});
    } else {
        std::string path = pathToFile.substr(0, lastSlashPos);
        std::string filename = pathToFile.substr(lastSlashPos + 1);
        std::cout << "Path: " << path << ", Filename: " << filename << std::endl;
        _commandHandler->SendCommand(Operation::CreateFile, {_username, _token, path, filename});
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
    _treeHandler->loadJsonToTreeView(ui->listTree, jsonDocument);
}

void JsonTreeHandler::loadJsonToTreeView(QTreeView* treeView, const QJsonDocument& jsonDocument) {
    if (model) {
        delete model;
    }

    model = new QStandardItemModel(treeView);
    model->setHorizontalHeaderLabels({"Name", "Type", "Can Read", "Can Write"});  // Заголовки для каждой колонки

    QStandardItem* rootItem = model->invisibleRootItem();

    // Проверяем, что jsonDocument является объектом
    if (jsonDocument.isObject()) {
        QJsonObject rootObject = jsonDocument.object();
        QJsonValue fileSystemValue = rootObject.value("file_system");

        // Если "file_system" - это массив
        if (fileSystemValue.isArray()) {
            QJsonArray fileSystemArray = fileSystemValue.toArray();
            for (const QJsonValue& file : fileSystemArray) {
                if (file.isObject()) {
                    populateTree(rootItem, file.toObject());  // Обрабатываем каждый элемент в массиве "file_system"
                }
            }
        }
    }

    treeView->setModel(model);
    treeView->expandAll();
}

void JsonTreeHandler::populateTree(QStandardItem* parentItem, const QJsonObject& jsonObject) {
    // Проверяем наличие параметров, таких как "name", "type", "can_read", "can_write"
    QJsonValue nameValue = jsonObject.value("name");
    QJsonValue typeValue = jsonObject.value("type");
    QJsonValue canReadValue = jsonObject.value("can_read");
    QJsonValue canWriteValue = jsonObject.value("can_write");

    // Если все параметры есть, добавляем их в одну строку
    if (nameValue.isString() && typeValue.isString() && canReadValue.isString() && canWriteValue.isString()) {
        QStandardItem* nameItem = new QStandardItem(nameValue.toString());
        QStandardItem* typeItem = new QStandardItem(typeValue.toString());
        QStandardItem* canReadItem = new QStandardItem(canReadValue.toString());
        QStandardItem* canWriteItem = new QStandardItem(canWriteValue.toString());

        // Если это директория, добавляем ее как узел
        if (typeValue.toString() == "DIR") {
            QStandardItem* folderItem = new QStandardItem(nameValue.toString());
            folderItem->setData("DIR", Qt::UserRole);  // Отметим, что это директория
            parentItem->appendRow({folderItem, typeItem, canReadItem, canWriteItem});  // Добавляем как дочерний элемент родителя

            // Рекурсивно добавляем вложенные файлы и каталоги
            QJsonValue filesValue = jsonObject.value("files");
            if (filesValue.isArray()) {
                QJsonArray filesArray = filesValue.toArray();
                for (const QJsonValue& file : filesArray) {
                    if (file.isObject()) {
                        // Добавляем файл в текущую директорию
                        populateTree(folderItem, file.toObject());
                    }
                }
            }
        } else if (typeValue.toString() == "FILE") {
            // Если это файл, добавляем его как лист
            parentItem->appendRow({nameItem, typeItem, canReadItem, canWriteItem});  // Добавляем как дочерний элемент родителя
        }
    }
}

void JsonTreeHandler::populateArray(QStandardItem* parentItem, const QJsonArray& jsonArray) {
    for (const auto& value : jsonArray) {
        QStandardItem* arrayItem = new QStandardItem(value.toString());
        parentItem->appendRow(arrayItem);

        if (value.isObject()) {
            populateTree(arrayItem, value.toObject());
        } else if (value.isArray()) {
            populateArray(arrayItem, value.toArray());
        }
    }
}

JsonTreeHandler::JsonTreeHandler() : model(nullptr) {}

JsonTreeHandler::~JsonTreeHandler() {
    if (model) {
        delete model;
    }
}

