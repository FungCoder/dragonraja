
#include "..\stdafx.h"
#include <mmsystem.h>
#include "DefaultHeader.h"
#include "Path.h"

#include "NPC_Pattern.h"
#include "CItem.h"
#include "OP_Magic.h"
#include "dr_network.h"
#include "RajaSystem.h"
#include "Map.h"
#include "SealStoneGameServer.h" 
#include "AIDefine.h"
#include "LTSSupport.h"			// LTS NEW LOCALWAR
#include "AIManager.h"
#include "UserManager.h"
#include "AdventManager.h"
#include "ArenaManager.h"
#include "RegenManager.h"
#include "GameplaySafety.h"

#define	HOME_DIST_1AREA					245
#define HOME_DIST_2AREA					625

extern struct				CItem_List Item_Ref ;
extern cLocalWarfield*		g_pLocalWarfield;			// LTS NEW LOCALWAR

extern void SendAttackNPCArea( t_connection c[], int npc );
extern void SendViewTypeChange( int cn, int viewtype );

extern inline int IsNKNation( LPCHARLIST ch );
extern inline int	IsHeNK( LPCHARLIST ch, int nation );
extern void CalcResist(int tolerance, CHARLIST* target, int resist_Type);
extern int	CheckActivedGroup();						// LTS AI2
extern int CheckEventAttack();	 // CSD-031007

extern NPCName_by_Gender	NPC_Name_Ref[Num_Of_NPC_Name];
extern NPCLev_to_Exp		NPC_Lev_Ref[Num_Of_NPC_Lev] ;
extern NPC_Generation		NPC_Gen_Ref[Num_Of_NPC_Generation] ;


extern int NPC_FemaleName_Count;
extern int NPC_MaleName_Count; 
extern int NPC_LastName_Count; 

int NpcPatternTable[20] = 
{ 
	NPC_PATTERN_HOSTILE_,
	NPC_PATTERN_FIND_CLOSE_PC_,
	NPC_PATTERN_ACCESS_PC_,
	NPC_PATTERN_ATTACK_PC_,
	NPC_PATTERN_BACKDRAW_,
	NPC_PATTERN_RETURN_,		
};



#define	MAX_ANIMATION		   24
#define	MAX_PC_ANIMATION_FRAME 40

typedef struct	tagPCANIMATIONTABLE
{	
	int		nCount;
	BYTE	cFrame[ MAX_PC_ANIMATION_FRAME ];
	
	char  	dx[ MAX_PC_ANIMATION_FRAME ];
	char  	dy[ MAX_PC_ANIMATION_FRAME ];
	char    attackf[ MAX_PC_ANIMATION_FRAME ];
	short   wave[  MAX_PC_ANIMATION_FRAME ];
	short 	height[ MAX_PC_ANIMATION_FRAME ];
	short   effect[ MAX_PC_ANIMATION_FRAME ];
} PCANIMATIONTABLE, *LPPCANIMATIONTABLE;

PCANIMATIONTABLE AnimationTable[ MAX_ANIMATION ];

#define MAX_CHARACTER_SPRITE_ 200 // CSD-030417

int WalkDelayTimeTable[ MAX_CHARACTER_SPRITE_];
int RunDelayTimeTable[ MAX_CHARACTER_SPRITE_];
int HorseDelayTimeTable[ MAX_CHARACTER_SPRITE_];
int movep;

sNPCgeneratePosition NPCgeneratePosition[ MAX_GENERATE_POSITION_];
int NPCgeneratePositionMax;


bool CharMovingThreadStart0;
bool CharMovingThreadStart1;
bool CharMovingThreadStart2;

inline int ran( int r )
{	
	if( r == 0 ) return 0;
	return rand()%r;
}	

#define FRAME_PER_SEC		15

//	府畔 : 0捞 酒囱 蔼捞搁 档馒抗惑矫埃.
DWORD ReturnDestTime( LPCHARLIST ch )
{	
	int dist = ch->MoveLength - ch->MovePathCount;
	if( dist <= 0 ) {return 0;}
	
	const DWORD timepertile = ::Get_MoveDelayTime( ch );
	const DWORD desttime = global_time + timepertile * (DWORD )dist;
	
	ch->desttime = desttime;
	
	return desttime;
}	

void SaveMoveDelayTable()
{	//< CSD-030806 : 1鸥老 捞悼窍绰单 吧府绰 矫埃阑 绵仿
	FILE* pFile = fopen("./OutPut/Monster_WalkingTime_mSecPerTile.txt", "wt");
	Dbg_Assert(pFile != NULL);

	for (int i = 0; i < MAX_CHARACTER_SPRITE_; ++i)
	{
		fprintf(pFile, "[%03d]%s(%d msec/tile)\n", i, NPC_Gen_Ref[i].Name, WalkDelayTimeTable[i]);
	}
		
	fclose(pFile);
}	//> CSD-030806

void InitMoveDelayTable( void )
{	
	int i,j;
	double d = 0.;
	double framepertile;
	char temp[ FILENAME_MAX];
	FILE *fp;
	
	for( i = 0 ; i < MAX_CHARACTER_SPRITE_ ; i ++)
	{
		sprintf( temp, "%s/Char/%03dAniTable.bin", GameServerDataPath, i );
		fp = fopen( temp, "rb" );
		if( fp )
		{
			fseek( fp, 20, SEEK_SET );
			fseek( fp,  1, SEEK_CUR );
			fread( AnimationTable, sizeof( PCANIMATIONTABLE ), MAX_ANIMATION, fp );
			fclose(fp);
			
			for( d = 0, j = 0 ; j < AnimationTable[1].nCount ; j ++)
				d += (double)AnimationTable[1].dx[j];
			if( d )
			{
				framepertile = TILE_SIZE * (double)AnimationTable[1].nCount / d ;
				WalkDelayTimeTable[ i] = (int)(1000 * framepertile / FRAME_PER_SEC);
			}
			
			if( i == 0 || i == 1 ) 
			{
				for( d = 0, j = 0 ; j < AnimationTable[12].nCount ; j ++) // 崔府扁..
					d += (double)AnimationTable[12].dx[j];
				if( d )
				{
					framepertile = TILE_SIZE * (double)AnimationTable[12].nCount / d ;
					RunDelayTimeTable[ i] = (int)(1000 * framepertile / FRAME_PER_SEC);
				}
				
				
				for( d = 0, j = 0 ; j < AnimationTable[19].nCount ; j ++) // 富鸥扁....
					d += (double)AnimationTable[19].dx[j];
				if( d )
				{
					framepertile = TILE_SIZE * (double)AnimationTable[19].nCount / d ;
					HorseDelayTimeTable[ i] = (int)(1000 * framepertile / FRAME_PER_SEC);
				}
			}
		}
		else 
		{
			WalkDelayTimeTable[ i]	= 700;
			RunDelayTimeTable[ i]	= 300;
			HorseDelayTimeTable[i]	= 300;
		}
	}
}	

DWORD Get_MoveDelayTime( CHARLIST *ch )
{
	const int movp= ch->CalcMovePower();
	if(!movp){return 0;}
	
	DWORD ret = 0;
	switch( ch->SprNo )
	{
	case 0 :
	case 1 : 
		{
			switch( ch->MoveType )
			{
				// walking
			case 0 :	
				{
					if( ch->MoveLength <=4 ) 
					{
						ret = WalkDelayTimeTable[ ch->SprNo] * 50 / movp;
					}
					else 
					{
						ret = RunDelayTimeTable[ ch->SprNo] * 50 / movp;
					}
				}break;
				// running
			case 1 : 	
				{
					ret = RunDelayTimeTable[ ch->SprNo] * 50 / movp;
				}break;
				// Horsing..
			case 2 :	
				{
					ret = HorseDelayTimeTable[ ch->SprNo] * 50 / movp;
				}break;
			default :
				{
					ret = 500;
				}break;
			}
		}break;
	default :
		{
			ret = WalkDelayTimeTable[ ch->SprNo] * 50 / movp;
		}break;
	}
	return ret; 
}

int InDistance( CHARLIST *s, CHARLIST *d, int dis )
{	
	if( s->bAlive != ALIVE_ ) return 0;
	
	int sx = s->X;
	int sy = s->Y;
	int dx = d->X;
	int dy = d->Y;
	
	if( dis * dis > (dx-sx)*(dx-sx)+(dy-sy)*(dy-sy) ) return 1;
	
	return 0;
}	

bool NPC_CheckMustGoHome(CHARLIST *n,int tx,int ty)
{
	DWORD area1, area2;
	
	switch( n->patterntype )
	{
	case  NPC_PATTERN_ACCESS			:
	case  NPC_PATTERN_ATTACK			:
	case  NPC_PATTERN_ATTACKED			:
	case  NPC_PATTERN_BOSS_ACCESS		:
	case  NPC_PATTERN_BOSS_ATTACK		:
	case  NPC_PATTERN_BOSS_ATTACKED		:
		{
			area1 = (n->X/TILE_SIZE - n->homex)*(n->X/TILE_SIZE - n->homex) + (n->Y/TILE_SIZE - n->homey)*(n->Y/TILE_SIZE - n->homey);
			area2 = (tx - n->homex)*(tx - n->homex) + (ty - n->homey)*(ty - n->homey);
			if( area1 > HOME_DIST_1AREA )
			{	
				if( area2 > HOME_DIST_2AREA )
				{
					return true;
				}
			}	
			break;
		}
	}
	return false;
	
}

bool NPC_CanReach(CHARLIST *n,int tx,int ty,int *x,int *y)
{
	int d, i, j, tempdir;
	int ttx, tty;
	int dirt[ 5] = { 0,1,-1,-2,2 };
	
	*x = n->X / TILE_SIZE;  *y = n->Y / TILE_SIZE;
	tempdir = NPC_GetDir( tx, ty, n->X / TILE_SIZE, n->Y / TILE_SIZE );
	
	for( j = 1 ; j < 10 ; j ++)
	{		
		for( i = 0 ;  i < 5 ; i ++)
		{	
			ttx = tx;	tty = ty;
			d = ( tempdir + 8 + dirt[i] ) %8;
			switch( d )
			{
			case 0 :			tty +=j; break;
			case 1 : ttx -=j;	tty +=j; break;
			case 2 : ttx -=j;			 break;
			case 3 : ttx -=j;	tty -=j; break;
			case 4 :			tty -=j; break;
			case 5 : ttx +=j;	tty -=j; break;
			case 6 : ttx +=j;			 break;
			case 7 : ttx +=j;	tty +=j; break;
			}
			if(FreeTile( n, n->MoveSx, n->MoveSy, ttx, tty ))  
			{
				*x = ttx;  *y = tty;
				return true;
			}
		}	
	}	
	return false;
}

int NPC_NearCh( CHARLIST *n, CHARLIST *ch, int *x, int *y )			// LTS NPC MOVE
{
	int dirt[ 5] = { 0,1,-1,-2,2 };
	int tx = 0, ty  = 0;
	
	if( ch == NULL ){return 0;}
	
	tx = ch->MoveSx;
	ty = ch->MoveSy;
	
	if (ch->SprNo==0||ch->SprNo==1)
	{
		tx = ch->MoveGox/TILE_SIZE;
		ty = ch->MoveGoy/TILE_SIZE;
	}
	
	if (NPC_CheckMustGoHome(n,tx,ty))
	{
		if (!NPC_CanReach(n,n->homex,n->homey,x,y))
		{
			if (!NPC_CanReach(n,tx,ty,x,y))
			{
				return false;
			}
		}
	}
	else
	{
		if (!NPC_CanReach(n,tx,ty,x,y))
		{
			return false;
		}	
	}
	return true;
}

#define TILE_BLOCK_SIZE  11

BYTE TileBlock[ TILE_BLOCK_SIZE][ TILE_BLOCK_SIZE] = 
{
	{ 0,0,0,0,1,1,1,0,0,0,0 },
	{ 0,0,1,1,1,1,1,1,1,0,0 },
	{ 0,1,1,1,1,1,1,1,1,1,0 },
	{ 0,1,1,1,1,1,1,1,1,1,0 },
	{ 1,1,1,1,1,1,1,1,1,1,1 },
	{ 1,1,1,1,1,1,1,1,1,1,1 },
	{ 1,1,1,1,1,1,1,1,1,1,1 },
	{ 0,1,1,1,1,1,1,1,1,1,0 },
	{ 0,1,1,1,1,1,1,1,1,1,0 },
	{ 0,0,1,1,1,1,1,1,1,0,0 },
	{ 0,0,0,0,1,1,1,0,0,0,0 }
};

BYTE TileBlockBuf[ TILE_BLOCK_SIZE][ TILE_BLOCK_SIZE];

// 泅犁困摹俊辑 dist鸥老父怒  第肺 狐柳促. 
int NPC_NearBackCh( CHARLIST *n, CHARLIST *ch, int *x, int *y, int dist )
{	
	int dirt[ 5] = { 0,1,-1,-2,2 };
	int d, i, j, tempdir;
	int tx, ty;
	int ttx, tty;
	
	if( n->targetid == -1 ) 
	{
		//		n->targetid = NPC_IsWhoNearPC( n, 10  );
		//		if( n->targetid == -1 )  
		return 0;
	}
	
	// 傍拜 措惑捞 绝绢..
	if( !connections[n->targetid].dwAgentConnectionIndex || connections[n->targetid].state < CONNECT_JOIN ) return 0;
	if( connections[n->targetid].chrlst.bAlive >= DEAD_ ) return 0;
	
	tx = connections[ n->targetid].chrlst.MoveSx;
	ty = connections[ n->targetid].chrlst.MoveSy;
	*x = n->MoveSx;  *y = n->MoveSy;
	tempdir = NPC_GetDir( tx, ty, n->MoveSx, n->MoveSy );
	
	switch( tempdir )
	{
	case 0 :			ty +=dist;	break;
	case 1 : tx -=dist;	ty +=dist;	break;
	case 2 : tx -=dist;				break;
	case 3 : tx -=dist;	ty -=dist;	break;
	case 4 :			ty -=dist;	break;
	case 5 : tx +=dist;	ty -=dist;	break;
	case 6 : tx +=dist;				break;
	case 7 : tx +=dist;	ty +=dist;	break;
	}
	
	for( j = 0 ; j < 10 ; j ++)
	{
		for( i = 0 ;  i < 5 ; i ++)
		{
			ttx = tx;	tty = ty;
			d = ( tempdir + 8 + dirt[i] ) %8;
			switch( d )
			{
			case 0 :			tty +=j; break;
			case 1 : ttx -=j;	tty +=j; break;
			case 2 : ttx -=j;			 break;
			case 3 : ttx -=j;	tty -=j; break;
			case 4 :			tty -=j; break;
			case 5 : ttx +=j;	tty -=j; break;
			case 6 : ttx +=j;			 break;
			case 7 : ttx +=j;	tty +=j; break;
			}
			
			if( FreeTile( n, n->MoveSx, n->MoveSy, ttx, tty ) )  
			{
				*x = ttx;  *y = tty;
				return 1;
			}
		}
	}
	
	return 0;
}	

int NPC_GetDir( int sx, int sy, int ex, int ey )
{	
	if( sx > ex )  // 1/2/3
	{
		if( sy > ey )		return 3;
		else if( sy < ey )	return 1; 
		else				return 2;
	}
	else if( sx < ex ) // 5/6/7
	{
		if( sy > ey )		return 5;
		else if( sy < ey )	return 7; 
		else				return 6;
	}
	else 
	{
		if( sy > ey )		return 4;
		else if( sy < ey )	return 0;
	}
	
	return -1;
}		

int NPC_MakePath( CHARLIST *n, int tx, int ty, int how = 0 )
{		
	int dir;
	int c, i;
	int nx, ny, px, py;
	if( g_block_move )
	{
		return 0;
	}
	
	if (n->MoveP==0) return 0;		// LTS AI
	
	n->tx = tx;
	n->ty = ty;
	n->how = how;
	
	if( n->MovePathCount < 2 )
	{
		if( PathBuild( n, tx, ty ) == false ) { return 0; }
		
		int bOdd = 0;
		
		if( n->pathcount )
		{
			px = n->path[ n->pathcount -1][0];
			py = n->path[ n->pathcount -1][1];
			
			c = 0;
			for( i = n->pathcount -2 ; i >= 0 ; i -- )
			{		
				nx = n->path[i][0];
				ny = n->path[i][1];
				
				if( nx > px ) 
				{	
					if( ny > py )		{ dir = 7; }
					else if( ny < py )	{ dir = 5; }
					else				{ dir = 6; }
				}	
				else if( nx < px )
				{	
					if( ny > py )		{ dir = 1; }
					else if( ny < py )	{ dir = 3; }
					else				{ dir = 2; }
				}	
				else 
				{	
					if( ny > py )		{ dir = 0; }
					else				{ dir = 4; }
				}	
				
				
				if (  bOdd == 0){	n->MoveDirTbl[ (c/2) ] = dir << 4;	}
				else			{	n->MoveDirTbl[ (c/2) ] += dir;		}
				c++;
				bOdd = !bOdd;
				px = nx;
				py = ny;
			}
			
			n->MoveLength		= c;
			n->MovePathCount	= 0;
			n->MoveGox			= tx * TILE_SIZE + 16 -5 + ran(10);		// 捞悼困摹啊 函版
			n->MoveGoy			= ty * TILE_SIZE + 16 -5 + ran(10);		// 捞悼困摹啊 函版
			n->WalkTime			= global_time; // timeGetTime();
			n->MoveType			= 0;
			
			return 1;
		}
		
		return 0;
	}
	
	return 0;
}			

int NPC_MakePathBumn( CHARLIST *n, int tx, int ty, int how = 0 )
{
	int dir;
	int c, i;
	int nx, ny, px, py;
	
	n->tx = tx;
	n->ty = ty;
	n->how = how;
	
	if( g_block_move ) return 0;
	
	if( n->MovePathCount < 2 )
	{
		if( PathBuild( n, tx, ty ) == false ) { return 0; }
		
		int bOdd = 0;
		
		if( n->pathcount )
		{
			px = n->path[ n->pathcount -1][0];
			py = n->path[ n->pathcount -1][1];
			
			c = 0;
			for( i = n->pathcount -2 ; i >= 0 ; i -- )
			{		
				nx = n->path[i][0];
				ny = n->path[i][1];
				
				if( nx > px ) 
				{	
					if( ny > py )		{ dir = 7; }
					else if( ny < py )	{ dir = 5; }
					else				{ dir = 6; }
				}	
				else if( nx < px )
				{	
					if( ny > py )		{ dir = 1; }
					else if( ny < py )	{ dir = 3; }
					else				{ dir = 2; }
				}	
				else 
				{	
					if( ny > py )		{ dir = 0; }
					else				{ dir = 4; }
				}	
				
				
				if (  bOdd == 0){	n->MoveDirTbl[ (c/2) ] = dir << 4;	}
				else			{	n->MoveDirTbl[ (c/2) ] += dir;		}
				c++;
				bOdd = !bOdd;
				px = nx;
				py = ny;
			}
			
			n->MoveLength		= c;
			n->MovePathCount	= 0;
			n->MoveGox			= tx * TILE_SIZE + 16 -5 + ran(10);
			n->MoveGoy			= ty * TILE_SIZE + 16 -5 + ran(10);
			n->WalkTime			= global_time; // timeGetTime();
			n->MoveType			= 0;
			
			return 1;
		}
		
		return 0;
	}
	
	return 0;
}

inline int NPC_IsMoving( CHARLIST *n )
{			
	if( n->desttime > global_time ) return 1;
	return 0;
}			

int NPC_IsWhoNearPC( CHARLIST *npc, int tilerange  )
{
	int i;
	CHARLIST *ch;
	int x = npc->X;
	int y = npc->Y;
	int tx, ty;
	DWORD t, min = 0xFFFFFFFF;
	int findch;
	
	for( i = DRAGON_CONNECTIONS_START ; i < DRAGON_MAX_CONNECTIONS ; i ++)
	{
		if(connections[i].dwAgentConnectionIndex && connections[i].state >= CONNECT_JOIN )
		{
			ch = &connections[i].chrlst;
			if( ch->bAlive >= DEAD_ || ch->GetState() == CON_STONE) continue;
			
			tx = ch->X;
			ty = ch->Y;
			t = (tx-x)*(tx-x) + (ty-y)*(ty-y);
			if( t < min ) 
			{
				min = t;
				findch = i;
			}
		}
	}
	
	if( min > (DWORD)(TILE_SIZE * tilerange) * (DWORD)( TILE_SIZE *tilerange) ) 
	{
		
		return -1;
	}
	
	
	return findch;
}

CHARLIST* NPC_ReturnCharListPoint( const int id )
{//021030 lsw////叼厚单阁(1)捞搁 巩力啊 积辨 荐档 乐绢辑 绊闷嚼聪促.
	if( id < 0 ) return NULL;
	
	if( id >= 10000 )
	{	
		if( NPCList[ id-10000 ].bAlive >= DEAD_ ) return NULL;
		else return &NPCList[ id-10000];
	}	
	else 
	{	
		CHARLIST *pTempCh = ::CheckServerId( id );
		if(pTempCh)
		{
			if(pTempCh->bAlive < DEAD_)//磷篮 惑怕搁 棵赋聪促.
			{
				return pTempCh;
			}
		}
	}
	return NULL;
}		



CHARLIST* ReturnCharListPoint( char *name )
{	//< CSD-030415	
	if (name != NULL)
	{	
		for (int i = NPC_LIST_START ; i < MAX_NPC_LIST; ++i)
		{	
			if (NPCList[i].bAlive == ALIVE_)
			{
				if (strcmp(NPCList[i].Name, name) == 0)
				{
					return &NPCList[i];
				}
			}
		}	

		CHARLIST* pUser = g_pUserManager->GetCharList(name);

		if (pUser != NULL && pUser->bAlive == ALIVE_)
		{
			return pUser;
		}
	}

	return NULL;
}	//> CSD-030415

#define MAGIC_TEST_NO	30

bool SetTileOccupied(short int &iX, short int &iY, const bool bOccupiedState)
{
	if(	VILLAGE_SIZE <= iX
		||	VILLAGE_SIZE <= iY
		||	0 > iX
		||  0 > iY)
	{
		::MyLog(1,"Critical Warning!! SetTileOccupied(%d, %d) Call Developer",iX,iY);
		if(0 > iX)	{iX = 0;}
		if(0 > iY)	{iY = 0;}
		
		if(VILLAGE_SIZE > iX){iX = VILLAGE_SIZE-1;}
		if(VILLAGE_SIZE > iY){iY = VILLAGE_SIZE-1;}
		
		return false;
	}
	
	TileMap[iX][iY].occupied = bOccupiedState;
	return true;
}

void CharacterMovingSub( CHARLIST *ch, int spritetype )
{
	if( ch->MovePathCount >= ch->MoveLength ) 
	{
		ch->MovePathCount = ch->MoveLength = 0;
		ch->X = ch->MoveGox;
		ch->Y = ch->MoveGoy;
		return;
	}
	
	int dir = ch->MoveDirTbl[ ch->MovePathCount >> 1];
	
	if( ch->MovePathCount % 2 )
	{	
		dir &= 0x0f;
	}	
	else 
	{	
		dir >>= 4;
	}	
	
	::SetTileOccupied(ch->MoveSx,ch->MoveSy,false);
	
	ch->OldMoveSx = ch->MoveSx;
	ch->OldMoveSy = ch->MoveSy;
	
	switch( dir )
	{	
	case 0 : 					ch->MoveSy++;	break;
	case 1 : 	ch->MoveSx--; 	ch->MoveSy++;	break;
	case 2 : 	ch->MoveSx--;					break;
	case 3 : 	ch->MoveSx--; 	ch->MoveSy--;	break;
	case 4 : 					ch->MoveSy--; 	break;
	case 5 : 	ch->MoveSx++;	ch->MoveSy--;	break;
	case 6 : 	ch->MoveSx++; 					break;
	case 7 : 	ch->MoveSx++;	ch->MoveSy++;	break;
	}	
	
	::SetTileOccupied(ch->MoveSx,ch->MoveSy,true);
	
	ch->MovePathCount++;
	if( ch->MovePathCount >= ch->MoveLength )
	{
		ch->X = ch->MoveGox;
		ch->Y = ch->MoveGoy;
		ch->MovePathCount = ch->MoveLength = 0;
		
		if( spritetype == SPRITETYPE_NPC )
		{
			::SendModifyPositionNPC( connections, ch->GetServerID()); 
		}
	}	
	else 
	{	
		ch->X = (ch->MoveSx * TILE_SIZE ) + 16;
		ch->Y = (ch->MoveSy * TILE_SIZE ) + 16;
	}
}

void MovingNPC()
{
	const DWORD curtime = ::timeGetTime();
	
	for( int i = NPC_LIST_START ; i < MAX_NPC_LIST ; i++ ) 
	{
		CHARLIST* npc = ::CheckNpcId(i);
		if(!npc){continue;}
		
		if(	ALIVE_ != npc->bAlive	
			||	CON_STONE == npc->GetState() )
		{
			continue;
		}
		
		if( npc->MoveLength <= npc->MovePathCount )
		{
			if (npc->ChairNum)
			{
				g_pAIManager->ProcessNPCAI(npc);
			}
			
			continue;
		}
		
		const DWORD dwDelayTime = ::Get_MoveDelayTime( npc );
		if(dwDelayTime <= 0){continue;}
		
		const DWORD dwElapsedTime = curtime - npc->WalkTime;
		if( dwElapsedTime >= dwDelayTime ) 
		{
			const DWORD oldt =  dwElapsedTime / dwDelayTime;
			DWORD t = oldt;
			npc->WalkTime += t * dwDelayTime;
			for( ; t ; t-- )
			{
				::CharacterMovingSub( npc, SPRITETYPE_NPC );
			}
			if( oldt )
			{
				::SetArea(MOVE_NPC_AREA, i );
			}
		}
		
		switch (NPC_Gen_Ref[npc->npc_index].nAttr)
		{	//< CSD-031027
		case AT_PIERCING:
		case AT_ANTIMAGIC:
		case AT_BOSS:
		case AT_SEMIBOSS:
			{
				if (npc->ChairNum)
				{
					g_pAIManager->ProcessNPCAI(npc);
				}

				break;
			}
		}	//> CSD-031027
	}
}

void MovingCharacter()
{
	const DWORD dwCurtime = ::timeGetTime();
	
	for(int i = DRAGON_CONNECTIONS_START ; i < DRAGON_MAX_CONNECTIONS; i ++ )
	{
		CHARLIST *pc = ::CheckServerId(i);
		if(!pc){continue;}
		if(pc->MoveLength <= pc->MovePathCount ){continue;}
		
		const DWORD dwElapsedTime = dwCurtime - pc->WalkTime;
		const DWORD dwDelayTime = ::Get_MoveDelayTime( pc );
		if( dwDelayTime <= 0  )
		{
			pc->MoveLength = pc->MovePathCount = 0;
			continue;
		}
		
		if( dwElapsedTime >= dwDelayTime ) 
		{
			const DWORD oldt =  dwElapsedTime / dwDelayTime;
			DWORD t = oldt;
			
			pc->WalkTime += t * dwDelayTime;
			for( ; t ; t-- )
			{
				::CharacterMovingSub( pc, SPRITETYPE_CHARACTER );
				
				if( !pc->Peacests)
				{
					::HungryMuchAdd( pc, ((!pc->MoveType)?HT_WALK:HT_RUN) ); // 硅绊悄荐摹 皑家.
				}
				else 
				{
					::HungryMuchAdd( pc, ((!pc->MoveType)?HT_BATTLE_WALK :HT_BATTLE_RUN) ); // 硅绊悄荐摹 皑家.
				}
			}
			if( oldt ) 
			{
				::SetArea(MOVE_PC_AREA, i );
			}
		}
	}
}

void NPCAddBasicData(int i, t_server_npc_add* p)
{	//< CSD-030419
	CHARLIST* pNpc = &NPCList[i];
	p->idNpc = i + 10000;
	p->nStatus = pNpc->bAlive;
	p->nSprNo = pNpc->SprNo;
	p->nMutant = pNpc->mutant;
	p->nX = pNpc->X;
	p->nY = pNpc->Y;
	p->nHomeX = pNpc->homex;
	p->nHomeY = pNpc->homey;
	p->nNameNo = pNpc->namenumber;
	p->nEventNpc	= (pNpc->eventno > 0) ? 1:0;
	p->nAttr = NPC_Gen_Ref[pNpc->npc_index].nAttr;
	p->nViewType = pNpc->viewtype;
	p->nRace = pNpc->Race;
	
	if (pNpc->patterntype >= NPC_PATTERN_WANDER && pNpc->patterntype <= NPC_PATTERN_ATTACKED) // LTS AI
	{
		p->nAIType	= pNpc->ChairNum;	// LTS AI
	}
	else
	{
		p->nAIType = 0;
	}
	
	switch (pNpc->generationtype)
	{	//< CSD-031013
	case GT_SKILL_SUMMON:
		{
			p->nAttr = AT_SUMMON;
			break;
		}
	case GT_HUNT_MONSTER:
		{
			p->nRecall = 0;
			break;
		}
	default:
		{
			p->nRecall = (pNpc->GetMaster() == 0) ? 0:1;
			break;
		}
	}	//> CSD-031013
}	//> CSD-030419

// 弥家 1付府绰 惯积阑 窍霸 茄促. 
void NPCGeneratePosition( int npcindex, int x, int y, int eventno, int maxno )
{	
	if( NPCgeneratePositionMax >= MAX_GENERATE_POSITION_ ) return;
	
	NPCgeneratePosition[ NPCgeneratePositionMax].curnpc		= 0;
	
	NPCgeneratePosition[ NPCgeneratePositionMax].npcindex	= npcindex;
	NPCgeneratePosition[ NPCgeneratePositionMax].x			= x;
	NPCgeneratePosition[ NPCgeneratePositionMax].y			= y;
	
	// Event锅龋啊 0锅捞搁 Event锅龋甫 啊龙荐 乐绰 NPC父 0锅阑 啊瘤绊 唱赣瘤绰 
	// -1蔼阑 爱绰促. 
	if( eventno == 0 )
	{	
		switch (npcindex)
		{
		case 21 :
		case 22 :
		case 23 :
		case 24 :
		case 71 :
		case 72 :
		case 73 :
		case 74 :
		case 75 :
		case 76 :
		case 77 :
		case 78 :
		case 79 :
		case 80 :	NPCgeneratePosition[ NPCgeneratePositionMax].eventno	= eventno;
			break;
			
		default : 	NPCgeneratePosition[ NPCgeneratePositionMax].eventno	= -1;
		}
	}
	else
	{
		NPCgeneratePosition[NPCgeneratePositionMax].eventno = eventno;
	}
	
	
	// 版厚绰 Event锅龋啊 绝绢具 茄促. 
	// 010613 KHS
	switch( npcindex )
	{
	case 18 :
	case 19 :
	case 20 : NPCgeneratePosition[ NPCgeneratePositionMax].eventno	= -1;
		break;
	}
	
	if( maxno <= 0 ) maxno = 1;
	NPCgeneratePosition[ NPCgeneratePositionMax].maxno						= maxno;
	
	NPCgeneratePositionMax ++;
}	

int  GetDeleteAbleNPC()//030211 lsw
{//酒林 老馆利栏肺 攫力 磷咯档 惑包绝绰 NPC甫 惶澜
	int i = NPC_LIST_START;
	int iTypeId = -1;
	for(; i < MAX_NPC_LIST; i ++)
	{
		const int iGenerateType = NPCList[i].generationtype;
		const int iAliveFlag =  NPCList[i].bAlive;
		if(	iGenerateType == GT_SKB_FILE 
			||	iGenerateType == GT_SKILL_SUMMON )
		{
			iTypeId = i;
		}
		if( iAliveFlag == REMOVE_)
		{
			return i;
		}
	}
	
	if(iTypeId >= 0)
	{
		return iTypeId;
	}
	
	::MyLog( LOG_NORMAL, "Can't Get Normal NpcList" );
	return(-1);
}

int NPC_Create(int Num, int npcindex, int x, int y, int eventno, int generationpos, int generationtype,int GroupNo, int isBoss)	// LTS AI2
{	//< CSD-030408
	if (Num < 0) 
	{
		MyLog(LOG_NORMAL, "NPC_CREATE Failed!!! Argument is Lower than Zero, Num : %d",Num);
		return 0;
	}
	
	if (npcindex < 0) 
	{
		MyLog(LOG_NORMAL, "NPC_CREATE Failed!!! Argument is Lower than Zero, Index : %d",npcindex);
		return 0;
	}
	if (npcindex >= Num_Of_NPC_Generation)
	{
		MyLog(LOG_NORMAL, "NPC_CREATE Failed!!! Argument is Higher than Max Generation, Index : %d",npcindex);
		return 0;
	}
	
	NPC_Generation* pNpcTable = &NPC_Gen_Ref[npcindex];	
	CHARLIST* n = &NPCList[Num];
	memset(n, 0, sizeof(CHARLIST));
	n->SprType = SPRITETYPE_NPC;
	n->npc_index = npcindex;
	n->SetServerID(Num);
	n->GenerateNpcName(); // 捞抚 汲沥
	n->eventno = eventno;
	n->generationpos = generationpos;
	n->generationtype = generationtype;
	n->SprNo = pNpcTable->SprNO;
	n->mutant = pNpcTable->mutant;
	n->homemap = MapNumber;
	n->homex = x; // 积己等 困摹
	n->homey = y;
	n->Class = CREATURE; // 努贰胶 汲沥
	n->Race = pNpcTable->nNpcRace; // 辆练 汲沥
	n->SetLevel(pNpcTable->nLvMin + ran(pNpcTable->nLvMax - pNpcTable->nLvMin)); // CSD-030806 : 饭骇 汲沥
	n->Exp = pNpcTable->nEpMax; // 版氰摹 汲沥
	n->SetClassStep(pNpcTable->nStep);
	n->SetAttribute(pNpcTable->nNpcRace); // 加己 汲沥
	n->Face = 0;
	n->Sight = 12;
	n->ResetAbility(0);
	n->Hp = n->HpMax ;
	n->Mana = n->ManaMax;
	n->Hungry = n->HungryMax;
	n->name_status.nation = (pNpcTable->nNpcNK == 0) ? 2:MapInfo[MapNumber].nation;
	n->Money = pNpcTable->Money_min + ran(pNpcTable->Money_Max - pNpcTable->Money_min);
	n->SetState(CON_NORMAL);
	// 历亲仿(窜困 %)
	n->SetBasicResist(RT_POISON, 0);                // 刀拌凯 付过俊 措茄 历亲仿
	n->SetBasicResist(RT_CURSE, 0);	                // 历林拌凯 付过俊 措茄 历亲仿
	n->SetBasicResist(RT_HOLY, pNpcTable->nHoly);   // 脚仿拌凯 傍拜 付过俊 措茄 历亲仿
	n->SetBasicResist(RT_FIRE, pNpcTable->nFire);   // 阂拌凯 傍拜 付过俊 措茄 历亲仿
	n->SetBasicResist(RT_ICE, pNpcTable->nIce);	    // 倔澜拌凯 傍拜 付过俊 措茄 历亲仿
	n->SetBasicResist(RT_ELECT, pNpcTable->nElect); // 傈拜拌凯 傍拜 付过俊 措茄 历亲仿
	
	switch (pNpcTable->Sel_gender)
	{
	case 1:
	case 2: 
		{
			n->Gender = pNpcTable->Sel_gender - 1;		
			break;
		}
	case 0:
	case 3: 
		{
			n->Gender = ran(2);				
			break;
		}
	}
	
	if (pNpcTable->nNpcRace == SEALSTONE)	
	{	// LTS NEW LOCALWAR
		if (g_pLocalWarfield != NULL)
		{
			g_pLocalWarfield->CheckSealStonePtr(n);
		}
	}
	
	n->MoveSx = x;
	n->MoveSy = y;
	SetTileOccupied(n->MoveSx, n->MoveSy, false);
	
	n->OldMoveSx = n->MoveSx;
	n->OldMoveSy = n->MoveSy;
	n->X = n->MoveGox = n->MoveSx*TILE_SIZE + 16;
	n->Y = n->MoveGoy = n->MoveSy*TILE_SIZE + 16;
	n->ox = n->X;
	n->oy = n->Y;
	n->MoveP = pNpcTable->Movp + (rand()%20); // 捞悼仿 // LTS AI
	n->MovePathCount = 0;
	n->MoveType	= 0;
	
	int amount = 0;
	n->Get_meat	= pNpcTable->Get_meat;
	
	amount = pNpcTable->Meat_amount;	
	
	if (rand()%6)
	{ 
		amount = (amount) ? (rand()%amount + 1):0; 
	}
	else 
	{
		amount = 0; 
	}
	
	n->Meat_amount = amount;
	n->Get_skin = pNpcTable->Get_skin;
	
	amount = pNpcTable->Skin_amount;	
	
	if (rand()%3)
	{ 
		amount = (amount) ? (rand()%amount + 1):0; 
	}
	else 
	{
		amount = 0; 
	}
	
	n->Skin_amount = amount;
	n->Get_Wool	= pNpcTable->Get_Wool;
	
	amount = pNpcTable->Wool_amount;
	
	if (rand()%3)
	{ 
		amount = (amount) ? (rand()%amount + 1):0;
	}
	else 
	{ 
		amount = 0; 
	}
	
	n->Wool_amount = amount;
	n->Get_beewax = pNpcTable->Get_beewax;
	
	amount = pNpcTable->BeeWax_amount;	
	
	if (rand()%3)
	{ 
		amount = (amount) ? (rand()%amount + 1):0; 
	}
	else 
	{ 
		amount = 0; 
	}
	
	n->BeeWax_amount = amount;
	// NPC阑 磷菌阑 锭 掘阑 荐 乐绰 酒捞袍 汲沥
	if (!g_pArenaManager->IsColossusArena())
	{	//< CSD-030516
		SkillMgr.GenerateNPCItem2(n, pNpcTable);
	}	//> CSD-030516
	// 鸥捞怪 阁胶磐 沥焊
	n->targetid	= -1;
	n->attacked	= -1;
	n->WalkTime	= global_time;
	n->aitimedelay = 0;
	n->tame	= 0;
	n->tamingcountdown = 0;
	n->tamingable = pNpcTable->Tamingable;
	// A.I Pattern汲沥秦林绰镑
	n->ChairNum = (generationtype == GT_SKILL_SUMMON) ? 0:pNpcTable->nAIType; // LTS AI AI 且寸阑 包府茄促. 0: 努扼捞攫飘 1 : 辑滚
	n->patterntype = pNpcTable->nAIBase;
	
	if (n->ChairNum != 0) // LTS NEW AI
	{	//< CSD-030417
		if (pNpcTable->nAppearType > 0) 
		{ 
			n->CurrentPhase = pNpcTable->nAppearType;	
		}
		
		n->patterntype = g_pAIManager->GetNpcAIData(pNpcTable->nAINo, FSTART_AI_PATTERN);
	}	//> CSD-030417
	
	n->JoinLocalWar = GroupNo; // LTS AI2
	n->GainedFame = isBoss;    // LTS AI2
	n->ctrlpc = -1;
	n->bAlive = ALIVE_;
	++CurrentNPCNo;
	// 010301 KHS
	switch (n->generationtype)	
	{		
	case GT_SKB_FILE:
		{
			if (generationpos >= 0 && generationpos < NPCgeneratePositionMax)
			{
				NPCgeneratePosition[generationpos].curnpc++;
			}
			
			break;				
		}
	case GT_SCRIPT:	        
	case GT_SKILL_SUMMON:
		{
			break;
		}
	}		
	
	SetArea(ADD_NPC_AREA, Num);
	Send_CMD_SEALSTONE_STATUS(n, 1);
	return 1;		
}	//> CSD-030408

int NPC_AdjustPosition(int sprno, int* x, int* y)
{	//< CSD-030408
	switch (sprno)	
	{	// Dummy NPC肺辑
	case 92: 
	case 93: 
	case 94: 
	case 95: return 1;
	}
	
	int ax = *x;
	int ay = *y;
	
	const int maxx = 3;
	const int maxy = 3;
	
	for (int i = 0; i < maxx; ++i)
	{
		for (int j =  0; j < maxy; ++j)
		{	
			if (FreeTile(NULL, ax + i, ay + j, ax + i, ay + j)) 
			{
				*x = ax + i;
				*y = ay + j;
				return 1; 
			}
			
			if (FreeTile(NULL, ax + i, ay - j, ax + i, ay + j)) 
			{	
				*x = ax + i;
				*y = ay - j;	
				return 1; 
			}
			
			if (FreeTile(NULL, ax - i, ay + j, ax + i, ay + j))
			{	
				*x = ax - i;
				*y = ay + j;	
				return 1; 
			}
			
			if (FreeTile(NULL, ax - i, ay - j, ax + i, ay + j)) 
			{	
				*x = ax - i;
				*y = ay - j;	
				return 1; 
			}
		}				
	}	
	
	return 0;		
}	//> CSD-030408	

void NPC_AutoCreationProc(  void )
{	
	int id;	
	int x, y;
	sNPCgeneratePosition *gpos;
	int i;		
	static int MaxNPC;
	
	static DWORD checktime;
	
	if( global_time - checktime > 10000  )	// 10檬付促 
	{				
		checktime = global_time;
		
		for( i = 0 ; i < NPCgeneratePositionMax ; i ++)
		{			
			if(NPCgeneratePosition[i].maxno > NPCgeneratePosition[i].curnpc ) // 茄疙父 惯积矫挪促. 溜, Event侩捞鄂娟扁.. 
			{				
				gpos = &NPCgeneratePosition[i];
				if( gpos->curnpc < gpos->maxno )
				{			
					if( gpos->maxno > 1 )
					{		
						x = gpos->x -3 + (rand()%7);
						y = gpos->y -3 + (rand()%7);
					}		
					else	
					{		
						x = gpos->x;
						y = gpos->y;
					}		
					
					id	= GetAliveNPCList();
					
					if( id >= 0 )
					{
						if( NPC_AdjustPosition( gpos->npcindex, &x, &y ) )
						{	
							NPC_Create( id, gpos->npcindex, x, y, gpos->eventno, i, GT_SKB_FILE );
							
						}
					}
				}
			}
		}	
		return;
	}				
	
	return;
}	


void NPC_AutoCreationProc2()						// LTS AI2
{
	static DWORD checktime;
	int x,y;		
	int NPC_ID;
	
	if( global_time - checktime < 10000  )	return;			
	else checktime = global_time;
	
	for (int j=0;j<MAX_GROUP_NO;j++)			// 弊缝 贸澜何磐 场鳖瘤 捣促.
	{
		int tempStatus=g_pAdventManager->GetActiveStatusByIndex(j);
		if (tempStatus>0)				// 秦寸 弊缝狼 牢郸胶 锅龋啊 绝促.
		{
			for (int i=0;i< g_pAdventManager->GetDataCountByIndex(j);i++)		// 弊缝单捞磐狼 场鳖瘤 埃促.
			{
				NPCGenerationPos* Pos=NULL;
				Pos=g_pAdventManager->GetGenerationPosByIndex(j,i);
				if (!Pos) return;
				if (Pos->MaxNo>Pos->CurNPC)						//积己登瘤 臼篮 NPC啊 乐促.
				{
					if (Pos->MaxNo>1)
					{
						x = Pos->LocationX -3 + (rand()%7);
						y = Pos->LocationY -3 + (rand()%7);
					}
					else
					{
						x = Pos->LocationX;
						y = Pos->LocationY;
					}
					NPC_ID	= GetAliveNPCList();
					if( NPC_ID >= 0 )
						if( NPC_AdjustPosition( Pos->NPCNo, &x, &y ) )
						{	
							NPC_Create(NPC_ID,Pos->NPCNo, x, y,Pos->EventNo, i, GT_EVENT_MONSTER,j,Pos->isBoss );
							Pos->CurNPC++;
							MyLog(0,"NPC SPECIAL ID:%d, No:%d, x:%d, y:%d, event:%d, i:%d, Group:%d, GTType:%d",NPC_ID,Pos->NPCNo, x, y,Pos->EventNo, i, j,GT_EVENT_MONSTER);
						}
				}
			}
		}
	}
}

void NPC_AutoCreate()		// LTS AI2
{	//< CSD-030509
	NPC_AutoCreationProc();
	NPC_AutoCreationProc2(); // 捞亥飘 阁胶磐 殿厘
	// 荤成 阁胶磐 殿厘
	if (!g_pArenaManager->IsColossusArena())
	{
		g_pRegenManager->Regenerate(); 
	}
}	//> CSD-030509

//---------------------------------------------------------------------------------		
//	矫埃捞 DEAD_CHARACTER_CHECK_TIME捞 版苞窍搁 .bAlive啊 DEAD_登绢 乐绰 Character甫 REMOVE_矫挪促. 
//	NPC老版快 ITEM阑 积己矫挪促. 
//	葛电 NPC/PC狼 磷澜篮 咯扁辑 贸府茄促. 
//-----------------------------------------------------------------------------------
bool UsesRegularMonsterRespawn(CHARLIST* npc)
{
    if (!npc || !npc->IsNpc()) return false;
    const bool ordinaryRace = npc->Race == NORM_MON ||
        npc->Race == HOBBIT || npc->Race == DWARF;
    return IsRegularMonsterRespawnEligible(
        npc->generationtype == GT_SKB_FILE && ordinaryRace,
        npc->eventno, npc->GainedFame, npc->CurrentPhase, npc->IsTamedNpc());
}

void NPC_CheckRemove( void )
{		
	static DWORD time;
	int i;	
	CHARLIST *ch;
	
	if( g_curr_time - time < DEAD_CHARACTER_CHECK_TIME ) return;
	time = g_curr_time;	
	
	for( i = NPC_LIST_START ; i < MAX_NPC_LIST ; i ++)
	{				
		if( NPCList[i].bAlive >= DEAD_ )
			if( NPCList[i].deadcount > 1 )
			{				
				ch = NPCList + i;
				ch->deadcount--;
				if( ch->deadcount == DEAD_NPC_COUNT_BOTTOM_ )	// Client俊霸辑 Remove矫挪促. 
				{		
					if( ( ch->eventno>0 && ch->Race==HUMAN ) || ( ch->Race ==  SEALSTONE ) )// 捞亥飘甫 啊瘤绊 乐绰 仇捞搁 ..
					{	
						ch->deadcount	=       0;
						ch->Peacests = 0;
						ch->OldMoveSx = ch->MoveSx = ch->X		=	ch->homex;
						ch->OldMoveSy = ch->MoveSy = ch->Y		=	ch->homey;
						ch->X *= TILE_SIZE;
						ch->Y *= TILE_SIZE;
						
						// 020701 YGI
						ch->Hp		= ch->HpMax;
						ch->Hungry  = ch->HungryMax;
						ch->Mana	= ch->ManaMax;
						ch->bAlive = ALIVE_;		// 020716 YGI
						
						SetArea( MOVE_NPC_AREA, i);
						//SkillMgr.CharacterToAlive(ch);//促矫 混赴促
						continue;
					}
					
					SetArea( REMOVE_NPC_AREA, i);
					// Client俊辑 荤扼瘤绰 矫埃阑 国扁困窃...
					// Client俊辑 荤扼瘤绰 悼救 弊困摹俊辑 积扁搁 救登扁 锭巩.
					NPCList[i].bAlive = BUFE_;
					ch->deadcount = HiddenMonsterDeathCount(
						UsesRegularMonsterRespawn(ch), ch->deadcount);
				}
				else if( ch->deadcount <= 1 )	// Server俊技档 Remove矫挪促. 
				{
					ch->deadcount = 0;
					RemoveNPCList( i );
				}
			}
			// Remove Process....
	}
	
	for( i = DRAGON_CONNECTIONS_START ; i < DRAGON_MAX_CONNECTIONS ; i ++)
	{				
		if( connections[i].dwAgentConnectionIndex && connections[i].state >= CONNECT_JOIN )
			if( connections[i].chrlst.bAlive >= DEAD_ )
				if( connections[i].chrlst.deadcount >= DEAD_PC_COUNT_BOTTOM_ ) 
				{		
					ch = &connections[i].chrlst;
					ch->deadcount --;
					
					if( ch->deadcount < DEAD_PC_COUNT_BOTTOM_ ) //  磷篮第 Ghost肺 函茄促. 
					{	
						RareEM.ClearDynamicRareEffect(ch); //蜡飞捞 登菌阑锭 努府绢 扁瓷酒捞袍 努府绢
						
						// 031009 YGI	// 酒饭唱俊辑绰 烹窍瘤 臼绰促. // 归捍傈 吝老订 烹窍瘤 臼绰促.
						extern bool CanUseRevivalPosion();
						if( CanUseRevivalPosion() && !g_pArenaManager->GetArenaInfo( GetOwnPort() ) && RareEM.IsHaveRivaval(ch) )//狞芒阑 倒绊辑 混府绰 酒捞袍捞 乐栏搁 混妨霖促 // 何劝狼 郡腐辑
						{	
							SkillMgr.CharacterToAlive(ch, 1);
						}
						else
						{
							SkillMgr.CharacterToGhost(ch);
						}
						// 某腐磐 付过沥焊 檬扁拳  
						ch->Clear();
					}
				}
	}
}	

void RecvCMD_MODIFY_POSITION(  int cn,  t_client_modify_position *p )
{	
	if( cn < DRAGON_CONNECTIONS_START || cn >= DRAGON_MAX_CONNECTIONS  ) return;
	LPCHARLIST ch = &connections[ cn].chrlst;
	//< CSD-021114
	ch->X = ch->MoveGox = p->x;
	ch->Y = ch->MoveGoy = p->y;
	//> CSD-021114
	ch->MoveSx = p->x / TILE_SIZE;
	ch->MoveSy = p->y / TILE_SIZE;
	
	ch->MovePathCount = ch->MoveLength = 0;
	
	SetArea(MOVE_PC_AREA, cn );
}	





/* ///////////////////////////////////////////////////////////////////////////////////////

  CMD_ALL_READY啊 坷搁 圾青茄促. 
  CMD_NPC_ADD甫 焊郴变 沁瘤父  Client俊辑 Data Loading窍蠢衬绊 矫埃阑 棱酒冈菌促.
  
////////////////////////////////////////////////////////////////////////////////////////  */
void SendReadyMoveDate( int cn )
{
	AREA *a =  connections[cn].area_tbl;
	int id;
	t_packet packet;
	
	for( int i = 0 ; i <  MAX_AREA_BUFFER ; i ++)
	{
		if( a[i].bAlive )
		{
			switch( a[i].Type )
			{
				// PC
			case 1: id = a[i].ID;
				if( connections[ id].chrlst.MovePathCount )// 泅犁 捞悼窍绊 乐绰 吝捞搁....
				{
					if( SettingMoveData_( id, &connections[id].chrlst, &packet ) )
					{
						packet.u.server_move.movetype += 100;	// 咯扁辑 焊郴林绰 困摹俊辑 矫累茄促. 
						packet.h.header.type = CMD_MOVE;
						QueuePacket( connections, cn, &packet, 1);
					}
				}
				else
				{
					packet.h.header.type = CMD_MODIFY_POSITION_ABSOLUTE;
					{
						packet.u.server_modify_position_absolute.id = id;
						packet.u.server_modify_position_absolute.x  = connections[id].chrlst.X;
						packet.u.server_modify_position_absolute.y  = connections[id].chrlst.Y;
					}
					packet.h.header.size = sizeof( t_server_modify_position_absolute );
					QueuePacket( connections, cn, &packet, 1);
				}
				
				break;
			case 2:	id = a[i].ID;
				if( NPCList[id].MovePathCount )// 泅犁 捞悼窍绊 乐绰 吝捞搁....
				{
					if( SettingMoveData_( id + 10000, &NPCList[id], &packet ))
					{
						packet.u.server_move.movetype += 100;	// 咯扁辑 焊郴林绰 困摹俊辑 矫累茄促. 
						packet.h.header.type = CMD_MOVE;
						QueuePacket(connections, cn, &packet, 1);
					}
				}
				else 
				{
					packet.h.header.type = CMD_MODIFY_POSITION_ABSOLUTE;
					{
						packet.u.server_modify_position_absolute.id = id+10000;
						packet.u.server_modify_position_absolute.x = NPCList[id].X;
						packet.u.server_modify_position_absolute.y = NPCList[id].Y;
					}
					packet.h.header.size = sizeof( t_server_modify_position_absolute );
					QueuePacket( connections, cn, &packet, 1);
				}
				break;
			}
		}
	}
	
	
	
}	




#define TAME_FAIL_CHANGE_NAME		0 // 捞抚官操扁甫  角菩. 
#define TAME_NO_CHAR				1 // 弊繁捞抚栏肺 辨甸牢 悼拱捞 绝嚼聪促.  
#define TAME_NO_NAME_CHAR			2 // 弊繁 捞抚阑 啊柳 措惑捞 绝嚼聪促. 
#define TAME_FOLLOW					3 // 蝶扼癌聪促. 
#define TAME_STOP					4 // 肛苗 挤聪促. 
#define TAME_ATTACK					5 // 傍拜钦聪促. 
#define TAME_SUCCESS_CHANGE_NAME	6 // 捞抚阑 官操菌嚼聪促. 


void RecvTameCommand( int cn, t_tame_command *p )
{
	int i;
	CHARLIST *n, *ch;
	t_packet packet;
	
	p->mybaby[19] = 0;
	if( p->mybaby[0] == 0 ) { SendTameResult( cn, TAME_NO_CHAR ) ; return; }
	p->toname[19] = 0;
	
	for( i = NPC_LIST_START ; i < MAX_NPC_LIST ; i ++ )
	{
		if( NPCList[i].bAlive == ALIVE_ )
		{
			if( strcmp(NPCList[i].Name, p->mybaby ) == 0 )
			{
				n = &NPCList[i];
				break;					
			}
		}
	}
	if( i == MAX_NPC_LIST ) { SendTameResult( cn, TAME_NO_CHAR ) ; return;  }	
	// 弊绰 辨甸牢 悼拱捞 酒聪菌促. 
	if (n->IsTamedNpc() == false)  { SendTameResult(cn, TAME_NO_CHAR); return; }
	// 寸脚捞 林牢捞 酒聪促. 
	if( strcmp( n->HostName, connections[cn].chrlst.Name)) {SendTameResult( cn, TAME_NO_CHAR ) ; return; }
	
	switch( p->cmd )						
	{										
	case LC_TAME_NAME :			if( p->toname[0] == 0 ) return;
		strcpy( n->Name, p->toname );
		packet.h.header.type = CMD_TAME_NAME_CHANGE;
		packet.u.tame_name_change.id = i + 10000;
		strcpy( packet.u.tame_name_change.name, n->Name );
		packet.h.header.size = sizeof( t_tame_name_change );
		CastNPC2Other( i, &packet );
		break;						
		
		
	case LC_TAME_ATTACK :		
		if( p->toname[0] == 0 )	
		{		
			n->TargetName[0] = 0;
			SendTameResult( cn, TAME_NO_NAME_CHAR ); return;
		}		
		else	
		{		
			ch = ReturnCharListPoint( p->toname );	// 傍拜 措惑阑 茫绰促. 

			if (ch == n)
			{	//< CSD-031106
				return;
			}	//> CSD-031106

			if( ch == NULL )	// 傍拜措惑捞 绝栏搁.
			{	
				n->TargetName[0] = 0;
				SendTameResult( cn, TAME_NO_CHAR ); return;
			}	
			else	// 傍拜措惑阑 茫疽促 !
			{	
				if( ch->SprType == SPRITETYPE_NPC ) 
				{
					n->targetid = ch->GetServerID() + 10000;
					SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TARGETID, n->targetid );
				}
				else 
				{
					n->targetid = ch->GetServerID();
					SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TARGETID, n->targetid );
				}
				
				n->tame_cmd = LC_TAME_ATTACK;
				SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TAME_CMD, LC_TAME_ATTACK );
				strcpy( n->TargetName, p->toname );
				SendTameResult( cn, TAME_ATTACK	); 
				SendNPC_StringParameter( n->GetServerID(), NPC_PARAMETER_TARGETNAME, n->TargetName );
				
				return;
			}
		}		
		break;						
	case LC_TAME_FOLLOWME :		ch = ReturnCharListPoint( n->HostName );
		if( ch )
		{
			n->HostId = ch->GetServerID();
			n->tame_cmd = LC_TAME_FOLLOWME;
			SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_HOSTID, n->HostId );
			SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TAME_CMD, LC_TAME_FOLLOWME );
			
			if( n->patterntype != NPC_PATTERN_TAME_ )
			{
				n->oldpatterntype = n->patterntype;
				n->patterntype = NPC_PATTERN_TAME_;
				SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_OLDPATTERNTYPE,	n->oldpatterntype );
				SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_PATTERNTYPE,		n->patterntype );
			}
			SendTameResult( cn, TAME_FOLLOW );
			return;
		}
		else
		{
			return;
		}
		break;
		
	case LC_TAME_STOP :			n->tame_cmd = LC_TAME_STOP;
		//2001/02/18 zhh
		if( n->patterntype != NPC_PATTERN_TAME_ )
		{
			n->oldpatterntype = n->patterntype;
			n->patterntype = NPC_PATTERN_TAME_;
			SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_OLDPATTERNTYPE,	n->oldpatterntype );
			SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_PATTERNTYPE,		n->patterntype );
			SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TAME_CMD, LC_TAME_STOP );
		}
		else 
		{
			SendNPC_Parameter( n->GetServerID(), NPC_PARAMETER_TAME_CMD, LC_TAME_STOP );
		}
		SendTameResult( cn, TAME_STOP );	return;
		break;
	}	
}

void SendTameResult( int cn, int result  )
{	
	t_packet p;
	
	p.h.header.type = CMD_TAME_COMMAND_RESULT;
	p.u.tame_command_result.result = result;
	p.h.header.size = sizeof( t_tame_command_result );
	
	QueuePacket( connections, cn, &p, 1 );
}	

void SendHostName( int cn, CHARLIST *n )
{
	t_packet p;
	
	p.h.header.type= CMD_TAME_HOST_NAME;
	p.u.tame_host_name.id = n->GetServerID() + 10000;
	strcpy( p.u.tame_host_name.hostname, n->HostName );
	p.h.header.size = sizeof( t_tame_host_name );
	
	QueuePacket( connections, cn, &p, 1 );
}
void SendToAllHostName( int cn, CHARLIST *n )
{
	t_packet p;
	
	p.h.header.type= CMD_TAME_HOST_NAME;
	p.u.tame_host_name.id = n->GetServerID() + 10000;
	strcpy( p.u.tame_host_name.hostname, n->HostName );
	p.h.header.size = sizeof( t_tame_host_name );
	
	QueuePacket( connections, cn, &p, 1 );
	CastMe2Other( cn, &p );
}


void SendHostEnd( int npc_id )
{
	t_packet p;
	
	p.h.header.type= CMD_TAME_END;
	p.u.tame_end.id = npc_id + 10000;
	p.h.header.size = sizeof( t_tame_host_name );
	CastNPC2Other( npc_id, &p );
}

void CheckNPCTammingTimeElapsed( CHARLIST *ch )
{		
	//	矫埃瘤巢俊 蝶扼 Tamming憋捞绰 何盒..
	
	if( ch->tame > 0 ) 
	{	
		ch->tamingcountdown --;
		if( ch->tamingcountdown <= 0 ) 
		{
			ch->tame --;
			if( ch->tame <= NPC_Gen_Ref[ch->SprNo].Taming_min )
			{
				ch->SetTamed(false);
				ch->HostName[0] = 0;
				ch->tame_cmd = LC_TAME_STOP;
				ch->patterntype = ch->oldpatterntype;
				SendHostEnd( ch->GetServerID());
				SendNPC_Parameter( ch->GetServerID(), NPC_PARAMETER_TAME_CMD,		ch->tame_cmd );
				SendNPC_Parameter( ch->GetServerID(), NPC_PARAMETER_PATTERNTYPE,	ch->patterntype );
			}
			ch->tamingcountdown = 15;
		}
	}	
}		





//-----------------  Smile Face...

void RecvSmileFace( int cn, int smileno )
{		
	t_packet p;
	
	if(	connections[cn].chrlst.viewtype == VIEWTYPE_GHOST_ ) return;
	
	p.h.header.type = CMD_SMILE_FACE;
	p.u.server_smile_face.id		= cn;
	p.u.server_smile_face.smileno	= smileno;
	p.h.header.size = sizeof( t_server_smile_face );
	
	
	//	QueuePacket( connections, cn, &p, 1 );
	CastMe2Other( cn, &p );
}	

/*//////////////////////////////////////////////////////////////////////

  2000.2.5
  
	1. NPC啊 瘤贺俊 啊妨咙.
	42锅 拳捞锅.
	54锅 刀荐府.
	
//////////////////////////////////////////////////////////////////////// */

///	  	NPC甫 PC俊辑 包府茄促. 
void SendNPCControlData( int cn, int npc )
{															
	t_packet p;	
	int type, size;
	LPCHARLIST n;									
	
	
	//	return;
	
	n = &NPCList[ npc];							
	
	switch( NPCList[ npc].patterntype )		
	{										
	case NPC_PATTERN_MURI_8_		:	type = CMD_NPCCTRL_MURI_8;	
		p.u.npcctrl_muri_8.id				= npc + 10000;
		p.u.npcctrl_muri_8.bossid			= n->bossid;
		p.u.npcctrl_muri_8.Exp				= n->Exp;
		p.u.npcctrl_muri_8.oldpatterntype	= n->oldpatterntype;
		p.u.npcctrl_muri_8.patterntype	= n->patterntype;
		size = sizeof( t_npcctrl_muri_8 );
		break;
		
	case NPC_PATTERN_MURI_28_		:	type = CMD_NPCCTRL_MURI_28;		
		p.u.npcctrl_muri_28.id				= npc + 10000;
		p.u.npcctrl_muri_28.bossid			= n->bossid;
		p.u.npcctrl_muri_28.Exp				= n->Exp;
		p.u.npcctrl_muri_28.oldpatterntype	= n->oldpatterntype;
		p.u.npcctrl_muri_28.patterntype	= n->patterntype;
		size = sizeof( t_npcctrl_muri_28 );	 
		break;
		
	case NPC_PATTERN_WANDER_MAUL_	:	type = CMD_NPCCTRL_WANDER_MAUL;				
		p.u.npcctrl_wander_maul.id				= npc + 10000;			
		p.u.npcctrl_wander_maul.oldpatterntype	= n->oldpatterntype;
		p.u.npcctrl_wander_maul.patterntype	= n->patterntype;
		size = sizeof( t_npcctrl_wander_maul );	 
		break;
		
	case NPC_PATTERN_NO_MOVE_		:	type = CMD_NPCCTRL_NO_MOVE;		
		p.u.npcctrl_no_move.id					= npc + 10000;
		p.u.npcctrl_no_move.oldpatterntype		= n->oldpatterntype;
		p.u.npcctrl_no_move.patterntype			= n->patterntype;
		size = sizeof( t_npcctrl_no_move );	 
		break;
		
		
	case NPC_PATTERN_WANDER_GUARD_	:	type = CMD_NPCCTRL_WANDER_GUARD;	
		p.u.npcctrl_wander_guard.id				= npc + 10000;
		p.u.npcctrl_wander_guard.oldpatterntype= n->oldpatterntype;
		p.u.npcctrl_wander_guard.patterntype	= n->patterntype;
		size = sizeof( t_npcctrl_wander_guard );	 
		break;
		
	case NPC_PATTERN_WANDER_38_		:	type = CMD_NPCCTRL_WANDER_38;				 
		p.u.npcctrl_wander_38.id				= npc + 10000;
		p.u.npcctrl_wander_38.oldpatterntype	= n->oldpatterntype;
		p.u.npcctrl_wander_38.patterntype		= n->patterntype;
		p.u.npcctrl_wander_38.bossid			= n->bossid;
		
		size = sizeof( t_npcctrl_wander_38 );
		break;
		
	case NPC_PATTERN_TAME_			:	type = CMD_NPCCTRL_TAME;
		p.u.npcctrl_tame.id						= npc + 10000;
		p.u.npcctrl_tame.oldpatterntype			= n->oldpatterntype;
		p.u.npcctrl_tame.patterntype			= n->patterntype;		
		p.u.npcctrl_tame.HostId					= n->tame_cmd;
		p.u.npcctrl_tame.tame					= n->tame;
		p.u.npcctrl_tame.tame_cmd				= n->tame_cmd;
		
		strcpy( (char *)(p.u.npcctrl_tame.TargetName),	n->TargetName );
		size = sizeof( t_npcctrl_wander_38 );
		break;
		
	case NPC_PATTERN_SEALSTONE_NO_OPERATE_	:	
		type = CMD_NPCCTRL_SEALSTONE_;	
		p.u.npcctrl_sealstone.id				= npc + 10000;
		p.u.npcctrl_sealstone.patterntype		= n->patterntype;
		size = sizeof( t_npcctrl_sealstone );	 
		break;
		
	default : return;
	}
	
	p.h.header.type = type;
	p.h.header.size = size;
	QueuePacket( connections, cn, &p, 1 );
	
}	

int ManageNPC(  int i )
{			
	int mx,my;
	int sx, sy;
	int ex, ey;
	int range = 18;
	int id;
	int ctrlpc;					
	
	
	//	return 0;
	
	//	for( i = 0 ; i < MAX_NPC_LIST; i ++)
	{							
		ctrlpc = 0;				
		if( NPCList[i].bAlive != ALIVE_ ) return 0;
		
		mx = NPCList[i].MoveSx;	
		my = NPCList[i].MoveSy;	
		sx = mx - range;	if( sx < 0 ) sx = 0;
		ex = mx + range;	if( ex >= g_Map.file.wWidth ) ex = g_Map.file.wWidth-1;
		sy = my - range;	if( sy < 0 ) sy = 0;
		ey = my + range;	if( ey >= g_Map.file.wHeight) ey = g_Map.file.wHeight-1;
		
		if( NPCList[i].ctrlpc )	
		{						
			id = NPCList[i].ctrlpc;
			if( connections[id].dwAgentConnectionIndex && connections[id].state >= CONNECT_JOIN)
			{					
				if( BoxAndDotCrash( sx, sy, range+range, range+range, connections[id].chrlst.MoveSx,  connections[id].chrlst.MoveSy ) )
				{				
					ctrlpc = 1;	
				}				
			}					
		}
		
		if( ctrlpc )		
		{	
			
		}	
		else		// 磊脚	阑 Control窍绰 PC啊 绝促. 
		{	
		/*			NPCList[i].ctrlpc = -1;	
		for( ; sx <= ex ; sx ++ )
		{
		for( tsy = sy ; tsy < ey ; tsy ++)
		{
		t = Floor[ sx][ tsy];
		while( t )
		{
		if( t->Type == SPRITETYPE_NONE )
		{			
		tempctrlnpc = connections[ t->ID].chrlst.ctrlnpc;
		
		  for( j = 0 ; j < 3 ; j ++)
		  {		
		  if( *( tempctrlnpc + j ) == -1 ) 
		  {	
		  SendNPCControlData( t->ID, i );
		  *( tempctrlnpc + j ) = i;
		  NPCList[i].ctrlpc = t->ID;
		  return TRUE;
		  }	
		  }		
		  }			
		  t = t->next;
		  }				
		  }					
		  }						
			*/
		}							
	}									
	
	return FALSE;
}

// 0 <= id < MAX_NPC_LIST
void SendNPC_Parameter( int id, int type, int data )
{
	t_packet p;
	int ctrlpc = NPCList[ id].ctrlpc;
	
	if( ctrlpc == -1 )	return;
	p.h.header.type = CMD_SEND_NPC_PARAMETER;
	p.u.send_npc_parameter.id = id + 10000;
	p.u.send_npc_parameter.type = type;
	p.u.send_npc_parameter.data	= data;
	p.h.header.size = sizeof( t_send_npc_parameter );
	
	QueuePacket( connections,  ctrlpc, &p, 1 );
}		


void SendNPC_StringParameter( int id, int type, char *data )
{	
	t_packet p;
	
	int ctrlpc = NPCList[ id].ctrlpc;
	if( ctrlpc == -1 )	return;
	
	if( data == NULL ) return;
	if( *data == 0 ) return;
	if( strlen( data ) == 0 ) return;
	
	p.h.header.type		= CMD_SEND_NPC_STRING_PARAMETER;
	p.u.send_npc_string_parameter.id		= id + 10000;
	p.u.send_npc_string_parameter.type		= type;
	strcpy( p.u.send_npc_string_parameter.data, data );
	p.h.header.size = sizeof( t_send_npc_string_parameter) - 31 + strlen( data );
	
	QueuePacket( connections,  ctrlpc, &p, 1 );
}	


void RecvNPC_Parameter( int id, int type, int data )
{
	if( id < 10000 )	return;
	
	LPCHARLIST ch = &NPCList[ id-10000];
	
	switch( type )
	{
	case NPC_PARAMETER_TARGETID	:	ch->targetid= data;	break;
	case NPC_PARAMETER_BOSSID	:	ch->bossid	= data;	break;
	case NPC_PARAMETER_HOSTID	:	ch->HostId	= data;	break;
	case NPC_PARAMETER_TAME_CMD	:	ch->tame_cmd= data;	break;
	}
}	

void RecvNPC_StringParameter( int id, int type, char *data )
{
	if( id < 10000 )	return;
	LPCHARLIST ch = &NPCList[ id-10000];
	
	switch( type )
	{
	case NPC_PARAMETER_TARGETNAME	:	strcpy( ch->TargetName, data );
	default : break;
	}	
}		


void RecvMoveNear( int cn, int gox, int goy )
{		
	
}	

///////////////////////////////////////////////////////////////////////
// 瘤陛	鳖瘤 葛电巴篮 Client俊辑 贸府秦辑 家胶甫 眠啊且老捞 绝菌促.
void Recv_JustAni( t_packet *p )
{		
	if( p->u.just_ani.id >= 10000 )
	{	
		CastNPC2Other( p->u.just_ani.id - 10000, p );
	}	
	else 
	{	
		QueuePacket( connections, p->u.just_ani.id, p, 1 );
		CastMe2Other( p->u.just_ani.id, p );
	}	
}		

void RecvCMD_MOVE(const int cn,t_packet *packet)
{	
	if( g_block_move ){return;}
	
	const int dummysize = sizeof(t_client_move);
	t_client_move * p = &packet->u.client_move;
	
	if( p->movetype >= 100 )	// 碍力肺 捞悼茄促. ( 款康磊侩 )
	{
		p->movetype -=100;
	}
	
	const int id = p->server_id;
	LPCHARLIST ch = NULL;
	if( id >= 10000 )	
	{
		if( id-10000 < MAX_NPC_LIST ) 
		{
			ch = ::CheckNpcId(id - 10000);
			if(!ch){return;}
			
			if( ch->eventno > 0 )
			{
				::MyLog( LOG_NORMAL, "####  Cracking : Event Char Move  ID:%s ", connections[cn].id );
				return;
			}
		}
		else
		{
			::MyLog( LOG_NORMAL, "####  Cracking : Event Char Move  ID:%s NPCList[%d] ", connections[cn].id, id - 10000 );
			return;
		}
	}
	else			
	{
		ch = ::CheckServerId(cn);
		if(!ch){return;}
		ch->attackedblock = 0; 
	}
	
	if( p->sx < 0 || p->sx >= g_Map.file.wWidth ||
		p->sy < 0 || p->sy >= g_Map.file.wHeight )
	{	
		MyLog( LOG_NORMAL, "####  Cracking : RecvCMD_MOVE"
			"\nip:%s ID:%s(%s) MapSize(%d:%d) Packet(%d:%d)", 
			connections[cn].ip_address, connections[cn].id, ch->Name,
			g_Map.file.wWidth, g_Map.file.wHeight, p->sx, p->sy);
		return;
	}
	
	if (ch->CalcMovePower() <= 0){return;}
	
	ch->MoveLength				= (short int)p->length;
	ch->MoveType				=  p->movetype;		
	
	::SetTileOccupied(ch->MoveSx,ch->MoveSy,false);
	
	ch->OldMoveSx	= ch->MoveSx;
	ch->OldMoveSy	= ch->MoveSy;					
	
	short iNewMoveSX =(short int)p->sx;
	short iNewMoveSY =(short int)p->sy;
	ch->MoveSx		= iNewMoveSX;	
	ch->MoveSy		= iNewMoveSY;
	
	::SetTileOccupied(iNewMoveSX,iNewMoveSY,true);
	
	memcpy(ch->MoveDirTbl, p->dir_tbl, ch->MoveLength);
	
	ch->MoveGox					= (short int)p->gox;
	ch->MoveGoy					= (short int)p->goy;
	ch->MovePathCount			= 0;				
	
	ch->X = ch->MoveSx * TILE_SIZE + 16;			
	ch->Y = ch->MoveSy * TILE_SIZE + 16;			
	ch->WalkTime = timeGetTime();					
	
	packet->h.header.type = CMD_MOVE;
	
	if( id >= 10000 )
	{	
		SettingMoveData_( id, ch, packet );
		CastNPC2Other( id-10000,  packet);
	}	
	else 
	{	
		::SettingMoveData_( cn, ch, packet );
		::QueuePacket(connections, cn, packet, 1);
		::CastMe2Other( cn, packet);
		
		if( ch->Status )	// 0405 YGI	// 泪磊绊 乐绰 惑怕促...
		{
			::SendServerWeakUp( cn );		// 柄绢唱具窃
		}
	}
}
