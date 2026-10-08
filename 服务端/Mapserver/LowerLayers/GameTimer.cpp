//
// GameTimer.cpp
//

// --------------------
// ���� �۾����� �ý���
// --------------------
#include "GameTimer.h"
#include "..\stdafx.h"
#include <Mmsystem.h>
#include "monitor.h"
#include "mylog.h"
#include "..\HigherLayers\NPC_Pattern.h"
#include "..\HigherLayers\DefaultHeader.h"
#include "..\HigherLayers\Scrp_exe.h"
#include "..\HigherLayers\Citem.h"
#include "..\HigherLayers\Op_magic.h"
#include "..\HigherLayers\area.h"
#include "..\HigherLayers\SealStone.h"
#include "..\HigherLayers\QuestFunction.h"	// 021017 kyo
#include "..\HigherLayers\ArenaManager.h"
#include "..\HigherLayers\BattleManager.h"
#include "..\HigherLayers\UserManager.h"

void CheckElapsedTime_NK_Down( void );
void prepare(t_connection c[]);

CGameTimer GameTimerList[NUM_OF_GTES];	// GameTimerList �迭, NUM_OF_GAME_TIMERS �� ����Ÿ�̸��� �߰�, ������ ������ �� ������� �Ѵ�.(�߿�)
DWORD end_of_tick = 0;					// SetGameTimer()���� �ڵ� ������ �ֹǷ� ��ĥ �ʿ� ����.
DWORD dwTimerFlag = 0;
bool IsGameTimerWorking = false;

extern void UpdateEventMonster();		// LTS AI2


void NPCRegen( DWORD all, DWORD now )
{
	static DWORD lasttime = 0;
	static DWORD curtime = 0;
	int total = 40;

	int from;
	int to;

	static int increasement;

	curtime = timeGetTime();

	static int counter = 0;

	static int run = 0;

	if( all > 1 ) {
		if( now == 0 ) MyLog( LOG_IGNORE, "NPCRegen::Called %d(%ldms) (%d/%d) Started", counter, (curtime-lasttime), now+1, all);
		if( now == (all-1) ) MyLog( LOG_IGNORE, "NPCRegen::Called %d(%ldms) (%d/%d) Finished", counter, (curtime-lasttime), now+1, all);
		//MyLog( LOG_NORMAL, "NPCRegen::Called %d(%ldms) (%d/%d)", counter, (curtime-lasttime), now+1, all);
	} else {
		MyLog( LOG_NORMAL, "NPCRegen::Called %d(%ldms)", counter, (curtime-lasttime));
	}
	counter++;

	lasttime = curtime;

	if( now == 0 ) {
		increasement = (int)((float)total/(float)all);
	}

	from = now*increasement;
	// �������� �������� �� ó��
	if( now == (all-1) ) { to = total; run++; MyLog(LOG_IGNORE, "OKOKOKOKOK"); }

	else to = from+increasement;

	for( int i = from; i < to; i++ ) {
		if( run < 3 ) {
			Sleep(100);
		} else Sleep(10);
	}	
		
	return;
}		
		
// Modified by chan78 at 2000/11/29, �ϴ� �з��� ��.
void very_short_terms()		// 150 ms
{	//< CSD-HK-030829		
	prepare(connections);
	g_pUserManager->Execute();
}	//> CSD-HK-030829
		
void short_terms()		// 300 ms
{		
	MovingNPC();
	MovingCharacter();
	RunArea(connections);
	return;
}		
		
void normal_terms()		// 450 ms
{
	ReCallScript();
	CheckSkillTimeElapsed();
}
	
void long_terms()		// 600 ms
{
	NPC_CheckRemove();
	SendWeatherRoutine(connections);
	NPC_AutoCreate();
	SendAliveCheckRoutine(connections);
	return;
}		
	
extern void CheckTimeForMenu_Kein();		// 010419 YGI		
extern void UpdateDragonLordWarTime();		// LTS DRAGONLORD

void very_long_terms()	// 750 ms
{		
	g_pBattleManager->Elapse();    
	g_pArenaManager->Ready();   // CSD-030521
	g_pArenaManager->Execute(); // CSD-030516
	CheckNationWar();	
	CharacterAutoUpdate();
	CheckItemTimeElapsed();
	CheckTimeForMenu_Kein(); // 010419 YGI
	UpdateEventMonster();		 // LTS AI2
	UpdateDragonLordWarTime();		// LTS DRAGONLORD
	g_QuestInMap.RunQuestTimer();// 021104 kyos
	CheckElapsedTime_NK_Down();
	return;
}		
		
		
CGameTimer::CGameTimer()
{		
	sTimerProcessName[0] = '\0';
	dwTickSize = 0;
	bIsDividable = false;
	dwExpectedTime = 0;
	wNumOfDividedBy = 1;
	wNextNumOfDividedBy = 0;
	wCurrentProcessPos = 0;
	dwTotalTakeTime = 0;
	DividableFuncToCall = NULL;
	NormalFuncToCall = NULL;
}	
	
CGameTimer::~CGameTimer()
{	
}	
	
DWORD CGameTimer::GetExpectedTime()
{	
	return dwExpectedTime;
}		
void CGameTimer::SetGameTimer(char *name, DWORD ticksize, bool dividable, DWORD expecttime, MYFUNC func)
{	
	if( strlen(name) > TIMER_PROCESS_NAME_LENGTH ) {
		strncpy(sTimerProcessName, name, TIMER_PROCESS_NAME_LENGTH);
		sTimerProcessName[TIMER_PROCESS_NAME_LENGTH] = '\0';
	} else strcpy(sTimerProcessName, name);
	dwTickSize = ticksize;
	if( dwTickSize > end_of_tick ) end_of_tick = dwTickSize;
	bIsDividable = dividable;
	dwExpectedTime = expecttime;

	DividableFuncToCall = func;
	NormalFuncToCall = NULL;

	MyLog(LOG_IGNORE, "%s, (tick)%d, (able)%d, (expect)%d are ADDED", name, ticksize, dividable, expecttime);
	return;
}	
	
void CGameTimer::SetGameTimer(char *name, DWORD ticksize, bool dividable, DWORD expecttime, VOIDFUNCS func)
{	
	if( strlen(name) > TIMER_PROCESS_NAME_LENGTH ) {
		strncpy(sTimerProcessName, name, TIMER_PROCESS_NAME_LENGTH);
		sTimerProcessName[TIMER_PROCESS_NAME_LENGTH] = '\0';
	} else strcpy(sTimerProcessName, name);
	dwTickSize = ticksize;
	if( dwTickSize > end_of_tick ) end_of_tick = dwTickSize;
	bIsDividable = dividable;
	dwExpectedTime = expecttime;

	DividableFuncToCall = NULL;
	NormalFuncToCall = func;

	MyLog(LOG_IGNORE, "%s, (tick)%d, (able)%d, (expect)%d are ADDED", name, ticksize, dividable, expecttime);
	return;
}	
	
bool CGameTimer::IsTicToRun(DWORD tick)
{
	if( !bIsDividable && !(tick % dwTickSize) ) return true;
	if( bIsDividable ) {
		if( wCurrentProcessPos ) return true;
		// (tick%dwTickSzie) �������� 
		else if( (dwTickSize-(tick%dwTickSize)) == (DWORD)(dwTickSize - (wNumOfDividedBy/2)) ) return true;
	}
	return false;
}

WORD CGameTimer::Run()
{
	DWORD StartTime, FinishTime, TakeTime;
	static int counter = 0;
	StartTime = timeGetTime();							// ���⼭���� �ҿ� �ð� üũ

	// Run
	if( bIsDividable ) {
		DividableFuncToCall( wNumOfDividedBy, wCurrentProcessPos );
		if ( wNumOfDividedBy > 1 ) {
			wCurrentProcessPos++;

			// �� ������ ������.
			if( wCurrentProcessPos == wNumOfDividedBy )
				wCurrentProcessPos = 0;
		}
	} else {
		NormalFuncToCall();
	}

	FinishTime = timeGetTime();							// �ҿ�ð� üũ ��
	TakeTime = FinishTime - StartTime;					// �ɸ� �ð��� ����Ѵ�.

	// �ҿ�ð� ��� �����
	if( !bIsDividable || wNumOfDividedBy == 1 ) {
		dwTotalTakeTime = TakeTime;						// ���� �Ұ����� �Լ��̰ų� ���Ұ��������� ���ҵ��� ���� �۾��̸� �̹��� �ɸ� �ð��� dwTotalTakeTime�� �ִ´�.
	} else {
		counter++;
		dwTotalTakeTime = dwTotalTakeTime + TakeTime;	// ���ҵ� �۾��� �Ѵ����� �۾��� ���������� �ɸ��ð��� ���Ѵ�.
		if (wCurrentProcessPos == 0 ) {
			MyLog( LOG_IGNORE, "TotalTakeTime(%d/%d)", counter, dwTotalTakeTime );
			counter = 0;
		}
	}

	if( bIsDividable && !wCurrentProcessPos ) {			// ���Ұ����� ����̰�, �����۾��� �������� �ƴϸ� ����/��ħ ���θ� �Ǵ��Ѵ�.
		if ( wNumOfDividedBy > 1 ) {					// ���ҵ� �۾��� ��� �� �۾��� �ҿ�ð��� ����� ���Ѵ�.
			dwTotalTakeTime = dwTotalTakeTime / wNumOfDividedBy;
		}
		DWORD dwTakeTimeAvr = dwTotalTakeTime;			// ��հ����� ���.
		dwTotalTakeTime = 0;							// �� Ŭ�������ش�.

		bool bIsExcess;
		DWORD dwDiffTime;

		if( dwTakeTimeAvr > dwExpectedTime ) {
			if( wNumOfDividedBy == dwTickSize )			// �̹� �ִ밪 �̻��� �ð��� �ɸ��� ������...
				return (WORD)TakeTime;
			bIsExcess = true;
			dwDiffTime = dwTakeTimeAvr - dwExpectedTime;
		} else if( dwTakeTimeAvr < dwExpectedTime ) {
			if( wNumOfDividedBy <= 1 )					// 1 ���ϸ� �� ������ ����.
				return (WORD)TakeTime;
			bIsExcess = false;
			dwDiffTime = dwExpectedTime - dwTakeTimeAvr;
		} else {
			return (WORD)TakeTime;
		}

		float factor = (float)dwDiffTime / (float)dwExpectedTime;

		if( (bIsExcess &&(factor > 0)) || (!bIsExcess && (factor > 0.25)) ) {
			if( bIsExcess )
				factor = (float)1.0 + factor;
			else
				factor = (float)1.0 - factor;
			
			wNumOfDividedBy = (int)((float)wNumOfDividedBy * factor);
			MyLog( LOG_NORMAL, "TakeTimeAvr(%d), ExpectedTime(%d), Factor(%f), Changed to(%d)", dwTakeTimeAvr, dwExpectedTime, factor, wNumOfDividedBy);
			if( wNumOfDividedBy > dwTickSize ) {	// wNumOfDividedBy�� dwTickSize���� Ŭ �� ����.
				MyLog( LOG_IMPORTANT, "GTE(%s) takes too long time but can't be devided more(%d:%d)", sTimerProcessName, wNumOfDividedBy, dwTickSize);
				wNumOfDividedBy = (WORD)dwTickSize;
			}
		}
	}
	return (WORD)TakeTime;
}


// �� ��������� �Ѵ�...
void SettingGameTimers( void )
{
	// �̸�, ������ ƽ �ѹ�, �����۾����Ұ��ɿ���, ���ð�, ȣ���� �Լ�.
	// ����! 0���� NUM_OF_GAME_TIMERS-1 ���� �迭�� ���� ���� �ȵ�.
	// ���� �ε����� �߰��Ҽ��� ó�� �켱������ ����. �Ϲ������� ȣ��ƽ�� ª������ ���� �ε����� ����ϴ°��� ����.

// Commected by chan78 at 2000/11/29
//	GameTimerList[0].SetGameTimer("Run Them all", 2, false, 400, imsi );
// 1�� 150 ms
	GameTimerList[0].SetGameTimer("Very Short Terms",	1, false,	100, very_short_terms	);
	GameTimerList[1].SetGameTimer("Short Terms",		2, false,	300, short_terms		);
	GameTimerList[2].SetGameTimer("Normal Terms",		3, false,	450, normal_terms		);
	GameTimerList[3].SetGameTimer("Long Terms",			4, false,	600, long_terms			);
	GameTimerList[4].SetGameTimer("Very Long Terms",	7, false,	1000, very_long_terms	);
	GameTimerList[5].SetGameTimer("Quest30Second",		10, false, 	100, QuestSecond	);

	StartGameTimers();

	return;
}

void StartGameTimers( void )
{
	IsGameTimerWorking = true;
	return;
}

void StopGameTimers( void )
{
	IsGameTimerWorking = false;
	return;
}

// GTE_SIZE �� �ѹ��� ȣ��Ǵ� func,
// timer.cpp ����
void __stdcall GameTimerProcess(DWORD dwValue)//020511 lsw
{
	static DWORD tick = 1;
	static int LastProcess = 0;
	WORD TakeTime;

	if( !IsGameTimerWorking ) return;

	for( int i = LastProcess; i < NUM_OF_GTES; i++ ) {

		if( !GameTimerList[i].IsTicToRun(tick) )
			continue;

		// ���⼭ �ɸ� �ð��� �ִ� ��� �ð��� ����� ���� �� ���� �����־ ���ξ������ �������ش�.
		// tic�� ������Ű�� �ʰ� LastProcess�� i+1�� ���� ���ԵǹǷ� �����ִ� �۾����� ó���ϰ� �ȴ�.
		if( (TakeTime = GameTimerList[i].Run()) > GameTimerList[i].GetExpectedTime() ) {
			// �ð� ������ �α��ϰ�������...
			//MyLog( LOG_IGNORE, "GameTimerList[%d].Run() > dwExpectedTime(%d)", i, TakeTime );

			LastProcess = i+1;
			if( LastProcess == NUM_OF_GTES ) {		// ������ �̹� �۾��� �������� ���...
				LastProcess = 0;
			tick++;
			}
			return;
		}

		// LastProcess�� 0�� �ƴѰ��� ���� ȣ�⶧ ������ ���� tic�� �������� �������̹Ƿ� ƽ�� ������Ű��
		// ���� �̹��Ͽ� �ؾ��� �ϵ��� ó�����ش�. �����ϸ� ƽ�� �� �и����� �ϱ� ���� ó����.
		if( i == (NUM_OF_GTES-1) ) {
			if( LastProcess != 0 ) {
				LastProcess = 0;
				tick++;
				i = 0;
			}
		}

	}

	// �ؾ��� ��� �۾��� ����� �ð��ȿ� ���´�.
	LastProcess = 0;
	tick++;

	return;
}

