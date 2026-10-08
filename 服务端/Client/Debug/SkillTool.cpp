#include "stdafx.h"
#include "stdio.h"
#include "../../Library/Shared/LegacySkillMap.h"
#include <windowsx.H>
#include "Resource.h"
#include "SkillTool.h"
#include "Map.h"
#include "gameproc.h"
#include "Char.h"
#include "Tool.h"
#include "Hong_Sub.h"

#include "Effect.h"
#include "Hong_sprite.h"
#include "directdraw.h"
#include "dragon.h"
#include "object.h"

HWND			InputSkillTableHwnd;
HWND			CheckHouseObjectHwnd;
//< CSD-030324	
char subFarmType[20][25] = {{""},};
char subMineType[20][25] = {{""},};
char subHubType[20][25] = {{""},};
//> CSD-030324
int             radio_Statue = 0;

MAPSKILLTABLE	g_MapSkillTable;
lpMAPSKILLTABLE Header[8][8];
MYHOUSETOOL		g_MyhouseTool;
DRAGMOUSE		g_DragMouse;
BOOL			g_MyhouseDlgOpen;

extern NPC_INFO g_infNpc[MAX_CHARACTER_SPRITE_]; // CSD-030419
//--------------------------------------------------------------------------------------------------------------
//  扁  瓷 :  NPC硅摹甫 瘤款促. 
//  牢  磊 : 0 :葛电 NPC硅摹甫 绝矩促. 
//           n : n锅 NPC硅摹甫 绝矩促. 
//  搬  苞 : 绝娇.
//--------------------------------------------------------------------------------------------------------------
//////////////////////// 0613 lkh 眠啊 /////////////////////////
void DeleteNPCGenerate(int npc_Num)
{
	for( int y=0; y<g_Map.file.wHeight; y++)
	{
		for(int x=0; x<g_Map.file.wWidth; x++)
		{
			lpMAPSKILLTABLE result;
			
			if(TileMap[x][y].attr_skill == 1)
			{
				result=FindSkill(&Header[(int)(x/(int)((g_Map.file.wWidth+7)/8))][(int)(y/(int)((g_Map.file.wHeight+7)/8))], x, y);
				
				if(result==NULL)
					TileMap[x][y].attr_skill = 0;
				else if(result!= NULL && result->skillno == 6)		// npc积己 器牢飘 包访 单捞磐牢 版快
				{
					if(!npc_Num)
					{
						TileMap[x][y].attr_skill=0;
						DeleteSkill(&Header[(int)(x/(int)((g_Map.file.wWidth+7)/8))][(int)(y/(int)((g_Map.file.wHeight+7)/8))], result);
					}
					else if(result->type_Num == npc_Num)
					{
						TileMap[x][y].attr_skill=0;
						DeleteSkill(&Header[(int)(x/(int)((g_Map.file.wWidth+7)/8))][(int)(y/(int)((g_Map.file.wHeight+7)/8))], result);
					}
				}
			}
		}
	}
}




void ChangeNPCGenerate(int npc_from, int npc_to )
{
	for( int y=0; y<g_Map.file.wHeight; y++)
	{
		for(int x=0; x<g_Map.file.wWidth; x++)
		{
			lpMAPSKILLTABLE result;
			
			if(TileMap[x][y].attr_skill == 1)
			{
				result=FindSkill(&Header[(int)(x/(int)((g_Map.file.wWidth+7)/8))][(int)(y/(int)((g_Map.file.wHeight+7)/8))], x, y);
				
				if(result==NULL)
					TileMap[x][y].attr_skill = 0;
				else if(result!= NULL && result->skillno == 6)		// npc积己 器牢飘 包访 单捞磐牢 版快
				{
					if( result->type_Num == npc_from )
					{
						result->type_Num = npc_to;
					}
				}
			}
		}
	}
}



void DeleteAllSkillData( void )
{
	for( int i = 0 ; i < 8 ; i ++ )
	{
		for( int j = 0 ; j < 8 ; j ++ )
		{
			while( Header[i][j] )
			{
				DeleteSkill( &Header[i][j], Header[i][j] );
			}
		}
	}
}

/*id DeleteAllSkillData( void )
{
	for( int y=0; y<g_Map.file.wHeight; y++)
	{
		for(int x=0; x<g_Map.file.wWidth; x++)
		{
			lpMAPSKILLTABLE result;
			
			if(TileMap[x][y].attr_skill == 1)
			{	
				result=FindSkill(&Header[(int)(x/(int)((g_Map.file.wWidth+7)/8))][(int)(y/(int)((g_Map.file.wHeight+7)/8))], x, y);
				if(result==NULL)
					TileMap[x][y].attr_skill = 0;
				else
				{
					TileMap[x][y].attr_skill = 0;
					DeleteSkill(&Header[(int)(x/(int)((g_Map.file.wWidth+7)/8))][(int)(y/(int)((g_Map.file.wHeight+7)/8))], result);
				}
			}
		}
	}
}*/  // 0907 KHS



void DrawSkillBox(int mox, int moy)
{
	int sx=(mox-Mapx)/TILE_SIZE*32;
	int sy=(moy-Mapy)/TILE_SIZE*32;
	Box( sx+1, sy+1, sx+TILE_SIZE-1, sy+TILE_SIZE-1, RGB(29,184,12));
}

BOOL LoadSkillMapTable(void)
{
	int		i=0;
	char	temp[FILENAME_MAX];
	FILE *fp ;
	MAPSKILLTABLE st;
	int c=0;
	
 	sprintf( temp, "./skill/%s.skb", MapName );
	fp = Fopen( temp, "rb" );
	if(fp == NULL) 		return FALSE;

	int dx = (g_Map.file.wWidth+7)/8;
	int dy = (g_Map.file.wHeight+7)/8;
	if (dx <= 0 || dy <= 0 || fseek(fp, 0, SEEK_END) != 0)
	{
		fclose(fp);
		return FALSE;
	}
	const long fileSize = ftell(fp);
	if (fileSize < 0 || fileSize % sizeof(LegacySkillMapRecord) != 0 ||
		fseek(fp, 0, SEEK_SET) != 0)
	{
		g_DBGLog.Log(LOG_LV1, "[SKB] Invalid file length: %s (%ld)", temp, fileSize);
		fclose(fp);
		return FALSE;
	}

	while (c < fileSize / sizeof(LegacySkillMapRecord))
	{
		LegacySkillMapRecord disk;
		if (fread(&disk, sizeof(disk), 1, fp) != 1 ||
			!IsValidLegacySkillMapRecord(disk, g_Map.file.wWidth, g_Map.file.wHeight))
		{
			g_DBGLog.Log(LOG_LV1, "[SKB] Invalid record %d: %s", c, temp);
			fclose(fp);
			DeleteAllSkillData();
			return FALSE;
		}
		DecodeLegacySkillMapRecord(disk, st);

		int x = (int)(st.x / dx);
		int y = (int)(st.y / dy);
		if (x < 0 || x >= 8 || y < 0 || y >= 8)
		{
			fclose(fp);
			DeleteAllSkillData();
			return FALSE;
		}
		TileMap[ st.x ][ st.y ].attr_skill = 1;
		AddSkill( &Header[ x][y], &st );
		c++;
	}

	fclose(fp);

	return TRUE;
}


void MapSkillTool_Attr( int mx, int my )		//秦寸 鸥老狼 扁贱 加己蔼 劝己 咯何 悸泼窃荐
{
	if( mx < 0 ) return;
	if( my < 0 ) return;

	if( g_Map.file.wWidth <= mx ) return;
	if( g_Map.file.wHeight <= my ) return;
	
	LPTILE t = &TileMap[ mx][my];

	if( g_MapSkillTable.skillno!=0 )
	{
		if( t->attr_skill==FALSE )		//秦寸 鸥老狼 扁贱 加己阑 弥檬肺 悸泼窍绰 版快
		{
			t->attr_skill = 1;
			AddSkill( &Header[(int)(mx/(int)((g_Map.file.wWidth+7)/8))][(int)(my/(int)((g_Map.file.wHeight+7)/8))], &g_MapSkillTable);
		}
		else							//秦寸 鸥老狼 扁贱 加己俊 捞固 加己捞 悸泼登绢 乐绰 版快
		{
			lpMAPSKILLTABLE	result;
			result=FindSkill( &Header[(int)(mx/(int)((g_Map.file.wWidth+7)/8))][(int)(my/(int)((g_Map.file.wHeight+7)/8))], mx, my);
			if(result==NULL)		return;		//弊繁 老篮 绝摆瘤父 秦寸谅钎蔼阑 爱绰 傅农府胶飘 绝绰 版快
	
			result->x = g_MapSkillTable.x;
			result->y = g_MapSkillTable.y;
			result->type = g_MapSkillTable.type;
			result->skillno = g_MapSkillTable.skillno;

			if(result->skillno == TOOL_DONTSKILL )
			{
				result->tile_Range	= 0;
				result->probability	= 0;
				result->type_Num	= 0;
				result->subType		= 0;
			}
			else if(result->skillno == TOOL_BUILDHOUSE)
			{
				result->tile_Range	= g_MapSkillTable.tile_Range;
				result->probability	= 0;
				result->type_Num	= 0;
				result->subType		= 0;
			}
			else
			{
				result->tile_Range	= g_MapSkillTable.tile_Range;
				result->probability	= g_MapSkillTable.probability;
				if(result->skillno == TOOL_NPC_GENER )
				{
					result->type_Num= g_MapSkillTable.type_Num;
					result->subType = 0;
				}
				else
				{
					result->type_Num= 0;
					result->subType	= g_MapSkillTable.subType;
				}
			}
		}
	}
	return;
}


void AddSkill( lpMAPSKILLTABLE *Header, lpMAPSKILLTABLE	lpST )		//傅农靛 府胶飘狼 赣府俊 嘿咯 持扁
{		
	lpMAPSKILLTABLE t, temp; 
	
	if( *Header == NULL )		//傅农靛 府胶飘 弥檬 积己
	{	
		MemAlloc( *Header, sizeof( MAPSKILLTABLE ));
		(*Header)->type			= lpST->type;
		(*Header)->x			= lpST->x; 
		(*Header)->y			= lpST->y; 
		(*Header)->skillno		= lpST->skillno;
		
		(*Header)->tile_Range	= lpST->tile_Range;
		(*Header)->probability	= lpST->probability;
		(*Header)->type_Num		= lpST->type_Num;
		(*Header)->subType		= lpST->subType;
		(*Header)->prev			= NULL;
		(*Header)->next			= NULL;
	}		
	else						//捞固 积己等 府胶飘俊 梅啊 楷搬/肺爹秦柯 单捞磐啊 乐阑 版快
	{	
		t = NULL;
		MemAlloc( t, sizeof( MAPSKILLTABLE ));

		t->type			= lpST->type;
		t->x			= lpST->x; 
		t->y			= lpST->y; 
		t->skillno		= lpST->skillno;
		
		t->tile_Range	= lpST->tile_Range;
		t->probability	= lpST->probability;
		t->type_Num		= lpST->type_Num;
		t->subType		= lpST->subType;

		temp = *Header ;
		temp->prev = t;
		t->next = temp;
		t->prev = NULL;
		*Header = t;
	}
	/*	
		if(lpST->skillno == TOOL_FARMING || lpST->skillno == TOOL_MINING || lpST->skillno == TOOL_FISHING ||
			lpST->skillno == TOOL_CHOPPING || lpST->skillno == TOOL_HUB )
		{
			t->tile_Range	= lpST->tile_Range;
			t->probability	= lpST->probability;
			t->type_Num		= 0;
			t->subType		= lpST->subType;
		}

		else if(lpST->skillno == TOOL_NPC_GENER )
		{
			t->tile_Range	= lpST->tile_Range;
			t->probability	= lpST->probability;
			t->type_Num		= lpST->type_Num;
			t->subType		= 0;
		}

		else if(lpST->skillno == TOOL_BUILDHOUSE )
		{
			t->tile_Range	= lpST->tile_Range;
			t->probability	= 0;
			t->type_Num		= 0;
			t->subType		= 0;
		}
		
		temp = *Header ;
		temp->prev = t;
		t->next = temp;
		t->prev = NULL;
		*Header = t;
	}	
	*/
}	
	
	
void DeleteSkill( lpMAPSKILLTABLE *Header, lpMAPSKILLTABLE f)
{	
	lpMAPSKILLTABLE  t = *Header;//g_lpMapSkillTable;
	
	while( t != NULL )
	{		
		if( t == f )
		{
			if( f == *Header )		//header啊 力芭登绰 版快
			{
				t = (*Header)->next;

				if(*Header!=NULL)
					MemFree( *Header );

				if(t!=NULL)
				{
					*Header = t;
					(*Header)->prev = NULL;
				}
				return;
			}
			else 
			{
				if ( t->prev != NULL )
				{
					t->prev->next = t->next;
				}
				if( t->next != NULL )
				{
					t->next->prev = t->prev;
				}

				MemFree( t );
				return;
			}
		}

		t= t->next;
	}
}	
	
//	甘篮 例措谅钎...
// 010314 KHS  
lpMAPSKILLTABLE FindSkill( lpMAPSKILLTABLE *H, int x, int y, int order )
{	
	int c = 0;
	lpMAPSKILLTABLE t;
	t = *H;
	while( t != NULL )
	{	
		if( t->x == x && t->y == y )
		{
			if( order == c ) return t;
			c++;
		}
		t = t->next;
	}		
	return NULL;
}	
	
	
///////////////////////////// 0724 lkh 荐沥 ///////////////////////////////
BOOL CALLBACK SkillToolproc( HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam )
{	
	char			temp[FILENAME_MAX];
	static int		map_X, map_Y;
	static POINT	checkON_Tile;
	int				i=0,j=0;
	static int		radio_Table;
	static int		skill_Tile_Range=DEF_SKILLTILE_RANGE;
	int				Farmming_Count;
	int				Mining_Count;
	int				Fishing_Count;
	int				Chopping_Count;
	int				Hub_Count;
	int				NPC_Count;
	int				NPCPositionCount;
	int				NPCEventCount;	
	int				NPCNoEventCount;
	static int		delete_Type;
	FILE			*fp;
	RECT			rect, grect;	
	
	int tempmonsterno;
	
	switch(Message)		
	{						
	case WM_INITDIALOG:	
		map_X=Mox/32;
		map_Y=Moy/32;
		sprintf (temp, "%d", g_MapSkillTable.skillno);
		Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//扁贱加己
	
		sprintf (temp, "%d", g_MapSkillTable.x);
		Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_TILE_X ), temp);	//急琶等 鸥老 辆谅钎		

		sprintf (temp, "%d", g_MapSkillTable.y);
		Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_TILE_Y ), temp);	//急琶等 鸥老 染谅钎

		sprintf (temp, "%d", g_MapSkillTable.tile_Range);
		Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);	//加己康氢捞 固摹绰 芭府(馆瘤抚)

		sprintf (temp, "%d", g_MapSkillTable.type_Num);
		Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_NPCNUM ), temp);	//急琶等 NPC 锅龋
		//< CSD-030419
		if (IsExistNpcSprNo(g_MapSkillTable.type_Num))
		{
			tempmonsterno = g_MapSkillTable.type_Num;
		}
		else
		{
			tempmonsterno = g_MapSkillTable.type_Num%100;
		}
		
		Edit_SetText(GetDlgItem(hDlg, IDC_MONSTER_NAME), g_infNpc[tempmonsterno].szName);
		//> CSD-030419
		
		sprintf (temp, "%d", g_MapSkillTable.probability);
		Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);	//扁贱狼 己傍 咯何 犬伏

		switch(delete_Type)
		{
		case 0:sprintf(temp, lan->OutputMessage(4,151));break;
		case 1:sprintf(temp, lan->OutputMessage(4,152));break;
		case 2:sprintf(temp,lan->OutputMessage(4,153) );break;
		case 3:sprintf(temp, lan->OutputMessage(4,154));break;
		case 4:sprintf(temp, lan->OutputMessage(4,155));break;
		case 5:sprintf(temp, lan->OutputMessage(4,156));break;
		case 6:sprintf(temp, lan->OutputMessage(4,157));break;
		case 7:sprintf(temp, lan->OutputMessage(4,158));break;
		}

		// 扼叼坷 滚瓢 眉农&免仿

		switch( g_MapSkillTable.skillno )
		{
			case TOOL_FARMING				 :  radio_Table = IDC_FARMING;		break;
			case TOOL_MINING				 :	radio_Table = IDC_MINING;		break;
			case TOOL_FISHING				 : 	radio_Table = IDC_FISHING;		break;
			case TOOL_CHOPPING				 :	radio_Table = IDC_CHOPPING;		break;
			case TOOL_HUB					 :	radio_Table = IDC_HUB;			break;
			case TOOL_NPC_GENER				 :  radio_Table = IDC_NPC_GENER;	break;
			case TOOL_DONTSKILL				 :	radio_Table = IDC_DONTSKILL;	break;
				//default							 :	radio_Table = NULL;
		}
		CheckRadioButton( hDlg, IDC_FARMING, IDC_DONTSKILL, radio_Table );

		switch( radio_Statue )
		{
			case 0							 :  radio_Statue = IDC_RADIO_ALL;		break;
			case 1	          				 :	radio_Statue = IDC_RADIO_FARM;		break;
			case 2           				 : 	radio_Statue = IDC_RADIO_MINE;		break;
			case 3            				 :	radio_Statue = IDC_RADIO_FISH;		break;
			case 4             				 :	radio_Statue = IDC_RADIO_CHOPPING;	break;
			case 5            				 :  radio_Statue = IDC_RADIO_HUB;		break;
			case 6             				 :	radio_Statue = IDC_RADIO_NPC;		break;
				//default							 :	radio_Table = NULL;
		}
		CheckRadioButton( hDlg, IDC_RADIO_ALL, IDC_RADIO_NPC, radio_Statue );

	GetWindowRect( hDlg, &rect );// 拳搁 坷弗率捞 嘿绢唱坷霸 窍妨绊 茄扒单 ... 绢痘霸 秦具 且瘤..
	GetWindowRect( g_hwndMain, &grect );// 拳搁 坷弗率捞 嘿绢唱坷霸 窍妨绊 茄扒单 ... 绢痘霸 秦具 且瘤..
	MoveWindow( hDlg, grect.right+1, 0, rect.right, rect.bottom, true);

		return TRUE;	

	case WM_PAINT:
		break;			

	case WM_CLOSE:
		tool_ID_SKILL_INPUT=0;
		break;
						
	case WM_COMMAND:	
		switch( LOWORD( wParam )) 
			{		
				case IDOK     :
						Edit_GetText( GetDlgItem(hDlg, IDC_VIEW_ATTRIB), temp, 9);
						g_MapSkillTable.skillno = atoi (temp );
						Edit_GetText( GetDlgItem(hDlg, IDC_INPUT_TILE_X), temp, 9);
						g_MapSkillTable.x = atoi (temp );
						Edit_GetText( GetDlgItem(hDlg, IDC_INPUT_TILE_Y), temp, 9);
						g_MapSkillTable.y = atoi (temp );
						Edit_GetText( GetDlgItem(hDlg, IDC_INPUT_RANGE), temp, 9);
						g_MapSkillTable.tile_Range = atoi (temp );
						Edit_GetText( GetDlgItem(hDlg, IDC_INPUT_PERCENT), temp, 9);
						g_MapSkillTable.probability = atoi (temp );
						Edit_GetText( GetDlgItem(hDlg, IDC_VIEW_NPCNUM), temp, 9);
						g_MapSkillTable.type_Num = atoi (temp );
						
						EndDialog(hDlg, LOWORD(wParam));
						tool_ID_SKILL_INPUT=0;
						return TRUE;

				case IDCANCEL :	
						EndDialog(hDlg, LOWORD(wParam));	
						tool_ID_SKILL_INPUT=0;
						return TRUE;
				
				case IDC_BUTTON_SAVE :

						if(YesOrNo( "SkilTable Save OK? ", "Warning" ) == IDYES )
						{
							sprintf( temp, "./Skill/%s.skb", MapName );
							fp = Fopen( temp, "wb" );
							if(fp)
							{
								int c=0;
								for( int a=0; a<8; a++)
									for(int b=0; b<8; b++)
									{
										lpMAPSKILLTABLE h=Header[a][b];
										while(h!=NULL)
										{
											fwrite(h, sizeof(MAPSKILLTABLE), 1, fp);
											c++;
											h=h->next;
										}
									}
								fclose(fp);
							}
							
							int step;
							int serial=1;

							sprintf( temp, "./Skill/%s.stx", MapName );
							fp = Fopen( temp, "wt" );
							if(fp)
							{
								SYSTEMTIME l;										
								GetLocalTime(&l);
								fprintf( fp, "\n MapName : %s \n", MapName );
								fprintf( fp, "   Date  : %4d.%2d.%2d %2d:%2d.%2d \n\n", l.wYear, l.wMonth, l.wDay, l.wHour, l.wMinute, l.wSecond );

								Farmming_Count=0; 
								Mining_Count=0; 
								Fishing_Count=0; 
								Chopping_Count=0; 
								Hub_Count=0; 
								NPC_Count=0;
								NPCPositionCount = 0;
								NPCEventCount = 0;
								NPCNoEventCount = 0;

								char temp[ FILENAME_MAX];
								sprintf( temp, "%s\n",lan->OutputMessage(4,161) );
								fprintf (fp, temp );

								for( step=1; step<8; step++)
								{
									for( int a=0; a<8; a++)
									{
										for(int b=0; b<8; b++)
										{
											lpMAPSKILLTABLE	h=Header[a][b];
											while(h!=NULL)
											{
												if(h->skillno==step)
												{
													fprintf(fp, "%3d	%3d	%3d	%3d	%3d	%3d	%3d	%3d\n",
													serial, h->skillno, h->x, h->y, h->tile_Range, h->probability, h->type_Num , h->subType );
													switch(h->skillno)
													{
													case 1:	Farmming_Count++;			break;
													case 2:	Mining_Count++;				break;
													case 3:	Fishing_Count++;			break;
													case 4:	Chopping_Count++;			break;
													case 5:	Hub_Count++;				break;
													case 6:	NPCPositionCount++; NPC_Count += h->tile_Range;	
															if( h->probability ) NPCEventCount ++;	else NPCNoEventCount++;
																break;
													}
													serial++;
												}
												h=h->next;
											}
										}	
									}
								}
								
								
								sprintf( temp, "%s\n", lan->OutputMessage(4,162) );
								fprintf(fp, temp,	Farmming_Count, Mining_Count, Fishing_Count, Chopping_Count, Hub_Count);
								sprintf( temp, "%s\n", lan->OutputMessage(4,163) );
								fprintf(fp, temp,	NPC_Count, NPCPositionCount, NPCEventCount, NPCNoEventCount );
									
								fclose(fp);
							}
						}	
							
						return TRUE;
							
				case IDC_MINER10:
					{	//< CSD-030419
						g_MapSkillTable.type_Num -= 10;

						if (g_MapSkillTable.type_Num < 0)
						{
							g_MapSkillTable.type_Num=0;
						}

						sprintf (temp, "%d", g_MapSkillTable.type_Num);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_NPCNUM ), temp);
						
						if (IsExistNpcSprNo(g_MapSkillTable.type_Num))
						{
							tempmonsterno = g_MapSkillTable.type_Num;
						}
						else
						{
							tempmonsterno = g_MapSkillTable.type_Num%100;
						}
						
						Edit_SetText (GetDlgItem(hDlg, IDC_MONSTER_NAME), g_infNpc[tempmonsterno].szName);
						return TRUE;
					}	//> CSD-030419
				case IDC_MINER1:
					{	//< CSD-030419
						g_MapSkillTable.type_Num -= 1;
						if(g_MapSkillTable.type_Num<0)	g_MapSkillTable.type_Num=0;
						sprintf (temp, "%d", g_MapSkillTable.type_Num);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_NPCNUM ), temp);
						
						if (IsExistNpcSprNo(g_MapSkillTable.type_Num))
						{
							tempmonsterno = g_MapSkillTable.type_Num;
						}
						else
						{
							tempmonsterno = g_MapSkillTable.type_Num%100;
						}
						
						Edit_SetText(GetDlgItem(hDlg, IDC_MONSTER_NAME ), g_infNpc[tempmonsterno].szName);
						return TRUE;
					}	//> CSD-030419
				case IDC_PLUS1:
					{	//< CSD-030419
						g_MapSkillTable.type_Num += 1;
						if(g_MapSkillTable.type_Num>599)	g_MapSkillTable.type_Num=599;
						sprintf (temp, "%d", g_MapSkillTable.type_Num);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_NPCNUM ), temp);
						
						if (IsExistNpcSprNo(g_MapSkillTable.type_Num))
						{
							tempmonsterno = g_MapSkillTable.type_Num;
						}
						else
						{
							tempmonsterno = g_MapSkillTable.type_Num%100;
						}
						
						Edit_SetText(GetDlgItem(hDlg, IDC_MONSTER_NAME), g_infNpc[tempmonsterno].szName);
						return TRUE;
					}	//> CSD-030419
				case IDC_PLUS10	:
					{	//< CSD-030419
						g_MapSkillTable.type_Num += 10;
						if(g_MapSkillTable.type_Num>599)	g_MapSkillTable.type_Num=599;
						sprintf (temp, "%d", g_MapSkillTable.type_Num);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_NPCNUM ), temp);
					
						if (IsExistNpcSprNo(g_MapSkillTable.type_Num))
						{
							tempmonsterno = g_MapSkillTable.type_Num;
						}
						else
						{
							tempmonsterno = g_MapSkillTable.type_Num%100;
						}
						
						Edit_SetText(GetDlgItem(hDlg, IDC_MONSTER_NAME), g_infNpc[tempmonsterno].szName);
						return TRUE;
					}	//> CSD-030419
				case IDC_BUTTON_UP :
						g_MapSkillTable.tile_Range++;
						//if(g_MapSkillTable.tile_Range>MAX_SKILLTILE_RANGE)	g_MapSkillTable.tile_Range=MAX_SKILLTILE_RANGE;
						sprintf (temp, "%d", g_MapSkillTable.tile_Range);
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						return TRUE;
				case IDC_BUTTON_DOWN: 
						g_MapSkillTable.tile_Range--;
						if(g_MapSkillTable.tile_Range<0)	g_MapSkillTable.tile_Range=0;
						sprintf (temp, "%d", g_MapSkillTable.tile_Range);
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						return TRUE;
				case IDC_MINER_PERCENT:
						g_MapSkillTable.probability -= 10;
						if(g_MapSkillTable.probability<0)	g_MapSkillTable.probability=0;
						sprintf (temp, "%d", g_MapSkillTable.probability);
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_PLUS_PERCENT:
						g_MapSkillTable.probability += 10;
						if(g_MapSkillTable.probability>100)	g_MapSkillTable.probability=100;
						sprintf (temp, "%d", g_MapSkillTable.probability);
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_FARMING:
						g_MapSkillTable.skillno = TOOL_FARMING;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//版累 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_MINING:
						g_MapSkillTable.skillno = TOOL_MINING;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//盲堡 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_FISHING:
						g_MapSkillTable.skillno = TOOL_FISHING;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//超矫 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_CHOPPING:
						g_MapSkillTable.skillno = TOOL_CHOPPING;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//国格 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_HUB:
						g_MapSkillTable.skillno = TOOL_HUB;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//距檬 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_NPC_GENER:
						g_MapSkillTable.skillno = TOOL_NPC_GENER;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//NPC 积己 器牢飘 加己
						g_MapSkillTable.tile_Range=1;
						sprintf (temp, "1");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_DONTSKILL:
						g_MapSkillTable.skillno = TOOL_DONTSKILL;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//DON'T SKILL 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_BUILD_HOUSE:
						g_MapSkillTable.skillno = TOOL_BUILDHOUSE;
						sprintf (temp, "%d", g_MapSkillTable.skillno);
						Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_ATTRIB ), temp);	//笼垄扁 加己
						g_MapSkillTable.tile_Range=0;
						sprintf (temp, "0");
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_RANGE ), temp);
						g_MapSkillTable.probability=0;
						Edit_SetText (GetDlgItem(hDlg, IDC_INPUT_PERCENT ), temp);
						return TRUE;
				case IDC_FARM_MINER:
						if(g_MapSkillTable.skillno==1)
						{
							g_MapSkillTable.subType --;
							if(g_MapSkillTable.subType<0)	g_MapSkillTable.subType=0;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBFARM ), temp);	//版累狼 辑宏->配剧加己
						}
						return TRUE;
				case IDC_FARM_PLUS:
						if(g_MapSkillTable.skillno==1)
						{
							g_MapSkillTable.subType ++;
							if(g_MapSkillTable.subType>20)	g_MapSkillTable.subType=20;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBFARM ), temp);	//版累狼 辑宏->配剧加己
						}
						return TRUE;
				case IDC_MINE_MINER:
						if(g_MapSkillTable.skillno==2)
						{
							g_MapSkillTable.subType --;
							if(g_MapSkillTable.subType<0)	g_MapSkillTable.subType=0;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBMINE ), temp);	//盲堡狼 辑宏->堡拱加己
						}
						return TRUE;
				case IDC_MINE_PLUS:
						if(g_MapSkillTable.skillno==2)
						{
							g_MapSkillTable.subType ++;
							if(g_MapSkillTable.subType>20)	g_MapSkillTable.subType=20;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBMINE ), temp);	//盲堡狼 辑宏->堡拱加己
						}
						return TRUE;
				case IDC_FISH_MINER:
						if(g_MapSkillTable.skillno==3)
						{
							g_MapSkillTable.subType --;
							if(g_MapSkillTable.subType<0)	g_MapSkillTable.subType=0;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBFISH ), temp);	//荤绰 拱绊扁 辆幅
						}
						return TRUE;
				case IDC_FISH_PLUS:
						if(g_MapSkillTable.skillno==3)
						{
							g_MapSkillTable.subType ++;
							if(g_MapSkillTable.subType>20)	g_MapSkillTable.subType=20;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBFISH ), temp);	//荤绰 拱绊扁 辆幅
						}
						return TRUE;
				case IDC_CUT_MINER:
						if(g_MapSkillTable.skillno==4)
						{
							g_MapSkillTable.subType --;
							if(g_MapSkillTable.subType<0)	g_MapSkillTable.subType=0;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBCUT ), temp);	//荤绰 拱绊扁 辆幅
						}
						return TRUE;
				case IDC_CUT_PLUS:
						if(g_MapSkillTable.skillno==4)
						{
							g_MapSkillTable.subType ++;
							if(g_MapSkillTable.subType>20)	g_MapSkillTable.subType=20;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBCUT ), temp);	//荤绰 拱绊扁 辆幅
						}
						return TRUE;
				case IDC_HUB_MINER:
						if(g_MapSkillTable.skillno==5)
						{
							g_MapSkillTable.subType --;
							if(g_MapSkillTable.subType<0)	g_MapSkillTable.subType=0;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBHUB ), temp);	//距檬盲秒狼 辑宏->距檬加己
						}
						return TRUE;
				case IDC_HUB_PLUS:
						if(g_MapSkillTable.skillno==5)
						{
							g_MapSkillTable.subType ++;
							if(g_MapSkillTable.subType>20)	g_MapSkillTable.subType=20;
							sprintf (temp, "%d", g_MapSkillTable.subType);
							Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SUBHUB ), temp);	//距檬盲秒狼 辑宏->距檬加己
						}
						return TRUE;

				case IDC_SCRIPT_MINER10:
						if(g_MapSkillTable.skillno == 6)
						{
							g_MapSkillTable.probability -= 10;		//NPC狼 胶农赋飘 锅龋
							if(g_MapSkillTable.probability<0)	g_MapSkillTable.probability=0;
							sprintf (temp, "%d", g_MapSkillTable.probability);
							Edit_SetText (GetDlgItem(hDlg, IDC_SCRIPT_NO ), temp);
						}
						return TRUE;
				case IDC_SCRIPT_MINER1:
						if(g_MapSkillTable.skillno == 6)
						{
							g_MapSkillTable.probability --;		//NPC狼 胶农赋飘 锅龋
							if(g_MapSkillTable.probability<0)	g_MapSkillTable.probability=0;
							sprintf (temp, "%d", g_MapSkillTable.probability);
							Edit_SetText (GetDlgItem(hDlg, IDC_SCRIPT_NO ), temp);
						}
						return TRUE;
				case IDC_SCRIPT_PLUS1:
						if(g_MapSkillTable.skillno == 6)
						{
							g_MapSkillTable.probability ++;		//NPC狼 胶农赋飘 锅龋
							if(g_MapSkillTable.probability>50)	g_MapSkillTable.probability=50;
							sprintf (temp, "%d", g_MapSkillTable.probability);
							Edit_SetText (GetDlgItem(hDlg, IDC_SCRIPT_NO ), temp);
						}
						return TRUE;
				case IDC_SCRIPT_PLUS10:
						if(g_MapSkillTable.skillno == 6)
						{
							g_MapSkillTable.probability += 10;		//NPC狼 胶农赋飘 锅龋
							if(g_MapSkillTable.probability>50)	g_MapSkillTable.probability=50;
							sprintf (temp, "%d", g_MapSkillTable.probability);
							Edit_SetText (GetDlgItem(hDlg, IDC_SCRIPT_NO ), temp);
						}
						return TRUE;
				case IDC_NPCNO_UP:			//NPC惯积荐
						if(g_MapSkillTable.skillno == 6)
						{
							g_MapSkillTable.tile_Range++;
							//if(g_MapSkillTable.tile_Range>10)	g_MapSkillTable.tile_Range=10;
							sprintf (temp, "%d", g_MapSkillTable.tile_Range);
							Edit_SetText (GetDlgItem(hDlg, IDC_GENER_NO ), temp);
						}
						return TRUE;
				case IDC_NPCNO_DOWN:		//NPC惯积荐
						if(g_MapSkillTable.skillno == 6)
						{
							g_MapSkillTable.tile_Range--;
							if(g_MapSkillTable.tile_Range<1)	g_MapSkillTable.tile_Range=1;
							sprintf (temp, "%d", g_MapSkillTable.tile_Range);
							Edit_SetText (GetDlgItem(hDlg, IDC_GENER_NO ), temp);
						}
						return TRUE;

				case IDC_RADIO_ALL:		radio_Statue=0;	return TRUE;
				case IDC_RADIO_FARM:	radio_Statue=1;	return TRUE;
				case IDC_RADIO_MINE:	radio_Statue=2;	return TRUE;
				case IDC_RADIO_FISH:	radio_Statue=3;	return TRUE;
				case IDC_RADIO_CHOPPING:radio_Statue=4;	return TRUE;
				case IDC_RADIO_HUB:		radio_Statue=5;	return TRUE;
				case IDC_RADIO_NPC:		radio_Statue=6;	return TRUE;
				case IDC_RADIO_HOUSE:	radio_Statue=7;	return TRUE;

				case IDC_DELETE_UP:
						delete_Type++;
						if(delete_Type>8)	delete_Type=8;
						
						switch(delete_Type)
						{
						case 1:
							sprintf(temp, lan->OutputMessage(4,151));//lsw
							break;
						case 2:
							sprintf(temp, lan->OutputMessage(4,152));
							break;
						case 3:
							sprintf(temp, lan->OutputMessage(4,153));
							break;
						case 4:
							sprintf(temp, lan->OutputMessage(4,154));
							break;
						case 5:
							sprintf(temp, lan->OutputMessage(4,155));
							break;
						case 6:
							sprintf(temp, lan->OutputMessage(4,156));
							break;
						case 7:
							sprintf(temp, lan->OutputMessage(4,157));
							break;
						case 8:
							sprintf(temp, lan->OutputMessage(4,158));//lsw
							break;
						}
						Edit_SetText (GetDlgItem(hDlg, IDC_DELETE_TYPE ), temp);
						return TRUE;

				case IDC_DELETE_DOWN:
						delete_Type--;
						if(delete_Type<1)	delete_Type=1;
						
						switch(delete_Type)
						{
						case 1:
							sprintf(temp, lan->OutputMessage(4,151));	break;
						case 2:
							sprintf(temp, lan->OutputMessage(4,152));	break;
						case 3:
							sprintf(temp, lan->OutputMessage(4,153));	break;
						case 4:
							sprintf(temp, lan->OutputMessage(4,154));	break;
						case 5:
							sprintf(temp, lan->OutputMessage(4,155));	break;
						case 6:
							sprintf(temp, lan->OutputMessage(4,156));	break;
						case 7:
							sprintf(temp, lan->OutputMessage(4,157));	break;
						case 8:
							sprintf(temp, lan->OutputMessage(4,158));	break;
						}
						Edit_SetText (GetDlgItem(hDlg, IDC_DELETE_TYPE ), temp);
						return TRUE;

				case IDC_DELETE_BUTTON:
					if(YesOrNo( "Select Tile infomation delete?", "warning" ) == IDYES )//lsw
						DeleteAllType(delete_Type);
					return TRUE;

			}			
			break;		
	}				
					
	return FALSE;	
}



void tool_MyHouseLBU( WPARAM wParam, LPARAM lParam )
{
	RECT	rect,grect;
////////////////////////////////// 捞痹绕 1124 郴笼 矫胶袍 Tool ///////////////////////////////
		if(tool_ID_INPUT_MYHOUSE==1 && g_DragMouse.sx!=0 && g_MyhouseDlgOpen==0)
		{
			InputMyhouseHdlg = CreateDialog( g_hInstance, MAKEINTRESOURCE( IDD_INPUT_MYHOUSE ), g_hwndMain, (DLGPROC)MyhouseToolproc );
			ShowWindow( InputMyhouseHdlg, SW_HIDE);
			int ScreenX = GetSystemMetrics(SM_CXSCREEN);
			GetWindowRect( InputMyhouseHdlg, &rect );// 拳搁 坷弗率捞 嘿绢唱坷霸 窍妨绊 茄扒单 ... 绢痘霸 秦具 且瘤..
			GetWindowRect( g_hwndMain, &grect );// 拳搁 坷弗率捞 嘿绢唱坷霸 窍妨绊 茄扒单 ... 绢痘霸 秦具 且瘤..
			ShowWindow( InputMyhouseHdlg, SW_SHOW);
			g_MyhouseDlgOpen=1;
		}
///////////////////////////////////////////////////////////////////////////////////////////////
}



void tool_MyHouseLBD( WPARAM wParam, LPARAM lParam )
{
	g_DragMouse.sx=LOWORD(lParam)+Mapx;
	g_DragMouse.ex=LOWORD(lParam)+Mapx;
	g_DragMouse.sy=HIWORD(lParam)+Mapy;
	g_DragMouse.ey=HIWORD(lParam)+Mapy;

	SetCapture( g_hwndMain);
	if(tool_ID_OUTPUT_MYHOUSE)
		PutMyhouse((LOWORD(lParam)+Mapx)/TILE_SIZE, (HIWORD(lParam)+Mapy)/TILE_SIZE);
}











void CheckHouseObjectEdit ( HWND hwnd, HINSTANCE hInstance)
{	
	//HINSTANCE  hinst;		//error?
	hInstance = LoadLibrary("riched32.dll");
	DialogBox( hInstance, MAKEINTRESOURCE( IDD_INPUT_MYHOUSE ), NULL, (DLGPROC)MyhouseToolproc );
	FreeLibrary( hInstance /*hinst*/ );
}

BOOL CALLBACK MyhouseToolproc( HWND hDlg, UINT Message, WPARAM wParam, LPARAM lParam )
{
	char			temp[FILENAME_MAX];
	static int		map_X, map_Y;
	RECT rect, grect;
	static bool lButtonDown=false;
	int a,b;

	switch(Message)
	{
	case WM_INITDIALOG:
		/*
		map_X=Mox/32;
		map_Y=Moy/32;

		sprintf (temp, "%d", g_MyhouseTool.sx);
		Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SX ), temp);
		sprintf (temp, "%d", g_MyhouseTool.sy);
		Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_SY ), temp);
		sprintf (temp, "%d", g_MyhouseTool.ex);
		Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_EX ), temp);
		sprintf (temp, "%d", g_MyhouseTool.ey);
		Edit_SetText (GetDlgItem(hDlg, IDC_VIEW_EY ), temp);
		*/

		//CheckHouseObjectEdit = GetDlgItem(hDlg, IDC_OBJECT_EDIT );

		GetWindowRect( hDlg, &rect );// 拳搁 坷弗率捞 嘿绢唱坷霸 窍妨绊 茄扒单 ... 绢痘霸 秦具 且瘤..
		GetWindowRect( g_hwndMain, &grect );// 拳搁 坷弗率捞 嘿绢唱坷霸 窍妨绊 茄扒单 ... 绢痘霸 秦具 且瘤..
		MoveWindow( hDlg, grect.right+1, 0, rect.right, rect.bottom, true);
		return TRUE;

	case WM_CLOSE:
		tool_ID_INPUT_MYHOUSE=0;
		g_MyhouseDlgOpen=0;
		memset(&g_DragMouse, 0 , sizeof(DRAGMOUSE));
		break;

	case WM_COMMAND:
		switch( LOWORD( wParam ))
		{
			case IDOK:
				EndDialog(hDlg, LOWORD(wParam));
				tool_ID_INPUT_MYHOUSE=0;
				g_MyhouseDlgOpen=0;
				
				memset(&g_DragMouse, 0 , sizeof(DRAGMOUSE));
				return TRUE;
			case IDCANCEL:	
				EndDialog(hDlg, LOWORD(wParam));
				tool_ID_INPUT_MYHOUSE=0;
				g_MyhouseDlgOpen=0;
				memset(&g_DragMouse, 0 , sizeof(DRAGMOUSE));
				return TRUE;
			case IDC_SAVE:
				if(YesOrNo( "郴笼 矫胶袍 鸥老(捞固瘤) Table阑 历厘钦聪促. ", "犬牢" ) == IDYES )
				{
					FILE*		fp;
					TILE*		temp_Tile;
					int			object_Count=0;
//					MYHOUSETOOL Myhouse; //TileMap

					sprintf( temp, "./map/%s.mhb", MapName );	//犬厘磊->My House Binery
					fp = Fopen( temp, "wb" );
					if(fp)
					{
						/////////////////////////// 庆歹何盒 /////////////////////////
						int length_X=g_MyhouseTool.ex-g_MyhouseTool.sx;	//伎泛飘窍咯 磊弗 汗荐鸥老狼 辆鸥老 辨捞
						fwrite(&length_X, sizeof(int), 1, fp);
						int length_Y=g_MyhouseTool.ey-g_MyhouseTool.sy; //伎泛飘窍咯 磊弗 汗荐鸥老狼 染鸥老 辨捞
						fwrite(&length_Y, sizeof(int), 1, fp);
						int tile_Num=length_X*length_Y;					//伎泛飘等 鸥老狼 醚肮荐
						fwrite(&tile_Num, sizeof(int), 1, fp);
						
						//if(fgetpos(fp, temp_fp)==0)	
						//	;							//付瘤阜俊 坷宏璃飘狼 醚 肮荐甫 眉农茄饶 促矫 悸泼窍扁 困秦
						fwrite(&object_Count, sizeof(int), 1, fp);		//伎泛飘等 鸥老 困俊 困摹茄 醚 坷宏璃飘荐 
						///////////////////////////////////////////////////////////////

						//////////////////////////// 鸥老狼 沥焊 悸泼 //////////////////////////
						for( a=g_MyhouseTool.sx; a<=g_MyhouseTool.ex; a++)
							for( b=g_MyhouseTool.sy; b<=g_MyhouseTool.ey; b++)
							{
								temp_Tile=&TileMap[a][b];
								fwrite(temp_Tile, sizeof(TILE), 1, fp);
							}
						////////////////////////////////////////////////////////////////////////

						/////////////////////////// 坷宏璃飘狼 沥焊 悸泼 ///////////////////////////
						for(int i = 0 ; i < TotalMapObject ; i++ )
						{
							if( g_MyhouseTool.sx <= (Mo[i].x/TILE_SIZE) && g_MyhouseTool.ex >= (Mo[i].x/TILE_SIZE) &&
								g_MyhouseTool.sy <= (Mo[i].y/TILE_SIZE) && g_MyhouseTool.ey >= (Mo[i].y/TILE_SIZE) )
							{
								int temp_Mox=Mo[i].x;
								int temp_Moy=Mo[i].y;
								Mo[i].x -= (g_MyhouseTool.sx*TILE_SIZE);	//肋赴 鸥老狼 惑措 谅钎肺 傈券
								Mo[i].y -= (g_MyhouseTool.sy*TILE_SIZE);
								
								fwrite( &Mo[i], sizeof( MAPOBJECT ), 1, fp );
								Mo[i].x = temp_Mox;
								Mo[i].y = temp_Moy;
								object_Count++;
							}
						}
						///////////////////////////////////////////////////////////////////////////
						
						/*
						/////////////////////////// 瘤贺 鸥老 沥焊 悸泼 ///////////////////////////
						for(a=g_MyhouseTool.sx; a<=g_MyhouseTool.ex; a++)
							for(b=g_MyhouseTool.sy; b<=g_MyhouseTool.ey; b++)
							{
								temp_Tile=&TileMap[a][b];
								temp_Tile->attr_room;

									if ( TileMap[ a ][ b ].attr_room == 1 )		//瘤贺加己狼 鸥老捞哥 瘤贺捞 劝己拳(瘤贺捞 啊府绊 乐绰 版快)
									{
										LPEVENTLIST		lpEventList;
										LPROOFGROUP		lpRoofGroup;
										LPROOF			lpRoof;

										lpEventList = FindEventList( &g_EventListHeaderRoom, ( WORD )a, ( WORD )b );
										if ( lpEventList != NULL )
										{
											lpRoofGroup = FindRoofGroup( &g_RoofHeader, lpEventList->index );	//瘤贺 府胶飘俊辑 find
											if ( lpRoofGroup != NULL )
											{
												lpRoof = lpRoofGroup->lpFirst;
												while ( lpRoof )
												{
													TileMap[ lpRoof->x ][ lpRoof->y ].show_roof = 1;
													lpRoof = lpRoof->lpNext;
												}
											}
										}
									}
									else					//瘤贺加己 鸥老捞哥 瘤贺捞 厚劝己拳登绢 乐绰 版快(堆脖捞 凯妨乐绰 版快)
									{
										LPEVENTLIST		lpEventList;
										LPROOFGROUP		lpRoofGroup;
										LPROOF			lpRoof;

										lpEventList = FindEventList( &g_EventListHeaderRoom, ( WORD )a, ( WORD )b );
										if ( lpEventList != NULL )
										{
											lpRoofGroup = FindRoofGroup( &g_RoofHeader, lpEventList->index );
											if ( lpRoofGroup != NULL )
											{
												lpRoof = lpRoofGroup->lpFirst;
												while ( lpRoof )
												{
													TileMap[ lpRoof->x ][ lpRoof->y ].show_roof = 0;

													lpRoof = lpRoof->lpNext;
												}
											}
										}
									}
								
							}
							*/
						///////////////////////////////////////////////////////////////////////////

						fseek(fp, 3*sizeof(int), SEEK_SET);
						fwrite(&object_Count, sizeof(int), 1, fp);
						fseek(fp, 0, SEEK_END);
						fclose(fp);
						g_MyhouseTool.object_Num=object_Count;
						sprintf (temp, "%d", g_MyhouseTool.object_Num);
						Edit_SetText (GetDlgItem(InputMyhouseHdlg, IDC_VIEW_TILENUM ), temp);
					}
				}
				return TRUE;
			
			//case IDC_OBJECT_EDIT:		richEdit 皋矫瘤 贸府
			//	return TRUE;
		}				
	}				
					
	return FALSE;	
}

void PutMyhouse(int x, int y)		// 窜 牢磊 x,y绰 例措 鸥老谅钎拌肺 逞绢客具 窃
{
	char			temp[FILENAME_MAX];
	static int		map_X, map_Y;
///	RECT			rect, grect;
	static bool		lButtonDown=false;
	int				a,b;
	FILE*			fp;
	TILE			temp_Tile;
	int				object_Count=0;
//	MYHOUSETOOL		Myhouse; //TileMap
	MAPOBJECT		temp_Object;

	sprintf( temp, "./map/%s.mhb", MapName );	//犬厘磊->My House Binery
	fp = Fopen( temp, "rb" );
	if(fp)
	{
		//////////////////////////////// 庆歹何盒 //////////////////////////////
		int length_X;//=g_MyhouseTool.ex-g_MyhouseTool.sx;	//伎泛飘窍咯 磊弗 汗荐鸥老狼 辆鸥老 辨捞
		fread(&length_X, sizeof(int), 1, fp);
		int length_Y;//=g_MyhouseTool.ey-g_MyhouseTool.sy; //伎泛飘窍咯 磊弗 汗荐鸥老狼 染鸥老 辨捞
		fread(&length_Y, sizeof(int), 1, fp);
		int tile_Num;//=length_X*length_Y;					//伎泛飘等 鸥老狼 醚肮荐
		fread(&tile_Num, sizeof(int), 1, fp);
				
		fread(&object_Count, sizeof(int), 1, fp);		//伎泛飘等 鸥老 困俊 困摹茄 醚 坷宏璃飘荐 
		////////////////////////////////////////////////////////////////////////

		//////////////////////////// 鸥老狼 沥焊 悸泼 //////////////////////////
		for( a=x; a<=x+length_X; a++)
			for( b=y; b<=y+length_Y; b++)
			{
				fread(&temp_Tile, sizeof(TILE), 1, fp);
				TileMap[a][b]=temp_Tile;
			}
		////////////////////////////////////////////////////////////////////////

		/////////////////////////// 坷宏璃飘狼 沥焊 悸泼 ///////////////////////////
		for(int i = 0 ; i < object_Count ; i++ )
		{
			fread(&temp_Object, sizeof(MAPOBJECT), 1, fp);
			temp_Object.x += x;		//offset 谅钎 冻备扁->32(TILE_SIZE)甫 蚌秦具瘤
			temp_Object.x *= TILE_SIZE;
			temp_Object.y += y;
			temp_Object.y *= TILE_SIZE;
			//temp_Object.id += MapObjectLevel;			//秦寸 饭骇狼 坷宏璃飘 绊蜡锅龋 可悸父怒 歹窃(1000窜困)
			Mo[TotalMapObject+i]=temp_Object;
		}
	}
}
//------------------------------------------------




void GetEffect2Pix(void)
{
	// EffectDataLoad...
	
	int j=0;
	char filename[141][30];

	FILE* file;
	int temp = 0;
	int buff = 0;
	char buf[30];

//	buf = (char*) calloc(30, sizeof(char));
//	filename = (char*) calloc( 141, sizeof(char)*30);

	char  DataPath[MAX_PATH]="./effect/list.txt";
	
	file = Fopen( DataPath, "rt" );			//read only+text file
	
	if(file)
	{
		for(int i=0; i<141; i++)
		{
			fscanf( file, "%s\n", buf);
//			strcpy(filename[i], buf);
			sprintf(filename[i], "./effect/%s",buf);
		}
		fclose( file );
	}

	for(int i=1; i<142;i++)
	{
		if(LoadEffectSprite(i, filename[i-1]))
		{
			for(int j=0; j<Effspr[i].Count; j++)
			{
				g_DestBackBuf = GetSurfacePointer( g_DirectDrawInfo.lpDirectDrawSurfaceBack );
				EraseScreen( &g_DirectDrawInfo, RGB( 0x00, 0x00, 0x00 ) );

				PutCompressedImage( 320, 240, &Effspr[i].EffSpr[j] );

				FlipScreen( &g_DirectDrawInfo );
				CaptureScreen();
			}
		}
	}
}


//--------------------------------------------------
		
void DeleteAllType(int delete_Type)
{
	lpMAPSKILLTABLE	temp;
	for( int i=0; i<8; i++)
	{
		for( int j=0; j<8; j++)
		{
			lpMAPSKILLTABLE temp_Table=Header[i][j];
			while(temp_Table!=NULL)
			{
				if(temp_Table->skillno == delete_Type)
				{
					TileMap[i][j].attr_skill=0;
					//temp_Skill = FindSkill(g_MapSkillTable, mx, my);
					temp=temp_Table->next;
					DeleteSkill( &Header[i][j], temp_Table);
					temp_Table = temp;
				}
				else temp_Table = temp_Table->next;

			}
		}
	}
}
