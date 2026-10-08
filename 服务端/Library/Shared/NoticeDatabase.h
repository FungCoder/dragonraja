#pragma once
#include <sql.h>
#include <sqlext.h>
#include "NoticeEncoding.h"

inline bool ReadDatabaseNoticeGbk(SQLHSTMT statement, SQLUSMALLINT column, std::string& gbk)
{
    WCHAR unicode[4097] = {};
    SQLLEN indicator = 0;
    const SQLRETURN result = SQLGetData(statement, column, SQL_C_WCHAR, unicode, sizeof(unicode), &indicator);
    if (indicator == SQL_NULL_DATA || (result == SQL_SUCCESS && indicator == 0)) { gbk.clear(); return true; }
    if (result != SQL_SUCCESS || indicator < 0 || indicator >= sizeof(unicode)) return false;
    return EncodeNoticeGbk(unicode, static_cast<int>(wcsnlen_s(unicode, 4097)), gbk);
}
