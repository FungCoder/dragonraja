#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <cassert>
#include <cstdio>
#include <initializer_list>

struct FakeSaveState
{
    SQLRETURN prepare = SQL_SUCCESS, bind = SQL_SUCCESS, execute = SQL_SUCCESS;
    SQLRETURN rowCount = SQL_SUCCESS, finish = SQL_SUCCESS, restore = SQL_SUCCESS;
    SQLLEN rows = 1;
    SQLUINTEGER autocommit = SQL_AUTOCOMMIT_ON;
    int executes = 0, commits = 0, rollbacks = 0, restores = 0, disconnects = 0;
};
static FakeSaveState state;
static SQLRETURN FakeGetAttr(SQLHDBC, SQLINTEGER, SQLPOINTER value, SQLINTEGER, SQLINTEGER*)
{ *static_cast<SQLUINTEGER*>(value) = state.autocommit; return SQL_SUCCESS; }
static SQLRETURN FakeAlloc(SQLSMALLINT, SQLHANDLE, SQLHANDLE* handle)
{ *handle = reinterpret_cast<SQLHANDLE>(2); return SQL_SUCCESS; }
static SQLRETURN FakePrepare(SQLHSTMT, SQLCHAR*, SQLINTEGER) { return state.prepare; }
static SQLRETURN FakeBind(SQLHSTMT, SQLUSMALLINT, SQLSMALLINT, SQLSMALLINT,
    SQLSMALLINT, SQLULEN, SQLSMALLINT, SQLPOINTER, SQLLEN, SQLLEN*) { return state.bind; }
static SQLRETURN FakeSetAttr(SQLHDBC, SQLINTEGER, SQLPOINTER value, SQLINTEGER)
{
    if (value == reinterpret_cast<SQLPOINTER>(SQL_AUTOCOMMIT_ON))
    { ++state.restores; return state.restore; }
    return SQL_SUCCESS;
}
static SQLRETURN FakeExecute(SQLHSTMT) { ++state.executes; return state.execute; }
static SQLRETURN FakeRows(SQLHSTMT, SQLLEN* rows) { *rows = state.rows; return state.rowCount; }
static SQLRETURN FakeFree(SQLSMALLINT, SQLHANDLE) { return SQL_SUCCESS; }
static SQLRETURN FakeFinish(SQLSMALLINT, SQLHANDLE, SQLSMALLINT action)
{ action == SQL_COMMIT ? ++state.commits : ++state.rollbacks; return state.finish; }
static SQLRETURN FakeDisconnect(SQLHDBC) { ++state.disconnects; return SQL_SUCCESS; }
#define SQLGetConnectAttr FakeGetAttr
#define SQLAllocHandle FakeAlloc
#define SQLPrepareA FakePrepare
#define SQLBindParameter FakeBind
#define SQLSetConnectAttr FakeSetAttr
#define SQLExecute FakeExecute
#define SQLRowCount FakeRows
#define SQLFreeHandle FakeFree
#define SQLEndTran FakeFinish
#define SQLDisconnect FakeDisconnect
#include "../Library/Shared/CharacterSave.h"

int main()
{
    assert(CharacterSaveSignedDword(0x80000000u) == -2147483647LL - 1);
    assert(CharacterSaveSignedDword(0xffffffffu) == -1);
    SQLBIGINT value = 42;
    CharacterSaveParameter parameter = {SQL_C_SBIGINT, SQL_BIGINT, &value, sizeof(value), 19};
    const auto save = [&]() { return ExecuteCharacterSave(reinterpret_cast<SQLHDBC>(1),
        "UPDATE fixture SET value=?", &parameter, 1); };
    char unterminated[20]; memset(unterminated, 'x', sizeof(unterminated));
    assert(!IsCharacterSaveTextValid(unterminated, sizeof(unterminated)));
    assert(!IsCharacterSaveTextValid("", 1));
    assert(IsCharacterSaveTextValid("name", 5));
    assert(save() == 1 && state.executes == 1 && state.commits == 1 && state.restores == 1);
    state = {}; state.prepare = SQL_ERROR;
    assert(save() == -5 && state.executes == 0 && state.commits == 0);
    state = {}; state.bind = SQL_ERROR;
    assert(save() == -5 && state.executes == 0);
    state = {}; state.execute = SQL_ERROR;
    assert(save() == -3 && state.executes == 1 && state.rollbacks == 1 && state.commits == 0);
    state = {}; state.execute = SQL_SUCCESS_WITH_INFO;
    assert(save() == -3 && state.rollbacks == 1 && state.commits == 0);
    for (SQLLEN rows : {SQLLEN(0), SQLLEN(2)})
    {
        state = {}; state.rows = rows;
        assert(save() == -3 && state.rollbacks == 1 && state.commits == 0);
    }
    state = {}; state.rowCount = SQL_ERROR;
    assert(save() == -3 && state.rollbacks == 1);
    state = {}; state.finish = SQL_ERROR;
    assert(save() == -6 && state.commits == 1 && state.restores == 0 && state.disconnects == 1);
    state = {}; state.execute = SQL_ERROR; state.finish = SQL_ERROR;
    assert(save() == -6 && state.rollbacks == 1 && state.restores == 0 && state.disconnects == 1);
    state = {}; state.restore = SQL_ERROR;
    assert(save() == -6 && state.disconnects == 1);
    state = {}; state.autocommit = SQL_AUTOCOMMIT_OFF;
    assert(save() == -5 && state.executes == 0 && state.restores == 0);
    std::puts("Save failure injection: prepare/bind/execute/warnings/row count/commit/rollback/restore passed");
}
