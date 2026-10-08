/*

*/

#ifndef	__DRAGON_H__
#define	__DRAGON_H__

#include <windows.h>
#include <windowsx.h>
#include <time.h>

#include "directdraw.h"
#include "directinput.h"
//#include "MenuDef.h"
#include "Network.h"
#include "item.h"

#define	CM_COMMAND			0
#define	CM_MESSAGE			1

#define CONNECT_REFUSE		10101010

////////////// 0427 lkh �߰� /////////////////
#define	DISPLAY_EARTHQUAKE	10
#define	DISPLAY_WAVEEARTH	1

extern HWND					g_hwndMain;
extern HINSTANCE			g_hInstance;

extern TCHAR				g_szTitle[ MAX_LOADSTRING ];
extern TCHAR				g_szWindowClass[ MAX_LOADSTRING ];
extern TCHAR				g_szCWD[ _MAX_PATH ];

extern BOOL					g_bIsActive;
extern BOOL					g_bCommandMode;

extern DIRECTDRAWINFO		g_DirectDrawInfo;

extern POINT				g_pointMouse, g_pointFuzz;
extern long					g_pointMouseX, g_pointMouseY;
extern int					g_WindowOuterW;
extern int					g_WindowOuterH;
extern void					ApplyWindowOuterSize(int outerMode);
extern void					MapClientToLogic(int* px, int* py);
extern int					g_nLButtonState, g_nRButtonState;
extern int					g_nLDButtonState, g_nRDButtonState;
extern int					g_nOldLButtonState, g_nOldRButtonState;
extern int					g_nSensitivity;

extern int					LButtonDownIng, RButtonDownIng;
//extern int					LButtonDown, RButtonDown;
extern DWORD				g_CurrentTime;
extern DWORD				g_ClientTime;	// GameProc()���� ��������� Check �Ѵ�. 
extern DWORD				g_ServerClientTimeGap;

extern DWORD				g_curr_time;  //  �������������ִ� ������ �ð�. �ʴ���..
extern DWORD				g_packet_recv_send_checktime;
////////////////////// 0128 lkh /////////////////////////
extern bool					g_Volume_Off;	//������ �Ҹ��� ���ϱ�?

extern int					g_Operator_Function;

// ----------------------- khs 0804

extern char				  *g_DestBackBuf, *g_OldDestBackBuf;
extern int					Mox, Moy;
extern int					Mapx, Mapy;
extern LPCHARACTER			Hero;

//-----------------------------------

extern int					QuitFlag;

extern DWORD				g_RecvBytes, g_SendBytes;

extern bool					MouseCursorOnOff;

extern DWORD				g_Inside;

extern int					NowLoading;




/////////////////// 0126 �̱��� //////////////////////
extern short int	g_Current_Volume;	//������ ���� ����(0~100���̰�)
extern short int	g_To_Volume		;	//��ȯ�Ǿ�� �ϴ� (����������) ��ǥ ���� ����
extern short int	g_Change_Volume	;		//���� ��ġ ��ȯġ(-+ ��ȭ��->Fade Out/In)

extern char IIDD[MAX_PATH];
extern char PPWW[MAX_PATH];

extern int	CheckSumError;	

extern int recvAttackResultCount;
extern FILE *keyplayfp;
extern FILE *keykeyplayfp;

extern bool		g_bIsChristmas;

typedef struct	tagGAMEINFO
{
	char		proxy_ip[3][ 18 ];
	int			proxy_port;
	char		agent_ip[18];
	int			agent_port;
	int			version;

	int			map_port; // �ʼ����� port��ȣ Port�� �˸� MapName�� �˼� �ִ�. 

	time_t		timeGame;
	BYTE		cWeather;
	BYTE		cWindVelocity;
	BYTE		cHumidity;

	POINT		pointMouse, pointFuzz;
	int			nLButtonState, nRButtonState;
	int			nOldLButtonState, nOldRButtonState;
	int			nSensitivity;

	
	DWORD		dwFrameRate;
	DWORD		dwFrameCount, dwFrameCount0;
	DWORD		dwFrameTime, dwFrameTime0;

	RECT		rectView, rectScreen;

	LPCHARACTER  lpcharacter;
	LPCHARACTER  lpoldcharacter;

	int			nSelectedSpriteType;
	LPVOID		lpvSelectedSprite;

	int			 PopupFaceCount;
	int			 checkcount; // ���콺�� �浹�ϸ� Count�� 5���̵Ǹ� ȭ�鿡 �̸��� �����ش�. 
	
	// ����� �پ��� ����� ���� �÷���
	int			displayOption;		
	int			displayOptionDelay;
	//< 040326 kjy
	int			preventSelectYilse;
	//> 040326 kjy

} GAMEINFO, *LPGAMEINFO;

extern GAMEINFO		g_GameInfo;

#ifdef __cplusplus
extern "C"
{
#endif

extern void		SetCurrentWorkingDirectory( void );
extern char*	GetCurrentWorkingDirectory( void );
extern void		SetInfoFile( void );
extern char*	GetInfoFile( void );
extern char*	EatFrontWhiteChar( char* );
extern char*	EatRearWhiteChar( char* );
extern BOOL		ShowErrorMessage( char* lpszMessage );

enum eExitAppType
{
	EA_NORMAL = 0,//�������� ����.
	EA_CHECK_ACCEL= 1,
	EA_BEFORE_EXE= 2,
	EA_BEFORE_EXE2= 3,
	EA_LOAD_CHAR_SPR_DATA_SUB= 4,
	EA_LOAD_RIDER_SPR_DATA_SUB= 5,
	EA_LOAD_RIDER_BAND_SPR_DATA_SUB= 6,
	EA_LOAD_RIDER_MANTLE_SPR_DATA_SUB= 7,
	EA_LOAD_WEAPON_SPR_DATA= 8,
	EA_LOAD_GUARD_SPR_DATA= 9,
	EA_LOAD_LOACL_WAR_INFO_SPR_DATA= 10,
	EA_LOAD_SPRITE_INFO= 11,
	EA_LOAD_RIDER_GRADE_SPR_DATA_SUB= 12,
	EA_LOAD_HORSE_SPR_DATA_SUB= 13,
	EA_LOAD_HORSE_SADDEL_SPR_DATA_SUB= 14,
	EA_LOAD_HORSE_GRADE_SPR_DATA_SUB= 15,
	EA_LOAD_LOACL_CHAIR_SPR_DATA= 16,
	EA_LOAD_LOACL_SQUAD_SPR_DATA= 17,
	EA_RECV_CONNECT_INFO= 18,
	EA_CPU_CLOCK_ERROR = 19,
	EA_CRACK_THREAD_FIND_CRACK = 20,
	EA_LANGUAGE_ERROR = 21,
	EA_JAPAN_LOGIN_ERROR = 22,
};

extern void		ExitApplication( const eExitAppType eEAType );

extern char*	EatRearWhiteChar( char* pStr );

enum eLoadingGameDataStep
{
	LGDS_FIRST = 0,
	LGDS_STEP1 = 1,
	LGDS_STEP2 = 2,
	LGDS_STEP3 = 3,
	LGDS_STEP4 = 4,
	LGDS_STEP5 = 5,
	LGDS_STEP6 = 6,
	LGDS_STEP7 = 7,
	LGDS_STEP8 = 8,
	LGDS_STEP9 = 9,
	LGDS_STEP10 = 10,
	LGDS_STEP11 = 11,
	LGDS_STEP12 = 12,
	LGDS_END	= 13,
};
extern int		LoadingGameData();

extern bool		chinese_input;
extern void		CheckChineseInput( HWND hwnd, LPARAM lParam );
// lsw
extern bool IsChatBoxActive();
extern void SetChatMode(const int iMode, bool bIsForce = false);

enum eCHATTARGETNORMAL
{
	CHAT_TARGET_NORMAL		= 0,//���� ä�� ���
	CHAT_TARGET_GUILD		= 1,//��� ä�� ���
	SELECT_WISPER_MODE		= 2,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER1		= 3,//�Ӹ���� ���� �ϱ�	//�ʵ�ѹ��� ��ġ
	CHAT_TARGET_WISPER2		= 4,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER3		= 5,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER4		= 6,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER5		= 7,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER6		= 8,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER7		= 9,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER8		= 10,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER9		= 11,//�Ӹ���� ���� �ϱ�
	CHAT_TARGET_WISPER10	= 12,//�Ӹ���� ���� �ϱ�
	
};
const int CHAT_TARGET_WISPER_START	= CHAT_TARGET_WISPER1;//�Ӹ���� ���� �ϱ� ������
const int CHAT_TARGET_WISPER_END	= CHAT_TARGET_WISPER5;//�Ӹ���� ���� �ϱ� ����

#ifdef __cplusplus
}
#endif

#endif	// __DRAGON_H__


