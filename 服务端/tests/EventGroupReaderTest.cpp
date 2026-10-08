#include "../Mapserver/stdafx.h"
#include "../Mapserver/HigherLayers/EventMonsterReader.h"
#include <cassert>
#include <cstdlib>

// The legacy umbrella header references this unrelated singleton at startup.
CLottoSystem* CLottoSystem::m_pClass = nullptr;

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
struct GuardedPosition
{
    unsigned char before[8];
    NPCGenerationPos position;
    unsigned char after[8];
};
#pragma pack(pop)

int main()
{
    const int sparse[] = {1, 3, 6};
    assert(FindEventGroupIndex(sparse, 3, 1) == 0);
    assert(FindEventGroupIndex(sparse, 3, 6) == 2);
    assert(FindEventGroupIndex(sparse, 3, 0) == -1);
    assert(FindEventGroupIndex(sparse, 0, 1) == -1);
    assert(FindEventGroupIndex(sparse, 4, 1) == -1);
    assert(!IsEventIndexValid(1, 1) && !IsEventIndexValid(-1, 1));
    assert(!IsEventIndexValid(0, 0) && IsEventIndexValid(0, 1));

    const char* settings = std::getenv("DRAGON_TEST_ODBC");
    if (!settings) { std::fprintf(stderr, "Read-only test connection is not configured.\n"); return 2; }
    SqlHandles sql;
    if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql.environment)) ||
        !SQL_SUCCEEDED(SQLSetEnvAttr(sql.environment, SQL_ATTR_ODBC_VERSION, reinterpret_cast<SQLPOINTER>(SQL_OV_ODBC3), 0)) ||
        !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, sql.environment, &sql.connection)) ||
        !SQL_SUCCEEDED(SQLDriverConnectA(sql.connection, NULL,
            reinterpret_cast<SQLCHAR*>(const_cast<char*>(settings)), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT)) ||
        !SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_STMT, sql.connection, &sql.statement)))
    {
        std::fprintf(stderr, "Read-only test connection failed.\n"); return 2;
    }
    const char* query = "SELECT * FROM dragonraja.event_monster ORDER BY Map_Port, GroupNo, `Index`";
    if (!SQL_SUCCEEDED(SQLExecDirectA(sql.statement,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(query)), SQL_NTS))) return 2;
    int rows = 0;
    SQLRETURN result;
    while (SQL_SUCCEEDED(result = SQLFetch(sql.statement)))
    {
        GuardedPosition fixture;
        memset(fixture.before, 0xa5, sizeof(fixture.before));
        memset(fixture.after, 0xa5, sizeof(fixture.after));
        if (!ReadEventMonsterRow(sql.statement, fixture.position)) return 1;
        for (int guard = 0; guard < 8; ++guard)
        {
            assert(fixture.before[guard] == 0xa5 && fixture.after[guard] == 0xa5);
        }
        assert(fixture.position.CurNPC == 0);
        ++rows;
    }
    if (result != SQL_NO_DATA) return 1;
    SQLFreeStmt(sql.statement, SQL_CLOSE);
    const char* legacyProbe = "SELECT CAST(7 AS SIGNED)";
    if (!SQL_SUCCEEDED(SQLExecDirectA(sql.statement,
        reinterpret_cast<SQLCHAR*>(const_cast<char*>(legacyProbe)), SQL_NTS)) ||
        !SQL_SUCCEEDED(SQLFetch(sql.statement))) return 2;
    GuardedPosition legacyFixture;
    memset(legacyFixture.after, 0xa5, sizeof(legacyFixture.after));
    SQLLEN indicator = 0;
    // Reproduce the old four-byte write inside an intentionally guarded fixture.
    if (!SQL_SUCCEEDED(SQLGetData(sql.statement, 1, SQL_C_SLONG,
        &legacyFixture.position.MaxNo, sizeof(int), &indicator))) return 2;
    assert(legacyFixture.after[0] != 0xa5 || legacyFixture.after[1] != 0xa5);
    std::printf("sparse-group/bounds checks passed; ODBC rows=%d; record_bytes=%zu; guards intact\n",
                rows, sizeof(NPCGenerationPos));
    std::printf("legacy four-byte/short-field overwrite reproduced in guarded fixture\n");
    return 0;
}
