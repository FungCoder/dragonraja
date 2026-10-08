/*****************************************************************************\
* Copyright (c), Future Entertainment World / Seoul, Republic of Korea        *
* All Rights Reserved.                                                        *
*                                                                             *
* This document contains proprietary and confidential information.  No        *
* parts of this document or the computer program it embodies may be in        *
* any way copied, duplicated, reproduced, translated into a different         *
* programming language, or distributed to any person, company, or             *
* corporation without the prior written consent of Future Entertainment World *
\*****************************************************************************/

#include "..\stdafx.h"
#include "DefaultHeader.h"
#include "object.h"
#include "map.h"
#include "Citem.h"
#include "RajaSystem.h"
#include "MapObjectReader.h"

static_assert(sizeof(MAPOBJECT_old) == 124, "Legacy map object layout changed");
static_assert(sizeof(MAPOBJECT) == 132, "Map object file layout changed");



///////////////////////////////////////////////////////////////////////////////
//
MAPOBJECT			Mo[ MAX_MAPOBJECT_];
WORD				TotalMapObjectID;
WORD				TotalMapObject;
int					MapObjectLevel;
short int			Doors[ 500];
int					DoorNum;


///////////////////////////////////////////////////////////////////////////////
//


/////////////////////////////////////////////////////////////////////////////
// user functions
	
	
	
/* Origin阑 棱阑锭, 

  扁夯狼 Saf绰 BMP俊辑 offsetx, offsety甫 哭率困俊辑 佬篮 image甫 toi狼 
  
	甫 嘛阑锭,,,,
*/


//---------------------------------------------------------------------------------
				
/////////////////////////////////////////////////////////////////////////////////////////////


//021030 YGI
void LoadItemDoorLine( MAPOBJECT *mo, int *dsx, int *dsy, int *ddx, int *ddy, char *filename )
{
	char temp[ FILENAME_MAX];
	FILE *fp = 0;
	*dsx = *dsy = *ddx = *ddy = 0;
	if (!mo || !filename) return;

	if( filename )
	{
		sprintf( temp, "%s/map/%s", GameServerDataPath, filename );
		fp = fopen( temp,"rb" );
	}
	if( !fp )
	{
		sprintf( temp, "%s/map/%s_toi2.b", GameServerDataPath, filename );
		fp = fopen( temp,"rb" );
	}
	
	if (!fp) return;
	short line[4] = {};
	if (ReadMapDoorLine(fp, mo->id + 13000, ITEM_FRAME_MAX_, line))
	{
		*dsx = mo->x + line[0];
		*dsy = mo->y + line[1];
		*ddx = mo->x + line[2];
		*ddy = mo->y + line[3];
	}
	fclose(fp);
}			
			
// Table狼 蔼阑 八荤茄促. 			
void CheckItemsBox( void )
{
	int j,i;
	for( i = 0 ; i < MAX_TABLE_ITEMS_IN_BOX ; i ++)
	{
		for( j = 0 ; j < 40 ; j ++)
		{
			if( Itemsinbox[i].item[j] )
			{
				ItemAttr item= GenerateItem( Itemsinbox[i].item[j] );//020509 lsw
				AddItemList( item.item_no, item.attr, 0, 1000 + TILE_SIZE  * j, 1000 + TILE_SIZE * i, 0,0,0,0,0,0 );
			}
		}
	}
}
			
int InputItemInBox( void )
{			
	int r = rand()%100;
	int i;	
			
	for( i  = 1 ; i < ItemsinboxMax ; i ++)
	{		
		if( Itemsinbox[i].lev > r ) 
		{	
			return i;
		}	
	}		
	return 0;
}
	

/*
	ItemsInBox Itemsinbox[ MAX_TABLE_ITEMS_IN_BOX];
		
		int dum		= Itemsinbox[n].item[rand()%40];
		int m		= Itemsinbox[n].money[rand()%10];
		int q		= Itemsinbox[n].quantity;
		int	attrbroadcast = 0;
		
		i->dum[0] = 0;
		i->dum[1] = 0;
		for( int j = 0 ; j < q ; j ++)	i->dum[j] = dum;
		 i->money = m;



	int n = i->dumno;
		
	int dum		= Itemsinbox[n].item[rand()%40];
	int m		= Itemsinbox[n].money[rand()%10];
	int q		= Itemsinbox[n].quantity;
	int	attrbroadcast = 0;
		
*/


			
//021030 YGI
extern int LoadTOI( int type, char *filename1, char *filename2 );
#include "teambattle.h"
int ReloadTOI( char *filename1, char *filename2 )
{
	// 先验证替换文件，失败时保留当前地图物件。
	return LoadTOI( 1, filename1, filename2 );
}

int LoadTOI( char *filename )
{
	return LoadTOI( 0, filename, filename );
}
int LoadTOI( int type, char *filename1, char *filename2 )
{
	FILE *fp = 0;
	char szFilePath[ FILENAME_MAX];
	MAPOBJECT mo;
	int dsx,dsy, ddx, ddy;
	int id;
	if (!filename1 || !filename2) return 0;

	if( type )
	{
		sprintf( szFilePath, "%s/Map/%s", GameServerDataPath, filename1 );
		fp = fopen( szFilePath, "rb" );
	}
	if( !fp )
	{
		sprintf( szFilePath, "%s/Map/%s.toi2", GameServerDataPath, filename1 );
		fp = fopen( szFilePath, "rb" );
	}
	if (!fp) return 0;
	std::vector<MAPOBJECT> objects;
	unsigned short imageCount = 0;
	const bool valid = ReadMapObjectFile<MAPOBJECT, MAPOBJECT_old>(
		fp, MAX_MAPOBJECT_, imageCount, objects);
	fclose(fp);
	if (!valid)
	{
		MyLog(0, "Invalid or truncated map object file: %s", szFilePath);
		return 0;
	}
	int requiredSlots = 0, requiredDoors = 0, freeSlots = 0;
	for (const MAPOBJECT& object : objects)
	{
		if (!IsServerMapObject(object.objectoritem)) continue;
		++requiredSlots;
		if (object.objectoritem == ITEMTYPE_DOOR) ++requiredDoors;
	}
	for (int slot = 0; slot < MAX_ITEM_LIST; ++slot)
		if (ItemList[slot].bAlive == FALSE ||
			(type && ItemList[slot].bAlive == ALIVE_ && ItemList[slot].bObject))
			++freeSlots;
	if (requiredDoors > sizeof(Doors) / sizeof(Doors[0]) || requiredSlots > freeSlots)
	{
		MyLog(0, "Map object capacity exceeded: %s (items=%d, doors=%d)",
			szFilePath, requiredSlots, requiredDoors);
		return 0;
	}
	if (type)
		for (int slot = 0; slot < MAX_ITEM_LIST; ++slot)
			if (ItemList[slot].bAlive == ALIVE_ && ItemList[slot].bObject)
				RemoveItemList(slot);
	DoorNum = 0;
	TotalMapObject = static_cast<WORD>(objects.size());
	TotalMapObjectID = imageCount;
	{
		for (const MAPOBJECT& object : objects)
		{			
			mo = object;
					
			if (!IsServerMapObject(mo.objectoritem)) continue;
										
			if( mo.objectoritem )
			{	
				ItemAttr I;
				
				dsx = dsy = ddx = ddy = 0;
				
				I.item_no = mo.id + 13000;
				I.attr[0] =  1;
				I.attr[1] =  1;
				I.attr[IATTR_ATTR] =0;
				I.attr[3] =  1;
				I.attr[4] =  1;
				I.attr[5] =  1;
							
				switch( mo.objectoritem )
				{			
				case ITEMTYPE_NORMAL	:
				case ITEMTYPE_CHAIR		:	
				case ITEMTYPE_TABLE		:	I.attr[IATTR_ATTR] =   0;	break;
							
				case ITEMTYPE_DOOR		:	I.attr[IATTR_ATTR] =  IA2_NOTMOVE | IA2_DOOR;	
											LoadItemDoorLine( &mo, &dsx, &dsy, &ddx, &ddy, filename2 );break;
						
				case ITEMTYPE_BOX		:	I.attr[IATTR_ATTR] =  IA2_NOTMOVE | IA2_BOX	;	
											mo.dum = InputItemInBox();
											break;
				case ITEMTYPE_COLOSSUS_STONE	:
					{
						I.attr[IATTR_ATTR] = IA2_COLOSSUS_STONE;
						static int count = 0;
						mo.dum = count;
						count++;
						break;
					}

				case ITEMTYPE_SEAL_SPC  :
				case ITEMTYPE_SEAL_NOR	:	I.attr[IATTR_ATTR]				=  IA2_SEAL_NOR | IA2_NOTMOVE;	
											I.attr[ IATTR_SEAL_STONE_NO]	=  mo.dum;						
											I.attr[IATTR_DURATION]			=  800000;						
											break;															
				}			
							
				id = AddItemList( I.item_no, I.attr, mo.dum, mo.x, mo.y, mo.offx, mo.offy, dsx, dsy, ddx, ddy  );
				if (!IsMapObjectItemSlotValid(id, MAX_ITEM_LIST))
				{
					MyLog(0, "Map object creation failed: %s (object=%u)", szFilePath, mo.id);
					return 0;
				}
				ItemList[id].bObject = 1;
				
				if( mo.objectoritem == ITEMTYPE_DOOR )
				{
					Doors[ DoorNum] = id;
					DoorNum++;
				}
				else if( mo.objectoritem == ITEMTYPE_COLOSSUS_STONE )
				{
					g_ColossusStone.AddStone( id );
				}

				

///////////////////////////////////////////////////


///////////////////////////////////////////////////



			}
		}
		return 1;
	}			
				
	return 0;	
}				
