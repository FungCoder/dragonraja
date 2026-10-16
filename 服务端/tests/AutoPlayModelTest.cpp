#include "../Client/Debug/AutoPlayModel.h"
#include <cstdlib>
#include <iostream>

static void Require(bool condition, const char* scenario)
{
    if (!condition) { std::cerr << scenario << '\n'; std::exit(1); }
}

int main()
{
    using namespace DragonAutoPlayClient;
    using namespace DragonAutoPlay;
    Require(RemainingMs(0xfffffff0u, 20) == 36, "clock wrap before deadline");
    Require(RemainingMs(20, 0xfffffff0u) == 0, "clock wrap after deadline");
    Require(RemainingMs(100, 100) == 0, "exact cooldown boundary");
    Require(!IsIntervalValid(-1) && !IsIntervalValid(86401), "invalid intervals");
    Require(IsIntervalValid(0) && IsIntervalValid(86400), "interval boundaries");
    Require(ConfiguredIntervalMs(0) > 0, "zero interval cannot create tight loop");
    QueryTicket ticket;
    Request request = {Version, Magic, 154, 42, 8};
    Response response = {};
    response.version = Version; response.kind = Magic; response.skill = 154;
    response.target = 42; response.sequence = 7;
    ticket.Begin(request, 100);
    Require(!ticket.Accept(response), "old response rejected");
    response.sequence = 8; response.target = 43;
    Require(!ticket.Accept(response), "wrong target rejected");
    response.target = 42; response.kind = Combat;
    Require(!ticket.Accept(response), "wrong action system rejected");
    response.kind = Magic; response.skill = 155;
    Require(!ticket.Accept(response), "wrong skill rejected");
    response.skill = 154;
    Require(ticket.Accept(response), "matching response accepted");
    Require(!ticket.Accept(response), "duplicate response rejected");
    ticket.Begin(request, 0xffffff00u);
    Require(!ticket.Expired(0x20u), "query survives counter wrap");
    Require(ticket.Expired(0x1400u), "query timeout survives counter wrap");
    ticket.pending = false;
    Require(!ticket.Accept(response), "stopped action cannot accept late response");
    std::cout << "AutoPlayModelTest: timing and stale-reply scenarios passed\n";
}
