#pragma once
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <cstring>

struct CharacterSaveParameter
{
    SQLSMALLINT cType;
    SQLSMALLINT sqlType;
    SQLPOINTER data;
    SQLLEN bytes;
    SQLULEN columnSize;
};

inline bool IsCharacterSaveTextValid(const char* text, size_t capacity)
{
    return text && capacity > 0 && text[0] &&
        std::memchr(text, 0, capacity) != NULL;
}

// Legacy packed DWORD fields are stored in signed SQL INT columns.
inline SQLBIGINT CharacterSaveSignedDword(DWORD value)
{
    static_assert(sizeof(value) == sizeof(SQLINTEGER), "DWORD storage must remain 32-bit");
    SQLINTEGER signedValue = 0;
    std::memcpy(&signedValue, &value, sizeof(signedValue));
    return signedValue;
}

// One statement is one save unit. Roll back if it does not match exactly one row.
// A failed commit has an unknown outcome and must never be retried blindly.
inline int ExecuteCharacterSave(SQLHDBC connection, const char* query,
    CharacterSaveParameter* parameters, size_t count)
{
    if (!connection || !query || !parameters || count == 0 || count > 96)
        return -5;
    SQLUINTEGER autocommit = 0;
    if (!SQL_SUCCEEDED(SQLGetConnectAttr(connection, SQL_ATTR_AUTOCOMMIT,
        &autocommit, sizeof(autocommit), NULL)) || autocommit != SQL_AUTOCOMMIT_ON)
        return -5;
    SQLHSTMT statement = SQL_NULL_HSTMT;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, connection, &statement)))
        return -5;
    SQLLEN lengths[96] = {};
    SQLRETURN result = SQLPrepareA(statement,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(query)), SQL_NTS);
    for (size_t index = 0; SQL_SUCCEEDED(result) && index < count; ++index)
    {
        const CharacterSaveParameter& parameter = parameters[index];
        if (!parameter.data || parameter.bytes < 0 || !parameter.columnSize)
        {
            result = SQL_ERROR;
            break;
        }
        lengths[index] = parameter.bytes;
        result = SQLBindParameter(statement, static_cast<SQLUSMALLINT>(index + 1),
            SQL_PARAM_INPUT, parameter.cType, parameter.sqlType,
            parameter.columnSize, 0, parameter.data, parameter.bytes, &lengths[index]);
    }
    if (!SQL_SUCCEEDED(result) ||
        !SQL_SUCCEEDED(SQLSetConnectAttr(connection, SQL_ATTR_AUTOCOMMIT,
            reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_OFF), 0)))
    {
        SQLFreeHandle(SQL_HANDLE_STMT, statement);
        return -5;
    }
    result = SQLExecute(statement);
    SQLLEN affected = 0;
    const bool saved = result == SQL_SUCCESS &&
        SQL_SUCCEEDED(SQLRowCount(statement, &affected)) && affected == 1;
    SQLFreeHandle(SQL_HANDLE_STMT, statement);
    const SQLRETURN completed = SQLEndTran(SQL_HANDLE_DBC, connection,
        saved ? SQL_COMMIT : SQL_ROLLBACK);
    if (!SQL_SUCCEEDED(completed))
    {
        // Do not turn autocommit on after a failed rollback: that can commit data.
        SQLDisconnect(connection);
        return -6;
    }
    if (!SQL_SUCCEEDED(SQLSetConnectAttr(connection, SQL_ATTR_AUTOCOMMIT,
        reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON), 0)))
    {
        SQLDisconnect(connection);
        return -6;
    }
    return saved ? 1 : -3;
}
