#pragma once
#include <sql.h>
#include <sqlext.h>
#include "AdventManager.h"

struct EventColumnBinding
{
    SQLSMALLINT type;
    SQLPOINTER value;
    SQLLEN size;
};

template<std::size_t Count>
bool ReadEventColumns(HSTMT statement, int firstColumn,
                      const EventColumnBinding (&fields)[Count])
{
    for (std::size_t field = 0; field < Count; ++field)
    {
        SQLLEN indicator = 0;
        const SQLRETURN result = SQLGetData(statement, static_cast<SQLUSMALLINT>(firstColumn + field),
            fields[field].type, fields[field].value, fields[field].size, &indicator);
        if (!SQL_SUCCEEDED(result) || indicator == SQL_NULL_DATA) return false;
    }
    return true;
}

inline bool ReadEventMonsterRow(HSTMT statement, NPCGenerationPos& position)
{
    const EventColumnBinding fields[] = {
        {SQL_C_STINYINT, &position.GroupNo, sizeof(position.GroupNo)},
        {SQL_C_STINYINT, &position.isBoss, sizeof(position.isBoss)},
        {SQL_C_SSHORT, &position.NPCNo, sizeof(position.NPCNo)},
        {SQL_C_SSHORT, &position.LocationX, sizeof(position.LocationX)},
        {SQL_C_SSHORT, &position.LocationY, sizeof(position.LocationY)},
        {SQL_C_SSHORT, &position.EventNo, sizeof(position.EventNo)},
        {SQL_C_SSHORT, &position.MaxNo, sizeof(position.MaxNo)}
    };
    return ReadEventColumns(statement, 3, fields);
}
