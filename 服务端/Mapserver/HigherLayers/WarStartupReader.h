#pragma once

// Callers select Index, DayofWeek, DHour explicitly in this order.
inline bool ReadWarStartupRow(SQLHSTMT statement, int& index, int& day, int& hour)
{
    int* values[] = { &index, &day, &hour };
    for (int column = 0; column < 3; ++column)
    {
        SQLLEN indicator = 0;
        if (SQLGetData(statement, static_cast<SQLUSMALLINT>(column + 1),
                       SQL_C_SLONG, values[column], sizeof(int), &indicator) !=
                SQL_SUCCESS || indicator == SQL_NULL_DATA)
        {
            return false;
        }
    }
    return index >= 0 && day >= 0 && day <= 6 && hour >= 0 && hour <= 23;
}

// POINTS contains 16-bit x/y; never let ODBC write a 32-bit integer into it.
template <class Point>
inline bool ReadWarPoint(SQLHSTMT statement, SQLUSMALLINT column, Point& point)
{
    int values[2] = {};
    for (int offset = 0; offset < 2; ++offset)
    {
        SQLLEN indicator = 0;
        if (SQLGetData(statement, static_cast<SQLUSMALLINT>(column + offset),
                       SQL_C_SLONG, &values[offset], sizeof(int), &indicator) !=
                SQL_SUCCESS || indicator == SQL_NULL_DATA ||
            values[offset] < 0 || values[offset] > 32767)
        {
            return false;
        }
    }
    point.x = static_cast<short>(values[0]);
    point.y = static_cast<short>(values[1]);
    return true;
}
