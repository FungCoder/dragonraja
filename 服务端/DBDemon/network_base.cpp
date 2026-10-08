#include "network_base.h"
#include "typedef.h"
#include "servertable.h"
#include "network_guid.h"
#include "recvmsg.h"
#include "monitor.h"
#include "mylog.h"

I4DyuchiNET* g_pINet = NULL;
HANDLE hKeyEvent = NULL;
//2001/02/19 zhh
extern HANDLE	g_hEvent;
HANDLE	g_hEvent = NULL;
#include "./china/mysocket.h"
extern CMySocket *ConC;
//#include "../China/QueryDBSocket.h"
	
// 021128 YGI
void __stdcall RecvMessageFromBillingServer(DWORD dwValue)//020511 lsw
{	 
	ConC->OnReceive(0);
}	
	
	
void __stdcall AwaitingProxyServerConnect(DWORD dwValue)//020511 lsw
{	
	LP_SERVER_DATA pProxy = NULL;
	
	// Proxy �� ������� ���� ���.
	if( g_pServerTable )
	{
		pProxy = g_pServerTable->GetOwnProxyServerData();
		if( pProxy )
		{
			if( !g_pServerTable->GetOwnProxyServerData()->dwConnectionIndex )
			{
				// Added by chan78 at 2001/04/11 :: �̹� ���� �õ����� ��� �õ����� �ʴ´�.
				if( g_pServerTable->GetConnectionResultData()->dwConnectionType != CONNECT_TYPE_NONE )
				{
					return;
				}

				MyLog( LOG_NORMAL, "@@ Trying to connect PROXY Server ..." );
				// ���� �õ�.
				if( g_pServerTable->ConnectToProxyServer() )
				{
					// Modified by chan78 at 2001/04/11 :: Ÿ�̸� ���� ó���� ���ó�� �ݹ��Լ�����.
					return;
				}
			}
			else
			{
				g_pINet->PauseTimer( 0 );
				return;
			}
		}
	}
	
	return;
}	
	
void __stdcall ShowDBServerStatus(DWORD dwValue)//020511 lsw
{	
	if( !g_pServerTable )
	{
		MyLog( LOG_NORMAL, "CServerTable Is Not Available." );
	}
	else
	{
		LocalMgr.DisplayLocalizingSet();//021007 lsw
		g_pServerTable->ShowServerStatus();
	}
	return;
}	
	
	
extern void __stdcall GameTimeFunc(DWORD dwValue);
bool InitDBDemon()
{	
	CUSTOM_EVENT ev[5];
	ev[0].dwPeriodicTime = 1000;
	ev[0].pEventFunc = AwaitingProxyServerConnect;
	ev[1].dwPeriodicTime = 0;
	ev[1].pEventFunc = ShowDBServerStatus;
	
	// 021128 YGI--------------
	ev[2].dwPeriodicTime = 0;
	ev[2].pEventFunc = RecvMessageFromBillingServer;
	
	ev[3].dwPeriodicTime = 1000*60;	// 1��
	ev[3].pEventFunc = GameTimeFunc;	
	//--------------------------

	DESC_NETWORK desc;

	desc.OnAcceptServer = OnAcceptServer;
	desc.OnAcceptUser = OnAcceptUser;
	desc.OnDisconnectServer = OnDisconnectServer;
	desc.OnDisconnectUser = OnDisconnectUser;
	desc.dwMainMsgQueMaxBufferSize = 10240000;	// 010322 KHS
	desc.dwMaxServerNum = 120;	//020110 LSW		
	desc.dwMaxUserNum = 100;			
	desc.dwServerBufferSizePerConnection = 512000;
	desc.dwServerMaxTransferSize = 65000;
	desc.dwUserBufferSizePerConnection = 65000;
	desc.dwUserMaxTransferSize = 8192;	
	desc.OnRecvFromServerTCP = ReceivedMsgServer;
	desc.OnRecvFromUserTCP = ReceivedMsgUser;
	desc.dwConnectNumAtSameTime = 200;//020110 LSW
										
	desc.dwFlag = 0; // CSD-040127 : NETDDSC_DEBUG_LOG : INetwork �α� ���
										
	//2001/02/19 zhh					
	desc.dwCustomDefineEventNum = 4;	// 030221 YGI
	
	desc.pEvent = ev;

	HRESULT hr;
	CoInitialize(NULL);

	g_pINet = NULL;
	hr = CoCreateInstance(
           CLSID_4DyuchiNET,
           NULL,
           CLSCTX_INPROC_SERVER,
           IID_4DyuchiNET,
           (void**)&g_pINet);

	if (FAILED(hr) || g_pINet == NULL)
	{
		MyLog(LOG_FATAL, "ERROR: CoCreateInstance(CLSID_4DyuchiNET) failed. hr=0x%08lx g_pINet=%p", hr, g_pINet);
		MyLog(LOG_FATAL, "       The 4DyuchiNET COM component is not registered. Please run regsvr32 on 4DyuchiNET.dll");
		MyLog(LOG_FATAL, "       Or run in 'DBDemon-only mode' by setting the environment variable DBDEMON_NO_NETWORK=1");
		return false;
	}

	if (!g_pINet->CreateNetwork(&desc,10,10))
	{
		MyLog(LOG_FATAL, "ERROR: g_pINet->CreateNetwork() failed.");
		return false;
	}

	// �ϴ� ����...
	//g_pINet->PauseTimer( 0 );	// 040406 kyo

	hKeyEvent = g_pINet->GetCustomEventHandle(1);

	//2001/02/19 zhh
	g_hEvent = g_pINet->GetCustomEventHandle(2);

	g_pServerTable = new CServerTable(DB_DEMON_INI_, MAX_SERVER_NUM, g_pINet);//021007 lsw

	if( !g_pServerTable )
	{
		MyLog( LOG_FATAL, "ERROR: g_pServerTable is NULL" );
		return false;
	}

	return true;
}

void EndDBDemon()
{
	if (g_pINet)
	{
		g_pINet->Release();
		g_pINet = NULL;
	}

	if (g_pServerTable)
	{
		delete g_pServerTable;
		g_pServerTable = NULL;
	}

	CoFreeUnusedLibraries();
	CoUninitialize();
}