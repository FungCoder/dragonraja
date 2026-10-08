// LottoSystem.cpp: implementation of the CLottoSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "LottoSystem.h"
#include "..\LowerLayers\servertable.h"
#include "CItem.h"
#include "UserManager.h"
#include <direct.h>

#define		ARENA_MAP_PORT	5570
#define		BROADCAST_TIME	1200000 //锟斤拷锟?锟矫帮拷锟斤拷 20锟斤拷 锟斤拷锟斤拷锟教达拷.
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLottoSystem* CLottoSystem::m_pClass = NULL;
CLottoSystem::CLottoSystem()
{
	Clear();
	srand(timeGetTime());
	m_pClass = this;

	m_nLottoNumberCount = 4;// 锟教帮拷锟斤拷 锟斤拷锟教猴拷锟斤拷锟斤拷 锟叫撅拷 锟铰达拷.
//	m_nGiveItemNumber = SADONIX_NO;//锟解夯锟斤拷 锟斤拷锟?锟斤拷锟叫斤拷锟斤拷 锟截达拷. 锟斤拷锟斤拷锟斤拷 锟斤拷锟教猴拷锟斤拷锟斤拷 锟叫撅拷麓锟?
	memset(m_anItemCount4Grade,0,sizeof(int)*5);//锟斤拷锟斤拷锟斤拷锟?锟斤拷锟睫登达拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷.
	memset(m_anWinNumberCount4Grade,0,sizeof(int)*5);//锟筋俺锟斤拷 锟斤拷龋锟斤拷 锟铰酒撅拷 锟斤拷锟斤拷伟锟?.
	memset(m_anGiveItemNumber,0,sizeof(int)*5);//锟斤拷锟斤拷锟斤拷锟?锟斤拷锟睫登达拷 锟斤拷锟斤拷锟斤拷 锟斤拷龋.
	m_nLottoPay = 10000; //锟斤拷锟斤拷飘锟斤拷 10000 农锟斤拷锟教达拷.//锟斤拷锟?锟斤拷锟斤拷 锟角撅拷 锟街达拷 锟斤拷锟斤拷 锟轿碉拷锟截撅拷 锟窖达拷.

	memset(&m_LotteryStart,0,sizeof(DATECHECK));
	memset(&m_LotteryEnd,0,sizeof(DATECHECK));

	mkdir("LotteryItem");
	m_bWinnerChecking = false;		// BBD 040127	锟轿讹拷 犬锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 锟矫凤拷锟斤拷
}

CLottoSystem::~CLottoSystem()
{
	Clear();
}

//锟斤拷锟斤拷 锟矫斤拷锟斤拷锟斤拷 锟捷肺硷拷锟斤拷 锟狡凤拷锟斤拷锟斤拷锟斤拷 锟窖癸拷锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷锟?锟窖达拷.
bool CLottoSystem::Create()
{
	if(m_pClass || ::g_pServerTable->GetOwnServerData()->wPort != ARENA_MAP_PORT )return false;
	new CLottoSystem;
	return true;
}

int CLottoSystem::CreateOneNumber()
{
	int nNumber = 0;
	while(1)
	{
		nNumber = RandomNumber();
		if(!IsMember(nNumber))break;
	}
	
	return nNumber;
}

int CLottoSystem::RandomNumber()
{
	int	nNumber = rand()%MAX_MAXIMUM_NUMBER + 1;
	return nNumber;
}

bool CLottoSystem::IsMember(int nNumber)
{
	for(int i = 0;i < 6;i++)
	{
		if(m_anNumbers[i] == nNumber) return true;
	}

	return false;
}

void CLottoSystem::Generate6Number()
{
	for(int i = 0;i < m_nLottoNumberCount;i++)
	{
		m_anNumbers[i] = CreateOneNumber();
	}

	Sort(m_anNumbers,m_nLottoNumberCount);

	m_bGenNumbers = true; // 锟斤拷龋锟斤拷 锟窖癸拷 锟斤拷锟斤拷锟斤拷锟?
}




int CLottoSystem::Check6Number(int an6Numbers[])
{
	int nCount = 0;
	for(int i = 0; i < m_nLottoNumberCount;i++)
	{
		if(IsMember(an6Numbers[i]))
			nCount++;
	}

	return nCount;
}


int CLottoSystem::SetLottoID(int nID)
{
	m_nRunID = nID; //1锟教伙拷锟斤拷 锟斤拷畎★拷锟?锟轿肚帮拷 锟斤拷锟斤拷前锟?锟街达拷锟斤拷锟教达拷.

	return m_nRunID;
}


void CLottoSystem::Sort(int aNumbers[], int size)
{
	if(size > 0)
	{
		int nMin = this->GetMinNumber(aNumbers,size);
		int i;
		for(i = 0;i < size;i++)
		{
			if(nMin == aNumbers[i]) break;			
		}

		int nTemp = aNumbers[i];
		aNumbers[i] = aNumbers[0];
		aNumbers[0] = nTemp;
		Sort(aNumbers + 1,size - 1);
	}
}

int CLottoSystem::GetMinNumber(int aNumbers[], int size)
{
	int nMin = 46;
	for(int i = 0;i < size;i++)
	{
		nMin = __min(aNumbers[i],nMin);
	}

	return nMin;
}

bool CLottoSystem::LoadTable(HDBC hDragonDB)
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	char		szQuerry[512];

	sprintf(szQuerry, "SELECT * FROM Lotto_Setting");
	SQLAllocStmt(hDragonDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	if (!SQLOK(retCode))
	{
		MyLog(0,"Lotto_Setting Table ....Loading Error!!!!");
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	retCode = SQLFetch(hStmt);
	if (!SQLOK(retCode))
	{
		MyLog(0,"Lotto_Setting Table ....No Data!!!!");
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLGetData(hStmt,1,SQL_C_LONG,&m_nLottoNumberCount,0,&cbValue);

	SQLGetData(hStmt,2,SQL_C_LONG,&m_anGiveItemNumber[0],0,&cbValue);
	SQLGetData(hStmt,3,SQL_C_LONG,&m_anGiveItemNumber[1],0,&cbValue);
	SQLGetData(hStmt,4,SQL_C_LONG,&m_anGiveItemNumber[2],0,&cbValue);
	SQLGetData(hStmt,5,SQL_C_LONG,&m_anGiveItemNumber[3],0,&cbValue);
	SQLGetData(hStmt,6,SQL_C_LONG,&m_anGiveItemNumber[4],0,&cbValue);

	SQLGetData(hStmt,7,SQL_C_LONG,&m_anItemCount4Grade[0],0,&cbValue);
	SQLGetData(hStmt,8,SQL_C_LONG,&m_anItemCount4Grade[1],0,&cbValue);
	SQLGetData(hStmt,9,SQL_C_LONG,&m_anItemCount4Grade[2],0,&cbValue);
	SQLGetData(hStmt,10,SQL_C_LONG,&m_anItemCount4Grade[3],0,&cbValue);
	SQLGetData(hStmt,11,SQL_C_LONG,&m_anItemCount4Grade[4],0,&cbValue);

	SQLGetData(hStmt,12,SQL_C_LONG,&m_anWinNumberCount4Grade[0],0,&cbValue);
	SQLGetData(hStmt,13,SQL_C_LONG,&m_anWinNumberCount4Grade[1],0,&cbValue);
	SQLGetData(hStmt,14,SQL_C_LONG,&m_anWinNumberCount4Grade[2],0,&cbValue);
	SQLGetData(hStmt,15,SQL_C_LONG,&m_anWinNumberCount4Grade[3],0,&cbValue);
	SQLGetData(hStmt,16,SQL_C_LONG,&m_anWinNumberCount4Grade[4],0,&cbValue);

	SQLGetData(hStmt,17,SQL_C_LONG,&m_nLottoPay,0,&cbValue);


	SQLFreeStmt(hStmt, SQL_DROP);


	

	sprintf(szQuerry, "SELECT * FROM Lottery_Date");
	SQLAllocStmt(hDragonDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	if (!SQLOK(retCode))
	{
		MyLog(0,"Lottery_Date Table ....Loading Error!!!!");
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}	
	retCode = SQLFetch(hStmt);
	if (!SQLOK(retCode))
	{
		MyLog(0,"Lottery_Date Table ....No Data!!!!");		
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	
	while(SQLOK(retCode))
	{
		SQLGetData(hStmt,2,SQL_C_LONG,&m_LotteryStart.nDay,0,&cbValue);
		SQLGetData(hStmt,3,SQL_C_LONG,&m_LotteryStart.nHour,0,&cbValue);
		SQLGetData(hStmt,4,SQL_C_LONG,&m_LotteryStart.nMin,0,&cbValue);
		SQLGetData(hStmt,5,SQL_C_LONG,&m_LotteryEnd.nDay,0,&cbValue);
		SQLGetData(hStmt,6,SQL_C_LONG,&m_LotteryEnd.nHour,0,&cbValue);
		SQLGetData(hStmt,7,SQL_C_LONG,&m_LotteryEnd.nMin,0,&cbValue);

		stLOTTERY_DATE	Lottery_date;
		Lottery_date.stLotteryStart = m_LotteryStart;
		Lottery_date.stLotteryEnd	= m_LotteryEnd;

		this->m_vtLottery_date.push_back(Lottery_date);
				
		retCode = SQLFetch(hStmt);
	}

	SQLFreeStmt(hStmt, SQL_DROP);



	sprintf(szQuerry, "SELECT * FROM Lotto_Event");
	SQLAllocStmt(hDragonDB, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)szQuerry, SQL_NTS);

	if (!SQLOK(retCode))
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}	
	
	retCode = SQLFetch(hStmt);

	LOTTO_EVENT_INFO	Lotto_Info;
	memset(&Lotto_Info,0,sizeof(Lotto_Info));

	int nLine = 1;

	if(!SQLOK(retCode))//锟斤拷锟教猴拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷.锟斤拷. 锟斤拷锟斤拷锟斤拷锟斤拷.
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		MyLog(0,"CLottoSystem::LoadTable() No data!!!!!.....Please Make One Data");
		return true;
	}

	while (SQLOK(retCode))
	{		
		memset(&m_Lotto_Info,0,sizeof(m_Lotto_Info));
		
		SQLGetData(hStmt, 1, SQL_C_LONG, &m_Lotto_Info.nLottoID, 0, &cbValue);//LottoID 锟斤拷锟斤拷锟斤拷 雀锟斤拷锟斤拷 锟轿讹拷 锟斤拷锟教碉拷 锟轿碉拷 锟饺达拷.
		SQLGetData(hStmt, 2, SQL_C_LONG, &m_Lotto_Info.StartDate.tm_year, 0, &cbValue);
		SQLGetData(hStmt, 3, SQL_C_LONG, &m_Lotto_Info.StartDate.tm_mon, 0, &cbValue);
		SQLGetData(hStmt, 4, SQL_C_LONG, &m_Lotto_Info.StartDate.tm_wday, 0, &cbValue);
		SQLGetData(hStmt, 5, SQL_C_LONG, &m_Lotto_Info.LotteryDate.tm_year, 0, &cbValue);
		SQLGetData(hStmt, 6, SQL_C_LONG, &m_Lotto_Info.LotteryDate.tm_mon, 0, &cbValue);
		SQLGetData(hStmt, 7, SQL_C_LONG, &m_Lotto_Info.LotteryDate.tm_wday, 0, &cbValue);
		SQLGetData(hStmt, 8, SQL_C_LONG, &m_Lotto_Info.nWinNumCount, 0, &cbValue);

		SQLGetData(hStmt, 9, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[0], 0, &cbValue);
		SQLGetData(hStmt, 10, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[1], 0, &cbValue);
		SQLGetData(hStmt, 11, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[2], 0, &cbValue);
		SQLGetData(hStmt, 12, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[3], 0, &cbValue);
		SQLGetData(hStmt, 13, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[4], 0, &cbValue);
		SQLGetData(hStmt, 14, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[5], 0, &cbValue);
		SQLGetData(hStmt, 15, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[6], 0, &cbValue);
		SQLGetData(hStmt, 16, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[7], 0, &cbValue);
		SQLGetData(hStmt, 17, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[8], 0, &cbValue);
		SQLGetData(hStmt, 18, SQL_C_LONG, &m_Lotto_Info.anWinNumbers[9], 0, &cbValue);
	
		retCode = SQLFetch(hStmt);		
	}
	
	
	SQLFreeStmt(hStmt, SQL_DROP);
	

	return true;	
}

void CLottoSystem::Clear()
{
	memset(&m_Lotto_Info,0,sizeof(m_Lotto_Info));
	memset(m_anNumbers,0,sizeof(int)*10);
	m_pClass = NULL;
	m_EventStatus = EVENT;
	m_bGenNumbers = false;
	m_dwBroadCastTime = 0;//soto-HK030924
}

//< soto_LottoADD
void CLottoSystem::RunProc()
{
	tm	CurDate = GetCurDate();

	if(m_EventStatus == LOTTERIED)//锟斤拷梅锟斤拷 锟饺扁埃锟教达拷.
	{
		if(timeGetTime() - m_dwBroadCastTime >= BROADCAST_TIME)
		{
			m_dwBroadCastTime = timeGetTime();
			BroadCastLotteryMsg();//锟睫硷拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷.
		}
		
		int nEventCount = this->m_vtLottery_date.size();
		for(int i = 0; i < nEventCount;++i)
		{
			DATECHECK LotteryEnd = this->m_vtLottery_date[i].stLotteryEnd;

			if(CurDate.tm_wday == LotteryEnd.nDay)//锟斤拷梅 锟斤拷锟斤拷 锟斤拷锟教讹拷 锟斤拷锟斤拷锟斤拷.
			{
				if(CurDate.tm_hour == LotteryEnd.nHour)//锟斤拷梅 锟斤拷锟斤拷 锟矫帮拷锟教讹拷 锟斤拷锟斤拷锟斤拷.
				{
					if(CurDate.tm_min == LotteryEnd.nMin)//锟斤拷 锟教猴拷飘锟斤拷 锟斤拷锟斤拷 锟角撅拷锟斤拷.
					{
						SendDBNewEvent();//锟斤拷 锟教猴拷飘锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷.
						break;
					}				
				}
			}
		}
	}
	else//锟斤拷锟斤拷 锟斤拷锟?锟解埃锟教达拷.
	{
		int nEventCount = this->m_vtLottery_date.size();
		
		for(int i = 0; i < nEventCount;++i)
		{
			DATECHECK LotteryStart = this->m_vtLottery_date[i].stLotteryStart;
			DATECHECK LotteryEnd = this->m_vtLottery_date[i].stLotteryEnd;		// BBD 040211 锟斤拷梅锟斤拷锟斤拷冒锟?
			if(CurDate.tm_wday == LotteryStart.nDay)//锟斤拷梅 锟斤拷锟斤拷锟斤拷锟教讹拷 锟斤拷锟斤拷锟斤拷.
			{
				if(CurDate.tm_hour >= LotteryStart.nHour &&
					CurDate.tm_hour <= LotteryEnd.nHour)	// BBD 040211 锟斤拷梅锟解埃 锟斤拷锟斤拷(锟矫帮拷) 锟教革拷
				{
					if(CurDate.tm_min >= LotteryStart.nMin &&
						CurDate.tm_min < LotteryEnd.nMin)	// BBD 040211 锟斤拷梅锟解埃 锟斤拷锟斤拷(锟斤拷) 锟教革拷
					{
						if(m_Lotto_Info.nLottoID)
						{
							SendDBLottery();//锟斤拷梅锟斤拷锟斤拷.
							break;
						}
					}
				}
			}
		}
	}
}

tm CLottoSystem::GetCurDate()
{
	tm	Time;
	time_t	LongTime;
	time(&LongTime);
	Time = *localtime(&LongTime);

	return Time;
}


void CLottoSystem::SendDBLottery()
{
	t_packet p; 
	p.h.header.type = CMD_LOTTERY_INFO;
	p.h.header.size = sizeof(LOTTO_EVENT_INFO);

	LOTTO_EVENT_INFO	Info;
	memcpy(&Info,&m_Lotto_Info,sizeof(LOTTO_EVENT_INFO));
	
	if(!m_bGenNumbers)Generate6Number(); //锟窖癸拷锟斤拷 锟斤拷锟斤拷锟?

	Info.nWinNumCount = m_nLottoNumberCount;
	memcpy(&Info.anWinNumbers,&m_anNumbers,sizeof(int)*this->m_nLottoNumberCount);
	Info.LotteryDate = GetCurDate();
			
	memcpy(&p.u.Lotto_Info,&Info,sizeof(LOTTO_EVENT_INFO));

	QueuePacket(connections,DB_DEMON,&p,1);
}

void CLottoSystem::SendDBNewEvent()
{
	t_packet	p;

	tm	Date = GetCurDate();


	Date.tm_year = Date.tm_year + 1900;//soto-LottoADD
		
	LOTTO_EVENT_INFO	Lotto_Info;memset(&Lotto_Info,0,sizeof(Lotto_Info));

	Lotto_Info.nLottoID = m_Lotto_Info.nLottoID + 1;
	memcpy(&Lotto_Info.StartDate,&Date,sizeof(tm));

	p.h.header.type = CMD_NEW_EVENT;
	p.h.header.size = sizeof(Lotto_Info);

	memcpy(&p.u.Lotto_Info,&Lotto_Info,sizeof(Lotto_Info));
	
	QueuePacket(connections,DB_DEMON,&p,1);

	MyLog(0,"Lotto : NewEvent Start!!!!  EventNumber : '%d' ",Lotto_Info.nLottoID);
}

void CLottoSystem::RecvDBNewEvent(LOTTO_EVENT_INFO* pInfo)
{
	m_EventStatus = EVENT;
	m_bGenNumbers = false;
	memcpy(&m_Lotto_Info,pInfo,sizeof(LOTTO_EVENT_INFO));
}

void CLottoSystem::RecvBuyLotto(t_BUY_LOTTO *pBuyLotto)
{
	int cn = ExistHe(pBuyLotto->strCharName);
		
	if(cn)
	{
		t_packet	p;
		p.h.header.type = CMD_LOTTO_BUY;
		p.h.header.size = sizeof(t_BUY_LOTTO);

		CHARLIST*	pChar = ::CheckServerId(cn);
		if(pChar)
		{
//			tm Date = GetCurDate();
			t_BUY_LOTTO	BuyLotto; memcpy(&BuyLotto,pBuyLotto,sizeof(BuyLotto));

			if(m_EventStatus == LOTTERIED)//锟斤拷梅 锟较凤拷 锟解埃锟教达拷.
			{
				memcpy(&p.u.Lotto_Buy,pBuyLotto,sizeof(t_BUY_LOTTO));
				p.u.Lotto_Buy.anLottoNumber[0] = -1;//锟斤拷龋锟斤拷 锟角撅拷锟斤拷 -1锟教革拷 
				QueuePacket(connections,cn,&p,1);
				return;
			}
			else
			{
				if(LocalMgr.IsAbleNation(TAIWAN | HONGKONG | CHINA))//锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 DBDEMON锟斤拷锟斤拷 锟斤拷没锟斤拷 锟窖达拷.
				{
					
					t_packet p2;
					p2.h.header.type = CMD_CAN_BUY;
					p2.h.header.size = sizeof(t_BUY_LOTTO);
					
					pBuyLotto->nLottoID = this->m_Lotto_Info.nLottoID;//soto-LottoADD

					memcpy(&p2.u.Lotto_Buy,pBuyLotto,sizeof(t_BUY_LOTTO));
					

					QueuePacket(connections,DB_DEMON,&p2,1);
				}
				else
				{
					if(pChar->GetBankMoney() < m_nLottoPay)//锟斤拷锟斤拷 锟斤拷锟节讹拷锟?
					{
						memcpy(&p.u.Lotto_Buy,pBuyLotto,sizeof(t_BUY_LOTTO));
						p.u.Lotto_Buy.anLottoNumber[0] = 0;//锟斤拷龋锟斤拷 锟角撅拷锟斤拷 0锟教革拷 锟斤拷锟斤拷 锟斤拷锟节讹拷锟?
						QueuePacket(connections,cn,&p,1);
					}
					else//锟斤拷锟?锟街达拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟角撅拷锟斤拷. 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟?锟竭猴拷 锟斤拷龋锟斤拷 锟街达拷锟斤拷 犬锟斤拷 锟截撅拷 锟窖达拷.
					{
						//锟斤拷锟解辑锟斤拷 DB_DEMON锟斤拷锟斤拷 犬锟斤拷 锟斤拷没锟斤拷 锟窖达拷.
						t_packet p2;
						p2.h.header.type = CMD_CAN_BUY;
						p2.h.header.size = sizeof(t_BUY_LOTTO);
						
						pBuyLotto->nLottoID = this->m_Lotto_Info.nLottoID;//soto-LottoADD

						memcpy(&p2.u.Lotto_Buy,pBuyLotto,sizeof(t_BUY_LOTTO));
						

						QueuePacket(connections,DB_DEMON,&p2,1);
					}
				}
			}
		}
	}	
}

void CLottoSystem::RecvCanBuyLotto(t_BUY_LOTTO *pCanBuyLotto)
{
	int cn = ExistHe(pCanBuyLotto->strCharName);
	
	if(cn)
	{
		CHARLIST *pChar = ::CheckServerId(cn);
		if(pChar)
		{
			t_packet	p;
			p.h.header.type = CMD_LOTTO_BUY;
			p.h.header.size = sizeof(t_BUY_LOTTO);
			
			memcpy(&p.u.Lotto_Buy,pCanBuyLotto,sizeof(t_BUY_LOTTO));

			if(p.u.Lotto_Buy.anLottoNumber[0] > 0)//锟斤拷锟?锟街达拷.//某锟斤拷锟酵匡拷DB_DEMON 锟斤拷锟斤拷 锟斤拷锟矫匡拷 锟斤拷锟斤拷锟斤拷.
			{
				p.u.Lotto_Buy.m_nLottoPay = m_nLottoPay;
				
				if(LocalMgr.IsAbleNation(KOREA | CHINA | THAI | HONGKONG | USA | JAPAN))//soto-LottoADD 锟诫父锟斤拷 锟斤拷锟斤拷飘锟斤拷 锟窖达拷.
				{									
					pChar->DecBankMoney(m_nLottoPay, BMCT_BUY_LOTTO); // CSD-030723
				}
				else
				{
					
				}

				QueuePacket(connections,cn,&p,1);
				QueuePacket(connections,DB_DEMON,&p,1);
			}
			else
			{
				QueuePacket(connections,cn,&p,1);//某锟斤拷锟酵匡拷锟皆革拷 锟斤拷锟斤拷锟斤拷.
			}
		}
	}
}

void CLottoSystem::RecvCheckWinner(int cn)
{

	if(m_EventStatus == LOTTERIED)
	{
		//<!	BBD 040127	锟轿讹拷 锟斤拷锟轿硷拷锟斤拷 锟斤拷锟斤拷
		if(m_bWinnerChecking)
		{
			// 锟轿讹拷 盲欧锟斤拷锟教达拷 锟斤拷迅锟斤拷锟斤拷 锟矫弊筹拷锟斤拷 锟斤拷锟斤拷锟斤拷
			t_packet	p;
			p.h.header.type = CMD_WINNER_CHECK;
			p.h.header.size = sizeof(t_CHECK_WINNER);

			memset(&p.u.Check_Winner,0,sizeof(t_CHECK_WINNER));

			p.u.Check_Winner.nWinItemCount = -10;	//	<--- 锟教筹拷锟斤拷 盲欧锟斤拷锟斤拷锟斤拷 钎锟斤拷锟窖达拷

			QueuePacket(connections,cn,&p,1);	//努锟斤拷锟教撅拷飘锟斤拷锟斤拷..盲欧锟斤拷锟教讹拷锟?锟睫硷拷锟斤拷.
			return;
		}
		//>		BBD 040127	锟轿讹拷 锟斤拷锟轿硷拷锟斤拷 锟斤拷锟斤拷
		
		t_packet	p;
		p.h.header.type = CMD_WINNER_CHECK;
		p.h.header.size = sizeof(t_CHECK_WINNER);

		memset(&p.u.Check_Winner,0,sizeof(t_CHECK_WINNER));

		p.u.Check_Winner.nLottoID = m_Lotto_Info.nLottoID;//soto-LottoADD
		strcpy(p.u.Check_Winner.strCharName,::CheckServerId(cn)->Name);
		memcpy(p.u.Check_Winner.anWinNumbers,m_Lotto_Info.anWinNumbers,sizeof(int)*10);
		

		QueuePacket(connections,DB_DEMON,&p,1);//锟斤拷锟襟俊达拷...犬锟斤拷 锟斤拷没锟斤拷 锟窖达拷.

		//<! BBD 040127	锟轿讹拷 锟斤拷锟斤拷 锟轿憋拷 锟斤拷锟斤拷
		m_bWinnerChecking = true;	// true锟较讹拷 锟斤拷锟斤拷锟斤拷锟教达拷
		MyLog(LOG_NORMAL, "Start Checking Lotto now UserID = %s",
			p.u.Check_Winner.strCharName);
		//> BBD 040127	锟轿讹拷 锟斤拷锟斤拷 锟轿憋拷 锟斤拷锟斤拷
	}
	else
	{
		t_packet	p;
		p.h.header.type = CMD_WINNER_CHECK;
		p.h.header.size = sizeof(t_CHECK_WINNER);

		memset(&p.u.Check_Winner,0,sizeof(t_CHECK_WINNER));

		strcpy(p.u.Check_Winner.strCharName,::CheckServerId(cn)->Name);
		memset(p.u.Check_Winner.anWinNumbers,0,sizeof(int)*10);
		p.u.Check_Winner.nWinItemCount = -3;

//		p.u.Check_Winner.anWinNumbers[0]
		QueuePacket(connections,cn,&p,1);//努锟斤拷锟教撅拷飘锟斤拷锟斤拷..锟斤拷梅 锟解埃锟斤拷 锟狡聪讹拷锟?锟睫硷拷锟斤拷.


	}
}

void CLottoSystem::RecvCheckOKWinner(t_CHECK_WINNER *pCheckOK)
{
	int cn = ::ExistHe(pCheckOK->strCharName);
	if(cn)
	{
		CHARLIST*	pChar = ::CheckServerId(cn);

		t_packet	p;
		p.h.header.type = CMD_WINNER_CHECK;
		p.h.header.size = sizeof(t_CHECK_WINNER);
		memcpy(&p.u.Check_Winner,pCheckOK,sizeof(t_CHECK_WINNER));

		if(pCheckOK->nWinItemCount > 0)//锟较达拷锟斤拷 锟斤拷梅 锟角撅拷锟斤拷锟?锟窖碉拷.
		{
		//某锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 芒锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟?犬锟斤拷锟斤拷锟绞夸啊 锟街达拷. 锟斤拷锟斤拷细锟? +锟斤拷 锟街撅拷 锟截达拷. 锟斤拷锟斤拷锟斤拷锟?锟斤拷锟斤拷锟斤拷 -1
			int nBlank = 0;
			for(int i = 0;i < 5;i++)
				for(int j = 0; j < 3;j++)
					for(int k = 0;k < 6;k++)
						if(!pChar->bank[i][j][k].item_no) nBlank++;
			
			int	nNeedBlank = pCheckOK->nWinItemCount;
			if(nBlank >= pCheckOK->nWinItemCount )
			{
				QueuePacket(connections,cn,&p,1);//锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟?
				//锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟?锟街撅拷锟?锟窖达拷.

				int nItemCounter = pCheckOK->nWinItemCount;
								
				//锟轿弊革拷 锟斤拷锟斤拷锟?
				for(int nGradeItemIndex = 0;nGradeItemIndex < 5;++nGradeItemIndex)
				{
					int nGradeItemCount = 0;
					ItemAttr	Item = ::GenerateItem(m_anGiveItemNumber[nGradeItemIndex]);
					if(nGradeItemCount = pCheckOK->anWinItemsCount[nGradeItemIndex])
					{
						for(int i = 0;i < 5;i++)
						{
							for(int j = 0; j < 3;j++)
							{
								for(int k = 0;k < 6;k++)
								{
									if(!pChar->bank[i][j][k].item_no)
									{
										if(nGradeItemCount > 0)
										{
											pChar->bank[i][j][k] = Item;
											nGradeItemCount--;
										}
									}
								}
							}
						}
						FILE*	fp = NULL;
						char szFileName[512] = {0,};
						sprintf(szFileName,"./LotteryItem/_%03d_ItemGive.txt",this->m_Lotto_Info.nLottoID);
						fp = fopen(szFileName,"at+");

						GetCurDate();

						if(fp != NULL)
						{
						fprintf(fp ,"Lotto 锟斤拷梅 : %s 锟皆匡拷锟斤拷 %d锟斤拷锟斤拷 %d锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟角撅拷锟斤拷锟较达拷.\n"
								,pChar->Name,pCheckOK->anWinItemsCount[nGradeItemIndex]
								,nGradeItemIndex+1);
						fclose(fp);
						}
						else
						{

						}
					}
				}

				p.h.header.type = CMD_DEL_LOTTO_USER;//锟斤拷锟襟俊达拷 锟斤拷锟斤拷 锟斤拷没锟斤拷 锟截撅拷 锟窖达拷.
				QueuePacket(connections,DB_DEMON,&p,1);
				
			}
			else
			{
				pCheckOK->nWinItemCount = -1;
				pCheckOK->anWinNumbers[0] = nNeedBlank;//锟绞匡拷锟斤拷 锟斤拷锟斤拷锟斤拷锟?锟斤拷.

				memcpy(&p.u.Check_Winner,pCheckOK,sizeof(t_CHECK_WINNER));	// BBD 040127	锟睫革拷 墨锟角帮拷 锟绞匡拷锟较达拷
				QueuePacket(connections,cn,&p,1);//锟斤拷锟斤拷 锟斤拷龋锟斤拷 锟斤拷锟斤拷锟斤拷.

				//<! BBD 040127	锟轿讹拷 锟斤拷锟斤拷锟斤拷锟斤拷 锟轿憋拷 锟斤拷锟斤拷
				m_bWinnerChecking = false;			//	锟轿讹拷 犬锟斤拷 锟斤拷锟斤拷锟较帮拷 钱锟斤拷锟斤拷锟斤拷
				MyLog(LOG_NORMAL, "End Checking Lotto now");
				//> BBD 040127	锟轿讹拷 锟斤拷锟斤拷锟斤拷锟斤拷 锟轿憋拷 锟斤拷锟斤拷
			}
		
		}
		else if(pCheckOK->nWinItemCount == 0)//锟斤拷锟教达拷.
		{
			QueuePacket(connections,cn,&p,1);//努锟斤拷锟教撅拷飘锟斤拷锟皆达拷 锟斤拷锟铰达拷.
			
			p.h.header.type = CMD_DEL_LOTTO_USER;//锟斤拷锟襟俊达拷 锟斤拷锟斤拷 锟斤拷没锟斤拷 锟截撅拷 锟窖达拷.
			QueuePacket(connections,DB_DEMON,&p,1);
		}		
	}
}

void CLottoSystem::RecvLottoInfo(LOTTO_EVENT_INFO *pLottoInfo)
{
	m_EventStatus = LOTTERIED;
	memcpy(&m_Lotto_Info,pLottoInfo,sizeof(LOTTO_EVENT_INFO));
}

void CLottoSystem::RecvOpenWinnerMenu(int cn)
{
	t_packet p;
	p.h.header.type = CMD_CHECK_WINNER_MENU;
	p.h.header.size = sizeof(t_WINNER_MENU);
	memset(&p.u.Lotto_Winner_Menu,0,sizeof(t_WINNER_MENU));
	p.u.Lotto_Winner_Menu.nLottoID = m_Lotto_Info.nLottoID;//soto-LottoADD
	strcpy(p.u.Lotto_Winner_Menu.strCharName,::CheckServerId(cn)->Name);
	p.u.Lotto_Winner_Menu.nMaxLottoCount = this->m_nLottoNumberCount;

	QueuePacket(connections,DB_DEMON,&p,1);
}

void CLottoSystem::RecvCheckOpenWinnerMenu(t_WINNER_MENU *pWinnerMenu)
{
	int cn = ::ExistHe(pWinnerMenu->strCharName);
	if(cn)
	{
		t_packet p;
		p.h.header.type = CMD_OPEN_WINNER_MENU;
		p.h.header.size = sizeof(t_WINNER_MENU);

		memcpy(&p.u.Lotto_Winner_Menu,pWinnerMenu,sizeof(t_WINNER_MENU));

		if(m_EventStatus == EVENT)
		{
			memset(p.u.Lotto_Winner_Menu.anWinNumbers,0,sizeof(int) * 10);
		}


		QueuePacket(connections,cn,&p,1);
	}
}

void CLottoSystem::RecvOpenLottoMenu(int cn)
{
	t_packet p;
	p.h.header.type = CMD_OPEN_LOTTOMENU;
	p.h.header.size = sizeof(t_OPEN_LOTTO_MENU);
	p.u.Lotto_Menu_Open.nMaxNumberCount = this->m_nLottoNumberCount;

	QueuePacket(connections,cn,&p,1);
}

void CLottoSystem::BroadCastLotteryMsg()
{
	t_packet p;
	p.h.header.type = CMD_LOTTERY_BROADCAST;
	p.h.header.size = sizeof(t_LOTTERY_INFO);
	p.u.Lotto_BroadCast.nLottoID = this->m_Lotto_Info.nLottoID;
	p.u.Lotto_BroadCast.nWinNumCount = this->m_Lotto_Info.nWinNumCount;
	memcpy(p.u.Lotto_BroadCast.anWinNumbers,this->m_Lotto_Info.anWinNumbers,sizeof(int)*10);

	::SendPacket2Maps(&p);
	g_pUserManager->SendPacket(&p); // CSD-CN-031213
}

int CLottoSystem::GetLottoID()
{
	return m_Lotto_Info.nLottoID;
}

int CLottoSystem::GetNextLotteryIndex()
{
	return -1;		
}

int CLottoSystem::FindSameDayIndex(int nDay)
{
	return -1;
}
//<! BBD 040127	锟轿讹拷 锟斤拷锟轿硷拷锟斤拷 锟斤拷锟斤拷
void CLottoSystem::CheckIsDelOk(bool bIsOk)
{
	// 锟斤拷锟?犬锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷锟角凤拷 锟劫革拷 努锟斤拷锟教撅拷飘锟斤拷 犬锟斤拷锟斤拷 锟斤拷锟斤拷汛锟?
	if(bIsOk && m_bWinnerChecking)
	{
		//<! BBD 040127	锟轿讹拷 锟斤拷锟斤拷锟斤拷锟斤拷 锟轿憋拷 锟斤拷锟斤拷
		m_bWinnerChecking = false;
		MyLog(LOG_NORMAL, "End Checking Lotto now");
		//> BBD 040127	锟轿讹拷 锟斤拷锟斤拷锟斤拷锟斤拷 锟轿憋拷 锟斤拷锟斤拷
	}
}
//> BBD 040127	锟轿讹拷 锟斤拷锟轿硷拷锟斤拷 锟斤拷锟斤拷
