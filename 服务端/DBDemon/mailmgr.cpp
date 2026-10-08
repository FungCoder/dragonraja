// MailMgr.cpp: implementation of the CMailMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "packed.h"
#include "servertable.h"
#include "MAIN.H"
#include "Scrp_exe.H"
#include "ID.h"
#include "SealStone.h"
#include "Debug.h"
#include "mainheader.h"
#include "monitor.h"
#include "Citem.h"
#include "Pay.h"
#include "ChrLog.h"
#include "dragonloginserver2.h"		// 010406 YGI
#include "hong_sub.h"
#include "MailMgr.h"
#include "MySQL.h"

extern HDBC		hDBC_ChrLogDB;
extern HDBC		hDBC_TotalDB;
extern HENV		hEnv ;
extern HDBC		hDBC ;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
extern void ConvertQueryString(const char* szStr, char* szResult, const int iMaxString = MAX_STEP);
CMailMgr g_MailMgr;
CMailMgr::CMailMgr()
{

}

CMailMgr::~CMailMgr()
{

}

void CMailMgr::RecvReqMailList(const int cn,t_packet *p)//����Ʈ ��û�� ����
{
	//���� �ʼ����� �ǵ���	//�̸��� ��.. ��.. �� �˻�����
	REQESTMAILLIST *pMailList = (LPREQESTMAILLIST)&p->u.Hwoa.rang.ReqestMailList;
	pMailList->szName[MAX_NAME-1] = 0;
	const char *szName = pMailList->szName;//�׻���� ���� ��� ��񿡰� ��û�� �Ѵ�	

	const int iFindType = pMailList->iWantMailType;//���������� ���� ������
	const int iPageIndex = pMailList->iPageIndex;

	if(0 > iPageIndex || MAX_MAIL_PAGE_COUNT <= iPageIndex)//������ �ε����� �̻��ϴٸ�
	{
		return;
	}
	//	CMD_MAIL_LIST_FROM_DBDEMON//��Ŷ Ÿ��
//	MAILDATA[MAX_MAIL_PAGE];
	
	t_packet packet;
	packet.h.header.type = CMD_MAIL_LIST_FROM_DBDEMON;
	packet.h.header.size = sizeof(MAILLISTRESULT);

	LPMAILLISTRESULT pMLR = &packet.u.Hwoa.rang.MailListResult;
	ZeroMemory(pMLR,sizeof(MAILLISTRESULT));

	pMLR->iWantViewType = iFindType;
	pMLR->iWantViewPage = iPageIndex;
	::memcpy(pMLR->szName, pMailList->szName, MAX_NAME);
	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	
	char szmailQuerry[MAX_PATH] = {0,};
	::sprintf(szmailQuerry, "CALL MailGetList( '%s', %d)", pMLR->szName,iFindType);
	
	retCode = SQLExecDirect(hStmt, (UCHAR *)szmailQuerry, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		int j = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i < (iPageIndex)*MAX_MAIL_PAGE)//���� ������ �ϴ� ������ ���� �۴�.
			{
				i++;
				retCode = ::SQLFetch(hStmt);
				if( retCode == SQL_NO_DATA ) 
				{
					::SQLFreeStmt(hStmt, SQL_DROP);
					break;
				}
				continue;
			}
			if(i > (iPageIndex+1)*MAX_MAIL_PAGE)//���� ������ �ϴ� �������� �Ѿ��.
			{
				::SQLFreeStmt(hStmt, SQL_DROP);
				break;
			}

			if(j >= MAX_MAIL_PAGE)//������ �� �Ѿ��� ���� ���� ����
			{
				::SQLFreeStmt(hStmt, SQL_DROP);
				break;
			}

			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&pMLR->MailData[j].iIndex,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&pMLR->MailData[j].iType,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_BIT	,	&pMLR->MailData[j].bIsRead,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	pMLR->MailData[j].szSender,		MAX_NAME, &cbValue);
			::EatRearWhiteChar(pMLR->MailData[j].szSender);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	pMLR->MailData[j].szReceiver,	MAX_NAME, &cbValue);
			::EatRearWhiteChar(pMLR->MailData[j].szReceiver);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	pMLR->MailData[j].szTitle,		60, &cbValue);
			::EatRearWhiteChar(pMLR->MailData[j].szTitle);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&pMLR->MailData[j].iBodyIndex,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_TIMESTAMP, &pMLR->MailData[j].date,	sizeof( TIMESTAMP_STRUCT ), &cbValue);

			i++;
			j++;
			retCode = ::SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				continue;
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				::SQLFreeStmt(hStmt, SQL_DROP);
				break;
			}
			else
			{
				::SQLFreeStmt(hStmt, SQL_DROP);
				return;
			}
		}	
	}		
	else 
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
	//���ù��� szName �̰�
	::QueuePacket(connections,cn,&packet,1);//�ʼ����� ����������
}

void CMailMgr::Recv(const int cn, t_packet *p)
{//insert �� �ϰ� ���� �����ش�
	MAILSEND *pMailSend = &p->u.Hwoa.rang.MailSend;

	pMailSend->szSender[MAX_NAME-1]			= 0;
	pMailSend->szTarget[MAX_NAME-1]			= 0;
	pMailSend->szTitle[60-1]				= 0;
	pMailSend->szBody[MAX_MAIL_BODY_LEN-1]	= 0;

	if( InsertMail( pMailSend->iType, pMailSend->szSender ,pMailSend->szTarget, pMailSend->szTitle, pMailSend->szBody ) ) 
	{
		SendInsertMailResult(  pMailSend->szSender, cn, CMD_SEND_MAIL_OK );	// ����
		if(LocalMgr.IsAbleNation(CHINA|TAIWAN|HONGKONG))//�븸�� mail2
		{
			InsertMail2( pMailSend->iType, pMailSend->szSender ,pMailSend->szTarget, pMailSend->szTitle, pMailSend->szBody );
		}
	}
	else 
	{
		SendInsertMailResult(  pMailSend->szSender, cn, CMD_SEND_MAIL_FAIL );	// ����
	}
}

bool CMailMgr::InsertMail( const int iType, const char *szSender, const char *szReceiver, char *szTitle, char *szBody )
{		
	char szMailQuerry[3072] = {0,};
	char szMailBody[2048] = {0,};
	
	::ConvertQueryString(szBody,szMailBody,1000);

	switch(iType)
	{
	case GUILD_MAIL:
		{
			int iGuildCode = 0;
			if(!::GetGuildCode(szSender, iGuildCode)){return false;}
		
			::sprintf(szMailQuerry, "CALL MailInsertGuild( %d, %d, %d, '%s', '%s', '%s')",
				MVT_RECV_LIST,	iType,	iGuildCode,
				szSender, szTitle, szMailBody );
		}break;
	default:
		{
			::sprintf(szMailQuerry, "CALL MailInsert( %d, %d, '%s', '%s', '%s', '%s')",
				MVT_RECV_LIST,	iType,	
				szSender, szReceiver, szTitle, szMailBody );
		}break;
	}

	if( 1 == ::Querry_SQL( szMailQuerry ))
	{
		SendAlert(szReceiver);//���� ������ �˸���.
		return true;
	}
	return false;
}

// 020808 YGI ----------------------------------------------------
bool CMailMgr::InsertMail2(const int iType, const char *szSender, const char *szReceiver, char *szTitle, char *szBody )
{	
	char szMailQuerry[3072] = {0,};
	char szMailBody[2048] = {0,};
	
	::ConvertQueryString(szBody,szMailBody,1000);

	switch(iType)
	{
	case GUILD_MAIL:
		{
			int iGuildCode = 0;
			::GetGuildCode(szSender, iGuildCode);

			if(iGuildCode){return false;}
		
			::sprintf(szMailQuerry, "CALL MailInsertGuild2( %d, %d, %d, '%s', '%s', '%s')",
				MVT_RECV_LIST,	iType,	iGuildCode,
				szSender, szTitle, szMailBody );
		}break;
	default:
		{
			::sprintf(szMailQuerry, "CALL MailInsert2( %d, %d, '%s', '%s', '%s', '%s')",
				MVT_RECV_LIST,	iType,	
				szSender, szReceiver, szTitle, szMailBody );
		}break;
	}

	::Querry_SQL( szMailQuerry );
	return true;
}

void CMailMgr::SendAlert( const char *szName )		// ������ ������ �������� �޴� ������� �˷��ش�.
{
	t_packet packet;
	packet.h.header.type = CMD_MAIL_ALERT;
	{
		strcpy( packet.u.kein.who_send_mail.name, szName );
	}
	packet.h.header.size = sizeof( k_who_send_mail );
	
	g_pServerTable->BroadCastToEveryServer( (char *)&packet, (sizeof(t_header)+packet.h.header.size ), SERVER_TYPE_MAP );
}

void CMailMgr::SendInsertMailResult( const char *szName, const int cn, const int cmd_type )
{
	t_packet packet;
	packet.h.header.type = cmd_type;
	
	strcpy( packet.u.kein.who_send_mail.name, szName );
	
	packet.h.header.size = sizeof(k_who_send_mail) ;
	QueuePacket( connections, cn, &packet, 1 );
}

bool CMailMgr::RecvDelete(const int cn, t_packet *p)//������ ������ 
{//���� ����
	//�켱 ���� �����ѿ� �ְ� ���������Կ� �ִ� �ֵ��� ���� �ǵ��� ó�� ����.
	char		szQuerry[255]= {0,};

	for (int i = 0; MAX_MAIL_PAGE>i; i++)
	{
		const int iDeleteTarget = p->u.Hwoa.rang.MailDelete.aiMailIndex[i];
		if(!iDeleteTarget){continue;}

		::sprintf(szQuerry, "CALL MailDelete( %d, '%s')",
			iDeleteTarget,
			p->u.Hwoa.rang.MailDelete.szName);
		
		::Querry_SQL( szQuerry );
	}
	return true;
}

bool CMailMgr::RecvRepair(const int cn, t_packet *p)//������ ������ 
{//���� ���� // Delete �� ���� ��Ŷ�� ���ϴ�. 
	//�켱 ���� �����ѿ� �ְ� ���������Կ� �ִ� �ֵ��� ���� �ǵ��� ó�� ����.
	
	char		szQuerry[255]= {0,};
	for (int i = 0; MAX_MAIL_PAGE>i; i++)
	{
		const int iDeleteTarget = p->u.Hwoa.rang.MailDelete.aiMailIndex[i];
		if(!iDeleteTarget){continue;}

		::sprintf(szQuerry, "CALL MailRepair( %d, '%s')",
			iDeleteTarget,
			p->u.Hwoa.rang.MailDelete.szName);
		
		::Querry_SQL( szQuerry );
	}
	return true;
}

void CMailMgr::RecvRequestBody(const int cn,t_packet *p)
{//�ٵ� ã���ϴ�.
	t_packet packet;
	packet.h.header.type = CMD_MAIL_REQ_BODY_RESULT;
	packet.h.header.size = sizeof(REQESTMAILBODYRESULT);
	ZeroMemory(&packet.u.Hwoa.rang.ReqestMailBodyResult,sizeof(REQESTMAILBODYRESULT));

	LPREQESTMAILBODYRESULT pRMBR= &packet.u.Hwoa.rang.ReqestMailBodyResult;
		 
	const int iMainIndex = p->u.Hwoa.rang.ReqestMailBody.iMailIndex;
	if(!iMainIndex){return;}
	
	char		szQuerry[MAX_PATH] = {0,};
	
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	::sprintf(szQuerry, "CALL MailGetData( %d, '%s')",
	iMainIndex,	p->u.Hwoa.rang.ReqestMailBody.szName);

	::SQLAllocStmt(hDBC, &hStmt);

	retCode = ::SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = ::SQLFetch(hStmt);
		int column = 0;
	
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&pRMBR->iMailIndex,	0, &cbValue);
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&pRMBR->iType,		0, &cbValue);
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	pRMBR->szSender,	MAX_NAME, &cbValue);
		::EatRearWhiteChar(pRMBR->szSender);
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	pRMBR->szReceiver,	MAX_NAME, &cbValue);
		::EatRearWhiteChar(pRMBR->szReceiver);
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	pRMBR->szTitle,		60, &cbValue);
		::EatRearWhiteChar(pRMBR->szTitle);
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_TIMESTAMP, &pRMBR->date,	sizeof( TIMESTAMP_STRUCT ), &cbValue);
		::SQLFreeStmt(hStmt, SQL_DROP);
	}
	else 
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
	
	::sprintf(szQuerry, "CALL MailGetBody( %d, '%s')",
	iMainIndex,	p->u.Hwoa.rang.ReqestMailBody.szName);	

	::SQLAllocStmt(hDBC, &hStmt);

	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
		int column = 0;
	
		retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	pRMBR->szBody,		MAX_MAIL_BODY_LEN, &cbValue);
		::EatRearWhiteChar(pRMBR->szBody);
		SQLFreeStmt(hStmt, SQL_DROP);
		const int iBodyLength = strlen(pRMBR->szBody);
		packet.h.header.size = sizeof(REQESTMAILBODYRESULT)+iBodyLength+1-MAX_MAIL_BODY_LEN;
	}
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}

	::QueuePacket(connections,cn,&packet,1);//�ʼ����� ����������
}


bool CMailMgr::IsExistNotRead(const int cn,t_packet *p)//������ ������ �ִ°�?
{
	char		szQuerry[MAX_PATH] = {0,};

	k_start_game_mail *pSGM = &p->u.kein.start_game_mail;
	
	::sprintf(szQuerry, "CALL  MailGetNotReadCount( '%s')",
	pSGM->name);

	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	::SQLAllocStmt(hDBC, &hStmt);

	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	int iCount = 0;

	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		retCode = SQLFetch(hStmt);
		int column = 0;
		
		retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG, &iCount, 0, &cbValue);
		
		::SQLFreeStmt(hStmt, SQL_DROP);
		
		if(iCount)
		{
			SendAlert(pSGM->name);//���� ������
		}
		return ((iCount)?true:false);
	}
	else 
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	return false;
}
