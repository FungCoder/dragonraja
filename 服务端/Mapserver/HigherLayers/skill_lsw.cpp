#include "..\stdafx.h"
#include "DefaultHeader.h"
#include "DR_NETWORK.h"
#include "CITEM.h"
#include "skill_lsw.h"
#include "MenuDefine.h"
#include "Auction.h"
#include "GmMgr.h"
#include "Menuserver.h"
#include "MailMgr.h"
#include "UserManager.h"
#include "LogManager.h"	// BBD 040308

extern void RecvCMD_COMFORM_RESET_ABILITY(t_packet &packet);//020820 lsw
extern void RecvCMD_TRADE_MODE_CHANGE(const int iCn, t_packet *p);//030127 lsw //Trade On/Off 悸泼

extern HDBC  hDBC;

t_raregroup			RareGroup[5][MAX_RARE_GROUP];
t_rareprobability	Rareprobability[MAX_RARE_PROBABILIY];
t_ItemFunction		ItemFunction[MAX_ITEM_FUNCTION];

int CanLearnSkill( CHARLIST *ch, int skill, int check_ability )
{
	if( !ch ) return 0;
	int inc = SkillTbl[skill].inclusive;	// 魄窜风凭 
	if( ch->Skill[skill] ) return 2;		// 捞固 硅奎澜
	if( check_ability && ch->skillexp[inc].skillexp < (DWORD)SkillTbl[skill].need_exp ) return 3;	// 瓷仿捞 葛磊恩
	
	return 1;		// 硅快扁俊 啊瓷窃.
}

void SendLearnSkillOk( int skill_no, t_connection c[], int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_LEARN_SKILL_OK;
	{
		packet.u.kein.server_learn_skill_ok.skill_no = skill_no;
	}
	packet.h.header.size = sizeof( k_server_learn_skill_ok );
	QueuePacket( c, cn, &packet, 1 );		// 努扼捞攫飘 傈价	罐篮芭 绊措肺 焊郴霖促.
}

void SendStartSkillExp( int inc, t_skillexp exp, t_connection c[], int cn )
{
	t_packet packet;
	packet.h.header.type = CMD_SKILL_EXP;
	{
		packet.u.kein.server_skill_exp.type = inc;
		packet.u.kein.server_skill_exp.exp = exp;
	}
	packet.h.header.size = sizeof( k_server_skill_exp );
	QueuePacket( c, cn, &packet, 1 );		// 努扼捞攫飘 傈价	罐篮芭 绊措肺 焊郴霖促.
}

int GetSkillMother( int kind, int skill_mother[], int max )
{
	memset( skill_mother, 0, sizeof( int )*max );
	int count = 0;
	for( int i=0; i<MAX_SKILLMAIN; i++ )
	{
		if( SkillTbl[i].MotherSkillType == kind )
		{
			skill_mother[count++] = i;
		}
	}
	return count;
}

void RecvLearnSkill( t_client_learn_skill *p, t_connection c[], int cn  )		//###1213 荐沥
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) return;

//	ch->Money = GetMoneyByItem( ch );

	int inc = SkillTbl[p->skillno].inclusive;
	int money = SkillTbl[p->skillno].money;

	int skill_mother[20];
	int count_max = GetSkillMother( inc, skill_mother, 20 );

	if( ch->Money < (DWORD)money )
	{
		SendServerResult( CM_LEARNSKILL, 4, cn );
		return; // 捣捞 何练秦...
	}

	int befor_enough = 1;		// 傈窜拌 胶懦阑 肪 50捞惑 劳躯绰啊?
	int check_ability=1;		// 瓷仿 眉农甫 秦具 窍绰啊?

	switch (inc)
	{
		case COOKING:
			{
				if( ch->skillexp[FARMING].skillexp/10000 < 50 && ch->skillexp[FISHING].skillexp/10000 < 50 ) befor_enough=0;
				break;
			}
		case BLACKSMITHY:
			{
				if( ch->skillexp[MINING].skillexp/10000 < 50 )	befor_enough=0;
				break;
			}
		case CANDLEMAKING:
			{
				if( ch->skillexp[FISHING].skillexp/10000 < 50 )	befor_enough=0;
				break;
			}
		case CARPENTRY:	
		case BOWCRAFTING:
			{
				if( ch->skillexp[CUTDOWN].skillexp/10000 < 50 ) befor_enough=0;
				break;
			}
		case TAILORING:
			{
				if( ch->skillexp[BUTCHERING].skillexp/10000 < 50 ) befor_enough=0;
				break;
			}
		case ALCHEMING:
			{
				if( ch->skillexp[HERBDIGGING].skillexp/10000 < 50 ) befor_enough=0;
				break;
			}
		case TAIMING : 
			{	//< CSD-030806
				if (ch->GetLevel() < 35 ) befor_enough = 0;
				check_ability = 0; // 歹捞惑 瓷仿 眉农绰 绝促.
				break;
			}	//> CSD-030806
		case ANIMAL_LORE:
			{
				if( ch->skillexp[TAIMING].skillexp/10000 < 40 ) befor_enough = 0;
				break;
			}
	}

	if( !befor_enough )
	{
		SendServerResult( CM_LEARNSKILL, 3, cn );
		return;
	}

	int ret_learn = 0;
	int error_learn=0;
	for( int i=0 ; i<count_max ; i++ ) 
	{
		int ret = CanLearnSkill( ch, skill_mother[i], check_ability );
		if( ret == 1 ) 
		{
			ch->Skill[skill_mother[i]] = true;
			SendLearnSkillOk(skill_mother[i], c, cn );
			ret_learn = 1;
		}
//		else ret = error_learn;			// Commented by chan78 at 2000/11/28
		else error_learn = ret;			// 001212 YGI
	}

	if( !ret_learn )
	{
		SendServerResult( CM_LEARNSKILL, error_learn, cn );	// 2:捞固 硅奎澜   3:瓷仿捞 何练窃.
		return;
	}
	SubtractMoney( money, ch );
	//SendSubtractMoney( money, cn );

	if( ch->skillexp[inc].skillexp < 50000 ) {ch->skillexp[inc].skillexp = 50000;}
	//011022 lsw >
	SkillMgr.SendSkillExp( inc, ch->skillexp[inc], cn);
	//011022 lsw <

	SendServerResult( CM_LEARNSKILL, 1, cn );
}

void RecvGmRegist( t_packet *p, short int cn, int makemode )
{
	CHARLIST *ch = CheckServerId( cn );
	if( !ch ) {return;}

	const int gm_list_index = p->u.kein.gm_regist.gm_list_index;
	if(0 > gm_list_index || MAX_GM_MAIN <= gm_list_index){return;}

	GM_QUEST *pGmMain = &g_GmMain[gm_list_index];

	const int iSkillNo = pGmMain->skill;

	const int ret = GMMgr.CanGmRegist( gm_list_index, ch );//pGmMain捞 1 何磐聪鳖
	if(0 > ret){return;}//021212 lsw //殿废 阂啊扼搁
	// 酒捞袍 昏力, 捣 昏力 棺 咯矾 贸府
	if(-1 == ::SubtractMoney( pGmMain->need_money, ch ))
	{
		return;//捣档 绝绰霸 你
	}
	for( int i=0; i<MAX_GM_NEED_ITEM_COUNT; i++ )
	{
		const int iItemNo = pGmMain->need_item[i];
		if( iItemNo )
		{
			::SendDeleteItemQuantity( ch, iItemNo, pGmMain->need_quantity[i] );		// 漂沥 酒捞袍阑 漂沥 肮荐父怒 昏力 茄促.
		}
	}

	switch(ret)
	{
	case 1:
	case 2:
	case 3:
		{
			ch->skillexp[iSkillNo].skillexp = 1000000;//011012 lsw
			ch->view_job = gm_list_index;// 焊咯临 流诀阑 GM栏肺 官槽促.
			{// 叼厚 历厘
			t_packet packet;
			packet.h.header.type = CMD_GM_REGIST;
			packet.u.kein.gm_regist_db.gm_list = gm_list_index;
			::strcpy( packet.u.kein.gm_regist_db.name , ch->Name );
			packet.h.header.size = sizeof( k_gm_regist_db );
			QueuePacket(connections, DB_DEMON, &packet, 1);
			}
		}break;
	case 4://扁己狼 版快绰 粱 促福促
		{//扁己篮 叼厚俊 历厘档 救茄促
			t_SkillExp3 *pExp = (t_SkillExp3*)(&ch->skillexp[iSkillNo]);
			pExp->skillType = gm_list_index;
			pExp->year = g_year;
			pExp->day = g_mon*30 +g_day+10;//10老阑 霖促
		}break;
	default:
		{
			return;
		}break;
	}//switch(ret)

	
	SkillMgr.SendSkillExp(iSkillNo,ch->skillexp[iSkillNo],ch->GetServerID());

	//010708 lsw 瘤骏 殿废苞 窃膊 胶懦捞 埃促.
	if(ret==2)
	{
		int iCheckJob = iSkillNo;
		
		int iMy2ndSkillNo = g_GmMain[gm_list_index].skill;
		int iResultGMSkillNumber =-1;
		
		if( ch->skillexp[iCheckJob].skillexp == 1000000)//011012 lsw
		{
			int ok =0;
			switch(iCheckJob)
			{
			case CARPENTRY		:	iResultGMSkillNumber = 106;	ok = 1;
				break;
			case BLACKSMITHY	:	iResultGMSkillNumber = 107;	ok = 1;
				break;
			case COOKING		:	iResultGMSkillNumber = 108;	ok = 1;
				break;
			case TAILORING		:	iResultGMSkillNumber = 109;	ok = 1;															
				break;
			case BOWCRAFTING	:	iResultGMSkillNumber = 110;	ok = 1;
				break;
			case ALCHEMING		:	iResultGMSkillNumber = 111;	ok = 1;
				break;
			case CANDLEMAKING	:	iResultGMSkillNumber = 112;	ok = 1;
				break;
			default:
				ok = 0;
				break;
			}
			if(	ok ==1)
			{
				ch->Skill[iResultGMSkillNumber] = true;
				::SendLearnSkillOk(iResultGMSkillNumber,connections,cn);
			}
		}
	}
	return;
}

const int Convert[2][5]={//020823 lsw
		{RARE_POWER,		RARE_POWER,		RARE_POWER,		RARE_VIGOR,			RARE_POWER},
		{RARE_VIGOR,		RARE_VIGOR,		RARE_VIGOR,		RARE_PIERCING,		RARE_PIERCING}};

bool CorrectRareKind(const int iResultAttr,const int iCompareAttr)
{
	RareMain *result = ((LPRareMain)(&iResultAttr));
	RareMain *compare = ((LPRareMain)(&iCompareAttr));

	const int iResultSok = result->soksung1;
	const int iTargetSok = compare->soksung1;
	
	if( result->grade != compare->grade){return false;}
	if( result->iHighLevel != compare->iHighLevel){return false;}
	if( result->soksung2 != compare->soksung2){return false;}
	if( result->soksung3 != compare->soksung3){return false;}
	if( result->IsDynamicRare != compare->IsDynamicRare){return false;}

	for(int i = 0; i < 5; i++)
	{
		if(	Convert [0][i] == iResultSok)
		{
			if(	Convert [1][i] == iTargetSok)
			{
				return true;
			}
		}
		else if(	Convert [1][i] == iResultSok)
		{
			if(	Convert [0][i] == iTargetSok)
			{
				return true;
			}
		}
	}
	return false;
}
void ConvertRare(RareMain &rare)
{
	for(int i = 0; i < 5; i++)
	{
		if(	Convert [1][i] == rare.soksung1)
		{
			rare.soksung1 = Convert [0][i];
		}
	}
}

void RecvCMD_SKILL_RARE_UPGRADE(const int cn, t_packet *p)
{
	const int iSkillNo	= p->u.SkillRareUpgrade.iSkillNo;
	LPCHARLIST ch		= ::CheckServerId(cn);
	if(!ch)
	{
		return;
	}

	if (!ch->IsPayedUser())
	{
		return;
	}


	float iSuccessRate = 0.000;
	ItemAttr SourceItem = GenerateItem(p->u.SkillRareUpgrade.SourceItem.item_no);

	if( !SourceItem.item_no ) return;

	int iSourceItemMuch = 0;//item_result 波啊 窍唱聪鳖

	unsigned int iResultSok	= 0;
	RareMain TempSokSung;

	CItem *	t = ItemUnit( SourceItem.item_no);
	SourceItem.attr[IATTR_RARE_MAIN] = p->u.SkillRareUpgrade.SourceItem.attr[IATTR_RARE_MAIN];
	memcpy(&TempSokSung,&p->u.SkillRareUpgrade.SourceItem.attr[IATTR_RARE_MAIN],sizeof(int));
	const bool bIsDynamicRare = TempSokSung.IsDynamicRare;
	if( !bIsDynamicRare)
	{
		ConvertRare(TempSokSung);
	}

	if( !t ){return;}

	WORD now =0, max =0, nowtemp=0, maxtemp=0;
	DWORD resultdur =0;
	

	int why = 0;
	int resourcelimit[MAX_UPGRADE_RARE] = {0,};
	
	int a[4] = {0,};//additem 1~4
	int al[4] = {0,};//additem Limit 1~4
	int HaveResource[4] = {0,};//鞘夸 酒捞袍捞 乐唱

	const int iKind = t->GetItemKind();
	const int AddItem[4] ={	ItemMutantKind[iKind].AddItem[0]/100,
							ItemMutantKind[iKind].AddItem[1]/100,
							ItemMutantKind[iKind].AddItem[2]/100,
							ItemMutantKind[iKind].AddItem[3]/100};

	POS pos;
	ItemAttr *i;
	int iIsCracked =0;
	for (int ti = 0; ti < MAX_UPGRADE_RARE ; ti++)//胶懦阑 矫档茄 酒捞袍阑 葛炼府 瘤款促
	{
		pos = p->u.SkillRareUpgrade.pos[ti];
		switch( pos.type )
		{
		case INV :		i = &ch->inv[pos.p1][pos.p2][pos.p3];	break;
		case EQUIP :	i = &ch->equip[pos.p3];					break;
		case QUICK :	i = &ch->quick[pos.p3];					break;
		default: continue; break;
		}
		
		LPRareMain pRare = ((LPRareMain)(&SourceItem.attr[IATTR_RARE_MAIN]));

		if( i->item_no )
		{
			if( SourceItem.item_no == i->item_no )//011224 lsw
			{
				if( 0 != pRare->iHighLevel )//饭绢 酒捞袍捞 酒聪搁(敲矾胶 酒捞袍捞芭唱 
				{
					iIsCracked = 1;//窍捞 酒捞袍 诀臂 窍妨搁
				}
				if( pRare->soksung2
				||	pRare->soksung3)
				{
					iIsCracked = 1;//钢萍饭绢 诀臂 窍妨搁 //0211120 lsw
				}

				if( SourceItem.attr[IATTR_RARE_MAIN] != i->attr[IATTR_RARE_MAIN] )
				{
					if(!CorrectRareKind(SourceItem.attr[IATTR_RARE_MAIN],i->attr[IATTR_RARE_MAIN]))
					{
						iIsCracked = 1;//MyLog(0,"诀臂 坷扁绰 吭绰单 钧蹲茄芭 酒捞袍父 哗狙绊 昏力..");	
					}
				}
				
				if(t->GetRbutton() ==DIVIDE_ITEM)
				{
					if(	DIVIDE_ITEM_UPGRDEABLE_MIN > i->attr[IATTR_DURATION]
					||	DIVIDE_ITEM_UPGRDEABLE_MAX < i->attr[IATTR_DURATION])
					{
						iIsCracked = 1;
					}
					max += i->attr[IATTR_DURATION];
				}
				else
				{
					GetItemDuration( *i, nowtemp, maxtemp);
					now += nowtemp;
					max += maxtemp;
				}
				resourcelimit[iSourceItemMuch]	= i->attr[IATTR_LIMIT]	;
				iSourceItemMuch++;
			}
			for(int addcount = 0;addcount <4;addcount++)
			{
				if(AddItem[addcount])//酒捞袍 逞滚啊 乐栏搁
				{
					if( AddItem[addcount]  == i->item_no )
					{
						if(!a[addcount])
						{
							a[addcount] = i->item_no;
							al[addcount]= i->attr[IATTR_LIMIT];
							HaveResource[addcount] = 1;
						}
					}
				}
				else// AddItem	//夸备 酒捞袍捞 绝扁 锭巩俊 府家胶 乐绰吧肺 眉农
				{
					HaveResource[addcount] = 1;
				}
			}
			::SendItemEventLog( i, cn, 0, SILT_RESOURCE_BY_SKILL, 1);	 //YGI acer
			::DeleteItem( i );//酒捞袍捞 乐菌扁 锭巩俊 瘤款促
		}
	}	

	if(iIsCracked){why |= 0x0001;}
	if(!iSourceItemMuch ){why |= 0x0002;}
	if(	!HaveResource[0]	||	!HaveResource[1]
	||	!HaveResource[2]	||	!HaveResource[3])
	{
		why |= 0x0004;
	}//夸备 窍绰 酒捞袍捞 绝促 (AddItem 捞 0 捞搁 磊悼栏肺 1眉农 窍聪鳖 || 甫 敬促

	
	if(!ItemGrade[TempSokSung.grade+1].iAble)
	{//弥措肺 诀弊饭捞靛 啊瓷茄 殿鞭牢啊?
		why |= 0x0008;//诀臂 阂啊
	}
	else
	{//殿鞭 诀弊饭捞靛啊 啊瓷
		//酒捞袍 逞滚客 俺荐甫 啊瘤绊
		const int iItemKind = t->GetItemKind();
		if (MAX_ITEM_MUTANT_KIND < iItemKind )
		{
			why |= 0x1000;//酒捞袍 墨牢靛 抛捞喉 坷滚 曼炼
		}
		
		int iKind = ItemMutantKind[iItemKind].upgrade_type ;
		if( !iKind ) 
		{
			iKind = 1;//快急 皋葛府 坷滚 曼炼 救窍霸 窍扁 困秦辑 1肺 霖促
			why |= 0x0100;
		}//诀弊饭捞靛 救登绰 酒捞袍 墨牢靛促
		
		const int PerpectMuch = ItemGrade[TempSokSung.grade+1].ResourceAmount[iKind-1];//1 老锭父 啊瓷 窍聪鳖 1阑 哗搁 0阑 牢郸胶肺 镜 荐 乐促

		if( PerpectMuch  <= 0) 
		{//夸备 肮荐啊 绝促..
			why |= 0x0010;
		}

		if( PerpectMuch )
		{//己傍伏阑 惶绰促
			iSuccessRate = float(iSourceItemMuch)/float(PerpectMuch);
			iSuccessRate	*=	100;

			if(iSuccessRate >= 100)
			{
				iSuccessRate = 100;
			}
			else
			{
				iSuccessRate /= 2;//肮荐啊 葛磊扼搁 1/5 肺 临咯 滚赴促
			}
		}
	}

	//菩哦 焊尘 霖厚
	t_packet packet;
	packet.h.header.type = CMD_SKILL_RARE_UPGRADE_RESULT;
	packet.h.header.size = sizeof( t_SkillRareUpgradeResult);
	ZeroMemory(&packet.u.SkillRareUpgradeResult,sizeof( t_SkillRareUpgradeResult));//皋葛府 檬扁拳 登菌澜

	const int iNowSuccess = (rand()%100);
	
	packet.u.SkillRareUpgradeResult.iSuccess = iNowSuccess;

	if( iSuccessRate > iNowSuccess)
	{	//己傍沁栏搁 甸绢哎 贸府		
		if (!why)//角菩 捞蜡啊 悸泼 登瘤 臼酒具 临 荐 乐促
		{
			if ( DIVIDE_ITEM == t->GetRbutton()) 
			{
				resultdur = (float)max/(float)iSourceItemMuch * (float)(1.100);
			}
			else
			{
				now = (float)now/(float)iSourceItemMuch * (float)(1.100);
				max = (float)max/(float)iSourceItemMuch * (float)(1.100);
				if(!now) {now = 1;}//捞凡 版快啊 乐衬??
				if(!max) {max = 1;}
				resultdur = MAKELONG( now, max);
			}

			if( ItemMgr.MakeRareAttr(iResultSok ,TempSokSung.grade+1,TempSokSung.soksung1,0,0,0,bIsDynamicRare))
			{
				SourceItem.attr[IATTR_RARE_MAIN]	=	iResultSok;
				SourceItem.attr[IATTR_DURATION]		=	resultdur;	
			}
			else
			{
				why |= 0x2000;//饭绢 炼扒捞 粱 捞惑茄啊焊促
			}

			packet.u.SkillRareUpgradeResult.iSuccess			=	100			;
			packet.u.SkillRareUpgradeResult.SourceItem			=	SourceItem	;
				
			if(!resultdur )
			{//郴备档啊 绝绢??
				why |= 0x0040;
			}

			int a,b,c;
			if(!why)// 茄锅歹 巩力 绝捞 吭促搁 
			{
				if( ::SearchInv( ch->inv, a, b, c ))//某腐磐 牢亥配府俊 酒捞袍阑 持绢霖促
				{
					POS pos;
					::SetItemPos( INV, a, b, c, &pos );
					ch->inv[a][b][c] = SourceItem;
					::SendServerEachItem( &pos , &SourceItem , ch->GetServerID());
					
					t_packet bbspacket;
					
					//傈眉 傍瘤
					bbspacket.h.header.type = CMD_RARE_UPGRADE_BBS_MAP_TO_MAP;

					::memcpy( bbspacket.u.RareUpgradeBBSMapToMap.name, ch->Name,20);
					bbspacket.h.header.size = sizeof(t_RareUpgradeBBSMapToMap );
					bbspacket.u.RareUpgradeBBSMapToMap.Item = SourceItem ;
					::SendNationPacket( &bbspacket, ch->name_status.nation );
					bbspacket.h.header.type = CMD_RARE_UPGRADE_BBS;
					g_pUserManager->SendPacket(&bbspacket); // CSD-CN-031213
				}
				else
				{
					why |= 0x0080;//磊府啊 绝窜促.. 富捞 登唱..
				}
			}
		}
	}
	else
	{
		why |=0x0020;//己傍伏 锭巩俊 角菩
	}

	Send_RareItemMakeLog( cn,SourceItem.item_no,0, 
		TempSokSung.grade+1,TempSokSung.soksung1,TempSokSung.soksung2,TempSokSung.soksung3,a[0],al[0],a[1],al[1],SourceItem.attr[IATTR_LIMIT],now,max,why,2002,
		resourcelimit[0],resourcelimit[1],resourcelimit[2],
		resourcelimit[3],resourcelimit[4],resourcelimit[5]);//搬苞 扁废

	::QueuePacket(connections, cn, &packet, 1);
	return;
}

void RecvCMD_SKILL_MASTER_MULTI_RARE_MAKE(const int cn, t_packet *p)
{
	t_SkillMasterMultiRareMake *pSMMRM = &p->u.Hwoa.rang.SMMultiRareMake;

	LPCHARLIST ch = ::CheckServerId(cn);
	if(!ch){return;}

	const int iSkillNo = pSMMRM->iSkillNo;
	const int iNowSkillNo = GMMgr.IsSkillMaster(ch);
	if(!iNowSkillNo){return;}//扁己 酒聪促//罐疽绰单 促福芭唱 窍促

	if (!ch->IsPayedUser())
	{
		return;
	}
	
	//酒捞袍 眉农
	ITEMATTR RecvSrcItem[2]; //罐篮芭
	RecvSrcItem[0]=	pSMMRM->MRS.SrcData[0].attr;
	RecvSrcItem[1]=	pSMMRM->MRS.SrcData[1].attr;

	ITEMATTR NowSrcItem[2];
	NowSrcItem[0] =	*GetItemByPOS(ch,pSMMRM->MRS.SrcData[0].pos);
	NowSrcItem[1] =	*GetItemByPOS(ch,pSMMRM->MRS.SrcData[1].pos);

	if(memcmp(RecvSrcItem,NowSrcItem,sizeof(ItemAttr))){return;}//辑滚客 努扼捞攫飘啊 促辅聪促.
	if(RecvSrcItem[0].item_no != RecvSrcItem[0].item_no){return;}//酒捞袍 锅龋啊 促福搁 钦磨 荐 绝嚼聪促.
	const int iResultItemNo = pSMMRM->MRS.SrcData[0].attr.item_no;
	CItem *	t = ItemUnit( iResultItemNo);
	if( !t ){return;}

	if(DIVIDE_ITEM == t->GetRbutton())
	{
		const int iPriCt = NowSrcItem[0].attr[IATTR_MUCH];
		const int iSecCt = NowSrcItem[1].attr[IATTR_MUCH];
		if(	DIVIDE_ITEM_UPGRDEABLE_MIN > iPriCt
		||	DIVIDE_ITEM_UPGRDEABLE_MAX < iPriCt
		||	DIVIDE_ITEM_UPGRDEABLE_MIN > iSecCt
		||	DIVIDE_ITEM_UPGRDEABLE_MAX < iSecCt )
		{
			::OutMessage(cn,2,266);//021120 lsw
			return;
		}
	}

	RareMain RecvRare1 = *(RareMain*)&NowSrcItem[0].attr[IATTR_RARE_MAIN];
	RareMain RecvRare2 = *(RareMain*)&NowSrcItem[1].attr[IATTR_RARE_MAIN];
	
	if(RecvRare1.soksung2 ||RecvRare1.soksung3
	|| RecvRare2.soksung2 ||RecvRare2.soksung3)
	{
		::OutMessage(cn,2,511);//021120 lsw
		return;
	}
	if(RecvRare1.soksung1 == RecvRare2.soksung1)
	{
		::OutMessage(cn,2,890);//021120 lsw
		return;
	}

	const int iGrade = (RecvRare1.grade+RecvRare2.grade)/2;//乞闭

	if( 5 < iGrade )
	{
		::OutMessage(cn,2,881);//021212 lsw
		return;
	}

	DWORD	resultdur =0;
	if( DIVIDE_ITEM == t->GetRbutton()) 
	{
		resultdur = (RecvSrcItem[0].attr[IATTR_MUCH]+RecvSrcItem[1].attr[IATTR_MUCH])*(float)(1.100)/2;
	}
	else
	{
		WORD	max1= 0, now1= 0;
		WORD	max2= 0, now2= 0;
		::GetItemDuration( RecvSrcItem[0], now1,max1);
		::GetItemDuration( RecvSrcItem[1], now2,max2);

		WORD now = (float)(now1+now2)*(float)(1.100)/2;
		WORD max = (float)(max1+max2)*(float)(1.100)/2;

		if(!now) {now = 1;}//捞凡 版快啊 乐衬??
		if(!max) {max = 1;}
		resultdur = MAKELONG( now, max);
	}
	ItemAttr ResultItem = ItemMgr.GenerateItem(iResultItemNo);//瘤鞭瞪 酒捞袍 悸泼
	ResultItem.attr[IATTR_MUCH] = resultdur;//郴备档 悸泼
	ItemMgr.MakeRareAttr(ResultItem.attr[IATTR_RARE_MAIN],iGrade,RecvRare1.soksung1,RecvRare2.soksung1,0,0,0);//钢萍 饭绢 父甸菌促

	bool iIsHaveResource = true;
	const ITEMMULTIRARE IMR = RareEM.GetIMR(iGrade);
{//犁丰狼 肮荐 眉农
	for(int xx= 0;MAX_IMR_FIELD_CT>xx;xx++)
	{//犁丰 眉农 茄促
		int iNationAddCt = 0;
		switch(ch->name_status.nation)
		{
		case NW_BY:{if(xx ==0){iNationAddCt = IMR.iAddCt;}}break;
		case NW_ZY:{if(xx ==1){iNationAddCt = IMR.iAddCt;}}break;
		case NW_YL:{if(xx ==2){iNationAddCt = IMR.iAddCt;}}break;
		}
		const int iNeedItemNo = IMR.aItemNo[xx];
		const int iNeedItemCt = IMR.aItemCt[xx] + iNationAddCt;
		if(iNeedItemNo && iNeedItemCt)
		{
			CItem *t = ::ItemUnit( iNeedItemNo );
			if(!t){continue;}
			const int iNowCt = ::CheckInventory(ch, iNeedItemNo, iNeedItemCt);//郴啊 盔茄 蔼俊辑 泅犁 搬苞甫 林聪鳖 -蔼捞 唱棵 荐 乐促
			if(0 > iNowCt)//肮荐 葛磊扼匙 ぱぱ;.
			{
				iIsHaveResource = false;
			}
		}
	}
}
	if(!iIsHaveResource)
	{
		::MyLog(1,"RecvCMD_SKILL_MASTER_MULTI_RARE_MAKE ,iIsHaveResource Check Error");
		return;
	}//犁丰 葛磊扼 摹况扼
{//犁丰甫 昏力

	POS &pos1 = pSMMRM->MRS.SrcData[0].pos;
	ItemAttr *pItem1 = GetItemByPOS(ch,pos1);
	
	::SendItemEventLog( pItem1, ch->GetServerID(), 0, SILT_MAKE_MULTI_RARE_RESOURCE, 1 ); //021209 lsw
	::DeleteItem(pItem1);
	::SendServerEachItem( &pos1, pItem1,ch->GetServerID());//焊郴扁

	POS &pos2 = pSMMRM->MRS.SrcData[1].pos;
	ItemAttr *pItem2 = GetItemByPOS(ch,pos2);
	
	::SendItemEventLog( pItem2, ch->GetServerID(), 0, SILT_MAKE_MULTI_RARE_RESOURCE, 1 ); //021209 lsw
	::DeleteItem(pItem2);
	::SendServerEachItem( &pos2, pItem2,ch->GetServerID());//焊郴扁

	for(int xx= 0;MAX_IMR_FIELD_CT>xx;xx++)//犁丰 猾促
	{	
		int iNationAddCt = 0;
		switch(ch->name_status.nation)
		{
		case NW_BY:{if(xx ==0){iNationAddCt = IMR.iAddCt;}}break;
		case NW_ZY:{if(xx ==1){iNationAddCt = IMR.iAddCt;}}break;
		case NW_YL:{if(xx ==2){iNationAddCt = IMR.iAddCt;}}break;
		}
		const int iNeedItemNo = IMR.aItemNo[xx];
		const int iNeedItemCt = IMR.aItemCt[xx] + iNationAddCt;
		if(iNeedItemNo && iNeedItemCt)
		{
			CItem *t = ItemUnit( iNeedItemNo );
			if(!t){continue;}
			const int iNowCt = ::CheckInventory(ch,iNeedItemNo,iNeedItemCt);//郴啊 盔茄 蔼俊辑 泅犁 搬苞甫 林聪鳖 -蔼捞 唱棵 荐 乐促
			if(0 <=iNowCt)//肮荐啊 嘎栏搁 瘤况扼
			{
				::SendDeleteItemQuantity( ch, iNeedItemNo, iNeedItemCt );		// 漂沥 酒捞袍阑 漂沥 肮荐父怒 昏力 茄促.
			}
		}
	}
}
	int a=0,b=0,c=0;
	if(::SearchInv(ch->inv,a,b,c))
	{
		const int iNow = rand()%100;
		if(45 > iNow)
		{
			POS pos;
			SetItemPos(INV,a,b,c,&pos);//pos悸泼 
			ITEMATTR *pItem = ::GetItemByPOS(ch,pos);//酒捞袍 林家 罐绊
			(*pItem) = ResultItem;
			::SendItemEventLog( pItem, ch->GetServerID(), 0, SILT_MAKE_MULTI_RARE_RESULT, 1 ); //021209 lsw
			::SendServerEachItem( &pos, pItem,ch->GetServerID());//焊郴扁
			::OutMessage(cn,2,901);//021120 lsw
		}
		else
		{//皋技瘤 焊辰促
			::OutMessage(cn,2,900);//021120 lsw
		}
	}
	else
	{//捞访 版快绰 绝绢夸
		
	}
	
/*
	Send_RareItemMakeLog( cn,SourceItem.item_no,0, 
		TempSokSung.grade+1,TempSokSung.soksung1,TempSokSung.soksung2,TempSokSung.soksung3,a[0],al[0],a[1],al[1],SourceItem.attr[IATTR_LIMIT],now,max,why,2002,
		resourcelimit[0],resourcelimit[1],resourcelimit[2],
		resourcelimit[3],resourcelimit[4],resourcelimit[5]);//搬苞 扁废
*/
//	::QueuePacket(connections, cn, &packet, 1);
	
	return;
}

//<! BBD 040308		单阁栏肺何磐狼 捞亥飘酒捞袍 瘤鞭 览翠矫 妮登绰 窃荐
void RecvCMD_EVENTRAREITEM_RES(const int cn, t_packet &p)
{
	const int MAX_EVENT_USER_FILED = 10;
	int nUserCn = p.u.Event_Item_List.nCn;
	
	CHARLIST *ch = CheckServerId(nUserCn);
	if(!ch)	// 某腐磐啊 蜡瓤茄啊?
	{
		// 酒捞袍捞 朝扼艾促. 肺弊 巢扁磊
		for(int i = 0; i < MAX_EVENT_USER_FILED; i++)
		{
			if(p.u.Event_Item_List.nIndex[i])
			{
				//i+1牢 牢郸胶狼 酒捞袍捞 朝酒艾促
				g_pLogManager->SaveEventItemLostLog(EILT_INVALID_CONNECTION, p.u.Event_Item_List.szChrName, i+1);
			}
		}
		return;
	}

	if(strcmp(p.u.Event_Item_List.szChrName, ch->Name))//捞抚捞 撇妨	
	{
		// 酒捞袍捞 朝扼艾促. 肺弊 巢扁磊.
		for(int i = 0; i < MAX_EVENT_USER_FILED; i++)
		{
			if(p.u.Event_Item_List.nIndex[i])
			{
				//i+1牢 牢郸胶狼 酒捞袍捞 朝酒艾促
				g_pLogManager->SaveEventItemLostLog(EILT_INVALID_NAME, p.u.Event_Item_List.szChrName, i+1);
			}
		}
		return;
	}

	// 牢亥狼 后沫 技扁
	int blankcount = 0;
	for(int a=0; a<3; a++)
	{
		for(int b=0; b<3; b++)
		{
			for(int c=0; c<8; c++)
			{
				if( !ch->inv[a][b][c].item_no )
				{
					blankcount++;
				}
			}
		}
	}
	
	if(blankcount < 10)	// 10沫 捞惑牢啊?
	{
		::OutMessage(ch,2,13);//烙矫,,牢亥配府 傍埃捞 面盒摹 臼嚼聪促.
		// 酒捞袍捞 朝扼艾促. 肺弊 巢扁磊.
		for(int i = 0; i < MAX_EVENT_USER_FILED; i++)
		{
			if(p.u.Event_Item_List.nIndex[i])
			{
				//i+1牢 牢郸胶狼 酒捞袍捞 朝酒艾促
				g_pLogManager->SaveEventItemLostLog(EILT_NOTENOUGH_INVENTORY, p.u.Event_Item_List.szChrName, i+1);
			}
		}
		return;
	}

	// 风橇甫 倒哥瘤鞭
	for(int i = 0; i < MAX_EVENT_USER_FILED; i++)
	{
		if(p.u.Event_Item_List.item[i].item_no)
		{
			// 酒捞袍 瘤鞭窍磊
			int a=0,b=0,c=0;
			if(::SearchInv(ch->inv,a,b,c))//傍埃捞 乐备唱
			{
				POS pos;
				SetItemPos(INV,a,b,c,&pos);//pos悸泼 
				ITEMATTR *pItem = ::GetItemByPOS(ch,pos);//酒捞袍 林家 罐绊
				(*pItem) = p.u.Event_Item_List.item[i];
				::SendServerEachItem( &pos, pItem,nUserCn);//焊郴扁
				// DB俊辑 积己登绢 柯 酒捞袍捞聪 DB俊辑 父甸菌促绊 肺弊 巢扁磊
				::SendItemEventLog( pItem, ch->GetServerID(), 0, SILT_MAKE_BY_DB, 1 );
			}
		}
	}
}
//> BBD 040308		单阁栏肺何磐狼 捞亥飘酒捞袍 瘤鞭 览翠矫 妮登绰 窃荐

int LoadRaregroup()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from rare_group order by no", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_RARE_GROUP) 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}

			retCode = SQLGetData(hStmt, 1, SQL_INTEGER	 ,	&RareGroup[0][i].no					, 0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_CHAR		 ,	RareGroup[0][i].name					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_CHAR		 ,	RareGroup[0][i].rareset[0].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[0].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[0].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 6, SQL_CHAR		 ,	RareGroup[0][i].rareset[1].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 7, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[1].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 8, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[1].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 9, SQL_CHAR		 ,	RareGroup[0][i].rareset[2].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 10, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[2].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 11, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[2].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 12, SQL_CHAR	 ,	RareGroup[0][i].rareset[3].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 13, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[3].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 14, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[3].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 15, SQL_CHAR	 ,	RareGroup[0][i].rareset[4].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 16, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[4].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 17, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[4].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 18, SQL_CHAR	 ,	RareGroup[0][i].rareset[5].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 19, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[5].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 20, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[5].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 21, SQL_CHAR	 ,	RareGroup[0][i].rareset[6].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 22, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[6].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 23, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[6].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 24, SQL_CHAR	 ,	RareGroup[0][i].rareset[7].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 25, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[7].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 26, SQL_INTEGER	 ,	&RareGroup[0][i].rareset[7].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 27, SQL_CHAR	 ,	RareGroup[0][i].group_buffer_1			, 40, &cbValue);
			retCode = SQLGetData(hStmt, 28, SQL_CHAR	 ,	RareGroup[0][i].group_buffer_2			, 40, &cbValue);
			retCode = SQLGetData(hStmt, 29, SQL_CHAR	 ,	RareGroup[0][i].group_buffer_3			, 40, &cbValue);
			retCode = SQLGetData(hStmt, 30, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_4		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 31, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_5		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 32, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_6		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 33, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_7		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 34, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_8		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 35, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_9		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 36, SQL_INTEGER	 ,	&RareGroup[0][i].group_buffer_10		, 0, &cbValue);

			i++;

			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 1;
}

void RareGroupSet()
{
	int i1 = 1,i2 = 1,	i3 = 1,	i4 = 1;
	for (;i1 < MAX_RARE_GROUP;i1++)
	{
		const int iGroupSet = RareGroup[1][i1].group_buffer_10;
		switch(iGroupSet)
		{
		case 2:
			{
				RareGroup[iGroupSet][i2] = RareGroup[1][i1];
				i2++;
			}break;
		case 3:
			{
				RareGroup[iGroupSet][i3] = RareGroup[1][i1];
				i3++;
			}break;
		case 4:
			{
				RareGroup[iGroupSet][i4] = RareGroup[1][i1];
				i4++;
			}break;
		default:
			continue;
			break;
		}
		
	}
}

int LoadItemFunctionGroup()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from Function_group order by no", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_RARE_GROUP) 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}

			retCode = SQLGetData(hStmt, 1, SQL_INTEGER	 ,	&RareGroup[1][i].no					, 0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_CHAR		 ,	RareGroup[1][i].name					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_CHAR		 ,	RareGroup[1][i].rareset[0].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[0].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[0].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 6, SQL_CHAR		 ,	RareGroup[1][i].rareset[1].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 7, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[1].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 8, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[1].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 9, SQL_CHAR		 ,	RareGroup[1][i].rareset[2].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 10, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[2].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 11, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[2].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 12, SQL_CHAR	 ,	RareGroup[1][i].rareset[3].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 13, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[3].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 14, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[3].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 15, SQL_CHAR	 ,	RareGroup[1][i].rareset[4].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 16, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[4].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 17, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[4].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 18, SQL_CHAR	 ,	RareGroup[1][i].rareset[5].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 19, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[5].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 20, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[5].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 21, SQL_CHAR	 ,	RareGroup[1][i].rareset[6].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 22, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[6].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 23, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[6].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 24, SQL_CHAR	 ,	RareGroup[1][i].rareset[7].rare					, 40, &cbValue);
			retCode = SQLGetData(hStmt, 25, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[7].rare_num			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 26, SQL_INTEGER	 ,	&RareGroup[1][i].rareset[7].rare_rate			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 27, SQL_CHAR	 ,	RareGroup[1][i].group_buffer_1			, 40, &cbValue);
			retCode = SQLGetData(hStmt, 28, SQL_CHAR	 ,	RareGroup[1][i].group_buffer_2			, 40, &cbValue);
			retCode = SQLGetData(hStmt, 29, SQL_CHAR	 ,	RareGroup[1][i].group_buffer_3			, 40, &cbValue);
			retCode = SQLGetData(hStmt, 30, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_4		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 31, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_5		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 32, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_6		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 33, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_7		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 34, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_8		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 35, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_9		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 36, SQL_INTEGER	 ,	&RareGroup[1][i].group_buffer_10		, 0, &cbValue);

			i++;

			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	RareGroupSet();
	return 1;
}

int LoadRareProbability()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;
	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from rare_probability order by num", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_RARE_PROBABILIY) 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			
			retCode = SQLGetData(hStmt, 1, SQL_C_LONG,	&Rareprobability[i].num					, 0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_LONG,	&Rareprobability[i].exp_rare_suc[0]		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_LONG,	&Rareprobability[i].exp_rare_suc[1]		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_LONG,	&Rareprobability[i].exp_rare_suc[2]		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_C_LONG,	&Rareprobability[i].exp_rare_suc[3]		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 6, SQL_C_LONG,	&Rareprobability[i].max_rare_day		, 0, &cbValue);
			retCode = SQLGetData(hStmt, 7, SQL_C_LONG,	&Rareprobability[i].bonus_suc			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 8, SQL_C_LONG,	&Rareprobability[i].max_suc				, 0, &cbValue);
			retCode = SQLGetData(hStmt, 9, SQL_C_LONG,	&Rareprobability[i].buffer_1			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 10, SQL_C_LONG,	&Rareprobability[i].buffer_2			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 11, SQL_C_LONG,	&Rareprobability[i].buffer_3			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 12, SQL_C_LONG,	&Rareprobability[i].buffer_4			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 13, SQL_C_LONG,	&Rareprobability[i].buffer_5			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 14, SQL_C_LONG,	&Rareprobability[i].buffer_6			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 15, SQL_C_LONG,	&Rareprobability[i].buffer_7			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 16, SQL_C_LONG,	&Rareprobability[i].buffer_8			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 17, SQL_C_LONG,	&Rareprobability[i].buffer_9			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 18, SQL_C_LONG,	&Rareprobability[i].buffer_10			, 0, &cbValue);
			
			i++;

			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 1;
}
/*
{
	DWORD	skillexp	:	20	;//胶懦 版氰摹
	DWORD	makecount	:	5	;//父电 肮荐
	DWORD	day 		:	8	;//历厘等 朝楼
	DWORD	month 		:	4	;//历厘等 朝楼
	DWORD	year 		:	1	;//历厘等 朝楼
}t_skillexp2, *LPSKILLEXP2;
typedef struct SkillExp3
{
	DWORD	skillexp	:	8	;//胶懦鸥涝
	DWORD	temp		:	2	;//胶懦鸥涝
	DWORD	year		:	9	;//斥档
	DWORD	day 		:	13	;//朝楼
}t_SkillExp3,*LPSKILLEXP3;
*/
int TryRareItemAbleDay(int cn,int MySkillLevel,t_skillexp2 *skillexp2)
{
	int mymax = Rareprobability[MySkillLevel].max_rare_day;
	//朝楼 快急 八祸	//朝楼 %8 岿 %8		购啊%2
	int day		=	g_day%8;
	int month	=	g_mon%8;//0-11 鳖瘤
	/*
	盔蘑.
	1.坷疵 朝楼捞搁 父甸 荐 乐促.登绢具
	2.朝楼啊 瘤唱搁 馆靛矫 府悸  茄促.(CheckRareMakeCount)
	*/
	if(day!=skillexp2->day) {return 0;}//促弗 朝楼 扁涝 登绢 乐栏搁 屁变促~
	else if(	month!=skillexp2->month){return 0;}//促弗 崔 扁涝 登绢 乐栏搁 屁变促~
	else//朝档 鞍绊 崔绊 鞍绊
	{
		if( mymax  <= skillexp2->makecount )//钙胶俊 崔窍绰啊?
		{
			OutMessage(cn,2,257);
			return 0;
		}
		else
		{
			skillexp2->makecount++;
			//父甸荐 乐促 墨款飘 棵扼啊扼
			return 1;
		}
	}
	return 0;
}

int TryRareItem(const int cn, const int iSkillNo, CItem_Join join, const int iTryRare, const int iRareGruop, const int iMakeCount,const int iSuccess)//011031 lsw 
{		
	if( iTryRare  && (0 > iRareGruop || MAX_RARE_GROUP <= iRareGruop) )	return 0;

	LPCHARLIST ch = &connections[cn].chrlst;
	CItem *t ;

	int iMotherSkillNo = SkillTbl[iSkillNo].MotherSkillType;
	int TotalPercent	=	0	;
	int NowPercent		=	0	;
	int tempPercent		=	0	;
	int RareType		=   0	;
	int ItemLevel		=	join.skill1_minimum; // 版氰摹 敲矾胶
	int MySkillLevel	=	ch->skillexp[iMotherSkillNo].skillexp/10000;
	int iMaxExp			=	0	;
	int iItemtakeExp	=	0	;

	t = ItemUnit( join.item_id/1000, join.item_id%1000);
	
	if(iTryRare && !t){	return 0;	}

	//磊脚狼 饭骇 焊促 臭栏搁 救登绊 鞍栏搁 等促
	if(MySkillLevel < ItemLevel )	{	return 0;	}
	
	unsigned int iSuccessRate	=	ch->skillexp[iMotherSkillNo].rare+Rareprobability[MySkillLevel-ItemLevel].bonus_suc;

	const int iItemUpgradeType = (ItemMutantKind[t->GetItemKind()].upgrade_type -1);
	bool bMultiEXP = false;

	switch(t->GetItemKind())//抗寇利牢 酒捞袍 辆幅
	{
	case IK_POTION_BAG :	// BBD 040213 器记冠胶
	case IK_CANDLE:
	case IK_POTION:
	case IK_FOOD_MASS:
	case IK_PLATE_FOOD:
	case IK_DRINK:
	case IK_GOOD_FOOD:
	case IK_GOOD_POTION:
	case IK_GOOD_CANDLE:
		{
			bMultiEXP = true;
			if( 1 <= iItemUpgradeType && 3 >= iItemUpgradeType )
			{
				iItemtakeExp	=	Rareprobability[ItemLevel].exp_rare_suc[iItemUpgradeType];
			} 
			else
			{
				iItemtakeExp	=	Rareprobability[ItemLevel].exp_rare_suc[0];
			}
		}break;
	default:
		{
			if( iItemUpgradeType< 0 || iItemUpgradeType >3) //捞芭 绢痘霸 窍聪.. ぱぱ;; 器记 澜侥 剧檬父 坷弗促
			{
				return 0;//捞繁幅狼 酒捞袍篮 己傍伏阑 林瘤 臼绰促
			}
			else
			{
				iItemtakeExp	=	Rareprobability[ItemLevel].exp_rare_suc[iItemUpgradeType];
			}
		}break;
	}	
	
	if(iTryRare && (iSuccessRate	< 0) )
	{
		ch->skillexp[iMotherSkillNo].rare = 0; //己傍伏捞 0 捞窍匙.. 0栏肺 悸泼
		return 0;	
	}

	if(iSuccessRate >1000)							{iSuccessRate=1000;}

	iMaxExp			=	Rareprobability[MySkillLevel-ItemLevel].max_suc;
	
	
	if( iTryRare )//饭绢 父甸扁 矫档葛靛
	{
		LPSKILLEXP2 skillexp2;
		skillexp2 = (t_skillexp2*)&ch->skillexp[iMotherSkillNo-14]; 

		//罚待 矫俊 0捞 唱坷搁 逞绢啊霸阐 茄促
		if(!iSuccess){return 0;}
		
		if(!TryRareItemAbleDay(cn,MySkillLevel,skillexp2))
		{
			SkillMgr.SendSkillExp(iMotherSkillNo-14, ch->skillexp[iMotherSkillNo-14], cn);
			return 0;
		}
		ch->skillexp[iMotherSkillNo].rare =0;
		SkillMgr.SendSkillExp(iMotherSkillNo-14, ch->skillexp[iMotherSkillNo-14], cn);
		if(rand()%(1001-iSuccessRate))//角菩 且锭 甸绢啊绰 风凭 !rand 啊 酒丛
		{
			RareType =  0;
			SkillMgr.SendSkillExp(iMotherSkillNo, ch->skillexp[iMotherSkillNo], cn);//己傍伏捞 何练
			return 0;//父甸扁 角菩
		}

		for(int i = 0;  i < 8 ;i++)
		{
			TotalPercent += RareGroup[iTryRare/10][iRareGruop].rareset[i].rare_rate;
		}

		if(iTryRare && !TotalPercent) 
		{	//己傍伏捞 何练 弊缝 己傍伏 葛磊恩
			return 0;	
		}//犬伏 葛磊扼辑 绝促.
		
		NowPercent = rand()%TotalPercent;
		
		int  index = 0;
		for(; index < 8 ;index++)
		{	tempPercent += RareGroup[iTryRare/10][iRareGruop].rareset[index].rare_rate;
			if( NowPercent < tempPercent )
			{break;	}
		}
		
		RareType = RareGroup[iTryRare/10][iRareGruop].rareset[index].rare_num;//饭绢鸥涝 搬沥

		if(RareType)
		{
			SkillMgr.SendSkillExp(iMotherSkillNo-14, ch->skillexp[iMotherSkillNo-14], cn);
		}//饭绢己傍
	}
	else//饭绢 矫档 救窃
	{
		if( bMultiEXP) //捞芭 绢痘霸 窍聪.. ぱぱ;; 器记 澜侥 剧檬父 坷弗促
		{
		//	iItemtakeExp *= iMakeCount;	//肮荐甫 蚌窍扁 秦霖促
			iItemtakeExp *= 2;	//蚌窍扁2 肺 秦 霖促
		}

		if( ch->skillexp[iMotherSkillNo].rare +(iItemtakeExp)> iMaxExp)
		{
				//版氰摹狼 眠啊 绝澜
			if( ch->skillexp[iMotherSkillNo].rare > iMaxExp )//钙胶 焊促 臭促-> 眠啊 绝促
			{
				//钙胶焊促 臭栏搁 别瘤 臼绰促.
			}
			else//钙胶客 鞍芭唱 撤促 弊烦 钙胶促
			{
				ch->skillexp[iMotherSkillNo].rare = iMaxExp;
			}
		}
		else
		{
			ch->skillexp[iMotherSkillNo].rare += iItemtakeExp ;
		}
	}
	SkillMgr.SendSkillExp(iMotherSkillNo, ch->skillexp[iMotherSkillNo], cn);
	return RareType;
}

int DeleteMakeItemResource( LPCHARLIST ch ,int item_no)
{
	int i,j,k;
	CItem *rit;
	ItemAttr *item;
	
	rit = ItemUnit(item_no);
	POS pos;
	if( rit )
	{
	for(i = 0;i < 3; i++ ) 
		for(j = 0;j < 3; j++ ) 
			for(k = 0; k< 8; k++ ) 
			{
				item = &ch->inv[i][j][k];
				if( item->item_no == item_no)
				{
					if( (rit->GetRbutton() == DIVIDE_ITEM))
					{
						if(item->attr[IATTR_DURATION] >0)
						{
							item->attr[IATTR_DURATION]--;
							return 1;
						}
						else
						{
							SendItemEventLog( item, ch->GetServerID(), SN_NOT_USER, SILT_RESOURCE_BY_SKILL, 4 ); //YGI acer
							DeleteItem( item );
							SetItemPos( INV, i, j, k, &pos );
							SendServerEachItem( &pos , item, ch->GetServerID());
						}
					}
					else
					{
						SendItemEventLog( item, ch->GetServerID(), SN_NOT_USER, SILT_RESOURCE_BY_SKILL, 4 ); //YGI acer
						DeleteItem( item );
						SetItemPos( INV, i, j, k, &pos );
						SendServerEachItem( &pos , item, ch->GetServerID());
						return 1;
					}
					
				}
			}
	}
	return 0;
}

int LoadItemFunction()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from item_function order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEM_FUNCTION) return 0;

			retCode = SQLGetData(hStmt, 1, SQL_C_LONG,	&ItemFunction[i].iNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_C_CHAR,	ItemFunction[i].Name,	40, &cbValue);
			retCode = SQLGetData(hStmt, 3, SQL_C_CHAR,	ItemFunction[i].NameAdd,40, &cbValue);
			retCode = SQLGetData(hStmt, 4, SQL_C_CHAR,	ItemFunction[i].Exp	,	50, &cbValue);
			retCode = SQLGetData(hStmt, 5, SQL_C_LONG,	&ItemFunction[i].ExpMark,	0, &cbValue);
			retCode = SQLGetData(hStmt, 6, SQL_C_LONG,	&ItemFunction[i].iEffectNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, 7, SQL_C_LONG,	&ItemFunction[i].iUpgradeAble,	0, &cbValue);
			retCode = SQLGetData(hStmt, 8, SQL_C_LONG,	&ItemFunction[i].iScrollNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, 9, SQL_C_LONG,	&ItemFunction[i].iMakeRand[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, 10, SQL_C_LONG,	&ItemFunction[i].iMakeRand[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, 11, SQL_C_LONG,	&ItemFunction[i].iMakeRand[2],	0, &cbValue);
			
			retCode = SQLGetData(hStmt, 12, SQL_C_LONG,	&ItemFunction[i].iGrade[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, 13, SQL_C_LONG,	&ItemFunction[i].iGrade[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, 14, SQL_C_LONG,	&ItemFunction[i].iGrade[2],	0, &cbValue);
			retCode = SQLGetData(hStmt, 15, SQL_C_LONG,	&ItemFunction[i].iGrade[3],	0, &cbValue);
			retCode = SQLGetData(hStmt, 16, SQL_C_LONG,	&ItemFunction[i].iGrade[4],	0, &cbValue);
			retCode = SQLGetData(hStmt, 17, SQL_C_LONG,	&ItemFunction[i].iGrade[5],	0, &cbValue);
			retCode = SQLGetData(hStmt, 18, SQL_C_LONG,	&ItemFunction[i].iGrade[6],	0, &cbValue);
			retCode = SQLGetData(hStmt, 19, SQL_C_LONG,	&ItemFunction[i].iGrade[7],	0, &cbValue);
			retCode = SQLGetData(hStmt, 20, SQL_C_LONG,	&ItemFunction[i].iGrade[8],	0, &cbValue);
			retCode = SQLGetData(hStmt, 21, SQL_C_LONG,	&ItemFunction[i].iGrade[9],	0, &cbValue);
			retCode = SQLGetData(hStmt, 22, SQL_C_LONG,	&ItemFunction[i].iGrade[10],	0, &cbValue);
			retCode = SQLGetData(hStmt, 23, SQL_C_LONG,	&ItemFunction[i].iGrade[11],	0, &cbValue);
			retCode = SQLGetData(hStmt, 24, SQL_C_LONG,	&ItemFunction[i].iGrade[12],	0, &cbValue);
			retCode = SQLGetData(hStmt, 25, SQL_C_LONG,	&ItemFunction[i].iGrade[13],	0, &cbValue);
			retCode = SQLGetData(hStmt, 26, SQL_C_LONG,	&ItemFunction[i].iGrade[14],	0, &cbValue);
			retCode = SQLGetData(hStmt, 27, SQL_C_LONG,	&ItemFunction[i].iGrade[15],	0, &cbValue);
			retCode = SQLGetData(hStmt, 28, SQL_C_LONG,	&ItemFunction[i].iGrade[16],	0, &cbValue);

			retCode = SQLGetData(hStmt, 29, SQL_C_LONG,	&ItemFunction[i].iTime[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, 30, SQL_C_LONG,	&ItemFunction[i].iTime[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, 31, SQL_C_LONG,	&ItemFunction[i].iTime[2],	0, &cbValue);
			retCode = SQLGetData(hStmt, 32, SQL_C_LONG,	&ItemFunction[i].iTime[3],	0, &cbValue);
			retCode = SQLGetData(hStmt, 33, SQL_C_LONG,	&ItemFunction[i].iTime[4],	0, &cbValue);
			retCode = SQLGetData(hStmt, 34, SQL_C_LONG,	&ItemFunction[i].iTime[5],	0, &cbValue);
			retCode = SQLGetData(hStmt, 35, SQL_C_LONG,	&ItemFunction[i].iTime[6],	0, &cbValue);
			retCode = SQLGetData(hStmt, 36, SQL_C_LONG,	&ItemFunction[i].iTime[7],	0, &cbValue);
			retCode = SQLGetData(hStmt, 37, SQL_C_LONG,	&ItemFunction[i].iTime[8],	0, &cbValue);
			retCode = SQLGetData(hStmt, 38, SQL_C_LONG,	&ItemFunction[i].iTime[9],	0, &cbValue);
			retCode = SQLGetData(hStmt, 39, SQL_C_LONG,	&ItemFunction[i].iTime[10],	0, &cbValue);
			retCode = SQLGetData(hStmt, 40, SQL_C_LONG,	&ItemFunction[i].iTime[11],	0, &cbValue);
			retCode = SQLGetData(hStmt, 41, SQL_C_LONG,	&ItemFunction[i].iTime[12],	0, &cbValue);
			retCode = SQLGetData(hStmt, 42, SQL_C_LONG,	&ItemFunction[i].iTime[13],	0, &cbValue);
			retCode = SQLGetData(hStmt, 43, SQL_C_LONG,	&ItemFunction[i].iTime[14],	0, &cbValue);
			retCode = SQLGetData(hStmt, 44, SQL_C_LONG,	&ItemFunction[i].iTime[15],	0, &cbValue);
			retCode = SQLGetData(hStmt, 45, SQL_C_LONG,	&ItemFunction[i].iTime[16],	0, &cbValue);

			retCode = SQLGetData(hStmt, 46, SQL_C_LONG,	&ItemFunction[i].iBuffer1,	0, &cbValue);
			retCode = SQLGetData(hStmt, 47, SQL_C_LONG,	&ItemFunction[i].iBuffer2,	0, &cbValue);
			
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 1;
}

void Send_RareItemMakeLog(	const int cn,				const int itemno,
							const int today_count,		const int grade,
							const int mutanttype1,		const int mutanttype2,
							const int mutanttype3,		const int addeditem1,
							const int addeditem1limit,	const int addeditem2,
							const int addeditem2limit,	const int resultlimit,
							const int resultnowdur,		const int resultmaxdur,
							const int why,				const int why2,
							const int resource1limit,	const int resource2limit,
							const int resource3limit,	const int resource4limit,
							const int resource5limit,	const int resource6limit)
{
	if( cn < DRAGON_CONNECTIONS_START || cn >= DRAGON_MAX_CONNECTIONS ) return;
	if( connections[cn].chrlst.SprType == SPRITETYPE_NPC ) return;
	t_packet p;
	t_rare_item_make_log *tp;

	p.h.header.type = CMD_RARE_ITEM_MAKE_LOG;
	p.h.header.size = sizeof( t_rare_item_make_log );
	tp = &p.u.rare_item_make_log;
	memcpy( tp->maker, connections[cn].name, 20 );
	tp->map = MapInfo[MapNumber].port;

	tp->itemno			=	itemno;
	tp->today_count		=	today_count;
	tp->grade			=	grade;
	tp->mutanttype1		=	mutanttype1;
	tp->mutanttype2		=	mutanttype2;
	tp->mutanttype3		=	mutanttype3;
	tp->addeditem1		=	addeditem1;
	tp->addeditem1limit	=	addeditem1limit;
	tp->addeditem2		=	addeditem2;
	tp->addeditem2limit	=	addeditem2limit;
	tp->resultlimit		=	resultlimit;
	tp->resultnowdur	=	resultnowdur;
	tp->resultmaxdur	=	resultmaxdur;
	tp->why				=	why;
	tp->why2			=	why2;
	tp->resource1limit	=	resource1limit;
	tp->resource2limit	=	resource2limit;
	tp->resource3limit	=	resource3limit;
	tp->resource4limit	=	resource4limit;
	tp->resource5limit	=	resource5limit;
	tp->resource6limit	=	resource6limit;
	QueuePacket( connections, DB_DEMON, &p, 1 );

	return;
}

int LoadLearnItemConvetrer()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from LearnItemConvert", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 0;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_LEARN_ITEM_CONVERT) return 0;

			retCode = SQLGetData(hStmt, 1, SQL_INTEGER	 ,	&LIC[i].iBeforeItemNo			, 0, &cbValue);
			retCode = SQLGetData(hStmt, 2, SQL_INTEGER	 ,	&LIC[i].iAfterItemNo			, 0, &cbValue);

			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return 0;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return 1;
}



bool LoadRareItemBag()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from RareItemBag order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 1;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_RAREITEMBAG)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
/*
	typedef struct RareItemBag
	{
		int iNo;
		int RareItemBagPercentTableNo;
		int iGradeMin;
		int iGradeMax;
		int ItemNo[21];//0 锅篮 绝绰 蔼
	}RareItemBag;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&RareItemBag[i].iNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&RareItemBag[i].iRareItemBagPercentTableNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&RareItemBag[i].iGradeMin,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&RareItemBag[i].iGradeMax,	0, &cbValue);
			
			int iTotal = 0;
			for(int count= 1; count <21; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt,  ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				RareItemBag[i].ItemNo[count] = iDummy;
				iTotal+=iDummy;
			}
			RareItemBag[i].ItemNo[0] = iTotal;
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return true;
}	
bool LoadItemControlPercent()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from ItemControlPercent order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 1;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEMCONTROLPERCENT)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
/*
			typedef struct ItemControlPercent
			{
				int iNo		
				int	Per[8];//0锅篮 配呕蔼 1~5绰 海捞流 弊缝 6~7篮 饭绢 弊缝
			}ItemControlPercent;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemControlPercent[i].iNo,	0, &cbValue);
			int iTotal = 0;
			for(int count= 1; count <8; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt,  ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemControlPercent[i].per[count] = iDummy;
				iTotal+=iDummy;
			}
			ItemControlPercent[i].per[0] = iTotal;
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return true;
	
}

bool LoadItemFallPercent()//逛冈绊 父甸磊~ 
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from ItemFallPercent order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 1;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEMFALLPERCENT) return false;
/*
			typedef struct ItemFallPercent
			{
				int iNo
				int per[21];//0 锅篮 配呕 蔼 捞促
			}ItemFallPercent;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemFallPercent[i].iNo,	0, &cbValue);
			int iTotal = 0;
			for(int count= 1; count <21; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt,  ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemFallPercent[i].per[count] = iDummy;
				iTotal+=iDummy;
			}
			ItemFallPercent[i].per[0] = iTotal;
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return true;
}
bool LoadBasicItemBag()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from BasicItemBag order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 1;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_BASICITEMBAG) 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
/*
			typedef struct BasicItemBag
			{
				int iNo;
				int BasicItemPercentTableNo;
				int ItemNo[21];//0锅篮 绝绰蔼
			}BasicItemBag;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&BasicItemBag[i].iNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&BasicItemBag[i].iBasicItemPercentTableNo,	0, &cbValue);
//			int iTotal = 0;
			for(int count= 1; count <21; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt,  ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				BasicItemBag[i].ItemNo[count] = iDummy;
//				iTotal+=iDummy;
			}
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return true;
}

bool LoadItemControl()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from ItemControl order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{		
		int i = 1;
		retCode = SQLFetch(hStmt);
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEMCONTROL) 
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
/*
			typedef struct ItemControl
			{
				int iNo;
				int ItemControlPercentNo;
				int ItemGroup[8];//0锅篮 绝绰蔼 1~5绰 海捞流 弊缝 6~7篮 饭绢 弊缝
				int	iFallItemCount;//冻绢龙 肮荐, 溜, 馆汗巩 龋免 冉荐
			}ItemControl;
*/
			int column = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemControl[i].iNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemControl[i].ItemControlPercentNo,	0, &cbValue);
//			int iTotal = 0;
			for(int count= 1; count <8; count ++)
			{
				int iDummy = 0;
				retCode = SQLGetData(hStmt,  ++column, SQL_C_LONG,	&iDummy,	0, &cbValue);
				ItemControl[i].ItemGroup[count] = iDummy;
//				iTotal+=iDummy;
			}
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&ItemControl[i].iFallItemCount,	0, &cbValue);
			
			i++;
			retCode = SQLFetch(hStmt);
			if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
			{
			}
			else if( retCode == SQL_NO_DATA ) 
			{
				break;
			}
			else
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
		}	
	}		
	else 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	SQLFreeStmt(hStmt, SQL_DROP);		// 0308 YGI
	return true;
}
extern void SendGameToClientWhoSendMail( char *name );
int HandleCommand2( t_packet *p, t_connection c[], int cn )
{
	switch( p->h.header.type )
	{
	case	CMD_CONFORM_SADONIX:
		{
			SkillMgr.RecvCMD_CONFORM_SADONIX(*p);
		}break;
	case	CMD_MOVE_TO_HOMETOWN:
		{
			if(g_MapPort != 5300)
			{
				CHARLIST *ch = CheckServerId(cn);
				if (ch)
				{
					if (ENABLE_HOMETOWN_LEVEL >= ch->GetLevel() && 0 < ch->GetLevel())
					{	//< CSD-030806
						MoveToHomeTown(ch);
					}	//> CSD-030806
				}
			}
		}break;
	case	CMD_MOVE_TO_HOMETOWN_WITH_ALIVE:
		{
			if(g_MapPort != 5300 && g_MapPort != 5830)//檬焊磊 荐访家啊 酒聪扼搁 //020827 lsw
			{
				CHARLIST *ch = CheckServerId(cn);
				if (ch)
				{
					if (ENABLE_HOMETOWN_LEVEL >= ch->GetLevel() && 0 < ch->GetLevel())
					{	//< CSD-030806
						MoveToHomeTownWithAilve(ch);
					}	//> CSD-030806
				}
			}
		}break;
	case	CMD_COMFORM_RESET_ABILILTY:
		{
			RecvCMD_COMFORM_RESET_ABILITY(*p);
		}break;
	case	CMD_MERCHANT_BUY_ITEM_SEARCH_CLIENT:
		{
			Auction.RecvCMD_MERCHANT_BUY_LIST_REQUEST(cn,*p);
		}break;
	case	CMD_MERCHANT_BUY_ITEM_SEARCH_RESULT:
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_BUY_LIST_REQUEST_RESULT(*p);	//叼厚单阁波父 牢沥
			}
			else
			{
				MyLog(0,"Crack CMD_MERCHANT_BUY_ITEM_SEARCH_RESULT");
			}
		}break;
	case	CMD_MERCHANT_SELL_LIST_REQUEST_RESULT:
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_SELL_LIST_REQUEST_RESULT(*p);	//叼厚单阁波父 牢沥
			}
			else
			{
				MyLog(0,"Crack CMD_MERCHANT_SELL_LIST_REQUEST_RESULT");
			}
		}break;
	case	CMD_MERCHANT_SELL_ITEM_INFO_REQUEST://努扼捞攫飘 惑牢捞 焊郴绰 菩哦
		{
			Auction.RecvCMD_MERCHANT_SELL_LIST_REQUEST(cn,*p);
		}break;
	case CMD_MERCHANT_ITEM_BUY	://努扼捞攫飘肺 何磐 罐绊
		{
			Auction.RecvCMD_MERCHANT_ITEM_BUY(cn,*p);
		}break;
	case CMD_MERCHANT_ITEM_BUY_RESULT:
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_ITEM_BUY_RESULT(*p);	//叼厚单阁波父 牢沥
			}
			else
			{
			}
		}break;
	case CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT://叼厚单阁捞 IsEnd甫 眉农 窍绊 览翠阑 林绰镑
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_ITEM_BUY_COMFORM_RESULT(cn,*p);	//叼厚单阁波父 牢沥
			}
			else
			{
				MyLog(0,"Crack CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT");
			}
			
		}break;
	case CMD_MERCHANT_SELL_ITEM_REGISTER://努扼捞攫飘肺何磐 酒捞袍 殿废 菩哦阑 罐酒忱聪促.
		{
			Auction.RecvCMD_MERCHANT_SELL_ITEM_REGISTER(cn,*p);
		}break;
	case CMD_MERCHANT_SELL_ITEM_REGISTER_RESULT://魄概酒捞袍阑 殿废茄 搬苞甫 叼厚肺 何磐 罐嚼聪促..
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_SELL_ITEM_REGISTER_RESULT(*p);	//叼厚单阁波父 牢沥
			}
			else{MyLog(0,"Crack CMD_MERCHANT_ITEM_BUY_COMFORM_RESULT");}
		}break;
	case CMD_MERCHANT_SELL_ITEM_REGISTER_COMFORM_RESULT:
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_SELL_ITEM_REGISTER_COMFORM_RESULT(cn,*p);	//叼厚单阁波父 牢沥
			}
			else{MyLog(0,"Crack CMD_MERCHANT_SELL_ITEM_REGISTER_COMFORM_RESULT");}
		}break;
		//昏力 窍扁 
	case CMD_MERCHANT_SELL_ITEM_DELETE://努扼捞攫飘肺 何磐 魄概 秒家甫 罐绰促
		{
			Auction.RecvCMD_MERCHANT_SELL_ITEM_DELETE(cn,*p);
		}break;
	case CMD_MERCHANT_SELL_ITEM_DELETE_RESULT://叼厚肺 何磐 秒家 搬苞甫 罐酒 努扼捞攫飘俊霸 焊辰促 
		{
			Auction.RecvCMD_MERCHANT_SELL_ITEM_DELETE_RESULT(cn,*p);
		}break;
	case CMD_MERCHANT_SELL_ITEM_DELETE_COMFORM_RESULT://叼厚肺 何磐 魄概秒家茄 搬苞甫 罐绰促
		{
			Auction.RecvCMD_MERCHANT_SELL_ITEM_DELETE_COMFORM_RESULT(cn,*p);
		}break;
	//芭贰 搬苞 府胶飘 >
	case CMD_MERCHANT_RESULT_LIST_REQUEST://府胶飘甫 夸没 罐疽嚼聪促.(努扼捞攫飘肺 何磐)
		{
			Auction.RecvCMD_MERCHANT_RESULT_LIST_REQUEST(cn,*p);
		}break;
	case CMD_MERCHANT_RESULT_LIST_REQUEST_RESULT://搬苞甫 罐疽谰聪促(叼厚肺 何磐)
		{
			Auction.RecvCMD_MERCHANT_RESULT_LIST_REQUEST_RESULT(*p);
		}break;
	//芭贰 搬苞 府胶飘 < 

	case CMD_MERCHANT_RESULT_TAKE://啊廉啊摆翠聪促.
		{
			Auction.RecvCMD_MERCHANT_RESULT_TAKE(cn,*p);
		}break;
	case CMD_MERCHANT_RESULT_TAKE_RESULT://搬苞拱阑 啊廉啊绰 搬苞甫 罐嚼聪促.(叼厚肺 何磐
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_RESULT_TAKE_RESULT(*p);
			}
			else{MyLog(0,"Crack CMD_MERCHANT_RESULT_TAKE_RESULT");}
		}break;
	case CMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT:
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT(cn,*p);
			}
			else{MyLog(0,"Crack CMD_MERCHANT_RESULT_TAKE_COMFORM_RESULT");}				
		}break;
//<! BBD 040303	 赣玫飘归诀 抛捞喉肺何磐 拱扒阑 茫摆促绰 皋矫瘤
	case CMD_MERCHANT_BACKUP_TAKE_REQUEST:
		{
			if(cn == DB_DEMON)
			{
				Auction.RecvCMD_MERCHANT_BACKUP_TAKE_REQUEST(cn, *p);
			}
			else
			{
				MyLog(0,"Crack CMD_MERCHANT_BACKUP_TAKE_REQUEST");
			}
		}break;
//> BBD 040303	 赣玫飘归诀 抛捞喉肺何磐 拱扒阑 茫摆促绰 皋矫瘤
//<! BBD 040308		捞亥飘 酒捞袍 瘤鞭 览翠
	case CMD_EVENTRAREITEM_RES:
		{
			if(cn == DB_DEMON)
			{
				RecvCMD_EVENTRAREITEM_RES(cn, *p);
			}
			else
			{
				MyLog(0,"Crack CMD_EVENTRAREITEM_RES");
			}
			break;
		}
//> BBD 040308		捞亥飘 酒捞袍 瘤鞭 览翠
	case CMD_MERCHANT_DIRECT_EXCHANGE_LIST_REQUSET:
		{
			Auction.RecvCMD_MERCHANT_DIRECT_EXCHANGE_LIST_REQUSET(cn,*p);
		}break;
	case CMD_MERCHANT_DIRECT_EXCHANGE_LIST_REQUSET_RESULT:
		{
			Auction.RecvCMD_MERCHANT_DIRECT_EXCHANGE_LIST_REQUSET_RESULT(*p);
		}break;
	case CMD_EXCHANGE_ITEM_READY		:	
		{
			::RecvItemExchange( &p->u.kein.exchange_item_start.item, p->u.kein.exchange_item_start.you_id, cn ,EXCHANGE_TYPE_NORMAL); 
		}break;
	case CMD_EXCHANGE_OK_SIGN			:	
		{
			RecvExchangeStateSign( p->u.kein.client_other_ch_inv.other_ch, cn , true); 
		}break;
	case CMD_EXCHANGE_CANCEL_SIGN:
		{
			RecvExchangeStateSign( p->u.kein.client_other_ch_inv.other_ch, cn , false); 
		}break;
	case CMD_EXCHANGE_ITEM_EACH			:	
		{
			RecvExchangeEach( &p->u.kein.exchange_item_start.item, p->u.kein.exchange_item_start.you_id, cn ); 
		}break;
	case CMD_EXCHANGE_CANCEL			:	
		{
			CHARLIST *pCh = ::CheckServerId(cn);
			if(!pCh){break;}
			::SendExchangeCancel( pCh->GetExchangeTargetId(), cn ); 				
		}break;
	case CMD_EXCHANGE_ITEM_DEL			:	
		{
			RecvExchangeItemDel( p->u.kein.exchange_item_del.pos, p->u.kein.exchange_item_del.you_id, cn ); 
		}break;
	case CMD_SKILL_MASTER_MULTI_RARE_MAKE:
		{
			RecvCMD_SKILL_MASTER_MULTI_RARE_MAKE(cn,p);
		}break;
	case CMD_EXCHANGE_BOND_MONEY://021126 lsw
		{
			Auction.RecvCMD_EXCHANGE_BOND_MONEY(cn,*p);
		}break;
	case CMD_TRADE_MODE_CHANGE://030127 lsw
		{ 
			::RecvCMD_TRADE_MODE_CHANGE(cn,p);
		}break;
	case CMD_MAIL_REQUEST_LIST://捞抚栏肺 皋老 八祸
		{
			g_MailMgr.RecvReqMailList(cn,p);
		}break;
	case CMD_MAIL_LIST_FROM_DBDEMON://叼厚单阁栏肺 何磐 罐篮 搬苞
		{
			g_MailMgr.RecvMailListFromDBDEMON(cn,p);
		}break;
	case CMD_MAIL_SEND:	
		{
			g_MailMgr.Recv(cn,p); 
		}break;
	case CMD_MAIL_DELETE:
		{
			g_MailMgr.RecvDelete(cn,p); 
		}break;
	case CMD_MAIL_REPAIR:
		{
			g_MailMgr.RecvRepair(cn,p); 
		}
	case CMD_MAIL_REQ_BODY:
		{
			g_MailMgr.RecvRequestBody(cn,p); 
		}break;
	case CMD_MAIL_REQ_BODY_RESULT:
		{
			g_MailMgr.RecvRequestBodyResult(cn,p); 
		}break;
	case CMD_MAIL_ALERT						:	SendGameToClientWhoSendMail( p->u.kein.who_send_mail.name ); break;
	case CMD_SEND_MAIL_OK					:
	case CMD_SEND_MAIL_FAIL					:	ReturnResultByName( p->u.kein.who_send_mail.name, p->h.header.type ); break;	// 010719 YGI
	default:
		{
			return 0;
		}break;
	}
	return 1;
}
