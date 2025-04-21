#include "server.h"

namespace bp = boost::process;

void Server::AcceptConnections() {
    NotifyObservers("Server is waiting!");
    _acceptor.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                NotifyObservers("Client connected!");
                auto socketPtr = std::make_shared<tcp::socket>(std::move(socket));
                std::make_shared<Session>(socketPtr, _observers, _threadPool, _config)->Start();
            } else {
                NotifyObservers("ERROR while connecting" + ec.message());
            }
            AcceptConnections();
        });
}

Server::Server(boost::asio::io_context& io_context, const Config& conf)
    : _acceptor(io_context, tcp::endpoint(tcp::v4(), conf.PORT)), _threadPool(NUMBER_OF_THREADS), _config(conf) {}

Session::Session(std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers, ThreadPool& threadPool, const Config& conf) 
    : _socket(std::move(socket)), _threadPool(threadPool), _worker(std::make_shared<Worker>(threadPool, _socket, _observers, conf)), Observable(observers) {}

void Session::Start() {
    ReadMessage();
}

void Session::ReadMessage() {
    auto self(shared_from_this());
    boost::asio::async_read_until(*_socket, _buffer, TERMINATING_STRING,
        [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
            if (!ec) {
                std::istream is(&_buffer);
                std::string data;
                std::getline(is, data, '\0');
                _buffer.consume(bytes_transferred);
                BaseCommand command(data);
                if (command._op == Operation::Quit) {
                    if (command._msg_data[0] != "") { // logged connection
                        _worker->ProccessOperation(command);
                    }
                    _socket->close();
                    return;
                }
                NotifyObservers("Сообщение получено\n");
                ReadMessage();
                _threadPool.EnqueueTask([this, command] { _worker->ProccessOperation(command); });
            } else {
                NotifyObservers("Ошибка при получении сообщения " + ec.message());
                boost::system::error_code ignored_ec;
                _socket->close(ignored_ec);
                NotifyObservers("Соединение закрыто");
            }
        });
}

pqxx::result Worker::MakeQuery(const std::string& query) {
    pqxx::result result;
    try {
        pqxx::work work(*_connection);
        result = work.exec(query);
        work.commit();
    } catch (std::string error) {
        NotifyObservers("Error: " + error);
    }
    return result;
}

Worker::Worker(ThreadPool& threadPool, std::shared_ptr<tcp::socket> socket, std::vector<std::shared_ptr<IServerObserver>>& observers, const Config& conf)
    : _threadPool(threadPool), _socket(socket), Observable(observers), _config(conf)
{
    QString PARAM_STRING = "host=" + conf.DB_HOST +
        " port=" + conf.DB_PORT +
        " dbname=" + conf.DB_NAME +
        " user=" + conf.DB_USER +
        " password=" + conf.DB_PASSWORD;
    _connection = std::make_unique<pqxx::connection>(PARAM_STRING.toStdString());
    _connectionRedis = std::make_unique<sw::redis::Redis>("tcp://127.0.0.1:6379");
    PrepareQueries();
}

void Worker::PrepareQueries() {
    _connection->prepare("add_file_to_group",
        "SELECT permission_app.AddFileToGroup($1, $2, $3)");
    _connection->prepare("is_user_owner_or_root",
        "SELECT permission_app.IsUserOwnerOrRoot($1, $2)");
    _connection->prepare("is_root",
        "SELECT permission_app.IsRoot($1)");
    _connection->prepare("check_user_rights_to_write",
        "SELECT permission_app.CheckUserRightsToWrite($1, $2)");
    _connection->prepare("check_user_rights_to_read",
        "SELECT permission_app.CheckUserRightsToRead($1, $2)");
    _connection->prepare("check_user_rights_to_exec",
        "SELECT permission_app.CheckUserRightsToExec($1, $2)");
    _connection->prepare("add_event",
        "SELECT permission_app.AddEvent($1, $2, $3)");
    _connection->prepare("update_user_permissions_by_mask",
        "SELECT permission_app.UpdateUserPermissionsByMask($1, $2)");
    _connection->prepare("update_group_permissions_by_mask",
        "SELECT permission_app.UpdateGroupPermissionsByMask($1, $2)");
    _connection->prepare("update_all_permissions_by_mask",
        "SELECT permission_app.UpdateAllPermissionsByMask($1, $2)");
    _connection->prepare("delete_group",
        "SELECT permission_app.DeleteGroup($1, $2)");
    _connection->prepare("create_group",
        "SELECT permission_app.CreateGroup($1, $2)");
    _connection->prepare("add_user_to_group",
        "SELECT permission_app.AddUserToGroup($1, $2)");
    _connection->prepare("get_group_list",
        "SELECT * FROM permission_app.GetGroupList()");
    _connection->prepare("get_users_list",
        "SELECT * FROM permission_app.GetUsersWithStatus()");
    _connection->prepare("get_files_list",
        "SELECT * FROM permission_app.GetFileTreeWithPermissions()");
    _connection->prepare("delete_file",
        "SELECT permission_app.DeleteFileByNameAndUser($1, $2)");
    _connection->prepare("create_file",
        "SELECT permission_app.AddFileToPath($1, $2, $3)");
    _connection->prepare("create_dir",
        "SELECT permission_app.AddDirectoryToPath($1, $2, $3)");
    _connection->prepare("quit",
        "CALL permission_app.UpdateExitTime($1)");
    _connection->prepare("get_salt_and_password",
        "SELECT permission_app.GetSaltAndPassword($1)");
    _connection->prepare("create_session",
        "SELECT permission_app.AddSession($1)");
    _connection->prepare("get_user_id",
        "SELECT permission_app.UserId($1)");
    _connection->prepare("create_user",
        "CALL permission_app.CreateUser($1, $2, $3)");
    _connection->prepare("add_read_event",
        "CALL permission_app.AddReadEvent($1, $2)");
    _connection->prepare("add_write_event",
        "CALL permission_app.AddWriteEvent($1, $2)");
    _connection->prepare("add_exec_event",
        "CALL permission_app.AddExecEvent($1, $2)");
    _connection->prepare("stop_active_sessions",
        "SELECT permission_app.DeleteActiveSessions()");
    _connection->prepare("get_token",
        "SELECT permission_app.GetToken($1)");
}

void Worker::StopActiveSession() {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("stop_active_sessions");
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return;
    }
    work.commit();
    return;
}

void Worker::SendResponse(const Operation op, const std::initializer_list<std::string>& data) {
    auto self(shared_from_this());
    BaseCommand msg(op, data);
    std::string abc = msg.toPacket();
    boost::asio::async_write(*_socket, boost::asio::buffer(msg.toPacket()),
        [this, self](boost::system::error_code ec, std::size_t writed) {
            if (ec) {
                NotifyObservers("Ошибка при отправке ответа " + ec.message());
                _socket->close();
            } else {
                NotifyObservers("Ответ отправлен " + std::to_string(writed));
            }
        });
}

bool Worker::ValidateRequest(const std::string& username, const std::string& token) {

    auto user_id_opt = _connectionRedis->get("auth:" + token);

    if (user_id_opt) {
        std::string user_id = *user_id_opt;
        std::cout << "Authorized user_id = " << user_id << std::endl;
        // Тут можно обновить last_activity в PostgreSQL:
        // txn.exec_params("UPDATE permission_app.sessions SET last_activity = NOW() WHERE session_token = $1", token);
        return true;
    } else {
        return false;
        // Токен не найден в Redis — значит истёк или удалён
        std::cerr << "Unauthorized: invalid or expired token" << std::endl;
    }
}

std::string Worker::GetToken(const std::string& username) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("get_token", username);
        return GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "";
    }
    work.commit();
    return "";
}


void Worker::ProccessOperation(const BaseCommand &command) {
    static std::string result;
    PrintActiveSessions();
    switch (command._op) {
        case Operation::Registrate:
            NotifyObservers("Registrate " + command._msg_data[0]);
            result = Registrate(command._msg_data[0], command._msg_data[1]); // login, password
            SendResponse(command._op, {result});
            break;
        case Operation::Login:
            NotifyObservers("Log in " + command._msg_data[0]);
            result = Login(command._msg_data[0], command._msg_data[1]); // login, password
            SendResponse(command._op, {result});
            break;
        case Operation::Quit:
            NotifyObservers("Quit " + command._msg_data[0]);
            _connection->disconnect();
            break;
         case Operation::QuitSession:
            NotifyObservers("QuitSession " + command._msg_data[0]);
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            Quit(command._msg_data[1], command._msg_data[0]); // token, username
            SendResponse(command._op, {command._msg_data[1]});
            break;
        case Operation::GetRole:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("Get role " + command._msg_data[0]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = GetRole(command._msg_data[0]); // username
            SendResponse(command._op, {result});
            break;
        case Operation::CreateFile:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("Create file " + command._msg_data[0] + ' ' + command._msg_data[2] + ' ' + command._msg_data[3]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = CreateFile(command._msg_data[0], command._msg_data[2], command._msg_data[3]); // username, path, filename
            SendResponse(command._op, {result});
            break;
        case Operation::CreateDir:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("Create file " + command._msg_data[0] + ' ' + command._msg_data[2] + ' ' + command._msg_data[3]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = CreateDir(command._msg_data[0], command._msg_data[2], command._msg_data[3]); // username, path, filename
            SendResponse(command._op, {result});
            break;
        case Operation::DeleteFile:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("Delete file " + command._msg_data[2]); // filename
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!IsRootOrOwner(command._msg_data[0], command._msg_data[2])) {
                result = "No access";
            } else {
                result = DeleteFile(command._msg_data[0], command._msg_data[2]);
            }
            SendResponse(command._op, {result});
            break;
        case Operation::GetFileList:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("GetFileList file " + command._msg_data[0]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = GetFileList();
            SendResponse(command._op, {result});
            break;
        case Operation::GetUsersList:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("GetUsersList " + command._msg_data[0]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = GetUsersList();
            SendResponse(command._op, {result});
            break;
        case Operation::GetGroupsList:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("GetGroupsList " + command._msg_data[0]); // username
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = GetGroupsList();
            SendResponse(command._op, {result});
            break;
        case Operation::AddUserToGroup:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("AddUserToGroup " + command._msg_data[2]); // group
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // acceptor username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!IsRoot(command._msg_data[0])) {
                result = "No access";
            } else {
                result = AddUserToGroup(command._msg_data[2], command._msg_data[3]); // group, username
            }
            SendResponse(command._op, {result});
            break;
        case Operation::CreateGroup:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("CreateGroup " + command._msg_data[2]); // group
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // acceptor username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!IsRoot(command._msg_data[0])) {
                result = "No access";
            }
            result = CreateGroup(command._msg_data[2], command._msg_data[0]); // group, username
            SendResponse(command._op, {result});
            break;
        case Operation::DeleteGroup:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("DeleteGroup " + command._msg_data[2]); // group
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = DeleteGroup(command._msg_data[2], command._msg_data[0]); // group, username
            SendResponse(command._op, {result});
            break;
        case Operation::AddFileToGroup:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("AddFileToGroup " + command._msg_data[2] + ' ' + command._msg_data[3]); // file, group
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            result = AddFileToGroup(command._msg_data[2], command._msg_data[3], command._msg_data[0]); // file, group, username
            SendResponse(command._op, {result});
            break;
        case Operation::ChangeRights:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("ChangeRights " + command._msg_data[2] + ' ' + command._msg_data[3]); // file, permissions
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!IsRootOrOwner(command._msg_data[0], command._msg_data[2])) {
                result = "No access";
            } else {
                result = ChangeRights(command._msg_data[2], command._msg_data[3], command._msg_data[0]); // file, permissions, username
            }
            SendResponse(command._op, {result});
            break;
        case Operation::WriteToFile:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("WriteToFile " + command._msg_data[0] + ' ' + command._msg_data[2]); // file, permissions
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!CheckUserRightsToWrite(command._msg_data[0], command._msg_data[2]) && !IsRoot(command._msg_data[0])) { // username, file
                result = "No access";
            } else {
                AddWriteEvent(command._msg_data[0], command._msg_data[2]);
                result = "Success";
            }
            SendResponse(command._op, {result});
            break;
        case Operation::ReadFromFile:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("ReadFromFile " + command._msg_data[0] + ' ' + command._msg_data[2]); // file, permissions
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!CheckUserRightsToRead(command._msg_data[0], command._msg_data[2]) && !IsRoot(command._msg_data[0])) { // username, file
                result = "No access";
            } else {
                AddReadEvent(command._msg_data[0], command._msg_data[2]);
                result = "Success";
            }
            SendResponse(command._op, {result});
            break;
        case Operation::ExecFile:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("ExecFile " + command._msg_data[0] + ' ' + command._msg_data[2]); // file, permissions
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!CheckUserRightsToExec(command._msg_data[0], command._msg_data[2]) && !IsRoot(command._msg_data[0])) { // username, file
                result = "No access";
            } else {
                AddExecEvent(command._msg_data[0], command._msg_data[2]);
                result = "Success";
            }
            SendResponse(command._op, {result});
            break;
        case Operation::MakeDBCopy:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("MakeDBCopy " + command._msg_data[0] + ' ' + command._msg_data[2]); // user, file
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!IsRoot(command._msg_data[0])) { // username
                result = "No access";
            } else {
                result = CreateBackup(command._msg_data[2]); // username, file
            }
            SendResponse(command._op, {result});
            break;
        case Operation::RecoverDB:
            UpdateActivity(command._msg_data[0]);
            NotifyObservers("RecoverDB " + command._msg_data[0] + ' ' + command._msg_data[2]); // user, file
            if (!ValidateRequest(command._msg_data[0], command._msg_data[1])) { // username, token
                NotifyObservers("Security warning");
                break;
            }
            if (!IsRoot(command._msg_data[0])) { // username
                result = "No access";
            } else {
                result = RecoverDB(command._msg_data[2]); // username, file
            }
            SendResponse(command._op, {result});
            break;
    }
    NotifyObservers("Result: " + result);
}

std::string Worker::CreateBackup(const std::string& fileName) {
    try {
        bp::environment env = boost::this_process::environment();
        env["PGPASSWORD"] = "sap1234";

        std::vector<std::string> args = {
            "-U", "db_admin", 
            "-h", _config.DB_HOST.toStdString(),
            "-p", _config.DB_PORT.toStdString(), 
            "-d", _config.DB_NAME.toStdString(),
            "-n", "permission_app",
            "-F", "c",
            "-f", fileName,
            "-O"
        };

        bp::child c("/usr/bin/pg_dump", bp::args(args), env);
        c.wait();

        if (c.exit_code() != 0) {
            throw std::runtime_error("Backup failed with exit code: " + std::to_string(c.exit_code()));
        }

        return "Backup created successfully!";
    }
    catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

std::string Worker::RecoverDB(const std::string& fileName) {
    try {
        bp::environment env = boost::this_process::environment();
        env["PGPASSWORD"] = "1234";

        std::vector<std::string> args = {
            "-U", "zloyaloha",
            "-h", _config.DB_HOST.toStdString(),
            "-p", _config.DB_PORT.toStdString(),
            "-d", _config.DB_NAME.toStdString(),
            "-n", "permission_app",
            "--no-owner",
            "-c",
            fileName
        };

        bp::child c("/usr/bin/pg_restore", bp::args(args), env);
        c.wait();

        if (c.exit_code() != 0) {
            throw std::runtime_error("Restore failed with exit code: " + std::to_string(c.exit_code()));
        }
        StopActiveSession();
        return "Success";
    }
    catch (const std::exception& e) {
        return "Error";
    }
}

void Worker::AddWriteEvent(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("add_write_event", userName, fileName);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return;
    }
    work.commit();
    return;
}

void Worker::AddReadEvent(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("add_read_event", userName, fileName);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return;
    }
    work.commit();
    return;
}

void Worker::AddExecEvent(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("add_exec_event", userName, fileName);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return;
    }
    work.commit();
    return;
}

bool Worker::CheckUserRightsToWrite(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("check_user_rights_to_write", userName, fileName);
        if (!result.empty() && result[0][0].is_null() == false) {
            return result[0][0].as<bool>();
        }
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return false;
    }
    return false;
}

bool Worker::CheckUserRightsToRead(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("check_user_rights_to_read", userName, fileName);
        if (!result.empty() && result[0][0].is_null() == false) {
            return result[0][0].as<bool>();
        }
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return false;
    }
    return false;
}

bool Worker::CheckUserRightsToExec(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("check_user_rights_to_exec", userName, fileName);
        if (!result.empty() && result[0][0].is_null() == false) {
            return result[0][0].as<bool>();
        }
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return false;
    }
    return false;
}

bool Worker::IsRoot(const std::string& userName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("is_root", userName);
        if (!result.empty() && result[0][0].is_null() == false) {
            return result[0][0].as<bool>();
        }
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return false;
    }
    return false;
}

bool Worker::IsRootOrOwner(const std::string& userName, const std::string& fileName) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("is_user_owner_or_root", userName, fileName);
        if (!result.empty() && result[0][0].is_null() == false) {
            return result[0][0].as<bool>();
        }
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return false;
    }
    return false;
}

int Worker::SetMask(int start, const std::string& permissions) {
    int mask = 0;
    for (int i = start; i < start + 3; ++i) {
        if (permissions[i] != '-') {
            mask |= (1 << (i % 3));
        }
    }
    return mask;
}

std::string Worker::ChangeRights(const std::string& fileName, const std::string& permissions, const std::string& userName) {
    int mask = SetMask(0, permissions);
    std::string userResult = SetUserRights(fileName, mask);
    mask = SetMask(3, permissions);
    std::string groupResult = SetGroupRights(fileName, mask);
    mask = SetMask(6, permissions);
    std::string allResult = SetAllRights(fileName, mask);
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("add_event", fileName, userName, permissions);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    if (userResult + groupResult + allResult == "ttt") {
        return "Success";
    } else {
        return "Wrong";
    }
}

std::string Worker::SetUserRights(const std::string& fileName, int mask) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("update_user_permissions_by_mask", fileName, mask);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::SetGroupRights(const std::string& fileName, int mask) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("update_group_permissions_by_mask", fileName, mask);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}

std::string Worker::SetAllRights(const std::string& fileName, int mask) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("update_all_permissions_by_mask", fileName, mask);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}

std::string Worker::AddFileToGroup(const std::string& fileName, const std::string& groupName, const std::string& userName) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("add_file_to_group", fileName, groupName, userName);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::DeleteGroup(const std::string& groupName, const std::string& userName) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("delete_group", groupName, userName);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::CreateGroup(const std::string& groupName, const std::string& userName) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("create_group", groupName, userName);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::AddUserToGroup(const std::string& groupName, const std::string& userName) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("add_user_to_group", groupName, userName);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::GetGroupsList() {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("get_group_list");
        QJsonObject groupsList = _jsonHandler.GenerateGroupsList(result);
        QJsonDocument doc(groupsList);
        output = doc.toJson(QJsonDocument::Compact).toStdString();
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    return output;
}


std::string Worker::GetUsersList() {
    std::string output;
    pqxx::work work(*_connection);

    try {
        pqxx::result result = work.exec_prepared("get_users_list");

        QJsonArray usersArray;

        for (const auto& row : result) {
            int user_id = row["id"].as<int>();
            QString username = QString::fromStdString(row["login"].as<std::string>());

            std::string sessionKey = "session:" + std::to_string(user_id);
            bool isOnline = _connectionRedis->exists(sessionKey) > 0;

            QJsonObject userJson;
            userJson["username"] = username;
            userJson["is_admin"] = row["is_admin"].as<bool>();
            userJson["is_active"] = isOnline;

            usersArray.append(userJson);
        }

        QJsonObject usersList;
        usersList["users"] = usersArray;
        QJsonDocument doc(usersList);
        output = doc.toJson(QJsonDocument::Compact).toStdString();

    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }

    return output;
}


std::string Worker::DeleteFile(const std::string& username, const std::string& filename) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("delete_file", username, filename);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::GetFileList() {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("get_files_list");
        QJsonObject filesList = _jsonHandler.GenerateFileTree(result);
        QJsonDocument doc(filesList);
        output = doc.toJson(QJsonDocument::Compact).toStdString();
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    return output;
}


std::string Worker::CreateFile(const std::string& username, const std::string& path, const std::string& filename) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("create_file", path, filename, username);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}


std::string Worker::CreateDir(const std::string& username, const std::string& path, const std::string& filename) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("create_dir", path, filename, username);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    work.commit();
    return output;
}

void Worker::Quit(const std::string& token, const std::string& username) {
    int user_id = UserID(username);

    // Завершение сессии в PostgreSQL
    try {
        pqxx::work work(*_connection);
        pqxx::result result = work.exec_prepared("quit", user_id);
        std::string output = GetStringQueryResult(result);
        work.commit();
    } catch (const std::exception& e) {
        NotifyObservers("PostgreSQL error: " + std::string(e.what()));
    }

    // Удаление из Redis
    try {
        std::string auth_key = "auth:" + token;
        std::string session_key = "session:" + std::to_string(user_id);

        long long deleted_auth = _connectionRedis->del(auth_key);
        long long deleted_session = _connectionRedis->del(session_key);

        if (deleted_auth > 0)
            std::cout << "Token deleted from Redis: " << auth_key << std::endl;
        else
            std::cout << "Token not found in Redis: " << auth_key << std::endl;

        if (deleted_session > 0)
            std::cout << "Session deleted from Redis: " << session_key << std::endl;
        else
            std::cout << "Session not found in Redis: " << session_key << std::endl;

    } catch (const std::exception& e) {
        NotifyObservers("Redis error: " + std::string(e.what()));
    }
}


void Worker::SaveActiveSession(int user_id, const std::string& ip) {
    std::string key = "session:" + std::to_string(user_id);

    std::unordered_map<std::string, std::string> session_data = {
        {"ip", ip},
        {"last_active", std::to_string(std::time(nullptr))}
    };

    _connectionRedis->hmset(key, session_data.begin(), session_data.end());

    _connectionRedis->expire(key, std::chrono::seconds(3600));
}

void Worker::UpdateActivity(const std::string& username) {
    int user_id = UserID(username);
    std::string key = "session:" + std::to_string(user_id);

    try {
        _connectionRedis->hset(key, "last_active", std::to_string(std::time(nullptr)));
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return;
    }
}

void Worker::PrintActiveSessions() const
{
    try {
        std::cout << "=== Active Sessions ===" << std::endl;

        long long cursor = 0;
        do {
            std::vector<std::string> keys;

            cursor = _connectionRedis->scan(cursor, "session:*", 100, std::back_inserter(keys));

            for (const auto& key : keys) {
                std::unordered_map<std::string, std::string> session_data;
                _connectionRedis->hgetall(key, std::inserter(session_data, session_data.begin()));

                std::cout << key << std::endl;
                for (const auto& [field, value] : session_data) {
                    std::cout << "  " << field << ": " << value << std::endl;
                }
                std::cout << std::endl;
            }

        } while (cursor != 0);

    } catch (const std::exception& e) {
        std::cerr << "Error while fetching sessions: " << e.what() << std::endl;
    }
}

void Worker::LoginEvent(const std::string& username, int id) const
{
    QJsonObject payload;
    payload["user_id"] = id;
    payload["username"] = QString::fromStdString(username);
    payload["timestamp"] = static_cast<qint64>(std::time(nullptr));
    payload["event"] = "login";

    QJsonDocument doc(payload);
    std::string message = doc.toJson(QJsonDocument::Compact).toStdString();

    _connectionRedis->publish("users:activity", message);
}

std::string Worker::GetRole(const std::string& role) {
    if (IsRoot(role)) {
        return "admin";
    } else {
        return "common";
    }
}

std::string Worker::Login(const std::string& login, const std::string& password) {
    int user_id = UserID(login);
    if (user_id == 0) {
        return "Invalid username";
    }
    std::pair<std::string, std::string> passwordAndSalt = GetSaltAndPassword(login);
    std::string hashedPassword = HashPassword(password, passwordAndSalt.second);
    if (hashedPassword != passwordAndSalt.first) {
        return "Invalid password";
    }
    LoginEvent(login, user_id);
    std::string token = CreateSession(user_id);
    return token;
}

std::string Worker::Registrate(const std::string& login, const std::string& password) {
    if (UserID(login) != 0) {
        return "Exists";
    }
    std::string salt = GenerateSalt();
    std::string hashedPassword = HashPassword(password, salt);
    CreateUser(login, hashedPassword, salt);
    return "Success";
}

std::pair<std::string, std::string> Worker::GetSaltAndPassword(const std::string& login) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("get_salt_and_password", login);
        return GetPairQueryResult(result);
    } catch (std::string error) {
        NotifyObservers("Error: " + error);
        return std::make_pair("", "");
    }
    work.commit();
    return std::make_pair("", "");
}

std::string Worker::GenerateToken() const {
    boost::uuids::random_generator gen;
    boost::uuids::uuid u = gen();
    return to_string(u);
}

std::string Worker::CreateSession(const int user_id) {

    std::string token = GenerateToken();
    std::string output;
    pqxx::work work(*_connection);

    if (IsUserActive(user_id)) {
        return "Session exists";
    }

    try {
        pqxx::result result = work.exec_prepared("create_session", user_id);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }

    work.commit();

    try {
        _connectionRedis->set("auth:" + token, std::to_string(user_id), std::chrono::seconds(3600));
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }

    try {
        SaveActiveSession(user_id, "127.0.0.0");
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return "Error";
    }
    return token;
}

bool Worker::IsUserActive(int user_id) const {
    std::string key = "session:" + std::to_string(user_id);
    return _connectionRedis->exists(key) == 1;
}

int Worker::UserID(const std::string& login) {
    std::string output;
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("get_user_id", login);
        output = GetStringQueryResult(result);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return -1;
    }
    work.commit();
    return std::stoi(output);
}

void Worker::CreateUser(const std::string &username, const std::string& hashed_password, const std::string& salt) {
    pqxx::work work(*_connection);
    try {
        pqxx::result result = work.exec_prepared("create_user", username, hashed_password, salt);
    } catch (const std::exception& e) {
        NotifyObservers("Error: " + std::string(e.what()));
        return;
    }
    work.commit();
}

std::string Worker::GenerateSalt(std::size_t size) const {
    unsigned char salt[size];
    if (!RAND_bytes(salt, sizeof(salt))) {
        throw std::runtime_error("Error generating random bytes.");
    }
    return ToHexString(salt, size);
}

std::string Worker::HashPassword(const std::string& password, const std::string& salt) const {
    std::string saltedPassword = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(saltedPassword.c_str()), saltedPassword.length(), hash);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string Worker::ToHexString(const unsigned char* data, std::size_t length) const {
    std::stringstream ss;
    for (std::size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);
    }
    return ss.str();
}

std::string Worker::GetStringQueryResult(const pqxx::result& result) const {
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
    return res;
}

std::pair<std::string, std::string> Worker::GetPairQueryResult(const pqxx::result& result) const {
    std::string res = "";
    for (const auto &row : result) {
        res += row[0].as<std::string>();
    }
    res = res.substr(1, res.size() - 2); 
    size_t comma = res.find(',');
    std::string password = res.substr(0, comma);
    std::string salt = res.substr(comma + 1);
    return std::make_pair(password, salt);  
}

Worker::~Worker() {
    _connection->disconnect();
}

void Observable::NotifyObservers(const std::string& event) {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    for (auto obs: _observers) {
        obs->Notify(localTime, event);
    }
}

void Observable::AddObserver(std::shared_ptr<IServerObserver> obs) {
    _observers.push_back(obs);
}

ServerTextObserver::ServerTextObserver(const std::string& file) : of(file) {}

void ServerTextObserver::Notify(const std::tm* time, const std::string& str) {
    of << std::put_time(time, "%d-%m-%Y %H:%M:%S") << '\t' << str << '\n';
}

void ServerTerminalObserver::Notify(const std::tm* time, const std::string& str) {
    std::cout << std::put_time(time, "%d-%m-%Y %H:%M:%S") << '\t' << str << std::endl;
}

QJsonObject JsonHandler::GenerateFileTree(const pqxx::result& result) {
    QJsonObject root;
    QJsonArray fileSystem;

    for (const auto& row : result) {
        FileInfo file;
        file.path = QString::fromStdString(row["path"].c_str());
        QStringList pathComponents = file.path.split('/');
        file.fileName = QString::fromStdString(row["name"].c_str());
        file.fileType = QString::fromStdString(row["type"].c_str());
        file.userName = QString::fromStdString(row["owner_name"].c_str());
        file.groupName = QString::fromStdString(row["group_name"].c_str());
        file.permissions = QString::fromStdString(row["permissions"].c_str());
        AddFileToTree(fileSystem, pathComponents, file);
    }

    root["file_system"] = fileSystem;
    return root;
}

void JsonHandler::AddFileToTree(QJsonArray& parentArray, const QStringList& pathComponents, const FileInfo& file) {

    if (pathComponents.isEmpty()) {
        return;
    }

    QString currentComponent = pathComponents.first();
    QJsonObject currentNode;

    // Поиск существующего узла
    bool nodeFound = false;
    for (int i = 0; i < parentArray.size(); ++i) {
        QJsonObject obj = parentArray[i].toObject();
        if (obj["name"].toString() == currentComponent) {
            currentNode = obj;
            nodeFound = true;
            parentArray.removeAt(i);
            break;
        }
    }

    if (!nodeFound) {
        currentNode["name"] = currentComponent;
        currentNode["type"] = file.fileType;
        currentNode["path"] = file.path;
        currentNode["userName"] = file.userName;
        currentNode["groupName"] = file.groupName;
        currentNode["permissions"] = file.permissions;
        currentNode["files"] = QJsonArray();
    }

    // Если есть оставшиеся компоненты пути, рекурсивно создаем вложенные узлы
    if (pathComponents.size() > 1) {
        QJsonArray filesArray = currentNode["files"].toArray();
        AddFileToTree(filesArray, pathComponents.mid(1), file);
        currentNode["files"] = filesArray;
    }

    // Возвращаем обновленный узел в массив
    parentArray.append(currentNode);
}

QJsonObject JsonHandler::GenerateUsersList(const pqxx::result& result) {
    QJsonObject root;
    QJsonArray list;

    for (const auto& row : result) {
        QJsonObject obj;
        obj["username"] = QString::fromStdString(row["login"].c_str());
        obj["is_admin"] = row["is_admin"].as<bool>();
        obj["is_active"] = row["is_active"].as<bool>();
        list.append(obj);
    }

    root["users"] = list;
    return root;
}

QJsonObject JsonHandler::GenerateGroupsList(const pqxx::result& result) {
    QJsonArray groupsArray;

    for (const auto& row : result) {
        QJsonObject obj;
        obj["group_name"] = QString::fromStdString(row["group_name"].c_str());
        std::string userList = row["user_list"].is_null() ? "" : row["user_list"].c_str();
        QJsonArray usersArray;
        if (!userList.empty()) {
            std::istringstream userStream(userList);
            std::string username;
            while (std::getline(userStream, username, ',')) {
                QJsonObject userObject;
                userObject["username"] = QString::fromStdString(username);
                usersArray.append(userObject);
            }
        }
        obj["users"] = usersArray;
        groupsArray.append(obj);
    }

    QJsonObject rootObject;
    rootObject["groups"] = groupsArray;
    return rootObject;
}