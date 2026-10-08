// LocalizingMgr.cpp: implementation of the CLocalizingMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LocalizingMgr.h"
#include "Mylog.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
const char szKorea	[]	= "KOREA";
const char szChina	[]	= "CHINA";
const char szTaiwan	[]	= "TAIWAN";
const char szThai	[]	= "THAI";
const char szHongKong[] = "HONGKONG";
const char szUsa	[]	= "USA";
const char szJapan	[]	= "JAPAN";
CLocalizingMgr LocalMgr;

CLocalizingMgr::CLocalizingMgr()
{
	m_iNationCode	= NOTSET;
	m_iMyCode		= 0;
	m_pszNationName	= NULL;
	m_pszTotalDbID	= NULL;
	m_pszTotalDbPW	= NULL;
	m_pszDragonDbID	= NULL;
	m_pszDragonDbPW	= NULL;
	m_pszChrlogDbID	= NULL;
	m_pszChrlogDbPW	= NULL;
}

#define SAFE_DELETE(a) if(a){delete []a;a = NULL;}
CLocalizingMgr::~CLocalizingMgr()
{
	SAFE_DELETE(m_pszNationName);
	SAFE_DELETE(m_pszTotalDbID);
	SAFE_DELETE(m_pszTotalDbPW);
	SAFE_DELETE(m_pszDragonDbID);
	SAFE_DELETE(m_pszDragonDbPW);
	SAFE_DELETE(m_pszChrlogDbID);
	SAFE_DELETE(m_pszChrlogDbPW);
}

void CLocalizingMgr::SetNationName(const char* szNationName)
{
	SAFE_DELETE(m_pszNationName);
	m_pszNationName = new char [strlen(szNationName)+1];
	strcpy(m_pszNationName,szNationName);
}

int CLocalizingMgr::SetDBAccount(const int iType, const char* szId,const char* szPw)
{
	switch(iType)
	{
	case TOTAL_DB:
		{
			SAFE_DELETE(m_pszTotalDbID);
			SAFE_DELETE(m_pszTotalDbPW);
			m_pszTotalDbID = new char [strlen(szId)+1];
			m_pszTotalDbPW = new char [strlen(szPw)+1];
			strcpy(m_pszTotalDbID,szId);
			strcpy(m_pszTotalDbPW,szPw);
		}break;
	case DRAGON_DB:
		{
			SAFE_DELETE(m_pszDragonDbID);
			SAFE_DELETE(m_pszDragonDbPW);
			m_pszDragonDbID = new char [strlen(szId)+1];
			m_pszDragonDbPW = new char [strlen(szPw)+1];
			strcpy(m_pszDragonDbID,szId);
			strcpy(m_pszDragonDbPW,szPw);
		}break;
	case CHRLOG_DB:	
		{
			SAFE_DELETE(m_pszChrlogDbID);
			SAFE_DELETE(m_pszChrlogDbPW);
			m_pszChrlogDbID = new char [strlen(szId)+1];
			m_pszChrlogDbPW = new char [strlen(szPw)+1];
			strcpy(m_pszChrlogDbID,szId);
			strcpy(m_pszChrlogDbPW,szPw);
		}break;
	default:
		{
			return 0;
		}break;
	}
	return 1;
}

int CLocalizingMgr::InitVersion(const char* szNationName,const int iIsFreeBeta)//�� �ѹ��� ȣ�� �ϵ��� �Ͻʽÿ�.
{
	return InitVersion(ConvertNameToCode(szNationName),iIsFreeBeta);
}

int CLocalizingMgr::InitVersion(const int iNationCode,const int iIsFreeBeta)
{
	switch(iNationCode)
	{
	case KOREA	:
		{
			SetNationName("KOREA");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");
			SetDBAccount(CHRLOG_DB,"","");
			m_iMyCode		= KOREA_MYCODE;//�����ڵ� �����
		}break;
	case CHINA	:
		{	
			SetNationName("CHINA");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");
			SetDBAccount(CHRLOG_DB,"","");
			m_iMyCode		= CHINA_MYCODE;//�����ڵ� �����
		}break;
	case TAIWAN	:
		{
			SetNationName("TAIWAN");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");
			SetDBAccount(CHRLOG_DB,"","");
			m_iMyCode		= TAIWAN_MYCODE;//�����ڵ� �����	
		}break;
	case THAI:
		{
			SetNationName("THAI");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");
			SetDBAccount(CHRLOG_DB,"","");			
			m_iMyCode		= THAI_MYCODE;//�����ڵ� �����
		}break;
	case HONGKONG:
		{
			SetNationName("HONGKONG");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");			
			SetDBAccount(CHRLOG_DB,"","");
			m_iMyCode		= HONGKONG_MYCODE;//�����ڵ� �����
		}break;
	case USA:
		{
			SetNationName("USA");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");
			SetDBAccount(CHRLOG_DB,"","");
			m_iMyCode		= USA_MYCODE;//�����ڵ� �����
		}break;
	case JAPAN	:
		{
			SetNationName("JAPAN");
			SetDBAccount(TOTAL_DB ,"","");
			SetDBAccount(DRAGON_DB,"","");
			SetDBAccount(CHRLOG_DB,"","");
			m_iMyCode		= JAPAN_MYCODE;//�����ڵ� �����
		}break;
	default:
	case NOTSET	:
		{
			MyLog( LOG_NORMAL, "Version Setting Fault!! Nation Code = ' %d '" ,m_iNationCode);		
			return 0;
		}break;
	}

	// Custom override: read DB credentials from DBDemon.ini AFTER the hardcoded
	// defaults are set above. Format:
	//   [db_account] dragon_id=xxx dragon_pw=yyy total_id=... total_pw=... chrlog_id=... chrlog_pw=...
	// These values override the hardcoded defaults.
	{
		char szTemp[256] = {0,};
		// DragonRajaDB (DRAGON_DB)
		if (GetPrivateProfileString("db_account", "dragon_id", "", szTemp, sizeof(szTemp), DB_DEMON_INI_) > 0
			&& szTemp[0] != 0)
		{
			char szPw[256] = {0,};
			GetPrivateProfileString("db_account", "dragon_pw", "", szPw, sizeof(szPw), DB_DEMON_INI_);
			SetDBAccount(DRAGON_DB, szTemp, szPw);
			MyLog(LOG_NORMAL, "[DB Account] Dragon DB account overridden by DBDemon.ini: %s", szTemp);
		}
		// TotalDB (TOTAL_DB)
		if (GetPrivateProfileString("db_account", "total_id", "", szTemp, sizeof(szTemp), DB_DEMON_INI_) > 0
			&& szTemp[0] != 0)
		{
			char szPw[256] = {0,};
			GetPrivateProfileString("db_account", "total_pw", "", szPw, sizeof(szPw), DB_DEMON_INI_);
			SetDBAccount(TOTAL_DB, szTemp, szPw);
			MyLog(LOG_NORMAL, "[DB Account] Total DB account overridden by DBDemon.ini: %s", szTemp);
		}
		// ChrLogDB (CHRLOG_DB)
		if (GetPrivateProfileString("db_account", "chrlog_id", "", szTemp, sizeof(szTemp), DB_DEMON_INI_) > 0
			&& szTemp[0] != 0)
		{
			char szPw[256] = {0,};
			GetPrivateProfileString("db_account", "chrlog_pw", "", szPw, sizeof(szPw), DB_DEMON_INI_);
			SetDBAccount(CHRLOG_DB, szTemp, szPw);
			MyLog(LOG_NORMAL, "[DB Account] ChrLog DB account overridden by DBDemon.ini: %s", szTemp);
		}
	}

	m_iNationCode	= iNationCode;//�����ڵ� �����
	m_iIsFreeBeta	= iIsFreeBeta;//�̰� ������Ÿ���� üũ��
	return 1;
}

int CLocalizingMgr::ConvertNameToCode(const char* szNationName)
{
	if(!stricmp(szNationName,szKorea)){return KOREA;}
	if(!stricmp(szNationName,szChina)){return CHINA;}
	if(!stricmp(szNationName,szTaiwan)){return TAIWAN;}
	if(!stricmp(szNationName,szThai)){return THAI;}
	if(!stricmp(szNationName,szHongKong)){return HONGKONG;}
	if(!stricmp(szNationName,szUsa)){return USA;}
	if(!stricmp(szNationName,szJapan)){return JAPAN;}
	
	return NOTSET;	
}

void CLocalizingMgr::DisplayLocalizingSet()const//���� ���ö���¡ ������ �����ݴϴ�.
{
	switch(m_iNationCode)
	{
	case KOREA	:
	case CHINA	:
	case TAIWAN	:
	case THAI:	
	case USA:
	case JAPAN	:
		{
		}break;
	default:
	case NOTSET	:
		{
			MyLog( LOG_NORMAL, "Version Setting Fault!! Nation Code = ' %d '" ,m_iNationCode);		
		}break;
	}
	MyLog( LOG_NORMAL, "########---------------------------------------########" );
	MyLog( LOG_NORMAL, "        Running For ' %s ' MyCode ' %d '",m_pszNationName,m_iMyCode);
	if(IsFreeBeta())
	{
		MyLog( LOG_NORMAL, "########!!This is FreeBetaServer NO Pay Money!!########" );
	}
	MyLog( LOG_NORMAL, "########---------------------------------------########" );
}

int CLocalizingMgr::IsAbleNation(const int iNationCode)const//�Ұ����� ������� 0�� ���� �����ϸ� 1�� ����
{
	return (m_iNationCode & iNationCode)?1:0;
}

int CLocalizingMgr::IsAbleMyCode(const int iMyCode)const//�Ұ����� �����ڵ��� 0�� ���� �����ϸ� 1�� ����
{
	return (m_iMyCode == iMyCode)?1:0;
}

const char *CLocalizingMgr::GetDBAccount(const int iType, bool bIsID)
{	
	switch(iType)
	{
	case TOTAL_DB:
		{
			return (bIsID)?m_pszTotalDbID:m_pszTotalDbPW;
		}break;
	case DRAGON_DB:
		{
			return (bIsID)?m_pszDragonDbID:m_pszDragonDbPW;
		}break;
	case CHRLOG_DB:	
		{
			return (bIsID)?m_pszChrlogDbID:m_pszChrlogDbPW;
		}break;
	default:
		{
			return "";
		}break;
	}
	return "";
}