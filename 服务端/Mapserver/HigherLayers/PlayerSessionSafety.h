#pragma once
#include "../../Library/Shared/MagicPreparationGate.h"

// Include after the map server's connection and character declarations.
namespace DragonGameplay
{
inline bool IsPlayerSession(int connection)
{
    if (connection < DRAGON_CONNECTIONS_START || connection >= DRAGON_MAX_CONNECTIONS)
        return false;
    const t_connection& session = connections[connection];
    return session.state == CONNECT_JOIN && !session.OnMapMoving &&
        session.chrlst.IsPlayer() && session.chrlst.GetServerID() == connection;
}

inline PreparationIdentity GetPlayerSessionIdentity(int connection)
{
    const t_connection& session = connections[connection];
    PreparationIdentity identity = {session.dwAgentConnectionIndex,
        session.dwIDForAgent, session.chrlst.total_id, uint32_t(MapNumber)};
    return identity;
}
}
