#include <vcl.h>
#pragma hdrstop
#include "db_stuff.h"
#pragma package(smart_init)

DBs gConnections;

// DBs implementation
DBs::DBs() : m_nConnections(0) {}

DBs::~DBs() { Destroy(); }

bool DBs::Create() {
    Destroy();
    m_nConnections = 0;
    return true;
}

bool DBs::Destroy() {
    m_pConnections.clear();
    m_nConnections = 0;
    return true;
}

DB* DBs::Add_Connection(const AnsiString& Name, const AnsiString& User, const AnsiString& Password,
                        const AnsiString& Host, const AnsiString& Port) {
    try {
        int port = Port.IsEmpty() ? 5432 : Port.ToIntDef(5432);
        auto connection = std::make_unique<DB>(Host.c_str(), port, Name.c_str(), User.c_str(), Password.c_str(), true);
        if (connection && connection->isConnected()) {
            DB* rawPtr = connection.get();
            m_pConnections.push_back(std::move(connection));
            m_nConnections++;
            return rawPtr;
        }
        return nullptr;
    } catch (const std::exception& e) {
        ShowMessage("Erro ao adicionar conexão: " + AnsiString(e.what()));
        return nullptr;
    }
}

std::vector<DB*> DBs::getConnections() const {
    std::vector<DB*> connections;
    for (const auto& conn : m_pConnections) {
        connections.push_back(conn.get());
    }
    return connections;
}

// DB implementation
DB::DB() : conn(nullptr, PQfinish), autoCommit(true), inTransaction(false) {}

DB::DB(const std::string& host, int port, const std::string& name, const std::string& user,
       const std::string& password, bool autoCommit)
    : conn(nullptr, PQfinish), autoCommit(autoCommit), inTransaction(false) {
    create(host.c_str(), port, name.c_str(), user.c_str(), password.c_str(), autoCommit);
}

bool DB::create(const AnsiString &Host, int Port, const AnsiString &Name,
                const AnsiString &User, const AnsiString &Password, bool bAutoCommit) {
    this->autoCommit = bAutoCommit;
    inTransaction = false;
    AnsiString connInfo = "dbname='" + Name + "' user='" + User +
                          "' password='" + Password + "' host='" + Host +
                          "' port=" + IntToStr(Port) + " client_encoding='UTF8'";
    conn = std::unique_ptr<PGconn, decltype(&PQfinish)>(PQconnectdb(connInfo.c_str()), PQfinish);
    if (!isConnected()) {
        reportError("Connection to database failed: " + std::string(PQerrorMessage(conn.get())));
        return false;
    }
    return ensureUTF8Encoding();
}

bool DB::connectAsync(const AnsiString& connInfo) {
    conn = std::unique_ptr<PGconn, decltype(&PQfinish)>(PQconnectStart(connInfo.c_str()), PQfinish);
    if (!conn) {
        reportError("Failed to start async connection!");
        return false;
    }
    while (PQconnectPoll(conn.get()) == PGRES_POLLING_ACTIVE) {
        int sock = PQsocket(conn.get());
        pg_usec_time_t timeout = PQgetCurrentTimeUSec() + 1000000; // 1 segundo
        if (PQsocketPoll(sock, 1, 1, timeout) <= 0) {
            reportError("Connection timeout!");
            return false;
        }
    }
    return isConnected() && ensureUTF8Encoding();
}

bool DB::execute(const std::string& sql) {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    PGresult* result = PQexec(conn.get(), sql.c_str());
    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK ||
                    PQresultStatus(result) == PGRES_TUPLES_OK);
    if (!success) {
        reportError("SQL execution failed: " + std::string(PQerrorMessage(conn.get())));
    }
    PQclear(result);
    return success;
}

bool DB::executeParams(const std::string& sql, const std::vector<std::string>& params) {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    std::vector<const char*> paramValues;
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }
    PGresult* result = PQexecParams(conn.get(), sql.c_str(), params.size(), nullptr,
                                    paramValues.data(), nullptr, nullptr, 0);
    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK ||
                    PQresultStatus(result) == PGRES_TUPLES_OK);
    if (!success) {
        reportError("SQL execution failed: " + std::string(PQerrorMessage(conn.get())));
    }
    PQclear(result);
    return success;
}

std::unique_ptr<Statement> DB::prepare(const AnsiString& sql) {
    return std::make_unique<Statement>(sql, conn.get());
}

std::unique_ptr<Statement> DB::query(const AnsiString& sql) {
	auto stmt = std::make_unique<Statement>(sql, conn.get());

    stmt->execute();
    return stmt;
}

bool DB::isConnected() const {
    return conn && PQstatus(conn.get()) == CONNECTION_OK;
}

bool DB::destroy() {
    conn.reset();
    inTransaction = false;
    return true;
}

bool DB::beginTransaction() {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    if (inTransaction) {
        reportError("Transaction already in progress!");
        return false;
    }
    PGresult* result = PQexec(conn.get(), "BEGIN");
    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK);
    if (success) {
        inTransaction = true;
    } else {
        reportError("Falha ao iniciar transação: " + std::string(PQerrorMessage(conn.get())));
    }
    PQclear(result);
    return success;
}

bool DB::commit() {
    if (!inTransaction) {
        reportError("No transaction in progress!");
        return false;
    }
    PGresult* result = PQexec(conn.get(), "COMMIT");
    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK);
    if (success) {
        inTransaction = false;
    } else {
        reportError("Falha ao executar commit: " + std::string(PQerrorMessage(conn.get())));
    }
    PQclear(result);
    return success;
}

bool DB::rollback() {
    if (!inTransaction) {
        reportError("No transaction in progress!");
        return false;
    }
    PGresult* result = PQexec(conn.get(), "ROLLBACK");
    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK);
    if (success) {
        inTransaction = false;
    } else {
        reportError("Falha ao executar rollback: " + std::string(PQerrorMessage(conn.get())));
    }
    PQclear(result);
    return success;
}

bool DB::savepoint(const std::string& savepointName) {
    if (!inTransaction) {
        reportError("No transaction in progress!");
        return false;
    }
    if (savepointName.empty() || savepointName.find('\'') != std::string::npos ||
        savepointName.find(';') != std::string::npos) {
        reportError("Invalid savepoint name!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedName(
        PQescapeIdentifier(conn.get(), savepointName.c_str(), savepointName.length()), PQfreemem);
    if (!escapedName) {
        reportError("Failed to escape savepoint name!");
        return false;
    }
    return execute("SAVEPOINT " + std::string(escapedName.get()));
}

bool DB::rollbackToSavepoint(const std::string& savepointName) {
    if (!inTransaction) {
        reportError("No transaction in progress!");
        return false;
    }
    if (savepointName.empty() || savepointName.find('\'') != std::string::npos ||
        savepointName.find(';') != std::string::npos) {
        reportError("Invalid savepoint name!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedName(
        PQescapeIdentifier(conn.get(), savepointName.c_str(), savepointName.length()), PQfreemem);
    if (!escapedName) {
        reportError("Failed to escape savepoint name!");
        return false;
    }
    return execute("ROLLBACK TO SAVEPOINT " + std::string(escapedName.get()));
}

bool DB::releaseSavepoint(const std::string& savepointName) {
    if (!inTransaction) {
        reportError("No transaction in progress!");
        return false;
    }
    if (savepointName.empty() || savepointName.find('\'') != std::string::npos ||
        savepointName.find(';') != std::string::npos) {
        reportError("Invalid savepoint name!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedName(
        PQescapeIdentifier(conn.get(), savepointName.c_str(), savepointName.length()), PQfreemem);
    if (!escapedName) {
        reportError("Failed to escape savepoint name!");
        return false;
    }
    return execute("RELEASE SAVEPOINT " + std::string(escapedName.get()));
}

bool DB::tableExists(const std::string& tableName) {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedTable(
        PQescapeIdentifier(conn.get(), tableName.c_str(), tableName.length()), PQfreemem);
    if (!escapedTable) {
        reportError("Failed to escape table name!");
        return false;
    }
    std::string sql = "SELECT EXISTS (SELECT FROM information_schema.tables WHERE table_name = $1)";
    std::vector<std::string> params = {tableName};
    auto stmt = query(sql.c_str());
    bool result = false;
    if (stmt->executeParams(sql, params)) {
        ResultSet res = stmt->fetchAll();
        if (!res.empty() && res[0]["exists"] == "t") {
            result = true;
        }
    }
    return result;
}

bool DB::schemaExists(const std::string& schemaName) {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedSchema(
        PQescapeIdentifier(conn.get(), schemaName.c_str(), schemaName.length()), PQfreemem);
    if (!escapedSchema) {
        reportError("Failed to escape schema name!");
        return false;
    }
    std::string sql = "SELECT EXISTS (SELECT FROM information_schema.schemata WHERE schema_name = $1)";
    std::vector<std::string> params = {schemaName};
    auto stmt = query(sql.c_str());
    bool result = false;
    if (stmt->executeParams(sql, params)) {
        ResultSet res = stmt->fetchAll();
        if (!res.empty() && res[0]["exists"] == "t") {
            result = true;
        }
    }
    return result;
}

bool DB::createSchema(const std::string& schemaName) {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedSchema(
        PQescapeIdentifier(conn.get(), schemaName.c_str(), schemaName.length()), PQfreemem);
    if (!escapedSchema) {
        reportError("Failed to escape schema name!");
        return false;
    }
    return execute("CREATE SCHEMA IF NOT EXISTS " + std::string(escapedSchema.get()));
}

std::vector<std::string> DB::listSchemas() {
    std::vector<std::string> schemas;
    if (!isConnected()) {
        reportError("No database connection!");
        return schemas;
    }
    std::string sql = "SELECT schema_name FROM information_schema.schemata WHERE schema_name NOT LIKE 'pg_%' AND schema_name != 'information_schema'";
    auto stmt = query(sql.c_str());
    if (stmt->execute()) {
        ResultSet result = stmt->fetchAll();
        for (const auto& row : result) {
            schemas.push_back(row.second.at("schema_name"));
        }
    }
    return schemas;
}

bool DB::changePassword(const std::string& user, const std::string& newPassword) {
    if (!isConnected()) {
        reportError("No database connection!");
        return false;
    }
    PGresult* result = PQchangePassword(conn.get(), user.c_str(), newPassword.c_str());
    bool success = (PQresultStatus(result) == PGRES_COMMAND_OK);
    if (!success) {
        reportError("Failed to change password: " + std::string(PQerrorMessage(conn.get())));
    }
    PQclear(result);
    return success;
}

bool DB::enterPipelineMode() {
    if (PQenterPipelineMode(conn.get()) != 1) {
        reportError("Failed to enter pipeline mode: " + std::string(PQerrorMessage(conn.get())));
        return false;
    }
    return true;
}

bool DB::exitPipelineMode() {
    if (PQexitPipelineMode(conn.get()) != 1) {
        reportError("Failed to exit pipeline mode: " + std::string(PQerrorMessage(conn.get())));
        return false;
    }
    return true;
}

bool DB::pipelineSync() {
    if (PQpipelineSync(conn.get()) != 1) {
        reportError("Failed to send pipeline sync: " + std::string(PQerrorMessage(conn.get())));
        return false;
    }
    return true;
}

std::unique_ptr<PGcancelConn, decltype(&PQcancelFinish)> DB::createCancel() {
    return std::unique_ptr<PGcancelConn, decltype(&PQcancelFinish)>(
        PQcancelCreate(conn.get()), PQcancelFinish);
}

bool DB::cancelQueryAsync() {
    auto cancelConn = createCancel();
    if (!cancelConn) {
        reportError("Failed to create cancel connection!");
        return false;
    }
    if (PQcancelStart(cancelConn.get()) != 1) {
        reportError("Failed to start cancel: " + std::string(PQcancelErrorMessage(cancelConn.get())));
        return false;
    }
    while (PQcancelPoll(cancelConn.get()) == PGRES_POLLING_ACTIVE) {
        // Aguardar polling (integrar com VCL event loop, se necessário)
    }
    return PQcancelStatus(cancelConn.get()) == CONNECTION_OK;
}

bool DB::ensureUTF8Encoding() {
    if (PQsetClientEncoding(conn.get(), "UTF8") != 0) {
        reportError("Failed to set UTF8 encoding: " + std::string(PQerrorMessage(conn.get())));
        return false;
    }
    return PQclientEncoding(conn.get()) == pg_char_to_encoding("UTF8");
}



void DB::setErrorHandler(ErrorHandler handler) {
    errorHandler = handler;
}

void DB::reportError(const std::string& message) const {
    if (errorHandler) {
        errorHandler(message);
    } else {
        ShowMessage(message.c_str());
    }
}

AnsiString DB::getLastError() const {
    if (conn) {
        return AnsiString(PQerrorMessage(conn.get()));
    }
    return AnsiString("Sem conexão ativa.");
}

UnicodeString DB::encryptPassword(const UnicodeString& password, const UnicodeString& user) {
    if (!isConnected()) {
        ShowMessage("No database connection for encryption.");
        throw ExcepcionMultiExport("No database connection for encryption.");
    }
    if (password.IsEmpty() || user.IsEmpty()) {
        ShowMessage("Password or user cannot be empty.");
        throw ExcepcionMultiExport("Password or user cannot be empty.");
    }
    std::string pwd = std::string(AnsiString(password));
    std::string usr = std::string(AnsiString(user));
    std::unique_ptr<char, decltype(&PQfreemem)> encrypted(
        PQencryptPasswordConn(conn.get(), pwd.c_str(), usr.c_str(), nullptr), PQfreemem);
    if (!encrypted) {
        ShowMessage("Failed to encrypt password. Ensure PostgreSQL is configured with 'password_encryption = scram-sha-256' and valid username.");
        throw ExcepcionMultiExport("Failed to encrypt password. Ensure PostgreSQL is configured with 'password_encryption = scram-sha-256' and valid username.");
    }
    return UnicodeString(encrypted.get());
}



// Statement implementation
Statement::Statement(const AnsiString& sql, PGconn* connection)
    : queryString(sql), conn(connection), resultSet(nullptr, PQclear),
      currentRow(0), totalRows(0), totalCols(0) {}

void Statement::clearResultSet() {
    resultSet.reset();
}

bool Statement::execute() {
    if (PQstatus(conn) != CONNECTION_OK) {
        ShowMessage("Database disconnected.");
        return false;
    }
    resultSet = std::unique_ptr<PGresult, decltype(&PQclear)>(PQexec(conn, queryString.c_str()), PQclear);
    currentRow = 0;
    totalRows = PQntuples(resultSet.get());
    totalCols = PQnfields(resultSet.get());
    return PQresultStatus(resultSet.get()) == PGRES_TUPLES_OK ||
           PQresultStatus(resultSet.get()) == PGRES_COMMAND_OK ||
           PQresultStatus(resultSet.get()) == PGRES_TUPLES_CHUNK;
}

bool Statement::executeParams(const std::string& sql, const std::vector<std::string>& params) {
    clearResultSet();
    std::vector<const char*> paramValues;
    for (const auto& param : params) {
        paramValues.push_back(param.c_str());
    }
    resultSet = std::unique_ptr<PGresult, decltype(&PQclear)>(
        PQexecParams(conn, sql.c_str(), params.size(), nullptr, paramValues.data(), nullptr, nullptr, 0), PQclear);
    ExecStatusType status = PQresultStatus(resultSet.get());
    if (status != PGRES_TUPLES_OK && status != PGRES_COMMAND_OK && status != PGRES_TUPLES_CHUNK) {
        return false;
    }
    totalRows = PQntuples(resultSet.get());
    totalCols = PQnfields(resultSet.get());
    currentRow = 0;
    return true;
}

bool Statement::setChunkedRowsMode(int chunkSize) {
    if (PQsetChunkedRowsMode(conn, chunkSize) != 1) {
        ShowMessage("Failed to set chunked rows mode: " + AnsiString(PQerrorMessage(conn)));
        return false;
    }
    return true;
}

Row Statement::fetch() {
    Row row;
    if (currentRow < totalRows) {
        for (int i = 0; i < totalCols; i++) {
            char* colName = PQfname(resultSet.get(), i);
            row[colName] = PQgetvalue(resultSet.get(), currentRow, i);
        }
        currentRow++;
    }
    return row;
}

ResultSet Statement::fetchAll() {
    ResultSet result;
    for (int i = 0; i < totalRows; i++) {
        Row row;
        for (int j = 0; j < totalCols; j++) {
            char* colName = PQfname(resultSet.get(), j);
            row[colName] = PQgetvalue(resultSet.get(), i, j);
        }
        result[i] = row;
    }
    currentRow = totalRows;
    clearResultSet();
    return result;
}

ResultSet Statement::fetchChunk() {
    ResultSet result;
    if (!resultSet || PQresultStatus(resultSet.get()) != PGRES_TUPLES_CHUNK) {
        return result;
    }
    totalRows = PQntuples(resultSet.get());
    totalCols = PQnfields(resultSet.get());
    for (int i = 0; i < totalRows; i++) {
        Row row;
        for (int j = 0; j < totalCols; j++) {
            char* colName = PQfname(resultSet.get(), j);
            row[colName] = PQgetvalue(resultSet.get(), i, j);
        }
        result[i] = row;
    }
    clearResultSet();
    return result;
}

unsigned int Statement::getMax(const std::string& field, const std::string& table) {
    if (field.empty() || table.empty() ||
        field.find('\'') != std::string::npos || table.find('\'') != std::string::npos ||
        field.find(';') != std::string::npos || table.find(';') != std::string::npos) {
        ShowMessage("Invalid field or table name!");
        return 0;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedField(
        PQescapeIdentifier(conn, field.c_str(), field.length()), PQfreemem);
    std::unique_ptr<char, decltype(&PQfreemem)> escapedTable(
        PQescapeIdentifier(conn, table.c_str(), table.length()), PQfreemem);
    if (!escapedField || !escapedTable) {
        ShowMessage("Failed to escape field or table name!");
        return 0;
    }
    std::string query = "SELECT MAX(" + std::string(escapedField.get()) + ") as maximo FROM " + std::string(escapedTable.get());
    clearResultSet();
    resultSet = std::unique_ptr<PGresult, decltype(&PQclear)>(PQexec(conn, query.c_str()), PQclear);
    if (PQresultStatus(resultSet.get()) != PGRES_TUPLES_OK) {
        ShowMessage("Failed to get max value: " + AnsiString(PQerrorMessage(conn)));
        clearResultSet();
        return 0;
    }
    totalRows = PQntuples(resultSet.get());
    totalCols = PQnfields(resultSet.get());
    currentRow = 0;
    unsigned int maxValue = 0;
    if (totalRows > 0 && !PQgetisnull(resultSet.get(), 0, 0)) {
        const char* value = PQgetvalue(resultSet.get(), 0, 0);
        maxValue = static_cast<unsigned int>(strtoul(value, nullptr, 10));
    }
    clearResultSet();
    return maxValue;
}

bool Statement::hasIndex(const std::string& table, const std::string& indexName) {
    if (table.empty() || indexName.empty() ||
        table.find('\'') != std::string::npos || indexName.find('\'') != std::string::npos ||
        table.find(';') != std::string::npos || indexName.find(';') != std::string::npos) {
        ShowMessage("Invalid table or index name!");
        return false;
    }
    std::unique_ptr<char, decltype(&PQfreemem)> escapedTable(
        PQescapeIdentifier(conn, table.c_str(), table.length()), PQfreemem);
    std::unique_ptr<char, decltype(&PQfreemem)> escapedIndex(
        PQescapeIdentifier(conn, indexName.c_str(), indexName.length()), PQfreemem);
    if (!escapedTable || !escapedIndex) {
        ShowMessage("Failed to escape table or index name!");
        return false;
    }
    std::string sql = "SELECT EXISTS (SELECT FROM pg_indexes WHERE tablename = $1 AND indexname = $2)";
    std::vector<std::string> params = {table, indexName};
    executeParams(sql, params);
    ResultSet result = fetchAll();
    if (!result.empty()) {
        auto it = result.find(0);
        if (it != result.end() && !it->second.empty()) {
            auto valueIt = it->second.find("exists");
            if (valueIt != it->second.end()) {
                return valueIt->second == "t";
            }
        }
    }
    return false;
}

std::string Statement::getReturnedValue(const std::string& columnName) {
    if (!resultSet) {
        ShowMessage("No result set available.");
        return "";
    }
    if (PQntuples(resultSet.get()) == 0) {
        ShowMessage("No rows returned.");
        return "";
    }
    int colIndex = PQfnumber(resultSet.get(), columnName.c_str());
    if (colIndex == -1) {
        ShowMessage("Column '" + AnsiString(columnName.c_str()) + "' not found in result set.");
        return "";
    }
    char* value = PQgetvalue(resultSet.get(), 0, colIndex);
    if (!value || PQgetisnull(resultSet.get(), 0, colIndex)) {
        ShowMessage("Value for column '" + AnsiString(columnName.c_str()) + "' is null or empty.");
        return "";
    }
    return std::string(value);
}
