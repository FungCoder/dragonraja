#include "stdafx.h"
#include "QuestTrace.h"
#include "NavigationModel.h"
#include "AutoPlay.h"
#include "Dragon.h"
#include "Char.h"
#include "CharDataTable.h"
#include "Map.h"
#include "Menu.h"
#include "MenuDefine.h"
#include "GameProc.h"
#include "Tool.h"
#include "Path.h"
#include "MenuNetwork.h"
#include "kh_cpp.h"
#include <map>
#include <string>

extern void DoHeroMove(int moveType, int x, int y);
namespace
{
HWND traceWindow = NULL, listWindow = NULL, statusWindow = NULL;
HWND xWindow = NULL, yWindow = NULL;
std::map<int, int> observedQuests;
bool collapsed = false, navigating = false, refreshing = false;
int mapIdentity = -1, actorIdentity = -1, targetX = 0, targetY = 0;
int page = 0;
DWORD queryDeadline = 0, nextRefresh = 0, segmentDeadline = 0, nextMove = 0;
std::wstring navigationStatus = L"未启动导航";
void Status(const wchar_t* text)
{
    navigationStatus = text;
    if (statusWindow) SetWindowTextW(statusWindow, text);
}
std::wstring DecodeName(const char* name, std::size_t capacity)
{
    std::size_t length = 0;
    while (length < capacity && name[length]) ++length;
    if (!length || length == capacity) return L"";
    int count = MultiByteToWideChar(CP_ACP, 0, name, static_cast<int>(length), NULL, 0);
    std::wstring result(count, L'\0');
    if (count) MultiByteToWideChar(CP_ACP, 0, name, static_cast<int>(length), &result[0], count);
    return result;
}
void RenderQuests()
{
    if (!listWindow) return;
    SendMessageW(listWindow, LB_RESETCONTENT, 0, 0);
    if (observedQuests.empty())
        SendMessageW(listWindow, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"等待服务器任务记录；空列表不代表没有任务。"));
    for (const auto& entry : observedQuests)
    {
        wchar_t prefix[128];
        if (entry.second == 2000) swprintf_s(prefix, L"[%d] 已完成", entry.first);
        else swprintf_s(prefix, L"[%d] 服务器步骤 %d", entry.first, entry.second);
        const int textIndex = entry.first - 50;
        std::wstring text = prefix;
        if (textIndex > 0 && textIndex < QUEST_MAX)
        {
            const std::wstring name = DecodeName(quest_text[textIndex].name, sizeof(quest_text[textIndex].name));
            if (!name.empty()) text += L"  " + name;
        }
        text += L"  [导航目标未配置]";
        SendMessageW(listWindow, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
    }
}
void RequestPage()
{
    SendReqQuestIndex(page);
    queryDeadline = g_ClientTime + 5000;
}
void Refresh()
{
    if (!Hero || SysInfo.notconectserver || ChangeMapFlag || refreshing) return;
    if (SMenu[MN_QUEST_INDEX].bActive)
    { nextRefresh = g_ClientTime + 5000; return; }
    page = 0;
    observedQuests.clear();
    RenderQuests();
    refreshing = true;
    RequestPage();
}
bool Coordinate(HWND control, int maximum, int& value)
{
    wchar_t text[16];
    if (!GetWindowTextW(control, text, 16)) return false;
    wchar_t* end = NULL;
    long parsed = wcstol(text, &end, 10);
    if (!end || *end || parsed < 0 || parsed >= maximum) return false;
    value = static_cast<int>(parsed);
    return true;
}
void StartNavigation()
{
    if (!Hero || ChangeMapFlag || IsDead(Hero) || SysInfo.notconectserver)
    { Status(L"当前角色状态不能导航"); return; }
    if (!Coordinate(xWindow, g_Map.file.wWidth, targetX) ||
        !Coordinate(yWindow, g_Map.file.wHeight, targetY))
    { Status(L"请输入当前地图范围内的格子坐标"); return; }
    if (TileMap[targetX][targetY].attr_dont)
    { Status(L"标记点不可通行"); return; }
    AutoPlayStop();
    navigating = true;
    mapIdentity = MapNumber;
    actorIdentity = Hero->id;
    nextMove = g_ClientTime;
    segmentDeadline = g_ClientTime + 30000;
    Status(L"导航中：手动标记的同地图坐标；手动移动会取消续行");
    ShowWindow(traceWindow, SW_HIDE);
    SetForegroundWindow(g_hwndMain);
}
HWND Control(const wchar_t* type, const wchar_t* text, DWORD style,
    int x, int y, int width, int height, int id)
{
    HWND control = CreateWindowExW(0, type, text, WS_VISIBLE | WS_CHILD | style,
        x, y, width, height, traceWindow, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_hInstance, NULL);
    SendMessageW(control, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
    return control;
}
LRESULT CALLBACK WindowProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CLOSE) { ShowWindow(window, SW_HIDE); return 0; }
    if (message == WM_COMMAND)
    {
        switch (LOWORD(wParam))
        {
        case 1: Refresh(); return 0;
        case 2:
            collapsed = !collapsed;
            SetWindowPos(window, NULL, 0, 0, 700, collapsed ? 78 : 435, SWP_NOMOVE | SWP_NOZORDER);
            return 0;
        case 3: StartNavigation(); return 0;
        case 4: QuestTraceCancelNavigation(); return 0;
        }
    }
    return DefWindowProcW(window, message, wParam, lParam);
}
}
void QuestTraceShow()
{
    nextRefresh = g_ClientTime;
    if (!traceWindow)
    {
        WNDCLASSW type = {};
        type.lpfnWndProc = WindowProcedure;
        type.hInstance = g_hInstance;
        type.hCursor = LoadCursor(NULL, IDC_ARROW);
        type.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        type.lpszClassName = L"DragonQuestTrace";
        RegisterClassW(&type);
        traceWindow = CreateWindowExW(WS_EX_TOOLWINDOW, type.lpszClassName, L"任务追踪与地图标记",
            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 700, 435,
            g_hwndMain, NULL, g_hInstance, NULL);
        if (!traceWindow) return;
        Control(L"BUTTON", L"刷新服务器任务", BS_PUSHBUTTON, 12, 10, 140, 26, 1);
        Control(L"BUTTON", L"折叠 / 展开", BS_PUSHBUTTON, 165, 10, 110, 26, 2);
        Control(L"STATIC", L"以下为服务器最近返回的任务步骤；不会按说明文本推算进度。", 0, 12, 44, 660, 24, 0);
        listWindow = Control(L"LISTBOX", L"", WS_BORDER | WS_VSCROLL | WS_HSCROLL | LBS_NOINTEGRALHEIGHT,
            12, 70, 660, 190, 0);
        SendMessageW(listWindow, LB_SETHORIZONTALEXTENT, 1200, 0);
        Control(L"STATIC", L"独立地图标记（来源：用户输入；不自动关联任务）", 0, 12, 270, 660, 22, 0);
        Control(L"STATIC", L"格子 X", 0, 12, 302, 45, 22, 0);
        xWindow = Control(L"EDIT", L"0", WS_BORDER | ES_NUMBER, 58, 298, 65, 24, 0);
        Control(L"STATIC", L"Y", 0, 135, 302, 20, 22, 0);
        yWindow = Control(L"EDIT", L"0", WS_BORDER | ES_NUMBER, 158, 298, 65, 24, 0);
        Control(L"BUTTON", L"导航至标记", BS_PUSHBUTTON, 240, 298, 120, 26, 3);
        Control(L"BUTTON", L"取消续行", BS_PUSHBUTTON, 372, 298, 110, 26, 4);
        statusWindow = Control(L"STATIC", navigationStatus.c_str(), 0, 12, 340, 660, 42, 0);
        RenderQuests();
    }
    ShowWindow(traceWindow, SW_SHOW);
    Refresh();
}
void QuestTraceReceive(const void* data, std::size_t size)
{
    if (!data || size < 1 || SysInfo.notconectserver) return;
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    const unsigned count = bytes[0];
    if (count > 14 || size != 1 + count * 4) return;
    for (unsigned index = 0; index < count; ++index)
    {
        short id = 0, step = 0;
        memcpy(&id, bytes + 1 + index * 4, 2);
        memcpy(&step, bytes + 3 + index * 4, 2);
        if (id > 50 && id < 300) observedQuests[id] = step;
    }
    RenderQuests();
    if (refreshing)
    {
        if (SMenu[MN_QUEST_INDEX].bActive)
        { refreshing = false; nextRefresh = g_ClientTime + 5000; return; }
        if (count == 14 && page < 17) { ++page; RequestPage(); }
        else { refreshing = false; nextRefresh = g_ClientTime + 10000; }
    }
}
void QuestTraceCancelNavigation()
{
    if (navigating) Status(L"续行已取消；当前短移动段由原移动流程结束");
    navigating = false;
}
void QuestTraceReset()
{
    QuestTraceCancelNavigation();
    refreshing = false;
    observedQuests.clear();
    RenderQuests();
    mapIdentity = actorIdentity = -1;
}
void QuestTraceTick()
{
    const DWORD now = g_ClientTime;
    if (SMenu[MN_QUEST_INDEX].bActive)
    { refreshing = false; nextRefresh = now + 5000; }
    if (refreshing && static_cast<LONG>(now - queryDeadline) >= 0)
    { refreshing = false; nextRefresh = now + 10000; }
    if (traceWindow && IsWindowVisible(traceWindow) && !refreshing &&
        static_cast<LONG>(now - nextRefresh) >= 0) Refresh();
    if (!navigating) return;
    if (!Hero || IsDead(Hero) || ChangeMapFlag || MapNumber != mapIdentity || Hero->id != actorIdentity ||
        SysInfo.notconectserver || AutoPlayIsRunning())
    { QuestTraceCancelNavigation(); return; }
    if (static_cast<LONG>(now - segmentDeadline) >= 0)
    { navigating = false; Status(L"导航停止：移动没有进展或目标不可达"); return; }
    if (Hero->moveFlag) return;
    const int startX = Hero->x / TILE_SIZE, startY = Hero->y / TILE_SIZE;
    if (Hero->x < 0 || Hero->y < 0 || startX >= g_Map.file.wWidth || startY >= g_Map.file.wHeight)
    { navigating = false; Status(L"导航停止：角色坐标超出当前地图"); return; }
    if (startX == targetX && startY == targetY)
    { navigating = false; Status(L"已到达手动标记"); return; }
    if (static_cast<LONG>(now - nextMove) < 0) return;
    nextMove = now + 500;
    // Reuse the live map collision rules; release all PathFind scratch nodes before movement.
    TILE& origin = TileMap[startX][startY];
    const DWORD occupied = origin.occupied;
    origin.occupied = 0;
    LPSP_NODE destination = PathFind(Hero, startX, startY, targetX, targetY);
    origin.occupied = occupied;
    DragonNavigation::Tile segment = {};
    const bool found = DragonNavigation::SelectSegment(destination, startX, startY, 16, segment);
    PathDestroy();
    if (!found)
    { navigating = false; Status(L"导航停止：现有寻路器未找到可用路线"); return; }
    DoHeroMove(1, segment.x * TILE_SIZE + TILE_SIZE / 2, segment.y * TILE_SIZE + TILE_SIZE / 2);
    if (Hero->moveFlag)
    {
        segmentDeadline = now + 30000;
    }
}
