#include "../Library/Shared/PacketSafety.h"
#include <cstdlib>
#include <iostream>
#include <limits>

namespace
{
void Require(bool condition, const char* scenario)
{
    if (!condition)
    {
        std::cerr << "Packet safety failure: " << scenario << '\n';
        std::exit(1);
    }
}
}

int main()
{
    using namespace DragonPacketSafety;
    const std::size_t header = 5;
    const std::size_t clientCapacity = 3000;
    const std::size_t mapCapacity = 10240;

    Require(IsCompleteFrameValid(header, 0, header, clientCapacity), "empty keepalive");
    Require(IsCompleteFrameValid(header + clientCapacity, 3000, header, clientCapacity), "maximum client payload");
    Require(IsCompleteFrameValid(header + mapCapacity, 10240, header, mapCapacity), "maximum map payload");
    Require(!IsCompleteFrameValid(header + 1, -1, header, clientCapacity), "negative payload declaration");
    Require(!IsCompleteFrameValid(header + 3001, 3001, header, clientCapacity), "oversized client payload");
    Require(!IsCompleteFrameValid(header + 10241, 10241, header, mapCapacity), "oversized forwarded payload");
    Require(!IsCompleteFrameValid(header + 8, 9, header, clientCapacity), "truncated body");
    Require(!IsCompleteFrameValid(header + 9, 8, header, clientCapacity), "trailing body bytes");
    Require(!IsCompleteFrameValid(header + 65536, 0, header, clientCapacity), "WORD truncation cannot hide oversized frame");
    Require(!IsFrameSizeValid(std::numeric_limits<std::size_t>::max(), header, clientCapacity), "very large frame");
    Require(!IsFrameSizeValid(0, std::numeric_limits<std::size_t>::max(), clientCapacity), "header subtraction underflow");
    for (std::size_t size = 0; size < header; ++size)
        Require(!IsFrameSizeValid(size, header, clientCapacity), "incomplete header");

    Require(IsReadWindowValid(2, 0, 2), "initial length prefix");
    Require(IsReadWindowValid(1, 1, 2), "fragmented length prefix");
    Require(IsReadWindowValid(3, 2, header), "fragmented application header");
    Require(IsReadWindowValid(1, 2999, clientCapacity), "last body byte");
    Require(!IsReadWindowValid(2, 2999, clientCapacity), "body write past capacity");
    Require(!IsReadWindowValid(-1, 0, clientCapacity), "negative pending receive");
    Require(!IsReadWindowValid(1, -1, clientCapacity), "negative receive offset");
    Require(!IsReadWindowValid(0, 0, clientCapacity), "zero-sized receive must not run");
    Require(!IsReadWindowValid(1, std::numeric_limits<int>::max(), clientCapacity), "receive offset overflow");
    std::cout << "PacketSafetyTest: all protocol boundary scenarios passed\n";
    return 0;
}
