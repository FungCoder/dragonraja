#include "../Mapserver/stdafx.h"
#include "../Mapserver/HigherLayers/WarStartupReader.h"
#include <cassert>
#include <cstdio>

CLottoSystem* CLottoSystem::m_pClass = NULL;

struct SqlHandles
{
    SQLHENV environment = SQL_NULL_HENV;
    SQLHDBC connection = SQL_NULL_HDBC;
    SQLHSTMT statement = SQL_NULL_HSTMT;
    ~SqlHandles()
    {
        if (statement) SQLFreeHandle(SQL_HANDLE_STMT, statement);
        if (connection) { SQLDisconnect(connection); SQLFreeHandle(SQL_HANDLE_DBC, connection); }
        if (environment) SQLFreeHandle(SQL_HANDLE_ENV, environment);
    }
};

#pragma pack(push, 1)
struct GuardedPoints
{
    unsigned char before[8];
    POINTS points[3];
    unsigned char after[8];
};
#pragma pack(pop)

int main()
{
    const char* settings = std::getenv("DRAGON_TEST_ODBC");
    if (!settings) return 2;
    SqlHandles sql;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql.environment)) ||
        !SQL_SUCCEEDED(SQLSetEnvAttr(sql.environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0)) ||
        !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, sql.environment, &sql.connection)) ||
        !SQL_SUCCEEDED(SQLDriverConnectA(sql.connection, NULL,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(settings)), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT)) ||
        !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, sql.connection, &sql.statement))) return 2;

    const char* schedules = "SELECT `Index`, DayofWeek, DHour FROM WarStartUpTBL WHERE Type IN (0,1,2) ORDER BY `Index`";
    if (!SQL_SUCCEEDED(SQLExecDirectA(sql.statement, (SQLCHAR*)schedules, SQL_NTS))) return 2;
    int scheduleCount = 0;
    SQLRETURN result;
    while (SQL_SUCCEEDED(result = SQLFetch(sql.statement)))
    {
        int index = -1, day = -1, hour = -1;
        assert(ReadWarStartupRow(sql.statement, index, day, hour));
        ++scheduleCount;
    }
    assert(result == SQL_NO_DATA && scheduleCount > 0);
    SQLFreeStmt(sql.statement, SQL_CLOSE);

    const char* points = "SELECT AttackerLivePointx,AttackerLivePointy,DefenderLivePointx,DefenderLivePointy,ReinforceLivePointx,ReinforceLivePointy FROM WarFieldTBL ORDER BY WarFieldCode";
    if (!SQL_SUCCEEDED(SQLExecDirectA(sql.statement, (SQLCHAR*)points, SQL_NTS))) return 2;
    int pointCount = 0;
    while (SQL_SUCCEEDED(result = SQLFetch(sql.statement)))
    {
        GuardedPoints guarded;
        memset(&guarded, 0xa5, sizeof(guarded));
        for (int team = 0; team < 3; ++team)
        {
            assert(ReadWarPoint(sql.statement, static_cast<SQLUSMALLINT>(1 + team * 2), guarded.points[team]));
            assert(guarded.points[team].x >= 0 && guarded.points[team].y >= 0);
        }
        for (int guard = 0; guard < 8; ++guard)
            assert(guarded.before[guard] == 0xa5 && guarded.after[guard] == 0xa5);
        ++pointCount;
    }
    assert(result == SQL_NO_DATA && pointCount > 0);
    SQLFreeStmt(sql.statement, SQL_CLOSE);

    const char* invalid[] = {
        "SELECT 1, NULL, 20", "SELECT 1, 7, 20", "SELECT 1, 1, 24"
    };
    for (const char* query : invalid)
    {
        assert(SQL_SUCCEEDED(SQLExecDirectA(sql.statement, (SQLCHAR*)query, SQL_NTS)));
        assert(SQL_SUCCEEDED(SQLFetch(sql.statement)));
        int index, day, hour;
        assert(!ReadWarStartupRow(sql.statement, index, day, hour));
        SQLFreeStmt(sql.statement, SQL_CLOSE);
    }
    const char* invalidPoints[] = { "SELECT -1, 10", "SELECT 10, 32768", "SELECT 10, NULL" };
    for (const char* query : invalidPoints)
    {
        assert(SQL_SUCCEEDED(SQLExecDirectA(sql.statement, (SQLCHAR*)query, SQL_NTS)));
        assert(SQL_SUCCEEDED(SQLFetch(sql.statement)));
        POINTS point = {11, 12};
        assert(!ReadWarPoint(sql.statement, 1, point));
        assert(point.x == 11 && point.y == 12);
        SQLFreeStmt(sql.statement, SQL_CLOSE);
    }
    std::printf("Read-only war startup rows=%d; warfield rows=%d; guarded coordinates and invalid inputs passed.\n", scheduleCount, pointCount);
    return 0;
}
