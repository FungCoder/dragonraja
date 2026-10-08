#include <windows.h>
#include <cassert>
#include <string>
#include "../Library/Shared/NoticeEncoding.h"
#include "../AgentServer/ManagerNotice.h"

static int calls = 0;
static std::string received;
static void FakeBroadcast(char* text, int length, DWORD* targets, DWORD* failed)
{
    ++calls;
    received.assign(text, length);
    *targets = 2;
    *failed = 0;
}

int main()
{
    std::string gbk;
    assert(EncodeNoticeGbk(L"\x4e2d\x6587\x516c\x544a", 4, gbk));
    assert(gbk == std::string("\xd6\xd0\xce\xc4\xb9\xab\xb8\xe6", 8));
    assert(!EncodeNoticeGbk(L"\xd83d\xde00", 2, received));
    assert(!EncodeNoticeGbk(L"", 0, received));
    BYTE request[64] = {}, reply[32] = {};
    memcpy(request, "DRN1", 4);
    ULONGLONG expires = 200000000ULL;
    DWORD length = static_cast<DWORD>(gbk.size());
    memcpy(request + 20, &expires, 8);
    memcpy(request + 28, &length, 4);
    memcpy(request + 32, gbk.data(), length);
    ProcessManagerNotice(request, 32 + length, 100000000ULL, FakeBroadcast, reply);
    DWORD status = 99;
    memcpy(&status, reply + 20, 4);
    assert(status == 0 && calls == 1 && received == gbk);
    ProcessManagerNotice(request, 32 + length, 300000000ULL, FakeBroadcast, reply);
    memcpy(&status, reply + 20, 4);
    assert(status == 2 && calls == 1);
    request[32] = 0;
    ProcessManagerNotice(request, 32 + length, 100000000ULL, FakeBroadcast, reply);
    memcpy(&status, reply + 20, 4);
    assert(status == 1 && calls == 1);
    request[32] = '%';
    ProcessManagerNotice(request, 32 + length, 100000000ULL, FakeBroadcast, reply);
    memcpy(&status, reply + 20, 4);
    assert(status == 1 && calls == 1);
    length = 259;
    memcpy(request + 28, &length, 4);
    ProcessManagerNotice(request, 32, 100000000ULL, FakeBroadcast, reply);
    assert(calls == 1);
    return 0;
}
