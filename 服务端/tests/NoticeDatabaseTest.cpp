#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <iostream>
#include <string>
#include "../Library/Shared/NoticeDatabase.h"

int main()
{
    // Receive credentials privately on stdin; never print the connection string.
    std::string connection;
    if (!std::getline(std::cin, connection)) return 1;
    SQLHENV environment = SQL_NULL_HENV;
    SQLHDBC database = SQL_NULL_HDBC;
    SQLHSTMT statement = SQL_NULL_HSTMT;
    bool success = false;
    int stage = 0;
    SQLRETURN result = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment);
    if (SQL_SUCCEEDED(result)) {
        stage = 1;
        SQLSetEnvAttr(environment, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
        result = SQLAllocHandle(SQL_HANDLE_DBC, environment, &database);
        if (SQL_SUCCEEDED(result)) {
            result = SQLDriverConnectA(database, NULL, (SQLCHAR*)connection.c_str(), SQL_NTS,
                NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
        }
        if (SQL_SUCCEEDED(result)) result = SQLAllocHandle(SQL_HANDLE_STMT, database, &statement);
        if (SQL_SUCCEEDED(result)) {
            // Read only: deterministic Chinese through the same SQL_C_WCHAR helper as RajaDB.
            const char* query = "SELECT CONVERT(0xE4B8ADE69687E585ACE5918A USING utf8mb4)";
            std::string gbk;
            stage = 2;
            result = SQLExecDirectA(statement, (SQLCHAR*)query, SQL_NTS);
            if (SQL_SUCCEEDED(result)) {
                stage = 3;
                result = SQLFetch(statement);
                if (SQL_SUCCEEDED(result)) {
                    stage = 4;
                    if (ReadDatabaseNoticeGbk(statement, 1, gbk)) {
                        stage = 5;
                        success = gbk == std::string("\xd6\xd0\xce\xc4\xb9\xab\xb8\xe6", 8);
                    }
                }
            }
        }
    }
    if (!success && database != SQL_NULL_HDBC) {
        SQLCHAR state[6] = {}, message[2] = {};
        SQLINTEGER native = 0;
        SQLSMALLINT length = 0;
        SQLGetDiagRecA(SQL_HANDLE_DBC, database, 1, state, &native, message, sizeof(message), &length);
        std::cout << "SQLState=" << state << " Native=" << native << std::endl;
    }
    if (statement != SQL_NULL_HSTMT) SQLFreeHandle(SQL_HANDLE_STMT, statement);
    if (database != SQL_NULL_HDBC) { SQLDisconnect(database); SQLFreeHandle(SQL_HANDLE_DBC, database); }
    if (environment != SQL_NULL_HENV) SQLFreeHandle(SQL_HANDLE_ENV, environment);
    std::cout << (success ? "Chinese SQL-to-GBK: Passed" : "Chinese SQL-to-GBK: Failed") << " stage=" << stage << std::endl;
    return success ? 0 : 1;
}
