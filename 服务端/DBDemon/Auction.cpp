// Auction.cpp: implementation of the CAuction class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
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
#include "Auction.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern HDBC		hDBC_ChrLogDB;
extern HDBC		hDBC_TotalDB;
extern HENV		hEnv ;
extern HDBC		hDBC ;
extern int	g_wday;

CAuction Auction;
CAuction::CAuction()
{
	Clear();
}

CAuction::~CAuction()
{
	Clear();
}

void CAuction::Clear()
{
}

//soto-030514
int CAuction::SearchAuctionItem(SEARCHRESULTLIST *List, const char* szQuery)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	::SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i > MAX_SEARCH_RESULT_LIST) 
			{
				goto __FAIL;
			}
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].iIndex	,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	List->ResultList[i].szSellerName,	20, &cbValue);
			::EatRearWhiteChar(List->ResultList[i].szSellerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	List->ResultList[i].szBuyerName,	20, &cbValue);
			::EatRearWhiteChar(List->ResultList[i].szBuyerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].iIsEnd,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].iSellerTake,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].iBuyerTake,	0, &cbValue);
			
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].iSellValue,	0, &cbValue);

			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&List->ResultList[i].m_ResultItem.attr[5],	0, &cbValue);
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO){}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				goto __FAIL;
			}
		}
		goto __SUCCESS;
	}
	goto __FAIL;

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
		return true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����Ʈ �˻� �ϱ� ���� (������
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//soto-030514
int CAuction::RecvCMD_MERCHANT_BUY_LIST_REQUEST(const int iCn, t_packet &p)//����Ʈ ��û(�����ڰ� ���°�//�˻��Ⱑ ȣ��
{
	SEARCHRESULTLIST FindResult;
	memset(&FindResult ,0, sizeof(SEARCHRESULTLIST));
	FindResult.iCn = p.u.SearchPacketServer.iCn;
	FindResult.iKey = p.u.SearchPacketServer.ClientMsg.iKey;//021113 lsw
	int	iIndex = p.u.SearchPacketServer.ClientMsg.iIndex;
	memcpy(FindResult.szName,p.u.SearchPacketServer.szName,20);
	char szQuery[1000] ={0,};
	char szKeyWord[21] = {NULL,};
	char szMerchant[21] = {NULL,};

	int nDay  = 0;

	switch(p.u.SearchPacketServer.ClientMsg.nPeriod)
	{
	case 0:
		nDay = 2;//1��.
		break;
	case 1:
		nDay = 4;//3��
		break;
	case 2:
		nDay = 8;//1����
		break;
	}


	memcpy(szKeyWord,p.u.SearchPacketServer.ClientMsg.szKeyWord,20);szKeyWord[20] = 0;
	memcpy(szMerchant,p.u.SearchPacketServer.ClientMsg.szMerchant,20);szMerchant[20] = 0;
	
	if(FindResult.iKey)
	{
		if(strlen(szKeyWord))//Ű���尡 ������.
		{
			if(strlen(szMerchant))//���� �̸��� ������.
			{
				//����� Ű����� ���� �̸��� �Ⱓ�� �����̴�.
				sprintf(szQuery,"CALL MerchantItemSearch_Key_Merchant_Day( '%s', '%s', %d, %d, %d, %d, %d)",
					szKeyWord,szMerchant,nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);
			}
			else
			{
				//����� Ű����� �Ⱓ ���� �����̴�.
				sprintf(szQuery,"CALL MerchantItemSearch_Key_Day( '%s', %d, %d, %d, %d, %d)",
					szKeyWord,nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);

			}
		}
		else//Ű���尡 ������.
		{
			if(strlen(szMerchant))//���� �̸��� ������.
			{
				//����� �����̸��� �Ⱓ�� �����̴�.
				sprintf(szQuery,"CALL MerchantItemSearch_Merchant_Day( '%s',%d, %d, %d, %d, %d)",
					szMerchant,nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);

			}
			else//����� �ܼ��� �Ⱓ���� �����̴�.
			{
				sprintf(szQuery,"CALL MerchantItemSearch_Day( %d, %d, %d, %d, %d)",
					nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);
			}
		}	
	}
	else
	{
		if(strlen(szKeyWord))//Ű���尡 ������.
		{
			if(strlen(szMerchant))//���� �̸��� ������.
			{
				//����� Ű����� ���� �̸��� �Ⱓ�� �����̴�.
				sprintf(szQuery,"CALL MerchantItemSearch_Key_Merchant_Day_Inverse( '%s', '%s', %d, %d, %d, %d, %d)",
					szKeyWord,szMerchant,nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);
			}
			else
			{
				//����� Ű����� �Ⱓ ���� �����̴�.
				sprintf(szQuery,"CALL MerchantItemSearch_Key_Day_Inverse( '%s', %d, %d, %d, %d, %d)",
					szKeyWord,nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);

			}
		}
		else//Ű���尡 ������.
		{
			if(strlen(szMerchant))//���� �̸��� ������.
			{
				//����� �����̸��� �Ⱓ�� �����̴�.
				sprintf(szQuery,"CALL MerchantItemSearch_Merchant_Day_Inverse( '%s',%d, %d, %d, %d, %d)",
					szMerchant,nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);

			}
			else//����� �ܼ��� �Ⱓ���� �����̴�.
			{
				sprintf(szQuery,"CALL MerchantItemSearch_Day_Inverse( %d, %d, %d, %d, %d)",
					nDay,
					IS_END_ALL_RIGHT, 0, 0,iIndex);
			}
		}		
	}


/*
	if(iKey)
	{
	sprintf(szQuery,"CALL MerchantItemSearchBuyerSide( %d, %d, %d, %d,	%d, %d, %d, %d)",
		iRareType, iItemLevel, iTacticType, iWearType,
		IS_END_ALL_RIGHT, 0, 0,iIndex);
	}
	else
	{
	
	sprintf(szQuery,"CALL MerchantItemSearchBuyerSideInverse( %d, %d, %d, %d,	%d, %d, %d, %d)",
		iRareType, iItemLevel, iTacticType, iWearType,
		IS_END_ALL_RIGHT, 0, 0,iIndex);
	}
*/
	if(SearchAuctionItem(&FindResult,szQuery))
	{
		t_packet rp;
		rp.h.header.type = CMD_MERCHANT_BUY_ITEM_SEARCH_RESULT;
		rp.h.header.size = sizeof(SEARCHRESULTLIST);
		rp.u.SearchResultList= FindResult;
		::QueuePacket(connections,iCn,&rp,1);
	}
	return true;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����Ʈ �˻� �ϱ� �� (������
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���� �ϱ� ����(������)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_ITEM_BUY_COMFORM(const int iCn,t_packet &p)
{
/*
	CREATE PROC MerchantItemBuyComform
	(
	@No	int,
	@SellerName char(20),
	@BuyerName char(20),
	@SetIsEnd int,
	@BeforeIsEnd int,
	@ItemNo int,
	@ItemAttr1 int,
	@ItemAttr2 int,
	@ItemAttr3 int,
	@ItemAttr4 int,
	@ItemAttr5 int,
	@ItemAttr6 int
	)
	as

	UPDATE MerchantSeller SET [IsEnd] = @SetIsEnd, [BuyerName] = @BuyerName WHERE   ([SellerName] =@SellerName) AND ([No] = @No)AND ([IsEnd] = @BeforeIsEnd)
	SELECT   count(*)FROM      MerchantSeller WHERE 
	([No] = @No)AND 
	([SellerName] =@SellerName) AND 
	([IsEnd ]=@SetIsEnd ) AND 
	([SellItemNo ]=@ItemNo) AND 
	([SellItemAttr1]= @ItemAttr1) AND 
	([SellItemAttr2 ]=@ItemAttr2) AND 
	([SellItemAttr3 ]=@ItemAttr3) AND 
	([SellItemAttr4 ]=@ItemAttr4) AND 
	([SellItemAttr5 ]=@ItemAttr5) AND 
	([SellItemAttr6 ]=@ItemAttr6) 


	MerchantItemBuyComform 1,'ȭ�����', 3,1
*/
	//isend �� �ŷ������� ���ش�
	//���� ���� �Ѱ�? //�ε���,�Ĵ³�,������,isEnd == 0(���Ű���) ���� �˻��Ѵ�
	//ī��Ʈ�� �޾Ƽ� �˻� ���� 0 �̸� ���Ǵٰ� 1 �̸� �Ǵ� ����.

	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	::SQLAllocStmt(hDBC, &hStmt);
	char szQuery[255] ={0,};
	MERCHANT_ITEM_BUY *pMIB = &p.u.MerchantItemBuy;
	MERCHANT_ITEM_BUY QueryResult = *pMIB;//�����ɷ� �ʱ�ȭ�� ��Ų��
	sprintf(szQuery,"CALL MerchantItemBuyComform( %d, '%s' , '%s', "//�ε���,�̸�,�ٲܰ�,������
		" %d, %d, "
		"%d, %d, %d, %d, %d, %d, %d)",
	pMIB->iIndex,
	pMIB->szSellerName,
	pMIB->szBuyerName,
	IS_END_BUYING,//������ ���·� �����
	IS_END_ALL_RIGHT,
	pMIB->SellItem.item_no,
	pMIB->SellItem.attr[0],
	pMIB->SellItem.attr[1],
	pMIB->SellItem.attr[2],
	pMIB->SellItem.attr[3],
	pMIB->SellItem.attr[4],
	pMIB->SellItem.attr[5]);

	//IS_END_ALL_RIGHT ���¿����� ���� �� �ִ� ������ ����°� �Ұ��� �ϴ�
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iIndex,			0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szSellerName,		20, &cbValue);
			::EatRearWhiteChar(QueryResult.szSellerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iKey,				0, &cbValue);//IsEnd���� ���´�
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.dwSellValue,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[5],	0, &cbValue);

			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	//���� ������ ����Ÿ�� �����ϴٴ� ����.(������Ʈ�� ���� �ߴ�.
				if(QueryResult.dwSellValue != pMIB->dwSellValue)
				{
					MyLog(0,"Auction Critical Warning!! Type => CMD_MERCHANT_ITEM_BUY_COMFORM SellValue NotMatch = DB %d, Client %d",QueryResult.dwSellValue, pMIB->dwSellValue);
					pMIB->dwSellValue = QueryResult.dwSellValue;
					MyLog(0,"Auction Critical Warning!! Type => CMD_MERCHANT_ITEM_BUY_COMFORM Force Change Value = %d ",QueryResult.dwSellValue);
				}
				goto __SUCCESS;
			}
			else //��� ���� ��Ȳ�� ����.
			{	//�̷� ���� �ȵǴ� ��Ȳ
				goto __FAIL;
			}
		}
		else
		{
COMMENT		MyLog(0,"Update Failed");
			goto __FAIL;
		}
	}

	goto __FAIL;

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);		
		p.h.header.type = CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT;//Ÿ���� ��ü �Ѵ�
		(*pMIB) = QueryResult;//���� ��Ŷ�� ������� �Ǿ� ������////������ ���·� ����� ���� ���̴� �ƴ϶�� �ʼ������� ���� �Ұ����̶�� ó�� �϶�
		::QueuePacket(connections,iCn,&p,1);	
		return;
	}
}

void CAuction::RecvCMD_MERCHANT_ITEM_BUY_COMFORM_RESULT(const int iCn,t_packet &p)
{	//isend �� �ŷ� ����� ���ش� �Ա� �Ϸ��
	const int iKey = p.u.MerchantItemBuy.iKey;
	if(	IS_END_WAIT_TAKE	!= iKey//�� �������� 
	&&	IS_END_ALL_RIGHT	!= iKey)//�� ������
	{//�̰� �ΰ��� �ƴϴ�
		MyLog(0,"Auction Critical Error!! Type => CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT");
		return;
	}
	//isend �� �ŷ������� ���ش�
	//���� ���� �Ѱ�? //�ε���,�Ĵ³�,������,isEnd == 0(���Ű���) ���� �˻��Ѵ�
	//ī��Ʈ�� �޾Ƽ� �˻� ���� 0 �̸� ���Ǵٰ� 1 �̸� �Ǵ� ����.
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	::SQLAllocStmt(hDBC, &hStmt);
	char szQuery[255] ={0,};
	MERCHANT_ITEM_BUY *pMIB = &p.u.MerchantItemBuy;
	MERCHANT_ITEM_BUY QueryResult = *pMIB;//�����ɷ� �ʱ�ȭ�� ��Ų��
	sprintf(szQuery,"CALL MerchantItemBuyComform( %d, '%s' , '%s', "//�ε���,�̸�,�ٲܰ�,������
		" %d, %d, "
		"%d, %d, %d, %d, %d, %d, %d)",
	pMIB->iIndex,
	pMIB->szSellerName,
	pMIB->szBuyerName,
	iKey,//�ʼ����� ������ ������ ���ش�
	IS_END_BUYING,//������ ���¿��� �ϰ�
	pMIB->SellItem.item_no,
	pMIB->SellItem.attr[0],
	pMIB->SellItem.attr[1],
	pMIB->SellItem.attr[2],
	pMIB->SellItem.attr[3],
	pMIB->SellItem.attr[4],
	pMIB->SellItem.attr[5] );

	//IS_END_ALL_RIGHT ���¿����� ���� �� �ִ� ������ ����°� �Ұ��� �ϴ�
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iIndex,			0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szSellerName,		20, &cbValue);
			::EatRearWhiteChar(QueryResult.szSellerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iKey,				0, &cbValue);//IsEnd���� ���´�
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.dwSellValue,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[5],	0, &cbValue);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	//���� ������ ����Ÿ�� �����ϴٴ� ����.(������Ʈ�� ���� �ߴ�.
				if(QueryResult.dwSellValue != pMIB->dwSellValue)
				{
					MyLog(0,"Auction Critical Warning!! Type => CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT SellValue NotMatch = DB %d, Client %d",QueryResult.dwSellValue, pMIB->dwSellValue);
					pMIB->dwSellValue = QueryResult.dwSellValue;
					MyLog(0,"Auction Critical Warning!! Type => CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT Force Change Value = %d ",QueryResult.dwSellValue);
				}
				goto __SUCCESS;
			}
			else //��� ���� ��Ȳ�� ����.
			{	//�̷� ���� �ȵǴ� ��Ȳ
				goto __FAIL;
			}
		}
		else
		{
COMMENT		MyLog(0,"Non Select Result");
			goto __FAIL;
		}
	}

	goto __FAIL;
	
__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		p.h.header.type = CMD_MERCHANT_ITEM_BUY_RESULT;//Ÿ���� ��ü �Ѵ�
		(*pMIB) = QueryResult;//���� ��Ŷ�� ������� �Ǿ� ������
		::QueuePacket(connections,iCn,&p,1);
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���� �ϱ� ��(������)
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����Ʈ �˻� �ϱ� ����(�Ǹ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_SELL_LIST_REQUEST(const int iCn,t_packet &p)//����Ʈ ��û(�Ǹ��ڰ� ���°�
{
	const char* szName = p.u.SellerItemRequest.szName;
	
	SEARCHRESULTLIST FindResult;
	memset(&FindResult ,0, sizeof(SEARCHRESULTLIST));
	FindResult.iCn = p.u.SellerItemRequest.iCn;
	FindResult.iKey = p.u.SellerItemRequest.iKey;//021113 lsw
	memcpy(FindResult.szName,p.u.SellerItemRequest.szName,20);
	char szQuery[1000] ={0,};
	if(p.u.SellerItemRequest.iKey)
	{
	sprintf(szQuery,"CALL MerchantItemSearchSellerSide( %d, '%s',%d)",
		p.u.SellerItemRequest.iIndex, szName,IS_END_ALL_RIGHT);//�ε��� ���� ũ�� �̸��� IsEnd �� 0 �ΰ��� �����ɴϴ�.
	}
	else
	{
	sprintf(szQuery,"CALL MerchantItemSearchSellerSideInverse( %d, '%s',%d)",
		p.u.SellerItemRequest.iIndex, szName,IS_END_ALL_RIGHT);//�ε��� ���� ũ�� �̸��� IsEnd �� 0 �ΰ��� �����ɴϴ�.
	}

	if(SearchAuctionItem(&FindResult,szQuery))
	{
		t_packet rp;
		rp.h.header.type = CMD_MERCHANT_SELL_LIST_REQUEST_RESULT;
		rp.h.header.size = sizeof(SEARCHRESULTLIST);
		rp.u.SearchResultList= FindResult;
		::QueuePacket(connections,iCn,&rp,1);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ����Ʈ �˻� �ϱ� �� (�Ǹ���
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���� �ϱ� ����(�Ǹ� ���)
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_SELL_ITEM_DELETE_COMFORM(const int cn,t_packet &p)//�Ǹ� �������� ����
{	//�̸��� �ε����� �׿����� �������� �޾ƿ����� �Ѵ�.
	//�������� ���� ��� �� ������ �� �غ��� �Ѵ�
/*	
CREATE PROC MerchantItemCancelComform
(
@No	int,
@SellerName char(20),
@SetIsEnd int,
@BeforeIsEnd int
)
as

UPDATE MerchantSeller SET [IsEnd] = @SetIsEnd WHERE   ([SellerName] =@SellerName) AND ([No] = @No)AND ([IsEnd] = @BeforeIsEnd)
SELECT   [No], [SellerName], [IsEnd],[SellItemNo],[SellItemAttr1],[SellItemAttr2],[SellItemAttr3],[SellItemAttr4],[SellItemAttr5],[SellItemAttr6] FROM      MerchantSeller WHERE  ([SellerName] =@SellerName) AND ([No] = @No)
MerchantItemCancelComform 1,'ȭ�����', 1,1 //�ڵ����� �� üũ�ؼ� ��� �޾ƿ��°��̴�.. ^^ IsEnd�� �ݵ�� ���ϵȴ�.
*/// �ѹ� ���� �Ǹ� �ι�° �ϴ��� ������ �Ұ��� �ϴ�
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	::SQLAllocStmt(hDBC, &hStmt);
	char szQuery[255] ={0,};
	SELLERITEMDELETE *pSID = &p.u.SellerItemDelete;
	SELLERITEMDELETE QueryResult = *pSID;//�����ɷ� �ʱ�ȭ�� ��Ų��
	sprintf(szQuery,"CALL MerchantItemCancelComform( %d, '%s' , %d , %d )",//�ε���,�̸�,�ٲܰ�,������
	pSID->iIndex ,pSID->szName ,IS_END_DELETING,IS_END_ALL_RIGHT );//�̸����� ī��Ʈ�� �޴´�
	//IS_END_ALL_RIGHT ���¿����� ���� �� �ִ� ������ ����°� �Ұ��� �ϴ�
	int iSellerCount = 0;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iIndex,			0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szName,				20, &cbValue);
			::EatRearWhiteChar(QueryResult.szName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iKey,				0, &cbValue);//IsEnd���� ���´�
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[5],	0, &cbValue);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	//���� ������ ����Ÿ�� �����ϴٴ� ����.(������Ʈ�� ���� �ߴ�.
				goto __SUCCESS;
			}
			else //��� ���� ��Ȳ�� ����.
			{	//�̷� ���� �ȵǴ� ��Ȳ
				goto __FAIL;
			}
		}
		else
		{
			QueryResult.iKey = -1;
			goto __FAIL;
		}
	}
	else 
	{
		QueryResult.iKey = -1;
		goto __FAIL;
	}

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		p.h.header.type = CMD_MERCHANT_SELL_ITEM_DELETE_COMFORM_RESULT;//Ÿ���� ��ü �Ѵ�
		*pSID = QueryResult;//���� ��Ŷ�� ������� �Ǿ� ������
		::QueuePacket(connections,cn,&p,1);	
		return;
	}	
}

void CAuction::RecvCMD_MERCHANT_SELL_ITEM_DELETE_COMFORM_RESULT(const int cn,t_packet &p)//�Ǹ� �������� ����
{	//�ʼ����� ���� ������ IS_END_DELETE_COMPLETE �� üũ �Ǿ� ���� ���̰� 
	//�ƴϸ� IS_END_ALL_RIGHT�� üũ �Ǿ� ���� ���̴�
	const int iKey = p.u.SellerItemDelete.iKey;
	if(	IS_END_DELETE_COMPLETE  != iKey
	&&	IS_END_ALL_RIGHT		!= iKey)
	{//�̰� �ΰ��� �ƴϴ�
		MyLog(0,"Auction Critical Error!! Type => CMD_MERCHANT_SELL_ITEM_DELETE_COMFORM_RESULT");
		return;
	}

	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	::SQLAllocStmt(hDBC, &hStmt);
	char szQuery[255] ={0,};
	SELLERITEMDELETE *pSID = &p.u.SellerItemDelete;
	SELLERITEMDELETE QueryResult = *pSID;//�����ɷ� �ʱ�ȭ�� ��Ų��
	sprintf(szQuery,"CALL MerchantItemCancelComform( %d, '%s' , %d , %d )",//�ε���,�̸�,�ٲܰ�,������
	pSID->iIndex,pSID->szName ,p.u.SellerItemDelete.iKey, IS_END_DELETING );//�̸����� ī��Ʈ�� �޴´�
	int iSellerCount = 0;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iIndex,			0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szName,				20, &cbValue);
			::EatRearWhiteChar(QueryResult.szName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iKey,				0, &cbValue);//IsEnd���� ���´�
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[5],	0, &cbValue);
			
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	//���� ������ ����Ÿ�� �����ϴٴ� ����.(������Ʈ�� ���� �ߴ�.
				goto __SUCCESS;
			}
			else //��� ���� ��Ȳ�� ����.
			{	//�̷� ���� �ȵǴ� ��Ȳ
				goto __FAIL;
			}
		}
			goto __FAIL;
	}		

	goto __FAIL;

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		p.h.header.type = CMD_MERCHANT_SELL_ITEM_DELETE_RESULT;//Ÿ���� ��ü �Ѵ�
		*pSID = QueryResult;//���� ��Ŷ�� ������� �Ǿ� ������
		::QueuePacket(connections,cn,&p,1);	
		return;
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���� �ϱ� ��(�Ǹ� ���)
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��� �ϱ�
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_SELL_ITEM_REGISTER_COMFORM(const int iCn,t_packet &p)//�Ǹ� �������� ���
{	//������ üũ�ϰ� //������� ������ ������
	//��� �� �� �ִ��� �ڽ��� ����� ���� üũ
/*
CREATE PROC MerchantItemRegistCountComform
(
@SellerName char(20)
)
as

select count(*) FROM      MerchantSeller where SellerName = @SellerName

MerchantItemRegistCountComform '�̸�'
*/
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	::SQLAllocStmt(hDBC, &hStmt);
	char szQuery[255] ={0,};
	sprintf(szQuery,"CALL MerchantItemRegistCountComform( '%s')",p.u.SellerItemRegister.szName);//�̸����� ī��Ʈ�� �޴´�
	int iSellerCount = 0;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&iSellerCount	,	0, &cbValue);
			goto __SUCCESS;
		}
		else
		{
			goto __FAIL;
		}
	}		

	goto __FAIL;

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		p.h.header.type	= CMD_MERCHANT_SELL_ITEM_REGISTER_COMFORM_RESULT;//���� �����
		p.u.SellerItemRegister.iKey = (iSellerCount <MAX_SEARCH_RESULT_LIST)?1:0;//�� �� ������ 1 �ƴϸ� 0//MAX_SEARCH_RESULT_LIST �ٲ��� ���� ���� �ٸ��� �־��
		::QueuePacket(connections,iCn,&p,1);//���� �ʼ����� �ٽ� ����
		return;//�Ĵ� ���� ����
	}
}

void CAuction::RecvCMD_MERCHANT_SELL_ITEM_REGISTER_COMFORM_RESULT(const int iCn,t_packet &p)//�Ǹ� �������� ���
{
	//����� �ϰ� IsEnd�� �ŷ������ �����. �̰� �ڵ��� ���� ��
	//	p.u.SellerItemRegister.	//Key�� ���� �սô� 
	//�ǸŹ�ǰ ���
/*
	CREATE PROC MerchantItemRegist
(
@SellerName char(20),
@ExchangeMoney numeric,
@RareType 	int,
@ItemLevel 	 int,
@TacticType 	 int,
@WearKind  	int,
@ItemNo		int,
@ItemAttr1	numeric,
@ItemAttr2	numeric,
@ItemAttr3	numeric,
@ItemAttr4	numeric,
@ItemAttr5	numeric,
@ItemAttr6	numeric
)
as

INSERT into  MerchantSeller(
	SellerName,	
	ExchangeMoney,
	RareType	,
	ItemLevel	,
	TacticType	 ,
	WearKind	 ,
	SellItemNo	 ,
	SellItemAttr1 ,
	SellItemAttr2 ,
	SellItemAttr3 ,
	SellItemAttr4 ,
	SellItemAttr5 ,
	SellItemAttr6 
) VALUES ( @SellerName, @ExchangeMoney, @RareType, @ItemLevel, @TacticType, @WearKind, 
@ItemNo, @ItemAttr1,@ItemAttr2,@ItemAttr3,@ItemAttr4,@ItemAttr5,@ItemAttr6 )

������.
exec MerchantItemRegist '����̸�',��,Ŭ����,����,�����۷���,��ƽ,�Դ����,������ ,attr1~6

*/
	lpSELLERITEMREGISTER lpSIR = &p.u.SellerItemRegister;
	char szQuery[500] = {0,};
	sprintf(szQuery,"CALL MerchantItemRegist( '%s', "
		"%d,	"
		"%d,%d,%d,%d,"
		"%d, %d,%d,%d,%d,%d,%d)",
	lpSIR->szName,
	
	(int)lpSIR->dwSellValue,

	lpSIR->iFKRareType,
	lpSIR->iFKLevel,
	lpSIR->iFKTacticType,
	lpSIR->iFKWearType,

	lpSIR->SellItem.item_no,
	(int)lpSIR->SellItem.attr[0],
	(int)lpSIR->SellItem.attr[1],
	(int)lpSIR->SellItem.attr[2],
	(int)lpSIR->SellItem.attr[3],
	(int)lpSIR->SellItem.attr[4],
	(int)lpSIR->SellItem.attr[5]
	);
		
	if( 1 == ::Querry_SQL(szQuery,hDBC))//�巡�� ���� ����
	{//����//��� �Ϸ�
		p.u.SellerItemRegister.iKey = 1;
	}
	else
	{//����
		p.u.SellerItemRegister.iKey = 0;
	}
	p.h.header.type = CMD_MERCHANT_SELL_ITEM_REGISTER_RESULT;//����� ������
	::QueuePacket(connections,iCn,&p,1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��� �ϱ� ��
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ŷ� ��� ����Ʈ �ޱ� ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_RESULT_LIST_REQUEST(const int iCn,t_packet &p)
{
	const char* szName = p.u.SellerItemRequest.szName;
	
	SEARCHRESULTLIST FindResult;
	memset(&FindResult ,0, sizeof(SEARCHRESULTLIST));
	FindResult.iCn = p.u.SellerItemRequest.iCn;
	FindResult.iKey = p.u.SellerItemRequest.iKey;//021113 lsw
	memcpy(FindResult.szName,p.u.SellerItemRequest.szName,20);
	char szQuery[255] ={0,};

	if(p.u.SellerItemRequest.iKey)
	{
		sprintf(szQuery,"CALL MerchantResultItemSearch( %d,'%s',%d)",
			p.u.SellerItemRequest.iIndex, szName,IS_END_WAIT_TAKE);//�̸��� IsEnd �� 0 �ΰ��� �����ɴϴ�.
	}
	else
	{
		sprintf(szQuery,"CALL MerchantResultItemSearchInverse( %d,'%s',%d)",
			p.u.SellerItemRequest.iIndex, szName,IS_END_WAIT_TAKE);//�̸��� IsEnd �� 0 �ΰ��� �����ɴϴ�.
	}
	if(SearchAuctionItem(&FindResult,szQuery))
	{
		t_packet rp;
		rp.h.header.type = CMD_MERCHANT_RESULT_LIST_REQUEST_RESULT;
		rp.h.header.size = sizeof(SEARCHRESULTLIST);
		rp.u.SearchResultList= FindResult;
		::QueuePacket(connections,iCn,&rp,1);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ŷ� ��� ����Ʈ �ޱ� ��
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ŷ� ����� �������� ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_RESULT_TAKE_COMFORM(const int iCn,t_packet &p)
{	//�̸��� �ε����� �׿����� �������� �޾ƿ����� �Ѵ�.
	//�������� ���� ��� �� ������ �� �غ��� �Ѵ�
	//SellerTake BuyerTake üũ�� �Ѵ�
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuery[255] ={0,};
	MERCHANTRESULTTAKE *pMRT = &p.u.MerchantResultTake;
	MERCHANTRESULTTAKE QueryResult = *pMRT;//�����ɷ� �ʱ�ȭ�� ��Ų��

COMMENT	::MyLog(0,"O Index %d ItemNo %d",pMRT->iIndex , pMRT->SellItem);
COMMENT	::MyLog(0,"N Index %d ItemNo %d",QueryResult.iIndex , QueryResult.SellItem);

	if(pMRT->iKey)//�Ǹ��� �̸�
	{
		sprintf(szQuery,"CALL MerchantResultItemTakeSellerSideComform( %d, '%s' , %d , %d, %d )",//�ε���,�̸�,�ٲܰ�,������
		pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE,IS_END_DELETING, IS_END_ALL_RIGHT );//�̸����� ī��Ʈ�� �޴´�
	}
	else
	{
		sprintf(szQuery,"CALL MerchantResultItemTakeBuyerSideComform( %d, '%s' , %d , %d , %d)",//�ε���,�̸�,�ٲܰ�,������
		pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE,IS_END_DELETING, IS_END_ALL_RIGHT );//�̸����� ī��Ʈ�� �޴´�
	}
COMMENT	MyLog(0,"RecvCMD_MERCHANT_RESULT_TAKE_COMFORM %s",szQuery);

	::SQLAllocStmt(hDBC, &hStmt);
	//IS_END_ALL_RIGHT ���¿����� ���� �� �ִ� ������ ����°� �Ұ��� �ϴ�
	int iSellerCount = 0;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iIndex,			0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szSellerName,		20, &cbValue);
			::EatRearWhiteChar(QueryResult.szSellerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szBuyerName,		20, &cbValue);
			::EatRearWhiteChar(QueryResult.szBuyerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iSellerTake,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iBuyerTake,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.dwSellValue,		0, &cbValue);//IsEnd���� ���´�
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[5],	0, &cbValue);
			
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	//���� ������ ����Ÿ�� �����ϴٴ� ����.(������Ʈ�� ���� �ߴ�.
COMMENT			MyLog(0,"Update Complete");
				goto __SUCCESS;
			}
			else //��� ���� ��Ȳ�� ����.
			{	//�̷� ���� �ȵǴ� ��Ȳ
COMMENT			MyLog(0,"Update Failed");
				goto __FAIL;
			}
		}
		else
		{
COMMENT		MyLog(0,"Non Select Result");
			QueryResult.iKey = -1;//�ָ� �ȵȴ�!!
			goto __SUCCESS;
		}
	}
	else 
	{
COMMENT	::MyLog(0,"Query Failed");
		goto __FAIL;
	}

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;	
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		p.h.header.type = CMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT;//Ÿ���� ��ü �Ѵ�
		*pMRT = QueryResult;//���� ��Ŷ�� ������� �Ǿ� ������
COMMENT	::MyLog(0,"O Index %d ItemNo %d",pMRT->iIndex , pMRT->SellItem);
COMMENT	::MyLog(0,"N Index %d ItemNo %d",QueryResult.iIndex , QueryResult.SellItem);
		::QueuePacket(connections,iCn,&p,1);
		return;
	}
}

void CAuction::RecvCMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT(const int iCn,t_packet &p)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuery[255] ={0,};
	MERCHANTRESULTTAKE *pMRT = &p.u.MerchantResultTake;
	MERCHANTRESULTTAKE QueryResult = *pMRT;//�����ɷ� �ʱ�ȭ�� ��Ų��

COMMENT	::MyLog(0,"O Index %d ItemNo %d",pMRT->iIndex , pMRT->SellItem);
COMMENT	::MyLog(0,"N Index %d ItemNo %d",QueryResult.iIndex , QueryResult.SellItem);

	switch(pMRT->iKey)
	{
	case 101://�Ǹ��� ���� ����
		{
			sprintf(szQuery,"CALL MerchantResultItemTakeSellerSideComform( %d, '%s' , %d , %d , %d)",//�ε���,�̸�,�ٲܰ�,������
			pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE,IS_END_DELETE_COMPLETE,IS_END_DELETING );//�̸����� ī��Ʈ�� �޴´�
		}break;
	case 100://������ ���� ����
		{
			sprintf(szQuery,"CALL MerchantResultItemTakeBuyerSideComform( %d, '%s' , %d , %d , %d )",//�ε���,�̸�,�ٲܰ�,������
			pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE,IS_END_DELETE_COMPLETE,IS_END_DELETING );//�̸����� ī��Ʈ�� �޴´�
		}break;
	case 11://�Ǹ��� ���� ����
		{
			sprintf(szQuery,"CALL MerchantResultItemTakeSellerSideComform( %d, '%s' , %d , %d , %d )",//�ε���,�̸�,�ٲܰ�,������
			pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE,IS_END_ALL_RIGHT ,IS_END_DELETING);//�̸����� ī��Ʈ�� �޴´�
			::Querry_SQL(szQuery);
			return;
		}break;
	case 10://������ ���� ����
		{
			sprintf(szQuery,"CALL MerchantResultItemTakeBuyerSideComform( %d, '%s' , %d , %d , %d )",//�ε���,�̸�,�ٲܰ�,������
			pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE,IS_END_ALL_RIGHT,IS_END_DELETING );//�̸����� ī��Ʈ�� �޴´�
			::Querry_SQL(szQuery);
			return;
		}break;
	default:
		{
			MyLog(0,"Auction Critical Error!! Type => RecvCMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT Query Make Fail %d",pMRT->iKey);
			return;
		}break;
	}
	::SQLAllocStmt(hDBC, &hStmt);
	
COMMENT	::MyLog(0,"RecvCMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT %s",szQuery);

	int iSellerCount = 0;
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		retCode = SQLFetch(hStmt);
		if( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int column = 0;
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iIndex,			0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szSellerName,		20, &cbValue);
			::EatRearWhiteChar(QueryResult.szSellerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_CHAR,	QueryResult.szBuyerName,		20, &cbValue);
			::EatRearWhiteChar(QueryResult.szBuyerName);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iSellerTake,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.iBuyerTake,		0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.dwSellValue,		0, &cbValue);//IsEnd���� ���´�
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.item_no,	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[0],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[1],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[2],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[3],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[4],	0, &cbValue);
			retCode = ::SQLGetData(hStmt, ++column, SQL_C_LONG,	&QueryResult.SellItem.attr[5],	0, &cbValue);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{	//���� ������ ����Ÿ�� �����ϴٴ� ����.(������Ʈ�� ���� �ߴ�.
COMMENT			MyLog(0,"Update Complete");				
				goto __SUCCESS;
			}
			else //��� ���� ��Ȳ�� ����.
			{	//�̷� ���� �ȵǴ� ��Ȳ
COMMENT			MyLog(0,"Non Select Result");
				goto __FAIL;
			}
		}
		else
		{
COMMENT		MyLog(0,"Non Select Result");
			goto __FAIL;
		}
	}
	else 
	{
COMMENT	MyLog(0,"Query Failed");
		goto __FAIL;
	}

__FAIL:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}
__SUCCESS:
	{
		::SQLFreeStmt(hStmt, SQL_DROP);
		
		p.h.header.type = CMD_MERCHANT_RESULT_TAKE_RESULT;//Ÿ���� ��ü �Ѵ�
		*pMRT = QueryResult;//���� ��Ŷ�� ������� �Ǿ� ������

COMMENT	::MyLog(0,"RO Index %d ItemNo %d",pMRT->iIndex , pMRT->SellItem);
COMMENT	::MyLog(0,"RN Index %d ItemNo %d",QueryResult.iIndex , QueryResult.SellItem);

		::QueuePacket(connections,iCn,&p,1);	
		return;
	}	
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
// �ŷ� ����� �������� ��
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//������ �ŷ� ����Ʈ ����
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAuction::RecvCMD_MERCHANT_DIRECT_EXCHANGE_LIST_REQUEST(const int iCn,t_packet &p)
{
	SEARCHRESULTLIST FindResult;
	memset(&FindResult ,0, sizeof(SEARCHRESULTLIST));
	FindResult.iCn = p.u.MerchantExchangeRequest.iCn;
	FindResult.iKey = p.u.MerchantExchangeRequest.iKey;//021113 lsw
	memcpy(FindResult.szName,p.u.MerchantExchangeRequest.szMyName,20);
	char szQuery[1000] ={0,};

	if(p.u.MerchantExchangeRequest.iKey)//�ε��� ���ΰ� �Ʒ��ΰ�.
	{
	sprintf(szQuery,"CALL MerchantItemSearchDirectExchage( '%s', %d, %d, %d, %d)",
		p.u.MerchantExchangeRequest.szSellerName,
		IS_END_ALL_RIGHT,
		IS_END_ALL_RIGHT,
		IS_END_ALL_RIGHT,
		p.u.MerchantExchangeRequest.iIndex);
	}
	else
	{
	sprintf(szQuery,"CALL MerchantItemSearchDirectExchageInverse( '%s', %d, %d, %d, %d)",
		p.u.MerchantExchangeRequest.szSellerName,
		IS_END_ALL_RIGHT,
		IS_END_ALL_RIGHT,
		IS_END_ALL_RIGHT,
		p.u.MerchantExchangeRequest.iIndex);
	}
	if(SearchAuctionItem(&FindResult,szQuery))
	{
		t_packet rp;
		rp.h.header.type = CMD_MERCHANT_DIRECT_EXCHANGE_LIST_REQUSET_RESULT;
		rp.h.header.size = sizeof(SEARCHRESULTLIST);
		rp.u.SearchResultList= FindResult;
		::QueuePacket(connections,iCn,&rp,1);
	}
	return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//������ �ŷ� ����Ʈ ��
////////////////////////////////////////////////////////////////////////////////////////////////////////////


//<! BBD 040226	�ŷ��� ����ǰų� �����ҷ� �ʿ���� ���ڵ带 �����ϴ� ��� �߰�
void CAuction::RecvCMD_MERCHANT_RECORD_DEL_CANCLE(const int iCn,t_packet &p)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuery[255] ={0,};
	SELLERITEMDELETE *pSID = &p.u.SellerItemDelete;
	// ���� ���ڸ� �Ű������� �ϴ� �������ν����� �ߵ��Ͽ� �����
	// (No, name = �Ĵ³�, IsEnd = IS_END_DELETE_COMPLETE)
	::SQLAllocStmt(hDBC, &hStmt);
	::memset(szQuery, 0L, sizeof(szQuery));
	sprintf(szQuery,"CALL MerchantDelbyCancel( %d, '%s' , %d)", pSID->iIndex, pSID->szName , IS_END_DELETE_COMPLETE);

	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		// ���� ����
		// ���������� �ƴ����� ���� ���������� �ʼ������� �˷��� �Ѵ�
		retCode = SQLFetch(hStmt);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			int count;
			retCode = ::SQLGetData(hStmt, 1, SQL_C_LONG,	&count,			0, &cbValue);

			if(count == 1)		// 1���� ���ڵ尡 ���� �Ǿ���
			{
				// Ȥ �����ߴٰ� �ʼ������� �˷��� �ұ�? �α׵� ����� �Դµ�?
//				p.h.header.type = CMD_MERCHANT_RECORD_DEL_CANCLE; //���ڵ� �����ߴٰ� �˸���
//				::QueuePacket(connections,cn,&p,1);
			}
			else
			{
				// �ش� ���ڵ尡 ���� BBD�� �߰��ϱ� �����κп��� �̹� ������ �߻��ߴ�
				// �Ǹ�����ϴٰ� �����ٰų� ��Ÿ����� ������ ����� ��Ұ� �ʉ��
			}

			::SQLFreeStmt(hStmt, SQL_DROP);
			return;
		}
	}
	else
	{
		// �̻�Ȳ�� ���� ����
		MyLog(0,"Query Failed");
		::SQLFreeStmt(hStmt, SQL_DROP);
		return;
	}


}

void CAuction::RecvCMD_MERCHANT_RECORD_DEL_COMPLETE(const int iCn,t_packet &p)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char szQuery[255] ={0,};
	MERCHANTRESULTTAKE *pMRT = &p.u.MerchantResultTake;

	// ���� ���ڸ� �Ű������� �ϴ� �������ν����� �ߵ��Ͽ� �����
	// (No, name = �Ĵ³��̳� ��³�, IsEnd = IS_END_WAIT_TAKE, SellerTake = IS_END_DELETE_COMPLETE, BuyerTake = IS_END_DELETE_COMPLETE)
	if(pMRT->iKey == 100|| pMRT->iKey == 101)
	{
		::SQLAllocStmt(hDBC, &hStmt);
		::memset(szQuery, 0L, sizeof(szQuery));
		sprintf(szQuery,"CALL MerchantDelbyComplete( %d, '%s', %d, %d, %d)"//�ε���,�̸�, 
			,pMRT->iIndex, pMRT->szMyName, IS_END_WAIT_TAKE, IS_END_DELETE_COMPLETE, IS_END_DELETE_COMPLETE);

        retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			// ���� ����
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
				int count;
				retCode = ::SQLGetData(hStmt, 1, SQL_C_LONG,	&count,			0, &cbValue);

				if(count == 1)		// 1���� ���ڵ尡 ���� �Ǿ���
				{
					// Ȥ �����ߴٰ� �ʼ������� �˷��� �ұ�? �α׵� ����� �Դµ�?
//					p.h.header.type = CMD_MERCHANT_RECORD_DEL_COMPLETE; //���ڵ� �����ߴٰ� �˸���
//					::QueuePacket(connections,cn,&p,1);
				}
				else
				{
					// �ش� ���ڵ尡 ����
					// ���� ����⿡�� ������ ���ġ ���ϴ�
				}

				::SQLFreeStmt(hStmt, SQL_DROP);
				return;
			}
			else
			{
				// �̻�Ȳ�� ���� ����
				MyLog(0,"Query Failed");
				::SQLFreeStmt(hStmt, SQL_DROP);
				return;
			}
		}
		else
		{
			// �̻�Ȳ�� ���� ����
			MyLog(0,"Query Failed");
			::SQLFreeStmt(hStmt, SQL_DROP);
			return;
		}
	}

}
//> BBD 040226	�ŷ��� ����ǰų� �����ҷ� �ʿ���� ���ڵ带 �����ϴ� ��� �߰�
//<!  BBD 040303 ������̺��� ������ ����Ʈ ��û
void CAuction::RecvCMD_MERCHANT_BACKUP_LIST_REQUEST(const int iCn,t_packet &p)
{
	short int server_id = p.u.kein.send_db_direct_map.server_id;
	SELLERITEMREQUEST *pData = (SELLERITEMREQUEST*)p.u.kein.send_db_direct_map.data;
	

	SEARCHRESULTLIST FindResult;
	memset(&FindResult ,0, sizeof(SEARCHRESULTLIST));
	FindResult.iCn = pData->iCn;
	FindResult.iKey =pData->iKey;
	memcpy(FindResult.szName,pData->szName,20);

	char szQuery[255] ={0,};
	if(FindResult.iKey)
	{
		sprintf(szQuery,"CALL MerchantBackupListSearch( %d, '%s')", FindResult.iKey, FindResult.szName);
			
	}
	else
	{
//		sprintf(szQuery,"CALL MerchantResultItemSearchInverse( %d,'%s',%d)",
//			p.u.SellerItemRequest.iIndex, szName,IS_END_WAIT_TAKE);//�̸��� IsEnd �� 0 �ΰ��� �����ɴϴ�.
	}
	if(SearchAuctionItem(&FindResult,szQuery))
	{
		DirectClient( CMD_MERCHANT_BACKUP_LIST_RESPONSE, server_id, iCn, &FindResult, sizeof( SEARCHRESULTLIST ) );
	}
}
//>  BBD 040303 ������̺��� ������ ����Ʈ ��û
//<! BBD 040303 ������̺��� ������ ���� ��û
void CAuction::RecvCMD_MERCHANT_BACKUP_TAKE_REQUEST(const int iCn,t_packet &p)
{
	short int server_id = p.u.kein.send_db_direct_map.server_id;
	t_MerchantResultTake *pData = (t_MerchantResultTake*)p.u.kein.send_db_direct_map.data;
	MERCHANTRESULTTAKE *pMRT = &p.u.MerchantResultTake;

	char szQuery[255] ={0,};
	sprintf(szQuery,"CALL MerchantBackupByNoName( %d, '%s')", pData->iIndex, pData->szMyName);

	SEARCHRESULTLIST FindResult;
	memset(&FindResult, 0L, sizeof(FindResult));

	if(!SearchAuctionItem(&FindResult,szQuery))	// ���� ������ index�ѹ��� �̸��� �����Ͽ� �˻��ѵ�
	{
		MyLog(0,"This Query Failed : %s", szQuery);		// �̻�Ȳ�� ���� ����
		return;	// ���� ���д�
	}
	SEARCHRESULT * pRecord = &(FindResult.ResultList[0]);

	// �����Ұ��ɵ� �ƴϰ� ����ã�� �����µ� �ƴ϶��
	if(pRecord->iIsEnd != IS_END_ALL_RIGHT && pRecord->iIsEnd != IS_END_WAIT_TAKE)
	{
		return;		// �̷����� ���� ������ �Գ�?
	}

	pMRT->iKey = pRecord->iIsEnd;	// �̶��� iKey�� IsEnd�뵵�� ����

	memset(szQuery, 0L, sizeof(szQuery));
	if(strcmp(pData->szMyName, pRecord->szSellerName))		// �̸����� buyer���� seller���� �����ϰ�
	{		// Buyer��...
		if(pRecord->iBuyerTake != IS_END_ALL_RIGHT)
		{
			return;			// ���� �Ա� �� �ĸ�������?
		}
		sprintf(szQuery, "CALL MerchantBackupTakeUpdate( %d, 1, %d)", pRecord->iIndex, IS_END_GIVIING);
	}
	else
	{		// Seller��...
		if(pRecord->iSellerTake != IS_END_ALL_RIGHT)
		{
			return;			// ���� �Ա� �� �ĸ�������?
		}
		sprintf(szQuery, "CALL MerchantBackupTakeUpdate( %d, 0, %d)", pRecord->iIndex, IS_END_GIVIING);
	}
	// �ʵ带 ������Ʈ ���ѳ���	...Take �ʵ忡 IS_END_GIVIING�� �̳��� �������̶�� ǥ�� ������
	{
		HSTMT		hStmt = NULL;
		RETCODE		retCode;
		::SQLAllocStmt(hDBC, &hStmt);
		retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
		if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
		}
		else
		{
			MyLog(0,"This Query Failed : %s", szQuery);		// �̻�Ȳ�� ���� ����
		}
		::SQLFreeStmt(hStmt, SQL_DROP);
	}
	// ����Ÿ ������
	pMRT->dwSellValue = pRecord->iSellValue;
	pMRT->iBuyerTake = pRecord->iBuyerTake;
	pMRT->iCn = pData->iCn;
	pMRT->iIndex = pRecord->iIndex;
	pMRT->iSellerTake = pRecord->iSellerTake;
	memcpy(&pMRT->SellItem, &pRecord->m_ResultItem, sizeof(ItemAttr));
	memcpy(pMRT->szBuyerName , pRecord->szBuyerName, sizeof(pMRT->szBuyerName));
	memcpy(pMRT->szMyName , pData->szMyName, sizeof(pMRT->szMyName));
	memcpy(pMRT->szSellerName , pRecord->szSellerName, sizeof(pMRT->szMyName));

	// �ʼ����� �����϶�� �˸���
	p.h.header.type = CMD_MERCHANT_BACKUP_TAKE_REQUEST;
	p.h.header.size	= sizeof(MERCHANTRESULTTAKE);		// BBD 040308
	::QueuePacket(connections,iCn,&p,1);
	
}

void CAuction::RecvCMD_MERCHANT_BACKUP_TAKE_RESPONSE(const int iCn,t_packet &p)
{
	MERCHANTRESULTTAKE *pMRT = &p.u.MerchantResultTake;

	char szQuery[255] ={0,};
	switch(pMRT->iKey)
	{
	case 101://�Ǹ��� ���� ����
		{
			sprintf(szQuery, "CALL MerchantBackupTakeUpdate( %d, 0, %d)", pMRT->iIndex, IS_END_DELETE_COMPLETE);
		}break;
	case 100://������ ���� ����
		{
			sprintf(szQuery, "CALL MerchantBackupTakeUpdate( %d, 1, %d)", pMRT->iIndex, IS_END_DELETE_COMPLETE);
		}break;
	case 11://�Ǹ��� ���� ����
		{
			sprintf(szQuery, "CALL MerchantBackupTakeUpdate( %d, 0, %d)", pMRT->iIndex, IS_END_ALL_RIGHT);
		}break;
	case 10://������ ���� ����
		{
			sprintf(szQuery, "CALL MerchantBackupTakeUpdate( %d, 1, %d)", pMRT->iIndex, IS_END_ALL_RIGHT);
		}break;
	default:
		{
		}break;
	}

	// ����Ǿ��� ������ �����ϰ�
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	::SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
	}
	else
	{
		MyLog(0,"This Query Failed : %s", szQuery);		// �̻�Ȳ�� ���� ����
	}
	::SQLFreeStmt(hStmt, SQL_DROP);

	if(pMRT->iKey >= 100)	// �̳���� �����ѳ�� ���ڵ�Ȯ���� �ʿ��ϴ�
	{
		// �� ���ڵ尡 ����� �ִ��� Ȯ���Ѵ�
		MerchantBackup_Check_and_Delete(pMRT->iIndex);
	}

	// Ŭ���̾�Ʈ���� ����Ʈ �ٽú��� ������
	SELLERITEMREQUEST data;
	data.iCn = pMRT->iCn;
	strcpy(data.szName, pMRT->szMyName);
	data.iIndex = 0;
	data.iKey = 1;
	p.u.kein.send_db_direct_map.server_id = pMRT->iCn;

	memcpy(&(p.u.kein.send_db_direct_map.data), &data, sizeof(SELLERITEMREQUEST));

	RecvCMD_MERCHANT_BACKUP_LIST_REQUEST(iCn, p);
}

void CAuction::MerchantBackup_Check_and_Delete(int iIndex)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	char szQuery[255] ={0,};

	sprintf(szQuery, "MerchantBackupRecordDelete %d", iIndex);
	::SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuery, SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
	}
	else
	{
		MyLog(0,"This Query Failed : %s", szQuery);		// �̻�Ȳ�� ���� ����
	}
	::SQLFreeStmt(hStmt, SQL_DROP);
}


//> BBD 040303 ������̺��� ������ ���� ��û
