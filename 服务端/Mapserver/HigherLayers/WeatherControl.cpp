// WheatherControl.cpp: implementation of the CWeatherControl class.
//
//////////////////////////////////////////////////////////////////////
#include "..\stdafx.h"
#include "WeatherControl.h"
#include "WeatherTableReader.h"
#include "DefaultHeader.h"

#include "CItem.h"
#include "UserManager.h"
	
int RainTable[12][31];
short Temprature[12][31][2];


DWORD today_rainstart[5];
DWORD today_rainend[5];
int   today_rainamount[5];
int   today_temperature;
int   today_weathercount;

bool g_X_MAS;	

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CWeatherControl WeatherControl;

CWeatherControl::CWeatherControl()
{
	dwCheckedTime = timeGetTime();
}

CWeatherControl::~CWeatherControl()
{

}

void CWeatherControl::AutoSetDayLight(CHARLIST *ch)
{
	short int nTempDaylight = nDayLightControl;//nTempDaylight 悸泼瞪 搬苞拱
	if(ch->dwDayLightControlCustomTime  )
	{
		nTempDaylight = nDayLightControl;
		if(ch->dwDayLightControlCustomTime < global_time)//目胶乓 矫埃捞 乐促 弊矾唱 矫埃捞 瘤车促
		{
			ch->dwDayLightControlCustomTime = 0;
		}
		else
		{
			nTempDaylight = nDayLightControl+ ch->nDayLightControlCustom;
			if(nTempDaylight > nMaximumLight)
			{
				nTempDaylight = nMaximumLight;
			}
		}
	}

	if(ch->nDayLightControl != nTempDaylight )
	{
		t_packet packet;
		packet.h.header.type = CMD_DAYLIGHT;
		packet.h.header.size = sizeof(t_DayLight);

		ch->nDayLightControl = nTempDaylight;

		packet.u.DayLight.nDayLightControl	= nTempDaylight;//011215 lsw 
		QueuePacket(connections, ch->GetServerID(), &packet, 1);
	}
}

void CWeatherControl::CheckDayLight()
{
	if( timeGetTime()  <=  dwCheckedTime )//矫埃 眉农
	{
		return;
	}
//	dwCheckedTime = timeGetTime()+ (60000 * 10 );//10盒
	dwCheckedTime = timeGetTime()+ (10 );//10盒

	switch(MapInfo[MapNumber].daylightcontrol)
	{
	case 1://2: 3: 1//货寒(0,1) , 撤(2,3,4), 广(5)
		{
			const int iGameHour = g_hour%6;
			short int nValue = nMinimumLight;
			switch(iGameHour)
			{
			case 0:
				{
					nValue = 15;
				}break;
			case 1:
				{
					nValue = 20;
				}break;
			case 2:
				{
					nValue = 25;
				}break;
			case 3:
				{
					nValue = nMaximumLight;
				}break;
			case 4:
				{
					nValue = 25;
					if(g_min >30)//30盒捞搁 扼捞飘啊 5 皑家
					{
						nValue-=5;
					}
				}break;
			case 5:
				{
					nValue = nMinimumLight;
				}break;			
			default:
				{
					nValue = nMinimumLight;
				}break;
			}

			SetDayLight(nValue);
/*			
			int min = (g_hour/4) * 60 + g_min;
			if( min >= 360 && min <= 360 + (nMaximumLight-nMinimumLight) )	
			{
				SetDayLight(min + nMinimumLight - 360);
			}
			else if( min > 360 + (nMaximumLight-nMinimumLight)  && min < 18 * 60 )	
			{
				SetDayLight(nMaximumLight);
			}
			else if( min >= 18 * 60 && min < 18*60 + (nMaximumLight-nMinimumLight))	
			{
				SetDayLight(18*60 + nMaximumLight - min);
			}
			else 
			{
				SetDayLight(nMinimumLight);
			}
*/
		}break;
	case 2:
		{
			SetDayLight(nMaximumLight);
		}break;
	case 0:
	default:
		{
			SetDayLight(nMinimumLight);
		}break;
	}
}

void CWeatherControl::SetChLight(CHARLIST *ch,const int iLight,const int iTimeSec )
{
	if(!ch)	{ return; }
	ch->dwDayLightControlCustomTime = global_time +iTimeSec*1000;//*1000;//5盒
	ch->nDayLightControlCustom = nDayLightControl + iLight;
	ch->nDayLightControl = nMinimumLight-1;
}

void CWeatherControl::SetDayLight(const short int nValue)
{
	nDayLightControl = nValue;
}

int LoadWeatherTable( void )
{
    int rain[12][31][1] = {};
    short temperature[12][31][2] = {};
    char path[MAX_PATH];
    const auto load = [&](const char* name, auto& table, size_t rows) {
        if (sprintf_s(path, "%s/data/%s", GameServerDataPath, name) < 0) return false;
        FILE* file = fopen(path, "rt");
        if (!file) { MyLog(LOG_NORMAL, "Weather resource missing: %s", name); return false; }
        const bool valid = ReadLegacyWeatherTable(file, table, rows);
        fclose(file);
        if (!valid) MyLog(LOG_NORMAL, "Invalid weather resource: %s", name);
        return valid;
    };
    // Keep existing global data intact if either resource is malformed.
    if (!load("rain.tbl", rain, 30) || !load("Temprature.tbl", temperature, 31)) return 0;
    for (int month = 0; month < 12; ++month)
        for (int day = 0; day < 31; ++day) RainTable[month][day] = rain[month][day][0];
    memcpy(Temprature, temperature, sizeof(Temprature));
    return 1;
}
		
void CurrentGameDate( const DWORD t, int *y, int *mo, int *d, int *h, int *mi, int *sec )
{		
	DWORD rest;
		
	*y		= t / 31536000;
	rest	= t % 31536000;
		
	*mo		= rest / 2592000;
	rest	= rest % 2592000;
		
	*d		= rest / 86400;
	rest	= rest % 86400;
		
	*h		= rest / 3600;
	rest	= rest % 3600;
		
	*mi		= rest / 60;
	*sec	= rest % 60;
}		

void CheckWeatherSystem( void )
{		
	static DWORD time = g_curr_time;
		
	if( g_curr_time - time > 600 )
	{	
		time = g_curr_time; // 茄 霸烙矫埃 Check茄促. 
	}	
	else return;
			
	static int mon, day, rain_amount;
	int y, mo, d, h, mi, sec;
	int i;	
			
	CurrentGameDate( g_curr_time * 6, &y, &mo, &d, &h, &mi, &sec );
	if (mo < 0 || mo >= 12 || d < 0 || d >= 31) return;
			
	static int to;
	//	父距俊 农府胶付胶捞搁..
	g_X_MAS = false;
	if( ( g_mon == 11 && g_day >=1 ) || (g_mon==0 && g_day <= 30 ) )		// 021128 YGI
	{
		g_X_MAS = true;
	}

	if( g_X_MAS && MapInfo[ MapNumber].rain )		// 传郴妨扼..
	{		
		to ++;
		if( to < 10 ) 
		{	
			today_rainstart[0]  =  time;
			today_rainend[0]    =  time + 1200;
			today_rainamount[0] =  600;
			today_weathercount  =  0;

			/*int xx, yy;
			for( i = 0 ; i < 10 ; i ++)
			{		
				int xx, yy;
				xx = rand()%g_Map.file.wWidth;
				yy = rand()%g_Map.file.wHeight;
				if( !TileMap[ xx][ yy].attr_inside && !TileMap[ xx][ yy].attr_dont )
				{	
					xx *= TILE_SIZE;
					yy *= TILE_SIZE;
					ItemAttr item;
					item = GenerateItem( 7, 44, (rand()%6)+10, 0, 100 );	
					AddItemList( item.item_no, item.attr,0,  xx+rand()%TILE_SIZE, yy+rand()%TILE_SIZE, 0,0 );
				}	
			}*/		
		}			
		else		
		{			
			today_rainstart[0]  =  0;
			today_rainend[0]    =  0;
			today_rainamount[0] =  0;
			today_weathercount  =  0;

			if( to > 13 ) to = 0;
		}
		
		return;
	}	
		
	if( mon == mo && day == d ) 
	{	
		today_weathercount --;
		if( today_weathercount < 0 ) return;
	}	
	else
	{	
		mon			= mo, day = d;	
		today_weathercount		= (rand()%5) +1;	// 哥割锅栏肺 唱穿绢 谎副巴牢啊?
		rain_amount = RainTable[mon][day] / today_weathercount;	// 坷疵 郴妨具且 剧俊辑 割刚栏肺 唱穿绢 谎副瘤 剧阑 拌魂茄促. 
		today_temperature	= (Temprature[mon][day][0] + Temprature[mon][day][1])/2;	// 坷疵狼 扁柯篮...
		
		for( i = 0 ; i < 5 ; i ++)
		{	
			today_rainstart[i]	= 0;
			today_rainend[i]	= 0;
			today_rainamount[i] = 0;
		}
		
		if( rain_amount )
		for( i = 0 ; i < today_weathercount ; i ++)
		{
			today_rainstart[i]  =  (3600 * 24 / time) * i + ( rand()% (3600 * 24 / time - 3600) );
			today_rainend[i]    = today_rainstart[i] + 60 * (rand()%10 + 20 );
			today_rainamount[i] = rain_amount;
		}
		today_weathercount--;
	}	
}

void SendWeatherRoutine(t_connection c[])
{	//< CSD-CN-031213
	static DWORD weathertime;

	if (today_weathercount < 0) 
	{
		today_weathercount = 1;
	}	

	if (global_time - weathertime > 10000)
	{
		weathertime = global_time;
		
		t_packet packet;
		packet.h.header.type = CMD_WEATHER;
		packet.h.header.size = sizeof(t_server_weather);
		packet.u.server_weather.weather = weathertime;
		packet.u.server_weather.another = g_curr_time;
		packet.u.server_weather.rainstart = today_rainstart[today_weathercount];
		packet.u.server_weather.rainend = today_rainend[today_weathercount];
		packet.u.server_weather.amount = today_rainamount[today_weathercount];
		packet.u.server_weather.temperature = getWeatherCorrection();
		g_pUserManager->SendPacket(&packet);
	}
}	//> CSD-CN-031213

short int getWeatherCorrection( void )
{//021230 lsw
	short int time = 0;
	time = (g_mon+1) <<10;
	time |= (g_day) <<5;
	time |= g_hour+1;

	return time;
}
