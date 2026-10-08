// ItemMgr.cpp: implementation of the CItemMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "CItem.h"
#include "ItemMgr.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CItemMgr	ItemMgr;

CItemMgr::CItemMgr()
{
	DeleteItem(NULLITEM);
}

CItemMgr::~CItemMgr()
{

}

CItem* CItemMgr::ConvertItemNoToCItem(const int iItemNo)
{
	const int iType	= 	iItemNo/1000;
	const int iNo	= 	iItemNo%1000;

	if(	(iType<0)	||	(iType>10) )	{return NULL;}
	if(	iNo<=0)							{return NULL;}
	if(	iNo > Item_Ref.nItem[iType])	{return NULL;}

	switch( iType )
	{
		case 	PLANT		:	return &CPlant		[iNo];	break;
		case 	MINERAL		:	return &CMineral	[iNo];	break;
		case 	HERB		:	return &CHerb		[iNo];	break;
		case 	COOK		:	return &CCook		[iNo];	break;
		case 	POTION		:	return &CPotion		[iNo];	break;
		case 	TOOL		:	return &CTool		[iNo];	break;
		case 	WEAPON		:	return &CWeapon		[iNo];	break;
		case 	DISPOSABLE	:	return &CDisposable	[iNo];	break;
		case 	ARMOR		:	return &CArmor		[iNo];	break;
		case 	ACCESSORY	:	return &CAccessory	[iNo];	break;
		case 	ETC			:	return &CEtc		[iNo];	break;
		default				:	return NULL;
	}
	return NULL;
}

bool CItemMgr::SwapItem(ITEMATTR &src,ITEMATTR &target)
{
	ITEMATTR temp;

// 捞芭绰 唱吝俊 持谰矫促..
	if(!CheckFaultItem(src,true))	{return false;}
	if(!CheckFaultItem(target,true)){return false;}

	temp = src;
	src = target;
	target = temp;

	return true;
}

int CItemMgr::GetItemWeight(const ITEMATTR item )
{
	int iDur = 0;
	int iType = item.item_no/1000;

	CItem *t = ConvertItemNoToCItem( item.item_no );
	if( !t ) return 0;

	int iWeight = t->GetWeight();

	if( iType == DISPOSABLE){iDur = t->GetItemQuantity();}
	
	if( t->GetRbutton() == DIVIDE_ITEM )
	{
		iDur= t->GetItemDuration();
		if(iDur){iWeight *= item.attr[IATTR_MUCH] / iDur;}
		else{iWeight *= item.attr[IATTR_MUCH];}
	}
	return iWeight;
}

int CItemMgr::GetItemValue(const ITEMATTR item)
{
	CItem *t = ConvertItemNoToCItem(item.item_no);
	if(!t){return 0;}
	
	int			iMuch	= 0;
	int			iPrice	= 0;
	const int	iType = item.item_no/1000;
	const int	iBase	= t->GetValue();
	
	if( t->GetRbutton()==DIVIDE_ITEM )		// 荐樊阑 狼固...
	{
		if( iType == DISPOSABLE ) 
		{
			iMuch = t->GetItemQuantity();	// dispoable老 版快
		}
		else
		{
			iMuch = t->GetItemDuration();		// 10俺 窜困
		}
		if( !iMuch ){ iMuch = 1;}
		iPrice = (unsigned int)((iBase * .9+.5)*(item.attr[IATTR_MUCH]/(float)iMuch));
		return iPrice;
	}
	
	unsigned short wNow, wMax;
	GetItemDur(item,wNow,wMax);

	unsigned int wDBMax = t->GetDuration();

	if( wMax < wDBMax ) wMax = wDBMax;

	if( wMax )
	{	
		/*
		if( (item->attr[IATTR_LIMIT]-g_curr_time) > 0 )		// 蜡烹扁茄捞 巢酒 乐绢具 茄促. 捞 俺充 荤扼咙 唱吝阑 困秦辑 林籍 贸府
		{
			price = (int)( (float)(base*.9)*d_curr/(float)d_max+.5 );
			return price;
		}
		*/
		iPrice = (unsigned int)( (float)(iBase*.9)*wNow/(float)wMax+.5 );		// 烙矫 
		return iPrice;
	}
	return 0;
}

int	CItemMgr::GetItemDur(const ITEMATTR item, unsigned short &NowDur,unsigned short &MaxDur)
{
	NowDur = 0;
	MaxDur = 0;
	CItem *t = ConvertItemNoToCItem(item.item_no);
	if(!t){return 0;}
	if( t->GetRbutton()==DIVIDE_ITEM ){return 0;}//唱穿绢瘤绰 酒捞袍 捞骨肺 掂饭捞记捞 酒聪促
	
	NowDur = LOWORD( item.attr[IATTR_DURATION] );
	MaxDur = HIWORD( item.attr[IATTR_DURATION] );
	return 1;
}

int	CItemMgr::GetItemMuch(const ITEMATTR item, int &NowMuch)//唱穿绢 瘤绰 酒捞袍老 版快
{
	NowMuch = 0;
	CItem *t = ConvertItemNoToCItem(item.item_no);
	if(!t){return 0;}
	if( t->GetRbutton()!=DIVIDE_ITEM ){return 0;}//唱穿绢瘤绰 酒捞袍捞 酒聪聪鳖 蔼阑 罐瘤 给窃
	NowMuch = item.attr[IATTR_MUCH];
	return 1;
}

int CItemMgr::ChangeItemAttribute(ITEMATTR &TargetItem, const int iType, const int iAttr)
{
	if(!(&TargetItem)){return 0;}//措惑狼 林家啊 壳篮瘤 八荤

	switch(iType)
	{
	case IATTR_ITEMNO:		{	TargetItem.item_no				= iAttr;	}	break;
	case IATTR_MUCH:		{	TargetItem.attr[IATTR_MUCH]		= iAttr;	}	break;
	case IATTR_LIMIT:		{	TargetItem.attr[IATTR_LIMIT]	= iAttr;	}	break;
	case IATTR_ATTR:		{	TargetItem.attr[IATTR_ATTR]		= iAttr;	}	break;
	case IATTR_ID:			{	TargetItem.attr[IATTR_RARE_MAIN]= iAttr;	}	break;//0020101 lsw
	case IATTR_RESERVED0:	{	TargetItem.attr[IATTR_RESERVED0]= iAttr;	}	break;
	case IATTR_RESERVED1:	{	TargetItem.attr[IATTR_RESERVED1]= iAttr;	}	break;
	default :				{	return 0;	}									break;
	}
	return CheckFaultItem(TargetItem,true);//官操绊 唱辑 捞惑茄 仇篮 瘤况 滚赴促 瘤况瘤搁 0 府畔 救瘤况瘤搁 1 府畔
}

int CItemMgr::ChangeItemAttribute(ITEMATTR &TargetItem, const ITEMATTR SourceItem)
{	
	if(!(&TargetItem)){return 0;}//措惑狼 林家啊 壳篮瘤 八荤
	TargetItem = SourceItem;
	return 1;
}

int	CItemMgr::GetItemTactic(const int iItemNo)//绝栏搁 林冈捞 扁夯 琶平
{
	CItem *t = ConvertItemNoToCItem(iItemNo);
	if(!t)	{return TACTICS_Crapple;}
	else	{return t->GetSkill_Ability();}
	return TACTICS_Crapple;
}

int	CItemMgr::GetItemKind(const int iItemNo) 
{
	CItem *t = ConvertItemNoToCItem(iItemNo);

	if(!t)	{return IK_NONE;}
	else	{return t->GetItemKind();}
	
	return IK_NONE;
}

// 031009 CI YGI
int	CItemMgr::GenerateItemLimitNumber( int item_no )
{	//< CSD-031229
	/*
	k_item_limit_number limit_number;
	limit_number.who_make = MapNumber+1;
	limit_number.number = GetLimitNumber( item_no );

	if( limit_number.number == LIMIT_OF_NO_HAVE_LIMIT_ITEM )		// 府固飘 锅龋啊 绝绰芭
	{
		limit_number.who_make = 255;
		limit_number.number = g_curr_time_with_out_year;
	}

	int limit;
	memcpy( &limit, &limit_number, sizeof( int ) );
	return limit;
	*/
	return g_curr_time;
}	//> CSD-031229

int	CItemMgr::GenerateItemDuration(const int iItemNo)
{
	int iDuration = 0;

	CItem *t = ConvertItemNoToCItem(iItemNo);
	if( t == NULL ) return 0;

	if( t->GetRbutton() == DIVIDE_ITEM ) //唱床 瘤绰 芭绰 MAKELONG 救窃
	{
		switch (iItemNo/1000)
		{
		case DISPOSABLE	:
			{
				iDuration = t->GetItemQuantity();
			}
			break;
		default			:
			{
				iDuration = t->GetDuration();if(iDuration < 0){iDuration = 1;}
			}
			break;
		}
	}
	else 
	{
		iDuration = MAKELONG( t->GetDuration(), t->GetDuration() );//促弗 老馆利 酒捞袍篮 
	}

	return iDuration;
}

#include "eventmgr.h"	// 040105 YGI 焊拱

ITEMATTR CItemMgr::GenerateItem(const int iItemNo)
{
	// 031110 YGI
	ITEMATTR	item = {0, };
	DeleteItem(item);

	// 031110 YGI
	if( !iItemNo ) return item;
	CItem *t = ConvertItemNoToCItem(iItemNo);

	if(!t)
	{
		MyLog( 0, "GenerateItem() : Can't generatie Item [%d]", iItemNo );
		return item;
	}
	if(!t->GetItemImage() || !t->GetObjectNumber())
	{
		MyLog( 0, "GenerateItem() : Can't generatie Item [%d] because wrong ObjectNumber", iItemNo );
		return item;
	}//坷宏璃飘 捞固瘤 逞滚 绝栏搁 瘤况咙
	
	item.item_no = iItemNo;

	int iDuration = GenerateItemDuration(iItemNo); 

	if( iDuration )		// dur捞 0老版快 积己矫虐瘤 臼绰促.
	{
		item.attr[IATTR_DURATION]	= iDuration;
		item.attr[IATTR_LIMIT]		= GenerateItemLimitNumber( item.item_no );	// 031009 CI YGI
	}
	else 
	{
		DeleteItem(item);//努贰胶 郴何波烙
		return item;
	}

	if( item.item_no == IN_COINS && item.attr[IATTR_MUCH] <= 5 ) item.item_no = IN_COIN;
	else if( item.item_no == IN_COIN && item.attr[IATTR_MUCH] > 5 ) item.item_no = IN_COINS;

	if( item.item_no == IN_NEW_COINS && item.attr[IATTR_MUCH] <= 5 ) item.item_no = IN_NEW_COIN;			// 010210 YGI
	else if( item.item_no == IN_NEW_COIN && item.attr[IATTR_MUCH] > 5 ) item.item_no = IN_NEW_COINS;
	// 040105 YGI 焊拱
	if( IsTreasureMapItem( item.item_no ) )
	{
		CTreasureMapItem treasure_attr = {0, };
		treasure_attr.isChecked = 0;
		
		memcpy( &item.attr[IATTR_TREASURE_MAP], &treasure_attr, sizeof( DWORD ) );
	}

	CheckFaultItem(item,true);
	return 	item;
}

ITEMATTR CItemMgr::GenerateItem(const int iItemNo, const int iType , const int iCustomValue)
{
	int iCustomValueResult = 0;
	ITEMATTR  item = GenerateItem(iItemNo);
	CItem *t = ConvertItemNoToCItem(iItemNo);
	
	if(!t){return item;}

	if( iType == IATTR_MUCH)
	{//荐樊 何盒篮 目胶乓 窍霸 悸泼 茄促//iCustomValueResult 甫 函拳 矫挪吧肺
		if(t->GetRbutton() == DIVIDE_ITEM ) {iCustomValueResult = iCustomValue;}
		else								{iCustomValueResult = MAKELONG( iCustomValue, iCustomValue);}
	}
	ChangeItemAttribute(item, iType, iCustomValueResult);
	CheckFaultItem(item,true);
	return item;
}

int	CItemMgr::CheckFaultItem(ITEMATTR &item,bool bDeleteFlag)//020205 lsw
{
	int rareresult = 0;
	CItem *t = ConvertItemNoToCItem(item.item_no);
	WORD ddur=0;
	WORD dmax=0;

	if(!t)							{goto _DELETE_ITEM;}
	if(!t->GetItemImage())			{goto _DELETE_ITEM;}//酒捞袍 捞固瘤 逞滚 绝栏搁 瘤况咙
	if(!t->GetObjectNumber())		{goto _DELETE_ITEM;}//坷宏璃飘 捞固瘤 逞滚 绝栏搁 瘤况咙

	if(!item.item_no)				{goto _DELETE_ITEM;}//酒捞袍 锅龋 绝栏搁 瘤况咙
	if(!item.attr[IATTR_DURATION])	{goto _DELETE_ITEM;}//酒捞袍 郴备档 绝栏搁 瘤况咙
	
	if(DIVIDE_ITEM == t->GetRbutton())
	{	//-蔼阑 啊柳巴篮 瘤快磊
		if( 0 > ((int)item.attr[IATTR_MUCH]) ){ goto _DELETE_ITEM; }
		if( MAX_DIVIDE_ITEM < (item.attr[IATTR_MUCH]) )//弥措 蔼 焊促 腹栏搁 100 父俺肺 力茄
		{
			item.attr[IATTR_MUCH] = MAX_DIVIDE_ITEM;
		}//100父俺 焊促 腹栏搁
	}
	else
	{
		if(!GetItemDur( item, ddur, dmax ))	{	goto _DELETE_ITEM;	}//郴备档甫 罐阑 荐 绝促?
		if( ddur > dmax)					{	goto _DELETE_ITEM;	}//钙胶 焊促 泅犁啊 腹酒?
	}

	if(item.attr[IATTR_RARE_MAIN])	//饭绢牢单
	{
		RareMain *pRare = (RareMain*)&item.attr[IATTR_RARE_MAIN];
		switch(pRare->iHighLevel)
		{
		case H_LV_NO_HIGH_ITEM:
			{//扁瓷酒捞袍档 咯扁俊 甸绢坷聪鳖 炼缴.  MAX_STATIC_RARE 甫 逞绰 Item_Fuction 捞 乐促搁 瘤况龙 巴涝聪促.
				if(pRare->soksung1)
				{
					if( MAX_STATIC_RARE < pRare->soksung1)//胶怕平 饭绢  弥措摹 焊促 农促搁
					{
						rareresult = 7;
					}
				}
				else
				{
					rareresult = 8;
				}
				if(	!pRare->grade//殿鞭捞 绝芭唱
				||	!pRare->soksung1)//加己 皋牢捞 绝匙..
				{	rareresult = 5;	}//弊烦 瘤况具瘤
			}break;
		case H_LV_HIGH_ITEM:
			{
				if(pRare->soksung1)
				{
					if( START_HIGH_RARE > pRare->soksung1)//窍捞饭绢 矫累痢 焊促 加己捞 撤促搁
					{
						rareresult = 7;
					}
					if(pRare->soksung2 || pRare->soksung3)//窍捞酒捞袍牢单 钢萍 加己捞扼搁
					{
						rareresult = 11;
					}
				}
				else//窍捞牢单 加己捞 绝促搁
				{
					rareresult = 8;
				}
				if(	!pRare->grade//殿鞭捞 绝芭唱
				||	!pRare->soksung1)//加己 皋牢捞 绝匙..
				{	rareresult = 5;	}//弊烦 瘤况具瘤
			}break;
		case H_LV_LEGEND_ITEM:
			{
				if(	!pRare->soksung2//饭傈靛 弊缝 逞滚
				&& 	!pRare->soksung3)//饭傈靛 弊缝 牢郸胶 绝促
				{	rareresult = 10;	}//弊烦 瘤况具瘤
			}break;
		default:
			{
				rareresult = 9;
			}break;
		}//switch(pRare->iHighLevel)

		if(rareresult)
		{
			item.attr[IATTR_RARE_MAIN] = 0;//饭绢 努府绢
		}

		switch(item.item_no)
		{
		case IN_COIN:
		case IN_COINS:
		case IN_NEW_COIN:
		case IN_NEW_COINS:
			{
				item.attr[IATTR_RARE_MAIN] = 0;//饭绢 努府绢
			}break;
		default:
			{	
			}break;
		}
	}
	return 1;

_DELETE_ITEM:
	{
		if(	bDeleteFlag)
		{
			DeleteItem(item);
		}
		return 0;
	}
}

int	CItemMgr::CheckAddAbleItem(const ITEMATTR SourceItem,const ITEMATTR TargetItem)
{
	CItem *tsrc		= ConvertItemNoToCItem(SourceItem);
	
	if(!tsrc){return 0;}
	if(	GetItemKind(SourceItem) == IK_MONEY		&&	GetItemKind(TargetItem) == IK_MONEY){return 1;}
	if(	GetItemKind(SourceItem) == IK_NEW_MONEY	&&	GetItemKind(TargetItem) == IK_NEW_MONEY){return 1;}	

	if(SourceItem.item_no != TargetItem.item_no)							{return 0;}
	if(tsrc->GetRbutton() != DIVIDE_ITEM)									{return 0;}
	if(SourceItem.attr[IATTR_RARE_MAIN]!=TargetItem.attr[IATTR_RARE_MAIN])	{return 0;}
	
	return 1;
}

int	CItemMgr::CombineItem(ITEMATTR &SourceItem,ITEMATTR &TargetItem)//酒捞袍 荐樊 钦摹扁
{
	if(!CheckAddAbleItem(SourceItem,TargetItem)){return 0;}

	ChangeItemAttribute(TargetItem,IATTR_MUCH,TargetItem.attr[IATTR_MUCH]+SourceItem.attr[IATTR_MUCH]);
	DeleteItem(SourceItem);

	return 1;
}


int	CItemMgr::DeleteItem(ITEMATTR &item)
{
	return ::DeleteItem( &item );
}

int CItemMgr::DecItemDur(ITEMATTR &item, const DWORD iValue)//剧荐肺 持阑巴 澜荐肺 持栏搁 郴备档 刘啊
{
	CItem* t = ConvertItemNoToCItem(item.item_no);
	if(!t){return 0;}

	if(t->GetRbutton() == DIVIDE_ITEM)
	{
		if(item.attr[IATTR_MUCH] <= iValue)//滑 荐樊捞 利芭唱 鞍促.
		{
			item.attr[IATTR_MUCH]  =0;//绝绢脸栏搁	
			return 1;
		}//困炼扒俊辑 逞绢 吭栏搁 哗档 等促
		item.attr[IATTR_MUCH] -=iValue;
		return 2;// 荐樊捞 函沁促
	}
	else
	{
		WORD wNow=0,wMax=0;
		if(!GetItemDur( item, wNow, wMax ))
		{
			return 0;
		}

		WORD dOld = wNow;

		if(wNow <= iValue)//滑 蔼捞 巢篮芭 焊促 腹促搁
		{
			DeleteItem( item );
			return 1;
		}//哗档 登绰 芭扼搁
		else
		{
			wNow -= iValue;
			
			if( wNow <= 0 ){return 0;}
			
			item.attr[IATTR_DURATION] = MAKELONG( wNow, wMax );

			if( wNow/10 == 0 ) // Duration捞 0捞 登菌促.
			{
				return 1;
			}
			if( wNow/10 != dOld/10 )  // 锅拳等 加己阑 PC俊霸 焊郴霖促.
			{
				return 2;
			}
		}
	}
	return 0;

	
	
}

ItemAttr CItemMgr::GiveLegendItem(const int item_no, const int iDur)
{
	ItemAttr Item= {0,};
	CItem * t = ItemUnit(item_no);
	if(!t){ return Item; }
	
	const int iLegendAble = t->GetLegendAble();
	if(!iLegendAble){return Item;}

	//iDur捞 0捞搁 扁夯栏肺 父甸绢 林绊 酒囱 版快绰 弊措肺 技泼茄促 澜荐捞搁 府畔
	if( iDur > 0)		{	Item = GenerateItem( item_no, IATTR_MUCH ,iDur); }
	else if	(iDur==0)	{	Item = GenerateItem( item_no);}
	else if	(iDur<0)	{	return Item;	}

	const int iLegendGroupNo = t->GetLegendGroup();
	if(!iLegendGroupNo){return Item;}
	//s1 --> 畴富 , s2 --> 弊缝逞滚 --> s3-->弊缝牢郸胶
	const int iGroupIndex = RareEM.GetAbleGroupIndex(iLegendGroupNo);
	const int iRareKind = RareEM.GetLegendBasicRareKind(iLegendGroupNo,iGroupIndex,t->GetWearAble());

	const int aSucessRate[6] = {5,10,15,30,40,60};//醚钦捞 160
	const int iNow = rand()%160;//
	
	int iTemp = 0;
	int iResultGrade = 0;
	for(int i = 0; 6 > i; i++)
	{
		iTemp += aSucessRate[i];
		if(iTemp> iNow)
		{
			iResultGrade = i+3;//0~5 捞扁 锭巩俊
			break;
		}
	}
	MakeRareAttr(Item.attr[IATTR_RARE_MAIN],iResultGrade,iRareKind,iLegendGroupNo,iGroupIndex,H_LV_LEGEND_ITEM,0);
	return Item;
}
//


ItemAttr CItemMgr::GiveRareItem(const int item_no, const int iDur,const int iCallType,const int iTryMulti, 
								const int iGradeMin, const int iGradeMax, RareMain &RareAttr,const int iHighLevel)
{
	ItemAttr Item = {0,};
	memset(&Item,0,sizeof(ItemAttr));

	int iNowGrade=0;
	
	CItem * t = ItemUnit(item_no);
	if(!t){ return Item; }
	
	const int iItemKind = t->GetItemKind();
	
	if( MAX_ITEM_MUTANT_KIND <= iItemKind)	{return Item;}
	
	const int iRareAble = ItemMutantKind[iItemKind].upgrade_type;
	
	//iDur捞 0捞搁 扁夯栏肺 父甸绢 林绊 酒囱 版快绰 弊措肺 技泼茄促 澜荐捞搁 府畔
	if( iDur > 0)		{	Item = GenerateItem( item_no, IATTR_MUCH ,iDur); }
	else if	(iDur==0)	{	Item = GenerateItem( item_no);}
	else if	(iDur<0)	{	return Item;	}

	//钙胶啊 累阑 版快绰 府畔, 鞍栏搁 Min阑 持绢林绊 瞒捞啊 剧荐搁 罚待
	if(iGradeMax > iGradeMin)
	{
		iNowGrade = iGradeMin+rand()%(iGradeMax-iGradeMin+1);//罚待 窍搁 弥绊蔼篮 救唱坷聪鳖.+1阑 茄巴栏肺
	}
	else if( iGradeMax == iGradeMin )
	{
		iNowGrade= iGradeMin;
	}
	else //殿鞭 弥措啊 弥窍焊促 累篮霸 绢调衬
	{
		memset(&Item,0,sizeof(ItemAttr)); return Item;
	}
	
	int iRare1 = 0, iRare2 = 0, iRare3 = 0;
	
	if( iRareAble )//饭绢 啊瓷 老锭
	{
		switch( iTryMulti )
		{
		case 1:
			{
				iRare1 = GetAbleRareKind(iCallType,iHighLevel);
			}break;
		case 2:
			{
				iRare1 = GetAbleRareKind(iCallType,iHighLevel);
				iRare2 = GetAbleRareKind(iCallType,iHighLevel);
			}break;
		case 3:
			{
				iRare1 = GetAbleRareKind(iCallType,iHighLevel);
				iRare2 = GetAbleRareKind(iCallType,iHighLevel);
				iRare3 = GetAbleRareKind(iCallType,iHighLevel);
			}break;
		default:
			{
			}break;							
		}
	}
	MakeRareAttr(Item.attr[IATTR_RARE_MAIN] ,iNowGrade,iRare1,iRare2,iRare3,iHighLevel,false);//020314 lsw
	RareAttr = *(RareMain*)(&Item.attr[IATTR_RARE_MAIN]);
/*
	Send_RareItemMakeLog(	cn,	item_no,
							0,0,0,0,
							0,0,0,0,
							item.attr[IATTR_LIMIT],
							item.attr[IATTR_MUCH],
							0,type,kind,//器飘
							0,0,0,0,
							3000,0
							);*/
	return Item;
}

int CItemMgr::AbleMakeOnThisMap(const int iMapNation, const int iNo)
{	
	const int iAbleNation  = ItemRare[iNo].iAbleNation;
	switch(iMapNation)
	{
	case NW_BY:
		{
			if((iAbleNation%1000)/100)
			{
				return true;
			}
		}break;
	case NW_ZY:
		{
			if((iAbleNation%100)/10)
			{
				return true;
			}
		}break;
	case NW_YL:
		{
			if((iAbleNation%10)/1)
			{
				return true;
			}
		}break;
	case 0:
		{
			if((iAbleNation%10000)/1000)//吝赋惫
			{
				return true;
			}
		}break;

	default :
		{
			return false;
		}break;
	}
	return false;
}

int CItemMgr::GetAbleRareKind(const int iCallType,const int iHighLevel)
{
	int oklist[MAX_ITEM_RARE+1] = {0,};//扁霖捞登绰 府胶飘 No肺甸绢 乐绰芭.
	int gamble[MAX_ITEM_RARE+1] = {0,};//唱棵 犬伏 拌魂阑 困秦辑
	int iNo = 0;
	int iMax = 0;

	switch( iHighLevel)
	{
	case H_LV_NO_HIGH_ITEM:
		{
			iNo		=0;
			iMax	=MAX_STATIC_RARE;
		}break;
	case H_LV_HIGH_ITEM:
		{
			iNo		= START_HIGH_RARE;
			iMax	= MAX_HIGH_RARE;
		}break;
	
	default:
		{
			return 0;
		}break;
	}
	
	int iList= 0;
	for( ;iNo < iMax ; iNo++)
	{
		if(! (ItemRare[iNo].iMakeAble & iCallType))					{continue;} //父甸扁啊 啊瓷 茄啊? 
		if(! AbleMakeOnThisMap(MapInfo[MapNumber].nation,iNo))		{continue;} //捞惫啊俊辑 唱坷唱??

		if( iCallType & RARE_ABLE )
		{
			if( iCallType == RARE_ABLE )
			{
				gamble[iList] = ItemRare[iNo].RaisePro[0];	
			}
			else if	(iCallType & RARE_ABLE_SQUAD)//惫啊
			{	
				gamble[iList] = ItemRare[iNo].RaisePro[1];	
			}//何措 饭绢
			else if	(iCallType & RARE_ABLE_GAMBLE_ROOM_4)//白喉4锅规 
			{	
				gamble[iList] = ItemRare[iNo].RaisePro[2];	
			}
			else if	(iCallType & RARE_ABLE_GAMBLE_ROOM_5)//白喉5锅规
			{	
				gamble[iList] = ItemRare[iNo].RaisePro[3];	
			}
		}

		if( gamble[iList] )
		{
			oklist[iList] = ItemRare[iNo].iNo;// 府胶飘俊 啊瓷 加己 逞滚 眠啊	
			gamble[MAX_ITEM_RARE] += gamble[iList];//犬伏 配呕 拌魂
			iList++;
		}
	}				

	if(gamble[MAX_ITEM_RARE]<=0)
	{
		return 0;//捞凡老篮 芭狼 绝促..酒聪.. 酒抗.. 弊矾唱.. 
	}

	int NowRating= rand()%gamble[MAX_ITEM_RARE];
	int NowSok=0;
	int CheckRating=0;
	
	for(NowSok=0 ; NowSok<MAX_ITEM_RARE; NowSok++)
	{
		CheckRating+=gamble[NowSok];
		if(NowRating<CheckRating)
		{
			break;
		}
	}

	return oklist[NowSok];
}

bool CItemMgr::MakeRareAttr(unsigned int &attr3 ,const int iNowGrade,
							const int iSokSung1,const int iSokSung2,const int iSokSung3,
							const int iHighLevel,const bool bIsDynamicRare)const
{
	attr3  = 0;
	if( iNowGrade	>15			||	iNowGrade	<=	0	)	{ return false;}
	if( iSokSung1	>UCHAR_MAX	||	iSokSung1	<=	0	)	{ return false;}
	if( iSokSung2	>UCHAR_MAX	||	iSokSung2	<	0	)	{ return false;}
	if( iSokSung3	>UCHAR_MAX	||	iSokSung3	<	0	)	{ return false;}
	if(	iHighLevel	>7			||	iHighLevel	<	0	)	{ return false;}

	((LPRareMain)&attr3)->grade			= iNowGrade	;
	((LPRareMain)&attr3)->soksung1		= iSokSung1	;
	((LPRareMain)&attr3)->soksung2		= iSokSung2	;
	((LPRareMain)&attr3)->soksung3		= iSokSung3	;
	((LPRareMain)&attr3)->iHighLevel	= iHighLevel;
	((LPRareMain)&attr3)->IsDynamicRare = bIsDynamicRare;
	return true;
}

// 021117 YGI
int CItemMgr::MakeDefaultRareItem( ItemAttr *item, const int iItemNo, const int iSok1, const int iGrade, const int iTryMulti)
{
	int iHighLv = 0;
	bool bIsDynamic = false;

	CItem *t = ::ItemUnit( iItemNo );
	if( !t ) {	return -1;	}

	*item = GenerateItem( iItemNo );
	if( !item->item_no ){ return -1;}		// 酒捞袍 锅龋啊 捞惑窍促.

	if( iSok1 >= START_HIGH_RARE )
	{
		iHighLv = H_LV_HIGH_ITEM;
	}
	else
	{
		if( t->GetRbutton() == USE_ITEM )		// 促捞唱雇 酒捞袍...
		{
			bIsDynamic = true;
			iHighLv = H_LV_NO_HIGH_ITEM;
		}
	}

	int iSok2=0, iSok3=0;
	if(iSok1)
	{
		switch( iTryMulti )
		{
		case 0 :
		case 1 :
			{
			}break;
		case 2:
			{
				iSok2 = iSok1;
			}break;
		case 3 :
			{
				iSok3 = iSok2 = iSok1;
			}break;
		}

		if(H_LV_HIGH_ITEM == iHighLv){	iSok3 = iSok2 =0;}

		return (MakeRareAttr( item->attr[IATTR_RARE_MAIN], iGrade, iSok1, iSok2, iSok3, iHighLv, bIsDynamic )?1:0);
	}
	return 1;
}

int CItemMgr::GetSkillPointByItem(const ITEMATTR &item) const 
{
	CItem *t = ItemUnit(item);
	
	if(!t){return 0;}
	
	const int iLv = t->GetLevel();
	if( 0 >= iLv)
	{
		return 0;
	}
	return (iLv * 3 /10 +1);
}

int CItemMgr::GetStaticFunctionNo(const ITEMATTR &item)
{
	CItem *t = ConvertItemNoToCItem(item.item_no);
	return ((t)?t->GetStaticFunctionNo():0);
}

// 031009 CI YGI
//////////////////////////////////////////////////////////////////////////
// CItemLimitMgr member functions
int	CItemLimitMgr::GetLimitNumber( int item_no )
{
	if( !IsCanHaveLimitItem( item_no ) )
	{
		return LIMIT_OF_NO_HAVE_LIMIT_ITEM;
	}

	m_nItemLimitCount++;
	//char str_number[50];
	//sprintf( str_number, "%d", m_nItemLimitCount );
	//WritePrivateProfileString( "option","limit", str_number, MAP_SERVER_INI_ );

	// 瘤陛鳖瘤狼 府固飘 锅龋甫 DB肺 焊辰促.
	t_packet packet;
	packet.h.header.type = CMD_WRITE_LIMIT_NUMBER;
		packet.u.kein.save_limit_number.port = ::GetOwnPort();
		packet.u.kein.save_limit_number.limit_number = m_nItemLimitCount;
	packet.h.header.size = sizeof( k_save_limit_number );
	QueuePacket( connections, DB_DEMON, &packet, 1 );

	return m_nItemLimitCount;
}

int	CItemLimitMgr::LoadHaveLimitItem()
{ 
	m_mapHaveLimitItem.clear();

	HSTMT hStmt = NULL;
	SQLAllocStmt(hDBC, &hStmt);

	RETCODE	retCode = SQLExecDirect(hStmt, (UCHAR*)"SELECT * FROM ItemList_HaveLimit", SQL_NTS);	
	if(retCode != SQL_SUCCESS_WITH_INFO && retCode != SQL_SUCCESS) 
	{
		SQLFreeStmt(hStmt, SQL_DROP);
		return false;
	}
	
	SQLLEN cbValue;
	int item_no = 0;
	int item_id= 0;
	int item_imageno= 0;
	char name[20]= {0, };
	while((retCode = SQLFetch(hStmt)) == SQL_SUCCESS)
	{
		SQLGetData(hStmt, 1, SQL_C_SLONG, &item_id, 0, &cbValue);
		SQLGetData(hStmt, 2, SQL_C_CHAR, name, 20, &cbValue);
		SQLGetData(hStmt, 3, SQL_C_SLONG, &item_imageno, 0, &cbValue);
		item_no = ((int)(item_imageno/10000))*100 + item_id;
		m_mapHaveLimitItem.insert( map<int, int>::value_type( item_no, 1 ) );
	}

	SQLFreeStmt(hStmt, SQL_DROP);
	return true;
}
