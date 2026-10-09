#include "../Library/Shared/CharacterSave.h"
#include <cassert>
#include <iostream>
#include <string>

static void ExecuteFixtureSql(SQLHDBC connection, const char* sql)
{
    SQLHSTMT statement = SQL_NULL_HSTMT;
    assert(SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, connection, &statement)));
    const SQLRETURN result = SQLExecDirectA(statement,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(sql)), SQL_NTS);
    SQLFreeHandle(SQL_HANDLE_STMT, statement);
    assert(SQL_SUCCEEDED(result));
}

static SQLBIGINT ReadFixtureValue(SQLHDBC connection, const char* sql)
{
    SQLHSTMT statement = SQL_NULL_HSTMT;
    assert(SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, connection, &statement)));
    assert(SQL_SUCCEEDED(SQLExecDirectA(statement,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(sql)), SQL_NTS)));
    assert(SQL_SUCCEEDED(SQLFetch(statement)));
    SQLBIGINT value = 0;
    SQLLEN indicator = 0;
    assert(SQL_SUCCEEDED(SQLGetData(statement, 1, SQL_C_SBIGINT, &value, sizeof(value), &indicator)));
    SQLFreeHandle(SQL_HANDLE_STMT, statement);
    return value;
}

int main()
{
    std::string privateConnection;
    if (!std::getline(std::cin, privateConnection)) return 2;
    SQLHENV environment = SQL_NULL_HENV;
    SQLHDBC connection = SQL_NULL_HDBC;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &environment))) return 2;
    SQLSetEnvAttr(environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0);
    SQLAllocHandle(SQL_HANDLE_DBC, environment, &connection);
    if (!SQL_SUCCEEDED(SQLDriverConnectA(connection, NULL,
        reinterpret_cast<SQLCHAR*>(&privateConnection[0]), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT)))
    {
        std::cerr << "Database fixture connection failed; credentials omitted\n";
        SQLFreeHandle(SQL_HANDLE_DBC, connection); SQLFreeHandle(SQL_HANDLE_ENV, environment);
        return 2;
    }
    // Session-local tables only; no production role or account rows are touched.
    ExecuteFixtureSql(connection, "CREATE TEMPORARY TABLE dr_save_fixture (name VARCHAR(32), value DECIMAL(10,0), payload VARBINARY(16), packed INT) ENGINE=InnoDB");
    ExecuteFixtureSql(connection, "INSERT INTO dr_save_fixture VALUES ('fixture',1,0x00,0),('duplicate',2,0x00,0),('duplicate',2,0x00,0),('quo''te',3,0x00,0)");
    SQLBIGINT value = 4294967295LL;
    unsigned char payload[] = {0, 1, 0xff, 0, 2};
    char name[] = "fixture";
    SQLBIGINT packed = CharacterSaveSignedDword(0xffffffffu);
    CharacterSaveParameter parameters[] = {
        {SQL_C_SBIGINT, SQL_BIGINT, &value, sizeof(value), 19},
        {SQL_C_BINARY, SQL_LONGVARBINARY, payload, sizeof(payload), sizeof(payload)},
        {SQL_C_SBIGINT, SQL_BIGINT, &packed, sizeof(packed), 19},
        {SQL_C_CHAR, SQL_VARCHAR, name, 7, 32}
    };
    const char* update = "UPDATE dr_save_fixture SET value=?,payload=?,packed=? WHERE name=?";
    assert(ExecuteCharacterSave(connection, update, parameters, 4) == 1);
    assert(ReadFixtureValue(connection, "SELECT value FROM dr_save_fixture WHERE name='fixture'") == value);
    assert(ReadFixtureValue(connection, "SELECT payload=0x0001ff0002 FROM dr_save_fixture WHERE name='fixture'") == 1);
    assert(ReadFixtureValue(connection, "SELECT packed FROM dr_save_fixture WHERE name='fixture'") == -1);
    assert(ExecuteCharacterSave(connection, update, parameters, 4) == 1); // unchanged values
    char duplicate[] = "duplicate";
    parameters[3].data = duplicate; parameters[3].bytes = 9;
    assert(ExecuteCharacterSave(connection, update, parameters, 4) == -3);
    assert(ReadFixtureValue(connection, "SELECT SUM(value) FROM dr_save_fixture WHERE name='duplicate'") == 4);
    char missing[] = "missing";
    parameters[3].data = missing; parameters[3].bytes = 7;
    assert(ExecuteCharacterSave(connection, update, parameters, 4) == -3);
    char quoted[] = "quo'te";
    parameters[3].data = quoted; parameters[3].bytes = 6;
    assert(ExecuteCharacterSave(connection, update, parameters, 4) == 1);
    assert(ExecuteCharacterSave(connection, "UPDATE dr_save_fixture SET missing_column=?", parameters, 1) == -5);
    assert(ReadFixtureValue(connection, "SELECT COUNT(*) FROM dr_save_fixture") == 4);
    SQLUINTEGER autocommit = 0;
    SQLGetConnectAttr(connection, SQL_ATTR_AUTOCOMMIT, &autocommit, sizeof(autocommit), NULL);
    assert(autocommit == SQL_AUTOCOMMIT_ON);
    SQLDisconnect(connection); SQLFreeHandle(SQL_HANDLE_DBC, connection); SQLFreeHandle(SQL_HANDLE_ENV, environment);
    std::cout << "Temporary-table saves: DWORD maximum, binary zeros, unchanged values, duplicate rollback, missing role and quoted names passed\n";
}
