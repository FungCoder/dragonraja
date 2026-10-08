// ----------------------------------------------------------------------
//    File Name: ServerMonitor.cpp
// Last Updated: 2000/11/28 (by chan78)
//  Description: Functions to handle commands that from Monitoring Client
// ----------------------------------------------------------------------
#include "ServerMonitor.h"
#include "servertable.h"
#include "RecvMsgFromUser.h"
#include "RecvMsgFromServer.h"
#include "mylog.h"

void SendConnectServerCount( DWORD dwConnectionIndex )
{
	t_packet packet;
	packet.h.header.type = CMD_SV_CONNECT_SERVER_COUNT;
	packet.u.kein.default_char = (unsigned char)g_pServerTable->GetNumOfConnectedServers();
	packet.h.header.size = sizeof( char );
	if( !g_pUserTable->SendToUserByConnectionIndex( dwConnectionIndex, (char *)&packet, sizeof(t_header)+packet.h.header.size) )
	{
		MyLog( LOG_FATAL, "SendConnectServerCount() :: Failed to send Packet(dwConnectionIndex:%d)", dwConnectionIndex );
	}
}

void SendCurrServerPort( DWORD dwConnectionIndex )
{
	t_packet packet;
	LP_SERVER_DATA cur = g_pServerTable->GetServerListHead();
	packet.h.header.type = CMD_SV_GET_CONNECT_SERVER_PORT;
	short int &count = packet.u.kein.server_port.count;
	count = 1;

	packet.u.kein.server_port.port[0] = g_pServerTable->GetOwnServerData()->wPort;
	
	while( cur )
	{
		if( cur->dwConnectionIndex != 0 ) {
			count++;
			packet.u.kein.server_port.port[count] = cur->wPort;
		}
		cur = cur->pNextServerData;
	}
	packet.h.header.size = sizeof( short int ) + sizeof( short int )*count;

	if( !g_pUserTable->SendToUserByConnectionIndex( dwConnectionIndex, (char*)&packet, sizeof(t_header)+packet.h.header.size ) ){
		MyLog( LOG_FATAL, "SendCurrServerPort() :: Failed to send Packet(dwConnectionIndex:%d)", dwConnectionIndex );
	}
}

void BroadcastManagerNotice(char* text, int length, DWORD* targets, DWORD* failed)
{
    *targets = 0;
    *failed = 0;
    if (!text || length <= 0 || length > 258 || !g_pServerTable || !g_pINet) return;
    t_packet packet = {};
    packet.h.header.type = CMD_LOGIN_BBS;
    memcpy(packet.u.login_bbs.bbs, text, length);
    packet.h.header.size = length + 1; // Include the terminator for the legacy client.
    char envelope[1 + sizeof(t_header) + sizeof(t_login_bbs)] = {};
    envelope[0] = (BYTE)PTCL_BROADCAST_TO_SERVERS;
    memcpy(envelope + 1, &packet, sizeof(t_header) + packet.h.header.size);
    for (LP_SERVER_DATA entry = g_pServerTable->GetServerListHead(); entry; entry = entry->pNextServerData) {
        if (entry->dwServerType == SERVER_TYPE_MAP && entry->dwConnectionIndex && entry->dwStatus == STATUS_ACTIVATED) {
            ++*targets;
            if (!g_pINet->SendToServer(entry->dwConnectionIndex, envelope,
                1 + sizeof(t_header) + packet.h.header.size, FLAG_SEND_NOT_ENCRYPTION)) ++*failed;
        }
    }
}

void SendPbs(char* text, int length)
{
    if (!text || length <= 0) return;
    const size_t actual = strnlen_s(text, MAX_PATH);
    if (actual == 0 || actual > 258) return;
    DWORD targets = 0, failed = 0;
    BroadcastManagerNotice(text, static_cast<int>(actual), &targets, &failed);
    if (failed) MyLog(LOG_FATAL, "Notice broadcast failed for %lu of %lu maps", failed, targets);
}