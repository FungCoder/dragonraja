#include "pay.h"
//#include "StdAfx.h"

#include "MAIN.H"
//#include "NPC_Pattern.h"
//#include "Map.h"
//#include "Scrp_exe.h"
#include "monitor.h"
#include "servertable.h"
//#include "Id.h"
//혹시라도 문제생기면 주석 풀것
const int month_tbl[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };	
// 主连接在服务启动阶段已完成大量表读取，运行状态稳定。旧版 TotalDB
// 专用连接在 x64 MySQL ODBC 驱动下首次分配语句句柄时会触发访问异常，
// 登录查询改由主连接访问带库名前缀的存储过程，避免依赖该失效句柄。
extern HDBC  hDBC;
		
extern int GetUserAge(const char* szID);	// 030929 kyo
extern bool IsLimitedTime();				// 030929 kyo

static void CheckPaySql(SQLRETURN result, SQLHSTMT statement, const char* stage)
{
	if (SQL_SUCCEEDED(result)) return;
	SQLCHAR state[6] = {0};
	SQLINTEGER nativeError = 0;
	SQLCHAR message[256] = {0};
	SQLSMALLINT messageLength = 0;
	if (statement)
		SQLGetDiagRec(SQL_HANDLE_STMT, statement, 1, state, &nativeError,
			message, sizeof(message), &messageLength);
	MyLog(LOG_FATAL, "[LOGIN] SQL failure at %s: rc=%d state=%s native=%ld",
		stage, result, state, nativeError);
}

COnePass onepass;
		
COnePass::COnePass()
{
}		

COnePass::~COnePass()
{		
}		
		
int COnePass::InsertUsedID_SQL_ForPay( LPSTR szMapName, LPSTR szUID, 
									  char *IP , char *joint_id, 
									  int type, WORD wPort, DWORD dwID )
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode = 0;
	
	char		szQuerry[512] = {0,};
	::sprintf(szQuerry, "insert into dragonraja_total.logintable (servername, user_id, type, ip, joint_id, port, agent_id, server_set_num, d_kyulje) values "
						"('%s','%s',"
						"'%d','%s','%s',"
						"'%d','%d','%d','%d')",
						szMapName, szUID , 
						type , IP, joint_id, 
						wPort, dwID,  g_pServerTable->GetServerSetNum(), this->d_kyulje);

	::SQLAllocStmt(hDBC, &hStmt);
	retCode = ::SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	::SQLFreeStmt(hStmt, SQL_DROP);
			
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		return 1;
	}		

	return 0;
}

//1207 zhh
int COnePass::DeleteUsedID_SQL_ForPay(LPSTR szMapName, LPSTR szUID,int port)
{	
	char	szQuerry[255]={0,};
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	//001218 zhh
	if(port==0)
	{
		if(szUID == NULL)
			sprintf(szQuerry, "delete from dragonraja_total.logintable where servername='%s' AND server_set_num = %d", szMapName, g_pServerTable->GetServerSetNum() );
		else
			sprintf(szQuerry, "delete from dragonraja_total.logintable where user_id='%s'", szUID);
	}		
	else	
	{		
		sprintf(szQuerry, "DELETE FROM dragonraja_total.logintable WHERE port=%d AND server_set_num = %d ", port,  g_pServerTable->GetServerSetNum());
		MyLog( LOG_NORMAL, szQuerry);
	}
			
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	SQLFreeStmt(hStmt, SQL_DROP);
		
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{	
		return 1;
	}	
	else
	{	
		MyLog( LOG_FATAL, "<< QUERY FAIL >> User '%s's DELETE QUERY has Failed!!!", szUID );
	}	
		
	return 0;
}
	
//2001/02/19 zhh
#include "./China/QueryDBSocket.h"
extern CQueryDBSocket *ConQ;

OUTPUT COnePass::OnePassID(const short nCn,LoginInfoPay &LIP,const bool bIsGMTool)
{
	OUTPUT Output = {0,};
	if(LocalMgr.IsAbleNation(JAPAN) && !bIsGMTool)//일본인데 일반 유저일 경우
	{
		Output.nRet		=	CheckLoginIDForJapan(nCn,LIP);
		Output.nType	=	LIP.type;
		Output.dwIndex	=	LIP.index;
	}
	else
	{
		Output.nRet		=	CheckPW_TotalDB_SQL(nCn,LIP);//로그인 가능한지에 대한 변수 입니다.
		Output.nType	=	LIP.type;//결제 타입입니다.
		Output.dwIndex	=	LIP.index;//
		// 중국의 과금에서 IP단위의 과금이 들어가면 이부분에 추가된다. 
		if(LocalMgr.IsAbleNation(TAIWAN|CHINA|HONGKONG))//021007 lsw//중국홍콩대만 모두 아이피를 추가한다. 
		{
			if( Output.nRet > 0 || Output.nRet == BT_NEED_PAY )//0보다 클 경우는 남은 날짜가 있는 것이고
			{
				ConQ->AskCheckLoginWithIP( LIP.id, LIP.ip );
				Output.nRet = COnePass::BT_WAIT_BILLING_MSG;//중국의 경우는
			}
		}
	}
	return Output;
}


// 프리 베타일 경우 접속 가능한 아이디 인지 날자로 확인 할때 사용
int GetAccessPossableDay( int &year, int &month, int &day )
{
	char szRegistDate[50]= {0,};
	// YYYY/MM/DD 형식이어야 함
	if( GetPrivateProfileString( "nation_set", "RegistDate", "" , szRegistDate, 50,DB_DEMON_INI_ ) )
	{
		char *token = strtok( szRegistDate, "/" );
		if( !token ) return 0;
		year = atoi(token);
		
		token = strtok( NULL, "/" );
		if( !token ) return 0;
		month = atoi(token);

		token = strtok( NULL, "/" );
		if( !token ) return 0;
		day = atoi(token);

		return 1;
	}
	
	return 0;
}

// 리턴값..
// -1 : ID 없음. 
// -2 : PW 틀림..
// -3 : 돈내야함..
//      남은 날짜..
extern bool IsFreeLevel( char *szUID );
int COnePass::CheckPW_TotalDB_SQL(const short nCn, LoginInfoPay& LIP)
{
    const size_t idLength = strnlen_s(LIP.id, sizeof(LIP.id));
    const size_t passwordLength = strnlen_s(LIP.pw, sizeof(LIP.pw));
    if (idLength == 0 || idLength > ID_LENGTH) return BT_WRONG_ID;
    if (passwordLength == 0 || passwordLength > PW_LENGTH) return BT_WRONG_PW;

    MyLog(LOG_NORMAL, "[LOGIN] Account lookup begins: cn=%d id='%s'", nCn, LIP.id);
    ::prepare(connections);
    ::EatRearWhiteChar(LIP.id);
    d_kyulje = 0;
    LIP.type = 0;
    LIP.index = 0;

    HSTMT statement = NULL;
    SQLLEN parameterLength = SQL_NTS;
    SQLLEN valueLength = 0;
    TIMESTAMP_STRUCT expires = {0};
    int remainingTime = 0;
    char storedPassword[PW_LENGTH + 1] = {0};

    RETCODE result = SQLAllocStmt(hDBC, &statement);
    if (!SQL_SUCCEEDED(result))
    {
        MyLog(LOG_FATAL, "[LOGIN] SQLAllocStmt failed: id='%s'", LIP.id);
        return BT_WRONG_ID;
    }

    result = SQLPrepare(statement,
        (UCHAR*)"CALL dragonraja_total.up_get_user_info2(?)", SQL_NTS);
    if (SQL_SUCCEEDED(result))
        result = SQLBindParameter(statement, 1, SQL_PARAM_INPUT, SQL_C_CHAR,
            SQL_VARCHAR, ID_LENGTH, 0, LIP.id, idLength + 1, &parameterLength);
    if (SQL_SUCCEEDED(result)) result = SQLExecute(statement);
    if (!SQL_SUCCEEDED(result))
    {
        CheckPaySql(result, statement, "account lookup");
        SQLFreeStmt(statement, SQL_DROP);
        return BT_WRONG_ID;
    }

    result = SQLFetch(statement);
    if (result == SQL_NO_DATA)
    {
        SQLFreeStmt(statement, SQL_DROP);
        MyLog(LOG_NORMAL, "[LOGIN] Account not found: id='%s'", LIP.id);
        return BT_WRONG_ID;
    }
    if (!SQL_SUCCEEDED(result))
    {
        CheckPaySql(result, statement, "account fetch");
        SQLFreeStmt(statement, SQL_DROP);
        return BT_WRONG_ID;
    }

    bool valid = true;
    result = SQLGetData(statement, 1, SQL_C_ULONG, &LIP.index,
                        sizeof(LIP.index), &valueLength);
    valid = valid && SQL_SUCCEEDED(result) && valueLength != SQL_NULL_DATA;
    result = SQLGetData(statement, 2, SQL_C_CHAR, storedPassword,
                        sizeof(storedPassword), &valueLength);
    valid = valid && SQL_SUCCEEDED(result) && valueLength != SQL_NULL_DATA;
    result = SQLGetData(statement, 3, SQL_C_LONG, &d_kyulje,
                        sizeof(d_kyulje), &valueLength);
    valid = valid && SQL_SUCCEEDED(result);
    result = SQLGetData(statement, 4, SQL_C_TIMESTAMP, &expires,
                        sizeof(expires), &valueLength);
    valid = valid && SQL_SUCCEEDED(result);
    result = SQLGetData(statement, 5, SQL_C_LONG, &remainingTime,
                        sizeof(remainingTime), &valueLength);
    valid = valid && SQL_SUCCEEDED(result);
    SQLFreeStmt(statement, SQL_DROP);

    if (!valid)
    {
        SecureZeroMemory(storedPassword, sizeof(storedPassword));
        MyLog(LOG_FATAL, "[LOGIN] Invalid account fields: id='%s'", LIP.id);
        return BT_WRONG_ID;
    }

    ::EatRearWhiteChar(storedPassword);
    const bool passwordMatches = strcmp(LIP.pw, storedPassword) == 0;
    SecureZeroMemory(storedPassword, sizeof(storedPassword));
    if (!passwordMatches)
    {
        MyLog(LOG_NORMAL, "[LOGIN] Password rejected: id='%s'", LIP.id);
        return BT_WRONG_PW;
    }

    if (LocalMgr.IsFreeBeta())
    {
        MyLog(LOG_NORMAL, "[LOGIN] Authentication accepted: id='%s' (free beta)", LIP.id);
        return BT_FREE;
    }

    if (expires.month < 1 || expires.month > 12 ||
        expires.day < 1 || expires.day > 31)
        return BT_NEED_PAY;

    const int limitDay = expires.year * 365 + month_tbl[expires.month - 1] + expires.day;
    const int today = g_year * 365 + g_yday;
    if (limitDay >= today) return 1 + limitDay - today;
    if (LocalMgr.IsAbleNation(TAIWAN | HONGKONG)) return BT_NEED_PAY;
    if (LocalMgr.IsAbleNation(CHINA))
        return IsFreeLevel(LIP.id) ? BT_FREE : BT_NEED_PAY;
    if (remainingTime > 0)
    {
        d_kyulje = 4000;
        LIP.type = 4000;
        return BT_COMMERCIAL_TIME_REMAIN;
    }
    return BT_NEED_PAY;
}



int COnePass::CheckGameBangIP_SQL( DWORD *can_use, LPSTR ip ,int &type, int &IP_TimeRemain,int &ip_idx)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	BOOL		bCheck;
	char		szQuerry[255];
	TIMESTAMP_STRUCT	date;
	int month_tbl[12] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };	
	int rt, ct;

	if(LocalMgr.IsAbleNation(TAIWAN|CHINA|HONGKONG))//021007 lsw
	{
		return BT_NEED_PAY;
	}

	bCheck = FALSE;
	//010104 zhh
	//sprintf(szQuerry, "select can_use, Billing_eday, ip_type from IP_USE where ip ='%s'", ip );
    /////////////////////////////////////////////////////////////////////////////
    sprintf(szQuerry, "EXEC up_get_ip_info '%s'", ip);
    /////////////////////////////////////////////////////////////////////////////
	SQLAllocStmt(hDBC_TotalDB, &hStmt);

	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
			
		if( retCode == SQL_SUCCESS ) // 등록된 IP가 존재함. 
		{	
			SQLGetData(hStmt, 1, SQL_C_ULONG, can_use, 0, &cbValue);
			SQLGetData(hStmt, 2, SQL_C_TIMESTAMP, &date,  sizeof( TIMESTAMP_STRUCT ), &cbValue);

			//1207 zhh
			SQLGetData(hStmt, 3, SQL_C_ULONG, &type, 0, &cbValue);
			SQLGetData(hStmt, 4, SQL_C_ULONG, &ip_idx, 0, &cbValue);
			
			SQLFreeStmt(hStmt, SQL_DROP);
			
			this->CheckGameBangIPAccount_SQL( ip_idx, IP_TimeRemain);
			//	날짜 계산.. 
			if( *can_use == 1 ) // 사용가능한 IP..
			{	
				rt = date.year * 365 + month_tbl[ date.month-1] + date.day;
				ct = g_year * 365 + g_yday;
				
				if( rt < ct )	return BT_NEED_PAY;
				else			return 1 + rt - ct;	// 앞으로 사용가능한 날짜에 
			}	
			else
			{	
				return -2;
			}	
		}		
		else	
		{		
			SQLFreeStmt(hStmt, SQL_DROP);
			return -2;
		}		
	}			
	else
	{
		SQLFreeStmt(hStmt, SQL_DROP);	
		return -1;							// 그런 겜방IP없음.
	}
	SQLFreeStmt(hStmt, SQL_DROP);			// 0414 YGI
	return(-2);
};

int COnePass::CheckLoginIDForJapan(const short nCn, LoginInfoPay &LIP)
{
	HSTMT	hStmt = NULL;
	RETCODE	retCode= 0;
	SQLLEN	cbValue= 0;
	char	szQuerry[ MAX_PATH ] = {0,};
	int		rt = 0, ct= 0;

	EatRearWhiteChar( LIP.id );
	EatRearWhiteChar( LIP.pw );

	sprintf(szQuerry, "SELECT   Memidx,Uid FROM NgcTempUser WHERE (Utid = '%s%s')", LIP.id,LIP.pw);//일본 DB참조//TID로 ID 가져오기.

	SQLAllocStmt(hDBC_NGCDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{															
		retCode = SQLFetch(hStmt);
		if( retCode != SQL_SUCCESS) 
		{
			goto ERROR_NO_ID_;	// 등록되지 않은 ID입니다. 
		}
		retCode = ::SQLGetData(hStmt, 1, SQL_C_LONG,	&LIP.index,	ID_LENGTH, &cbValue);
		
		char szID[ID_LENGTH]= {0,};
		retCode = ::SQLGetData(hStmt, 2, SQL_C_CHAR,		szID,	ID_LENGTH, &cbValue);
		::EatRearWhiteChar( szID );
		::CharUpper( szID );
		::strcpy(connections[nCn].id,szID);
		::strcpy(LIP.id,szID);
		::SQLFreeStmt(hStmt, SQL_DROP);
		return 100;//일본인 경우 공짜입니다. NGC에서 과금을 하기 때문에.
	}
ERROR_NO_ID_:

	::SQLFreeStmt(hStmt, SQL_DROP);	
	return BT_WRONG_ID;							// 그런 ID없음.
}

int COnePass::CheckGameBangIPAccount_SQL( DWORD ip_idx, int &IP_TimeRemain)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[255];

	if(LocalMgr.IsAbleNation(TAIWAN|CHINA|HONGKONG))//021007 lsw
	{
		IP_TimeRemain = 0;
		return 0;
	}

	sprintf(szQuerry, "select timeremain from IP_ACCOUNT where ip_idx ='%d'", ip_idx );

	SQLAllocStmt(hDBC_TotalDB, &hStmt);

	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
			
		if( retCode == SQL_SUCCESS ) // 등록된 IP가 존재함. 
		{	
			SQLGetData(hStmt, 1, SQL_C_ULONG, &IP_TimeRemain, 0, &cbValue);
			SQLFreeStmt(hStmt, SQL_DROP);			
			return IP_TimeRemain;
		}
	}
	SQLFreeStmt(hStmt, SQL_DROP);	
	return -1;
}

bool COnePass::CheckLimitedAgeAndTime(const char* szID) // 30929 kyo
{
	if(NULL == szID )
	{
		return false;
	}

	::prepare( connections );
	if( IsLimitedTime() )
	{
		int nAge = GetUserAge( szID );
		int nLimitedAge = ::GetPrivateProfileInt( "Thai Limited", "Age", 18, DB_DEMON_INI_ );
		if( nAge < nLimitedAge || nAge <=0 )
		{
			return true;
		}		
	}
	return false;	
}


