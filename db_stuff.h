#ifndef DB_STUFF_H
#define DB_STUFF_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <libpq-fe.h>
#include <vcl.h>
#include "ExcepcionMultiExport.h"
typedef std::map<std::string, std::string> Row;
typedef std::map<int, Row> ResultSet;

class DB; // Declaração forward

class Statement {
private:
    AnsiString queryString;
    PGconn* conn; // Gerenciado por DB
    std::unique_ptr<PGresult, decltype(&PQclear)> resultSet;
    int currentRow, totalRows, totalCols;
    void clearResultSet(); // Declaração apenas
public:
    Statement(const AnsiString& sql, PGconn* connection);
    ~Statement() = default;
    bool execute();
    bool executeParams(const std::string& sql, const std::vector<std::string>& params);
    bool setChunkedRowsMode(int chunkSize);
    Row fetch();
    ResultSet fetchAll();
    ResultSet fetchChunk();
    unsigned int getMax(const std::string& field, const std::string& table);
    bool hasIndex(const std::string& table, const std::string& indexName);
    std::string getReturnedValue(const std::string& columnName);
};

class DB {
public:
    using ErrorHandler = std::function<void(const std::string&)>;
private:
    std::unique_ptr<PGconn, decltype(&PQfinish)> conn;
    bool autoCommit;
    bool inTransaction;
    ErrorHandler errorHandler;
    friend class Statement;
public:
    DB();
    DB(const std::string& host, int port, const std::string& name, const std::string& user,
       const std::string& password, bool autoCommit = true);
    ~DB() = default;
    bool create(const AnsiString &Host, int Port, const AnsiString &Name,
                const AnsiString &User, const AnsiString &Password, bool autoCommit = true);
    bool connectAsync(const AnsiString& connInfo);
    bool execute(const std::string& sql);
    bool executeParams(const std::string& sql, const std::vector<std::string>& params);
    std::unique_ptr<Statement> prepare(const AnsiString& sql);
    std::unique_ptr<Statement> query(const AnsiString& sql);
    bool isConnected() const;
    bool destroy();
    bool beginTransaction();
    bool commit();
    bool rollback();
    bool savepoint(const std::string& savepointName);
    bool rollbackToSavepoint(const std::string& savepointName);
    bool releaseSavepoint(const std::string& savepointName);
    bool tableExists(const std::string& tableName);
    bool schemaExists(const std::string& schemaName);
    bool createSchema(const std::string& schemaName);
    std::vector<std::string> listSchemas();
    bool changePassword(const std::string& user, const std::string& newPassword);
    bool enterPipelineMode();
    bool exitPipelineMode();
    bool pipelineSync();
    std::unique_ptr<PGcancelConn, decltype(&PQcancelFinish)> createCancel();
    bool cancelQueryAsync();
    bool ensureUTF8Encoding();
	UnicodeString encryptPassword(const UnicodeString& password, const UnicodeString& user);
	void setErrorHandler(ErrorHandler handler);
    void reportError(const std::string& message) const;
    AnsiString getLastError() const;
};

class DBs {
public:
    DBs();
    ~DBs();
    bool Create();
    bool Destroy();
    DB* Add_Connection(const AnsiString& Name, const AnsiString& User, const AnsiString& Password,
                       const AnsiString& Host = "localhost", const AnsiString& Port = "5432");
    std::vector<DB*> getConnections() const;
private:
    std::vector<std::unique_ptr<DB>> m_pConnections;
    int m_nConnections;
};

extern DBs gConnections;

#endif // DB_STUFF_H

