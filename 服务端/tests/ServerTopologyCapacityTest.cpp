#include <windows.h>
#if defined(DR_TEST_AGENT)
#include "../AgentServer/define.h"
#elif defined(DR_TEST_DB)
#include "../DBDemon/define.h"
#else
#include "../Mapserver/LowerLayers/define.h"
#endif
#include <cstdio>
static_assert(MAX_SERVER_NUM == DRAGON_SERVER_CAPACITY, "Service capacity must use the shared limit");
static_assert(MAX_SERVER_NUM >= 103 + 3, "All catalogue maps and core services must fit");
int main()
{
    std::printf("Topology capacity=%u; 103 maps plus 3 services fit\n", unsigned(MAX_SERVER_NUM));
    return 0;
}
