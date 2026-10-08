// RareEffectMgr.cpp: implementation of the CRareEffectMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "..\stdafx.h"
#include "NationSys_Define.h"
#include "CItem.h"
#include "Scrp_exe.h"
#include "op_magic.h"
#include "mysql.h"
#include "SymbolItemMgr.h" // soto-1.02
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRareEffectMgr RareEM;

CRareEffectMgr::CRareEffectMgr()
{

}

CRareEffectMgr::~CRareEffectMgr()
{

}

int CRareEffectMgr::CorrectByStaticRare(void *a, void *d, int &iDmg)
{	
	CHARLIST *Caster = (CHARLIST*)a;
	CHARLIST *Target = (CHARLIST*)d;
	if(!Caster) {return 0;}
	if(!Target) {return 0;}
  if(!Caster->IsPlayer()) {return 0;}//锟矫凤拷锟教绢啊 锟狡聪革拷 锟斤拷锟斤拷 锟揭帮拷
	if(!Target->IsPlayer()) {return 0;}//锟矫凤拷锟教绢啊 锟狡聪革拷 锟斤拷锟斤拷 锟揭帮拷
	//锟窖达拷 锟斤拷锟?锟街撅拷锟?hp锟斤拷 0锟教伙拷 锟教撅拷锟? 龋锟斤拷 锟饺达拷
	if( Caster->IsDead() || Target->IsDead() || !iDmg)	{ return 0;	}//锟窖嫡匡拷 锟较筹拷锟斤拷 锟阶撅拷锟斤拷锟斤拷 //锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 (锟教斤拷锟斤拷 锟斤拷锟斤拷锟斤拷)
	
	const int iTempDmg = iDmg;	
	bool bManaSend = false, bHpSend = false, bHungrySend = false;
////////////////////////////////////////////////////////////////////////////////////////////////
// 锟斤拷锟斤拷锟节匡拷 锟斤拷锟斤拷 贸锟斤拷
{
	if( Target->Spell  == WIZARD_SPELL &&  (0 < Target->Mana) )//锟斤拷锟斤拷锟斤拷 锟街撅拷锟?锟斤拷锟铰达拷
	{
		const int nWitch = __min(iTempDmg	*	GetStaticRareWitch(Caster->StaticRare) / 1000 ,Caster->ManaMax-Caster->Mana);//锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟?锟截硷拷
		const int nDevil = __min(Target->Mana*	GetStaticRareDevil(Caster->StaticRare) / 100 ,Caster->ManaMax-Caster->Mana);
		if (nWitch > 0 && rand()%101 < RARE_MP_STEAL_RAND)
		{
			//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			//锟斤拷锟斤拷 锟斤拷 锟斤拷 锟斤拷锟斤拷 锟斤拷
		//	if(nWitch < Target->Mana )//&& Caster->Mana < Caster->ManaMax )
			{
				Target->DecMana(nWitch);
				Caster->IncMana(nWitch);
				bManaSend = true;
			}
		}

		if (nDevil > 0 && rand()%101 < RARE_MP_STEAL_RAND)
		{
		//	if(nDevil < Target->Mana )//&& Caster->Mana < Caster->ManaMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecMana(nDevil);
				Caster->IncMana(nDevil);
				bManaSend = true;
			}
		}
	}

	if( Target->Spell  == PRIEST_SPELL && (0 < Target->Mana) )//锟斤拷锟斤拷锟斤拷 锟街撅拷锟?锟斤拷锟铰达拷
	{
		const int nEvil		=	__min(iTempDmg		*	GetStaticRareEvil(Caster->StaticRare)  /1000,Caster->ManaMax-Caster->Mana);
		const int nSatan	=	__min(Target->Mana	*	GetStaticRareSatan(Caster->StaticRare) /100,Caster->ManaMax-Caster->Mana);
		
		if (nEvil > 0 && rand()%101 < RARE_DIVINE_STEAL_RAND)
		{
		//	if(nEvil < Target->Mana )//&& Caster->Mana < Caster->ManaMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecMana(nEvil);
				Caster->IncMana(nEvil);
				bManaSend = true;
			}
		}

		if (nSatan > 0 && rand()%101 < RARE_DIVINE_STEAL_RAND)
		{
		//	if(nSatan < Target->Mana )//&& Caster->Mana < Caster->ManaMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecMana(nSatan);
				Caster->IncMana(nSatan);
				bManaSend = true;
			}
		}
	}
	
	if( 0 < Target->Hp )//眉锟斤拷锟斤拷 锟街撅拷锟?锟斤拷锟铰达拷
	{	//HP 锟斤拷锟斤拷 锟接硷拷
		const int nDesire = __min(iTempDmg	 *GetStaticRareDesire(Caster->StaticRare) /100,	Caster->HpMax-Caster->Hp	);
		const int nRobber = __min(Target->Hp *GetStaticRareRobber(Caster->StaticRare) /100,	Caster->HpMax-Caster->Hp	);

		if (nDesire > 0 && rand()%101 < RARE_HP_STEAL_RAND)
		{
		//	if(nDesire < Target->Hp )//&& Caster->Hp < Caster->HpMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecLife(nDesire);
				Caster->IncLife(nDesire);
				bHpSend = true;
			}
		}

		if (nRobber > 0 && rand()%101 < RARE_HP_STEAL_RAND)
		{
		//	if(nRobber < Target->Hp )//&& Caster->Hp < Caster->HpMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecLife(nRobber);
				Caster->IncLife(nRobber);
				bHpSend = true;
			}
		}
	}
	if( 0 < Target->Hungry )//锟斤拷锟斤拷锟斤拷锟?锟街撅拷锟?锟斤拷锟铰达拷 
	{//锟斤拷锟斤拷锟?锟斤拷锟斤拷锟斤拷
		const int nWeary	=	__min(iTempDmg		*GetStaticRareWeary(Caster->StaticRare)	/1000,Caster->HungryMax - Caster->Hungry); 
		const int nFatigue	=	__min(Target->Hungry*GetStaticRareFatigue(Caster->StaticRare)/100,Caster->HungryMax - Caster->Hungry);
		
		if (nWeary > 0 && rand()%101 < RARE_HUNGRY_STEAL_RAND)
		{
		//	if(nWeary < Target->Hungry )//&& Caster->Hungry < Caster->HungryMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecHungry(nWeary);
				Caster->IncHungry(nWeary);
				bHungrySend = true;
			}
		}

		if (nFatigue > 0 && rand()%101 < RARE_HUNGRY_STEAL_RAND)
		{
		//	if(nFatigue < Target->Hungry )//&& Caster->Hungry < Caster->HungryMax)//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			{
				Target->DecHungry(nFatigue);
				Caster->IncHungry(nFatigue);
				bHungrySend = true;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
//	锟斤拷锟斤拷锟节匡拷 锟斤拷锟斤拷 贸锟斤拷 (锟捷帮拷锟教骨凤拷 Caster锟斤拷 锟斤拷锟斤拷 锟劫匡拷 锟截达拷)
{
//	if( 0 < Caster->Hp )//&& Caster->Spell  == PRIEST_SPELL)
	{	//锟捷帮拷
	//	const int nCounterattack	=	(Caster->Hp*GetStaticRareCounterattack(Target->StaticRare))/100;
		const int nCounterattack	=	Caster->HpMax	* GetStaticRareCounterattack(Target->StaticRare)/100;//HpMax锟斤拷锟斤拷
		const int nStrike			=	(iTempDmg		* GetStaticRareStrike(Target->StaticRare))/100;
		
		if (nCounterattack > 0 && rand()%101 < RARE_COUNTERATTCK_RAND)
		{
		//	if(nCounterattack < Caster->Hp )//锟捷拜凤拷锟斤拷 hp 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷//锟斤拷锟斤拷锟斤拷
			{
				Caster->DecLife(nCounterattack);//锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟教帮拷
			//	Target->IncLife(nCounterattack);
				bHpSend = true;
			}
		}

		if (nStrike > 0 && rand()%101 < RARE_COUNTERATTCK_RAND)
		{
		//	if(nStrike < Caster->Hp )//锟捷拜凤拷锟斤拷 hp 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷//锟斤拷锟斤拷锟斤拷
			{
				Caster->DecLife(nStrike);
			//	Target->IncLife(nStrike);
				bHpSend = true;
			}
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////
//	锟斤拷锟斤拷锟斤拷
{
	if(bHpSend)//hp锟绞匡拷 锟斤拷拳锟斤拷 锟街绰碉拷 锟教帮拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟绞帮拷 锟较憋拷 锟斤拷锟截硷拷
	{	
		if( !Caster->Hp )	{	Caster->Hp = 1;	}
		if( !Target->Hp )	{	Target->Hp = 1;	}
		Caster->SendCharInfoBasic( HP, Caster->Hp );//020704 lsw
		Target->SendCharInfoBasic( HP, Target->Hp );//020704 lsw
	}
	if(bManaSend)
	{
		Caster->SendCharInfoBasic( MP, Caster->Mana );//020704 lsw
		Target->SendCharInfoBasic( MP, Target->Mana );//020704 lsw
	}
	if(bHungrySend)
	{
		Caster->SendCharInfoBasic( SP, Caster->Hungry );//020704 lsw
		Target->SendCharInfoBasic( SP, Target->Hungry );//020704 lsw
	}
}
	return 0;
}

void CRareEffectMgr::ClearDynamicRareEffect(void* ch)
{
	CHARLIST *caster = (CHARLIST*)ch;
	memset(	&(caster->DynamicRare),		0,sizeof(DYNAMICRAREEFFECT));
	return;
}

void CRareEffectMgr::ClearRareEffect(void* ch)
{
	CHARLIST *caster = (CHARLIST*)ch;
	memset(	&(caster->StaticRare),		0,sizeof(STATICRAREEFFECT));
	return;
}

int CRareEffectMgr::ResetRareEffect(void* ch)//锟斤拷锟解俊.. 锟斤拷锟斤拷 锟轿猴拷 锟教凤拷 锟斤拷锟筋甫 锟街撅拷 锟斤拷 锟斤拷锟斤拷 锟街达拷 .
{
	CHARLIST *caster = (CHARLIST*)ch;
	bool bCanViewGhost	=false;
	bool bCanViewTrap	=false;
	bool bCanViewBomb	=false;
////////////////////////////////////////////////////////////////////////////////////////////////////
// 努锟斤拷锟斤拷 锟斤拷锟斤拷 贸锟斤拷 锟揭帮拷
	if(GetStaticRareGhost(caster->StaticRare)){bCanViewGhost	= true;}//锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟筋锭锟斤拷锟斤拷 锟斤拷锟斤拷
	if(GetStaticRareLandmine(caster->StaticRare)){bCanViewBomb	= true;}
	if(GetStaticRareTraping(caster->StaticRare)){bCanViewTrap	= true;}

////////////////////////////////////////////////////////////////////////////////////////////////////
	ClearRareEffect(caster);//努锟斤拷锟斤拷 
////////////////////////////////////////////////////////////////////////////////////////////////////
// 锟斤拷锟斤拷 锟较帮拷 锟街达拷 锟斤拷锟斤拷 锟斤拷锟斤拷飘
{
	for(int i = 0; i <ITEM_EQUIP_MAX; i++)
	{
		const LPRareMain pRare = (LPRareMain)&caster->equip[i].attr[IATTR_RARE_MAIN];
		const int	iGrade		=	pRare->grade;
		if(	!iGrade 	||	pRare->IsDynamicRare){continue;}//锟斤拷锟?锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷
		const int	iHighLv		=	pRare->iHighLevel;
		const int	iPri		=	pRare->soksung1;
		const int	iSec		=	pRare->soksung2;
		const int	iThi		=	pRare->soksung3;
		switch(iHighLv)
		{
		case H_LV_NO_HIGH_ITEM://锟斤拷锟教筹拷 锟斤拷锟斤拷 锟斤拷锟斤拷
			{
				ApplyStaticRareEffect(caster->StaticRare, iPri,	iGrade);
				ApplyStaticRareEffect(caster->StaticRare, iSec,	iGrade);
				ApplyStaticRareEffect(caster->StaticRare, iThi,	iGrade);
			}break;
		case H_LV_HIGH_ITEM://锟斤拷锟斤拷
		case H_LV_LEGEND_ITEM://锟斤拷锟斤拷锟斤拷 1
			{//锟斤拷锟教筹拷 锟斤拷锟斤拷锟斤拷锟?霉 锟接硷拷锟斤拷 锟斤拷锟斤拷 锟斤拷挪锟斤拷
				ApplyStaticRareEffect(caster->StaticRare, iPri,	iGrade);
			}break;
		default:{}break;
		}//switch(iHighLv)
	}
}	
////////////////////////////////////////////////////////////////////////////////////////////////////
//	锟轿措饭锟斤拷
	const int iSqaudGrade	=	caster->SquadRare.grade;
	if(iSqaudGrade)
	{
		const int iSqaudPriType	=	caster->SquadRare.soksung1;
		const int iSqaudSecType	=	caster->SquadRare.soksung2;
		const int iSqaudThiType	=	caster->SquadRare.soksung3;
		ApplyStaticRareEffect(caster->StaticRare, iSqaudPriType	,	iSqaudGrade);
		ApplyStaticRareEffect(caster->StaticRare, iSqaudSecType	,	iSqaudGrade);
		ApplyStaticRareEffect(caster->StaticRare, iSqaudThiType	,	iSqaudGrade);
	}
////////////////////////////////////////////////////////////////////////////////////////////////////
//锟斤拷锟斤拷锟斤拷 眉农
{
	int aLegendGNo[ITEM_EQUIP_MAX] = {0,};
	int aLegendGIndex[ITEM_EQUIP_MAX] = {0,};
	int i;
	for(i = 0; i <ITEM_EQUIP_MAX; i++)
	{
		const LPRareMain pRare = (LPRareMain)&caster->equip[i].attr[IATTR_RARE_MAIN];
		if( H_LV_LEGEND_ITEM != pRare->iHighLevel){continue;}//锟斤拷锟斤拷锟藉啊 锟狡达拷 锟斤拷锟?
		aLegendGNo[i]	= pRare->soksung2;
		aLegendGIndex[i]= pRare->soksung3;
	}

	for(i = 0; i <ITEM_EQUIP_MAX; i++)//眉农锟截硷拷 锟斤拷锟斤拷 锟街绰帮拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟教讹拷锟?
	{
		const int iGNo		= aLegendGNo[i];
		const int iGIndex	= aLegendGIndex[i];
		if(!iGNo || !iGIndex){continue;}
		const int iNeeds = m_aItemLegend[iGNo].aSub[iGIndex].iNeeds;
		int iLegendResult = 0;
		for(int aa = 0; aa <ITEM_EQUIP_MAX; aa++)//	needs 眉农锟斤拷 锟劫斤拷 8锟斤拷锟斤拷 锟剿伙拷锟截硷拷 锟斤拷飘锟斤拷 锟角撅拷锟斤拷锟斤拷 锟斤拷锟斤拷
		{
			if( aLegendGNo[aa]		== iGNo	//锟斤拷 锟斤拷锟解啊 iGNo锟斤拷 锟斤拷锟斤拷
			&&	aLegendGIndex[aa]	== iGIndex)//锟轿碉拷锟斤拷锟斤拷 锟斤拷摹 锟窖促革拷
			{
				int iCmpKey = 0;
				switch(aa)
				{
				case WT_WEAPON:	{iCmpKey = LEGEND_NEED_WEAPON;}break;
				case WT_SHIELD:	{iCmpKey = LEGEND_NEED_SHIELD;}break;
				case WT_HELMET:	{iCmpKey = LEGEND_NEED_HELMET;}break;
				case WT_ARMOR:	{iCmpKey = LEGEND_NEED_ARMOR;}break;
				case WT_NECK:	{iCmpKey = LEGEND_NEED_NECK;}break;
				case WT_SHOES:	{iCmpKey = LEGEND_NEED_SHOES;}break;
				case WT_UNIQUE1:{iCmpKey = LEGEND_NEED_UNIQUE;}break;
				case WT_UNIQUE2:{iCmpKey = LEGEND_NEED_UNIQUE;}break;
				}
				if( iNeeds & iCmpKey)//锟斤拷锟斤拷 锟较靛啊 aa锟斤拷 锟戒备 锟较革拷
				{
					iLegendResult |= iCmpKey;//锟斤拷锟斤拷锟藉俊 锟斤拷锟斤拷锟斤拷 锟竭达拷
					aLegendGNo[aa] = 0;//锟竭猴拷 眉农锟斤拷 锟饺登帮拷 锟较憋拷 锟斤拷锟截硷拷 锟斤拷锟斤拷锟?
					aLegendGIndex[aa] = 0;//锟竭猴拷 眉农锟斤拷 锟饺登帮拷 锟较憋拷 锟斤拷锟截硷拷 锟斤拷锟斤拷锟?
				}
			}
		}//for(int aa = 0; aa <ITEM_EQUIP_MAX; aa++)
		if(iNeeds == iLegendResult)
		{//锟斤拷 谩锟斤拷.. 锟斤拷.
			for(int a= 0; MAX_ITEM_LEGEND_SUB_BONUS_MAX>a;a++)
			{
				const int iBonus = m_aItemLegend[iGNo].aSub[iGIndex].aiBonus[a];//锟斤拷锟绞斤拷 1锟斤拷 锟斤拷锟斤拷 盲锟斤拷锟?
				for(int b = 0; MAX_ITEM_LEGEND_BONUS_KIND>b;b++)
				{
					ApplyStaticRareEffect(
						caster->StaticRare, 
						m_aLegendBonus[iBonus].aiKind[b],	
						m_aLegendBonus[iBonus].aiGrade[b]);
				}
			}
		}//(iNeeds == iLegendResult)
	}//for(i = 0; i <ITEM_EQUIP_MAX; i++)//眉农锟截硷拷 锟斤拷锟斤拷 锟街绰帮拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟教讹拷锟?
}
////////////////////////////////////////////////////////////////////////////////////////////////////
//	锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟角撅拷锟?锟较绰帮拷
	if( bCanViewGhost )//锟狡憋拷 锟斤拷 锟斤拷 锟街撅拷锟斤拷锟斤拷
	{
		if(!GetStaticRareGhost(caster->StaticRare))//锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷飘 锟斤拷锟筋啊 锟斤拷锟斤拷
		{//锟阶凤拷锟斤拷 锟斤拷锟斤拷锟斤拷
			SkillMgr.SetCanViewGhost(caster,1);//锟斤拷锟斤拷 锟斤拷锟斤拷 bCanViewGhost锟斤拷 false锟斤拷
		}
	}
	else
	{
		if(GetStaticRareGhost(caster->StaticRare))//锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷飘 锟斤拷锟筋啊 锟斤拷锟斤拷
		{
			SkillMgr.SetCanViewGhost(caster,5);//锟斤拷锟斤拷 锟斤拷锟斤拷 bCanViewGhost锟斤拷 true锟斤拷
		}
	}

	if( bCanViewBomb )
	{
		if(!GetStaticRareLandmine(caster->StaticRare))//锟斤拷锟斤拷锟斤拷 锟斤拷 锟斤拷 锟斤拷锟斤拷
		{
			SkillMgr.SetCanViewBomb(caster,0);
		}
	}
	else	//锟狡憋拷 锟斤拷 锟矫达拷
	{
		if(GetStaticRareLandmine(caster->StaticRare))//锟斤拷锟斤拷锟斤拷 锟斤拷 锟斤拷 锟街达拷
		{
			SkillMgr.SetCanViewBomb(caster,5);
		}
	}

	if( bCanViewTrap )
	{
		if(GetStaticRareTraping(caster->StaticRare))
		{
			SkillMgr.SetCanViewTrap(caster,0);
		}
	}
	else
	{
		if(GetStaticRareTraping(caster->StaticRare))
		{
			SkillMgr.SetCanViewTrap(caster,5);
		}
	}
	return 0;
}

inline int	CRareEffectMgr::CheckFaultStaticRareType(const int iKind)const
{
	//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟?锟斤拷锟斤拷锟斤拷 锟街撅拷锟截达拷
	if( iKind	> MAX_ITEM_RARE	|| iKind	<	1	)	{ return 0; }
	return 1;
}

inline int	CRareEffectMgr::CheckFaultDynamicRareType(const int iKind)const
{
	//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟?锟斤拷锟斤拷锟斤拷 锟街撅拷锟截达拷
	if( iKind	> MAX_ITEM_FUNCTION	|| iKind	<	1	)	{ return 0; }
	return 1;
}

inline int	CRareEffectMgr::CheckFaultRareGrade(const int iGrade)const
{
	//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟?锟斤拷锟斤拷锟斤拷 锟街撅拷锟截达拷
	if( iGrade	> MAX_ITEM_GRADE		|| iGrade	<	1	)	{ return 0; }
	return 1;
}

inline int	CRareEffectMgr::GetStaticRareEffectValue(const int iKind, const int iGrade)const
{
	if(!CheckFaultStaticRareType(iKind)){ return 0;}
	if(!CheckFaultRareGrade(iGrade)){ return 0;}

	return ItemRare[iKind].GRADE[iGrade];
}

inline int	CRareEffectMgr::GetDynamicRareEffectValue(const int iKind, const int iGrade)const
{
	if(!CheckFaultDynamicRareType(iKind)){ return 0;}
	if(!CheckFaultRareGrade(iGrade)){ return 0;}

	return ItemFunction[iKind].iGrade[iGrade];
}

inline DWORD	CRareEffectMgr::GetDynamicRareEffectTime(const int iKind, const int iGrade)const
{
	if(!CheckFaultDynamicRareType(iKind)){ return 0;}
	if(!CheckFaultRareGrade(iGrade)){ return 0;}

	return ItemFunction[iKind].iTime[iGrade];
}


inline short CRareEffectMgr::GetStaticRareMaxValue(const int iKind)const
{
	if(!CheckFaultStaticRareType(iKind)){ return 0;}

	const short nMax = (short)ItemRare[iKind].GRADE[0];//0锟斤拷锟斤拷 锟狡斤拷锟斤拷

	return nMax;
}

inline int	CRareEffectMgr::GetDynamicRareMaxValue(const int iKind)const
{
	return 0;
}

inline int	CRareEffectMgr::AddANDCheckStaticRareMaxValue(const int iKind,short &nNow, const int iAddValue,bool bSymbol)//soto-1.02
{
	int iMax	= 0;
	int iAdded	= 0;
	
	if(bSymbol)
	{
		nNow += iAddValue;
	}
	else
	{
		iMax		= GetStaticRareMaxValue(iKind);
		iAdded	= nNow+iAddValue;//锟斤拷锟斤拷锟斤拷 锟斤拷 
		if( !iMax ) { return 0; }//锟狡斤拷锟斤拷锟斤拷 0 锟轿芭达拷 锟斤拷锟?锟斤拷锟斤拷.

		if( iMax <= iAdded )//锟狡斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷 锟斤拷锟斤拷锟斤拷
		{
			nNow = iMax;//锟狡斤拷锟斤拷 锟斤拷锟斤拷
		}
		else
		{
			nNow = iAdded;
		}
		
		if(nNow < 0)
		{
			nNow = 0;
		}
	}

	return 1;
}

//soto-1.02
inline int	CRareEffectMgr::ApplyStaticRareEffect(STATICRAREEFFECT	&SR, const int iKind, const int iGrade,bool bSymbol)
{
	int iValue  = 0;
	if(bSymbol)
	{
		iValue = iGrade; //锟缴猴拷 锟斤拷锟斤拷锟斤拷锟较讹拷锟斤拷. iGrade 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟诫蔼锟斤拷锟斤拷 锟斤拷锟轿达拷.
	}
	else
	{
		iValue  = GetStaticRareEffectValue(iKind,iGrade);
	}
	
	if( !iValue ) {return 0;}
	
	switch(iKind)
	{
	case  RARE_POWER			:{AddANDCheckStaticRareMaxValue(iKind, SR.nPower			, iValue);	}break;	
	case  RARE_ACCURATE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nAccurate			, iValue);	}break;	
	case  RARE_WISE				:{AddANDCheckStaticRareMaxValue(iKind, SR.nWise				, iValue);	}break;	
	case  RARE_BEAR				:{AddANDCheckStaticRareMaxValue(iKind, SR.nBear				, iValue);	}break;	
	case  RARE_DISTANT			:{AddANDCheckStaticRareMaxValue(iKind, SR.nDistant			, iValue);	}break;	
//	case  RARE_VIGOR			:{AddANDCheckStaticRareMaxValue(iKind, SR.nVigor			, iValue);	}break;	
	case  RARE_VIGOR			:{AddANDCheckStaticRareMaxValue(iKind, SR.nPower			, iValue);	}break;	//锟斤拷趴锟?锟侥匡拷 锟斤拷锟斤拷
	case  RARE_CURSE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nCurse			, iValue);	}break;	
	case  RARE_AID				:{AddANDCheckStaticRareMaxValue(iKind, SR.nAid				, iValue);	}break;	
	case  RARE_MASKING			:{AddANDCheckStaticRareMaxValue(iKind, SR.nMasking			, iValue);	}break;	
	//	case  RARE_PIERCING			:{AddANDCheckStaticRareMaxValue(iKind, SR.nPiercing			, iValue);	}break;
	case  RARE_PIERCING			:{AddANDCheckStaticRareMaxValue(iKind, SR.nPower			, iValue);	}break;//020823 lsw
	case  RARE_BASIC_RESIST		:{AddANDCheckStaticRareMaxValue(iKind, SR.nBasicResist		, iValue);	}break;	
	case  RARE_HARDEST			:{AddANDCheckStaticRareMaxValue(iKind, SR.nHardest			, iValue);	}break;	
	case  RARE_SMART			:{AddANDCheckStaticRareMaxValue(iKind, SR.nSmart			, iValue);	}break;	
	case  RARE_RESIST_FREEZE	:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_freeze	, iValue);	}break;	
	case  RARE_RESIST_VENOM		:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_venom		, iValue);	}break;	
	case  RARE_RESIST_LIGHTNING	:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_lightning	, iValue);	}break;	
	case  RARE_RESIST_BLAZE		:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_blaze		, iValue);	}break;	
	case  RARE_RESIST_PARALYSIS	:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_paralysis	, iValue);	}break;	
	case  RARE_RESIST_HOLY		:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_holy		, iValue);	}break;	
	case  RARE_RESIST_ALL		:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_all		, iValue);	}break;	
	case  RARE_SPEED			:{AddANDCheckStaticRareMaxValue(iKind, SR.nSpeed			, iValue);	}break;	
	case  RARE_LIFE				:{AddANDCheckStaticRareMaxValue(iKind, SR.nLife				, iValue);	}break;	
	case  RARE_MANA				:{AddANDCheckStaticRareMaxValue(iKind, SR.nMana				, iValue);	}break;	
	case  RARE_DIVINE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nDivine			, iValue);	}break;	
	case  RARE_HEALTH			:{AddANDCheckStaticRareMaxValue(iKind, SR.nHealth			, iValue);	}break;	
	case  RARE_BROOD			:{AddANDCheckStaticRareMaxValue(iKind, SR.nBrood			, iValue);	}break;	
	case  RARE_MIND				:{AddANDCheckStaticRareMaxValue(iKind, SR.nMind				, iValue);	}break;	
	case  RARE_SPIRIT			:{AddANDCheckStaticRareMaxValue(iKind, SR.nSpirit			, iValue);	}break;	
	case  RARE_YOUTH			:{AddANDCheckStaticRareMaxValue(iKind, SR.nYouth			, iValue);	}break;	
	case  RARE_FATAL			:{AddANDCheckStaticRareMaxValue(iKind, SR.nFatal			, iValue);	}break;	
	case  RARE_MAGIC			:{AddANDCheckStaticRareMaxValue(iKind, SR.nMagic			, iValue);	}break;	
	case  RARE_NATURE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nNature			, iValue);	}break;	
	case  RARE_STRENGTH			:{AddANDCheckStaticRareMaxValue(iKind, SR.nStrength			, iValue);	}break;	
	case  RARE_DEXTERITY		:{AddANDCheckStaticRareMaxValue(iKind, SR.nDexterity		, iValue);	}break;	
	case  RARE_CONSTITUTION		:{AddANDCheckStaticRareMaxValue(iKind, SR.nConstitution		, iValue);	}break;	
	case  RARE_WISDOM			:{AddANDCheckStaticRareMaxValue(iKind, SR.nWisdom			, iValue);	}break;	
	case  RARE_INTELLIGENCE		:{AddANDCheckStaticRareMaxValue(iKind, SR.nIntelligence		, iValue);	}break;	
	case  RARE_CHARISMA			:{AddANDCheckStaticRareMaxValue(iKind, SR.nCharisma			, iValue);	}break;	
	case  RARE_ENDURANCE		:{AddANDCheckStaticRareMaxValue(iKind, SR.nEndurance		, iValue);	}break;	
	case  RARE_MOVEMENT			:{AddANDCheckStaticRareMaxValue(iKind, SR.nMovement			, iValue);	}break;	
	case  RARE_MORAL			:{AddANDCheckStaticRareMaxValue(iKind, SR.nMoral			, iValue);	}break;	
	case  RARE_LUCKY			:{AddANDCheckStaticRareMaxValue(iKind, SR.nLucky			, iValue);	}break;	
	case  RARE_WIZARD			:{AddANDCheckStaticRareMaxValue(iKind, SR.nWizard			, iValue);	}break;	
	case  RARE_PRIEST			:{AddANDCheckStaticRareMaxValue(iKind, SR.nPriest			, iValue);	}break;	
//	case  RARE_MAGIC_POWER		:{AddANDCheckStaticRareMaxValue(iKind, SR.nMagic_power		, iValue);	}break;	
	case  RARE_MAGIC_VIGOR		:{AddANDCheckStaticRareMaxValue(iKind, SR.nMagic_vigor		, iValue);	}break;	
	case  RARE_MAGIC_STAY		:{AddANDCheckStaticRareMaxValue(iKind, SR.nMagic_stay		, iValue);	}break;	
//	case  RARE_NATURE_POWER		:{AddANDCheckStaticRareMaxValue(iKind, SR.nNature_power		, iValue);	}break;	
	case  RARE_NATURE_VIGOR		:{AddANDCheckStaticRareMaxValue(iKind, SR.nNature_vigor		, iValue);	}break;	
	case  RARE_NATURE_STAY		:{AddANDCheckStaticRareMaxValue(iKind, SR.nNature_stay		, iValue);	}break;	
//	case  RARE_LIFE_RECOVERY	:{AddANDCheckStaticRareMaxValue(iKind, SR.cLife_recovery	, iValue);	}break;	
//	case  RARE_MANA_RECOVERY	:{AddANDCheckStaticRareMaxValue(iKind, SR.cMana_recovery	, iValue);	}break;	
	case  RARE_CRITICAL			:{AddANDCheckStaticRareMaxValue(iKind, SR.nCritical			, iValue);	}break;	
//	case  RARE_GUARDIAN			:{AddANDCheckStaticRareMaxValue(iKind, SR.nGuardian			, iValue);	}break;	
	case  RARE_EVASION			:{AddANDCheckStaticRareMaxValue(iKind, SR.nEvasion			, iValue);	}break;	
	case  RARE_56				:{					}break;	
	case  RARE_57				:{					}break;	
	case  RARE_58				:{					}break;	
	case  RARE_59				:{					}break;	
	case  RARE_60				:{					}break;	
	case  RARE_61				:{					}break;	
	case  RARE_62				:{					}break;	
	case  RARE_63				:{					}break;	
	case  RARE_64				:{					}break;	
	case  RARE_65				:{					}break;	
	case  RARE_66				:{					}break;	
	case  RARE_67				:{					}break;	
	case  RARE_WITCH			:{AddANDCheckStaticRareMaxValue(iKind, SR.nWitch			, iValue);	}break;	
	case  RARE_DEVIL			:{AddANDCheckStaticRareMaxValue(iKind, SR.nDevil			, iValue);	}break;	
	case  RARE_DESIRE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nDesire			, iValue);	}break;	
	case  RARE_ROBBER			:{AddANDCheckStaticRareMaxValue(iKind, SR.nRobber			, iValue);	}break;	
	case  RARE_EVIL				:{AddANDCheckStaticRareMaxValue(iKind, SR.nEvil				, iValue);	}break;	
	case  RARE_SATAN			:{AddANDCheckStaticRareMaxValue(iKind, SR.nSatan			, iValue);	}break;	
	case  RARE_WEARY			:{AddANDCheckStaticRareMaxValue(iKind, SR.nWeary			, iValue);	}break;	
	case  RARE_FATIGUE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nFatigue			, iValue);	}break;	
	case  RARE_COUNTERATTACK	:{AddANDCheckStaticRareMaxValue(iKind, SR.nCounterattack	, iValue);	}break;	
	case  RARE_STRIKE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nStrike			, iValue);	}break;	
//	case  RARE_78				:{		}break;	
	case  RARE_SNAG				:{AddANDCheckStaticRareMaxValue(iKind, SR.nSnag				, iValue);	}break;	
	case  RARE_VERTIGLY			:{AddANDCheckStaticRareMaxValue(iKind, SR.nVertigly			, iValue);	}break;	
	case  RARE_POISON			:{AddANDCheckStaticRareMaxValue(iKind, SR.nPoison			, iValue);	}break;	
	case  RARE_STIFF			:{AddANDCheckStaticRareMaxValue(iKind, SR.nStiff			, iValue);	}break;	
	case  RARE_SLACK			:{AddANDCheckStaticRareMaxValue(iKind, SR.nSlack			, iValue);	}break;	
//	case  RARE_84				:{		}break;	
	case  RARE_GHOST			:{AddANDCheckStaticRareMaxValue(iKind, SR.nGhost			, iValue);	}break;	
	case  RARE_LANDMINE			:{AddANDCheckStaticRareMaxValue(iKind, SR.nLandmine			, iValue);	}break;	
	case  RARE_TRAPING			:{AddANDCheckStaticRareMaxValue(iKind, SR.nTraping			, iValue);	}break;	
	case  RARE_DETECT			://锟轿帮拷锟斤拷锟斤拷 锟斤拷模 瓤锟斤拷
		{	
			AddANDCheckStaticRareMaxValue(RARE_LANDMINE, SR.nLandmine		, iValue);
			AddANDCheckStaticRareMaxValue(RARE_TRAPING, SR.nTraping		, iValue);
		}
		break;	
	case  RARE_HIGH_POWER:{AddANDCheckStaticRareMaxValue(iKind, SR.nHighPower			, iValue);	}break;	
	case  RARE_HIGH_SHIELD:{AddANDCheckStaticRareMaxValue(iKind, SR.nHighShield			, iValue);	}break;	
	case  RARE_HIGH_BLOOD:{AddANDCheckStaticRareMaxValue(iKind, SR.nHighBlood			, iValue);	}break;	
	case  RARE_HIGH_MAGIC:{AddANDCheckStaticRareMaxValue(iKind, SR.nHighMagic			, iValue);	}break;	
	case  RARE_HIGH_NATURE:{AddANDCheckStaticRareMaxValue(iKind, SR.nHighNature			, iValue);	}break;		
	case  RARE_HIGH_RESIST_ALL:{AddANDCheckStaticRareMaxValue(iKind, SR.nResist_all		, iValue);	}break;	
	default						:{return 0;} break;	
	}
	return 1;
}

inline int	CRareEffectMgr::AddANDCheckDynamicRareMaxValue(const int iKind,const int iValue)
{
	return 0;
}
	

int	CRareEffectMgr::ApplyDynamicRareEffect(void* ch, const int iKind, const int iGrade, const int iOrder)
{
	const int iValue  = GetDynamicRareEffectValue(iKind,iGrade);
	const DWORD dwTime  = GetDynamicRareEffectTime(iKind,iGrade);
	CHARLIST *Caster = (CHARLIST*)ch;
	if( !iValue ) {return 0;}
	switch(iKind)//锟斤拷锟竭匡拷 锟斤拷锟斤拷锟教革拷 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷 锟劫曹帮拷  g_curr_time; 锟斤拷 锟斤拷锟?锟窖达拷
	{
	case FITEM_MOVABLE			:	
		{
			Caster->ClearCurse(1);	
			::SendCharacterCondition(Caster, 0);
		}break;	//锟斤拷锟斤拷,锟斤拷拳,去锟斤拷 锟斤拷锟斤拷	锟斤拷锟斤拷杩拷锟斤拷锟?
	case FITEM_RECOVERY			:	
		{
			Caster->ClearCurse(2);	
			Caster->ClearCurse(6);
			::SendCharacterCondition(Caster, 0);
		}break;	//锟接碉拷锟斤拷锟斤拷 锟斤拷锟斤拷	锟接碉拷锟斤拷锟斤拷 锟斤拷锟斤拷	
	case FITEM_DETOX			:	
		{	
			Caster->ClearCurse(3);	
			::SendCharacterCondition(Caster, 0);
		}break;	//锟截碉拷锟桔匡拷	锟截碉拷	
	case FITEM_ENLIGHT			:	
		{	
			Caster->ClearCurse(4);	
			SendMagicCommand(Caster, SET_CUREBLIND, 0, 0);	//锟矫仿伙拷锟?雀锟斤拷
		}break;	//锟矫凤拷 雀锟斤拷	锟矫仿伙拷锟?
	//case FITEM_NULL			:	//	5	NULL	NULL	
	//case FITEM_NULL			:	//	6	NULL	NULL	
	//case FITEM_REVIVAL			:	//	7	//锟节碉拷 锟斤拷劝	KillCharacter 锟斤拷 锟斤拷锟斤拷 case 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷
	case FITEM_COME_BACK		:	//	8	//锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷券	锟节斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟狡匡拷 	
		{
			MoveToHomeTown(Caster);
		}break;
	case FITEM_GHOST			:	//	23	//锟斤拷锟斤拷锟斤拷 锟斤拷 锟斤拷 锟斤拷锟斤拷	锟斤拷锟斤拷锟斤拷 锟斤拷 锟斤拷 锟斤拷锟斤拷	
		{
			SkillMgr.SetCanViewGhost(Caster,dwTime);//锟斤拷锟斤拷 锟斤拷锟斤拷 bCanViewGhost锟斤拷 true锟斤拷
		}break;
	//锟斤拷锟阶凤拷锟教碉拷 锟斤拷锟缴凤拷锟斤拷 锟教帮拷 眉农
	//CheckDynamicRareMaxValue
	case FITEM_RESIST_UP		://	9	//锟斤拷锟?锟借凯 锟斤拷锟阶凤拷 锟斤拷锟?锟斤拷锟?锟借凯 锟斤拷锟斤拷 锟斤拷锟阶凤拷 锟斤拷锟?
	case FITEM_STEALTH			://	10	//锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷	锟斤拷锟斤拷 锟矫帮拷 锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷	
	case FITEM_INVALID_CURSE	://	11	//锟斤拷锟斤拷 锟借凯 锟斤拷锟斤拷 锟斤拷瓤拳	锟斤拷锟斤拷锟?锟斤拷锟斤拷 锟借凯 锟斤拷锟斤拷 锟斤拷瓤拳	
	case FITEM_INVALID_OFFENCE	://	12	//锟斤拷锟斤拷 锟借凯 锟斤拷锟斤拷 锟斤拷瓤拳	锟斤拷锟斤拷锟?锟斤拷锟斤拷 锟借凯 锟斤拷锟斤拷 锟斤拷瓤拳	
	case FITEM_POWER_UP			://	13	//锟斤拷锟斤拷锟斤拷 锟斤拷锟捷凤拷 锟斤拷锟斤拷	锟斤拷锟斤拷锟斤拷 锟斤拷锟捷凤拷 锟斤拷锟斤拷(%)	
	case FITEM_SPEED_UP			://	14	//锟教碉拷 锟接碉拷 锟斤拷锟斤拷	锟教碉拷 锟接碉拷 (锟教碉拷锟斤拷) 锟斤拷锟斤拷	
	case FITEM_HEAVY_ARMOR		://	15	//AC 锟斤拷锟斤拷	AC 锟斤拷摹 锟斤拷锟斤拷	
	case FITEM_SPELL_UP			://	16	//锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷	锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷(%)	
	//case FITEM_					://	17	//锟斤拷锟斤拷 锟斤拷锟斤拷 锟矫帮拷 锟斤拷锟斤拷	锟斤拷锟斤拷 锟斤拷锟斤拷 锟矫帮拷 锟斤拷锟斤拷(msec)	
	case FITEM_CRITICAL			://	18	//CRITICAL DAMAGE 	农锟斤拷萍锟斤拷 锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷	
	case FITEM_STROKE			:	//	19	//锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 100%	锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 100%	
	case FITEM_AGILITY			://	20	//锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 雀锟斤拷锟斤拷 90%	锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 雀锟斤拷锟斤拷 90%	
	case FITEM_INVALID_PROTECT	://	21	//锟斤拷锟斤拷锟?锟斤拷龋 锟借凯 锟斤拷锟斤拷 锟斤拷锟斤拷	锟斤拷锟斤拷锟?锟斤拷龋 锟借凯 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷锟较匡拷 锟斤拷锟斤拷(锟斤拷锟斤拷/锟斤拷锟斤拷 锟斤拷锟?	
	case FITEM_INVALID_RESIST	://	22	//锟斤拷锟斤拷锟?锟斤拷锟斤拷 锟斤拷锟阶凤拷 锟斤拷锟斤拷	锟斤拷锟斤拷锟?锟斤拷锟斤拷 锟斤拷锟阶凤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷 (锟斤拷锟斤拷 锟斤拷锟阶凤拷 0锟斤拷锟斤拷 锟斤拷锟斤拷)	
	case FITEM_SAMARITAN		:	//	24	//NK锟斤拷 锟斤拷锟斤拷锟斤拷 锟斤拷锟斤拷	NK锟斤拷 锟斤拷锟斤拷锟斤拷/锟斤拷锟斤拷 锟斤拷锟斤拷 锟斤拷锟?锟斤拷锟斤拷	
	case FITEM_SLIGHT			://25	//锟斤拷锟斤拷 某锟斤拷锟斤拷 锟矫帮拷 锟斤拷锟斤拷	锟斤拷锟斤拷 某锟斤拷锟斤拷 锟矫帮拷 锟斤拷锟斤拷(%)	
		{
		switch(iOrder)
			{
			case 1:
				{
				Caster->DynamicRare.cRareType1	= iKind;
				Caster->DynamicRare.iValue1		= iValue;
				Caster->DynamicRare.iRareTime1	= g_curr_time+dwTime;
				
				}break;
			case 2:
				{
				Caster->DynamicRare.cRareType2	= iKind;
				Caster->DynamicRare.iValue2		= iValue;
				Caster->DynamicRare.iRareTime2	= g_curr_time+dwTime;
				}break;
			case 3:
				{
				Caster->DynamicRare.cRareType3	= iKind;
				Caster->DynamicRare.iValue3		= iValue;
				Caster->DynamicRare.iRareTime3	= g_curr_time+dwTime;
				}break;
			default:break;
			}
		}break;
	default:{return 0;}break;
	}
	
	return 1;
}

void ResetAbility(CHARLIST *ch)
{
	if(!ch){return;}

	RareEM.ResetRareEffect(ch);	
	g_CSymbolMgr.AddSymbolEffect(ch);//soto-1.02 锟缴猴拷锟斤拷 锟缴凤拷锟斤拷 锟斤拷锟斤拷锟脚诧拷锟?
	const int nPassive = ch->GetPassiveCombat();
	ch->ResetAbility(nPassive);
	::SendAbilityOpen(ch->GetServerID());
}

int	CRareEffectMgr::GetDynamicRareValue(const int iDRType,const	DYNAMICRAREEFFECT	&DR) const
{
	if(!iDRType ) {return 0;}

	if(iDRType == DR.cRareType1)
	{
		return DR.iValue1;
	}
	if(iDRType == DR.cRareType2)
	{
		return DR.iValue2;
	}
	if(iDRType == DR.cRareType3)
	{
		return DR.iValue3;
	}
	return 0;
}

void CRareEffectMgr::AutoCheckDynamicRare(DYNAMICRAREEFFECT	&DR)
{	//瓤锟斤拷 锟矫帮拷锟斤拷 锟斤拷锟斤拷锟较绰碉拷 锟斤拷锟斤拷 锟矫帮拷锟斤拷锟斤拷 陋锟斤拷锟斤拷
	if( DR.iRareTime1 && (g_curr_time > DR.iRareTime1))
	{
		DR.iRareTime1	= 0;
		DR.cRareType1	= 0;
		DR.iValue1		= 0;
	}
	if( DR.iRareTime2 && (g_curr_time > DR.iRareTime2))
	{
		DR.iRareTime2	= 0;
		DR.cRareType2	= 0;
		DR.iValue2		= 0;
	}
	if( DR.iRareTime3 && (g_curr_time > DR.iRareTime3))
	{
		DR.iRareTime3	= 0;
		DR.cRareType3	= 0;
		DR.iValue3		= 0;
	}
	return;
}

int	 CRareEffectMgr::GetRareDisplayEffectNo(const bool bIsDynamicRare,const DYNAMICRAREEFFECT &DR)const	// 锟斤拷锟斤拷 锟斤拷锟斤拷飘 锟斤拷龋
{
	if(bIsDynamicRare)
	{
		return ItemFunction[DR.cRareType1].iEffectNo;
	}
	return 0;
}
int	 CRareEffectMgr::GetRareDisplayEffectTime(const DYNAMICRAREEFFECT &DR)const	// 锟斤拷锟斤拷锟矫帮拷
{
	if( DR.iRareTime1 && g_curr_time <= DR.iRareTime1)
	{
		return DR.iRareTime1 - g_curr_time;
	}
	return 0;
}

bool CRareEffectMgr::IsHaveRivaval(void *ch)
{
	CHARLIST *Caster = (CHARLIST *)ch;
	if(!Caster) {return false;}
	for(int i=0; i < ITEM_QUICK_MAX; i++)
	{
		ITEMATTR &rItem = Caster->quick[i];
		const int iFuncItem = ItemMgr.GetStaticFunctionNo(rItem);
		
		if(iFuncItem == FITEM_REVIVAL)
		{
			::SendItemEventLog( &rItem, Caster->GetServerID(), SN_NOT_USER, SILT_USE, 3 ); //YGI acer
			
			const int iResult = ItemMgr.DecItemDur(rItem,MULTIPLE_USE_ONE_TIME_DEC_DUR);// 锟斤拷锟斤拷寻锟?锟斤拷锟斤拷锟斤拷锟斤拷 锟斤拷麓锟?
			POS pos={0,};
			::SetItemPos( QUICK, i, &pos );
			::SendServerEachItem(&pos,&rItem,Caster->GetServerID());

			return true;
		}
	}
	return false;
}

int CRareEffectMgr::LoadItem_Multi_Rare_Table()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from Item_MultiRare order by `Grade`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int i = 0;
		retCode = SQLFetch(hStmt);
/*
		typedef struct ITEMSKILLMASTER
		{
			int iGrade;
			int iAble;
			int aItemNo[7];//0 锟斤拷锟斤拷, 1 锟斤拷锟斤拷, 锟较斤拷
			int aItemCt[7];//0 锟斤拷锟斤拷, 1 锟斤拷锟斤拷, 锟较斤拷
			int	iAddCt;
		}t_ItemSkillMaster,*LPITEMSKILLMASTER;
*/
		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEM_MULTI_RARE)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			int column = 0;
			int iIndex = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iIndex,	0, &cbValue);
			if(0>iIndex ||MAX_ITEM_MULTI_RARE<= iIndex)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			m_MRData[iIndex].iGrade = iIndex;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].iAble,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[2],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[3],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[4],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[5],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemNo[6],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[2],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[3],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[4],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[5],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].aItemCt[6],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_MRData[iIndex].iAddCt   ,	0, &cbValue);
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

int		CRareEffectMgr::LoadItem_Legend_Table()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from Item_Legend order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int i = 0;
		retCode = SQLFetch(hStmt);
		const int iMaxLine = MAX_ITEM_LEGEND*MAX_ITEM_LEGEND_SUB;

		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= iMaxLine)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			int column = 0;
			int iIndex = 0;
			int iGNo	= 0;
			int iGIndex = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iIndex,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iGNo,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iGIndex,	0, &cbValue);
			if(0>iGNo ||MAX_ITEM_LEGEND<= iGNo)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			if(0>iGIndex ||MAX_ITEM_LEGEND_SUB<= iGIndex)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&m_aItemLegend[iGNo].iGroupIndexMax   ,	0, &cbValue);

			m_aItemLegend[iGNo].iNo			= iIndex;
			m_aItemLegend[iGNo].iGroupNo	= iGNo;
			LPITEMLEGENDSUB pTemp = &m_aItemLegend[iGNo].aSub[iGIndex];
			pTemp->iGroupIndex = iGIndex;
			pTemp->iGroupMax = m_aItemLegend[iGNo].iGroupIndexMax;

			retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	pTemp->szName   ,	20, &cbValue);
			EatRearWhiteChar(pTemp->szName);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iNeeds	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWARHand2	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWALHand3	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWAHead4	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWABody5	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWANeck6	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWAShoe7	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWAUnique8	,	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->iWATwoHand10	,	0, &cbValue);	
			
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiBonus[0]	,	0, &cbValue);	
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiBonus[1]	,	0, &cbValue);	
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiBonus[2]	,	0, &cbValue);	
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiBonus[3]	,	0, &cbValue);	

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
	FILE *fp = fopen( "./output/Item_Legend.bin", "wb" );
	if( fp ) 
	{
		fwrite( m_aItemLegend, sizeof(ITEM_LEGEND)*MAX_ITEM_LEGEND, 1, fp );
		fclose( fp );
	}
	return true;
}

int		CRareEffectMgr::LoadItem_Legend_Bonus_Item()
{
	HSTMT		hStmt = NULL;
	RETCODE		retCode;
	SQLLEN		cbValue;

	SQLAllocStmt(hDBC, &hStmt);
	retCode = SQLExecDirect(hStmt, (UCHAR *)"Select * from Item_Legend_Bonus order by `no`", SQL_NTS);
	if(retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
	{
		int i = 0;
		retCode = SQLFetch(hStmt);

		while( retCode == SQL_SUCCESS || retCode == SQL_SUCCESS_WITH_INFO)
		{
			if(i >= MAX_ITEM_LEGEND_BONUS)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			int column = 0;
			int iIndex = 0;
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&iIndex,	0, &cbValue);
			if(0>iIndex ||MAX_ITEM_LEGEND_BONUS<= iIndex)
			{
				SQLFreeStmt(hStmt, SQL_DROP);
				return false;
			}
			m_aLegendBonus[iIndex].iNo = iIndex;
			
			LPITEMLEGENDBONUS pTemp = &m_aLegendBonus[iIndex];
			
			retCode = SQLGetData(hStmt, ++column, SQL_C_CHAR,	pTemp->szName,	20, &cbValue);
			EatRearWhiteChar(pTemp->szName);

			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiKind[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiKind[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiKind[2],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiKind[3],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiKind[4],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiKind[5],	0, &cbValue);
			
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiGrade[0],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiGrade[1],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiGrade[2],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiGrade[3],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiGrade[4],	0, &cbValue);
			retCode = SQLGetData(hStmt, ++column, SQL_C_LONG,	&pTemp->aiGrade[5],	0, &cbValue);

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
	
	FILE *fp = fopen( "./output/Item_Legend_Bonus.bin", "wb" );
	if( fp ) 
	{
		fwrite( m_aLegendBonus, sizeof(ITEM_LEGEND_BONUS)*MAX_ITEM_LEGEND_BONUS, 1, fp );
		fclose( fp );
	}
	return true;
}

int CRareEffectMgr::GetAbleGroupIndex(const int iGroupNo)
{	
	if(0>iGroupNo || MAX_ITEM_LEGEND < iGroupNo)
	{
		return 0;
	}
	const int iMax = m_aItemLegend[iGroupNo].iGroupIndexMax;
	
	return ((iMax)?(rand()%iMax)+1:0);
}

int CRareEffectMgr::GetLegendBasicRareKind(const int iGroupNo,const int iGroupIndex,const int iWearAble)
{	
	if(0>iGroupNo || MAX_ITEM_LEGEND < iGroupNo){return 0;}
	if(0>iGroupIndex || MAX_ITEM_LEGEND_SUB < iGroupIndex){return 0;}
	int iRareKind = 0;
	switch(iWearAble)
	{
	case WEAR_RIGHT_HAND:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWARHand2;
		}break;
	case WEAR_LEFT_HAND	:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWALHand3;
		}break;
	case WEAR_HEAD		:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWAHead4;
		}break;
	case WEAR_BODY		:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWABody5;
		}break;
	case WEAR_NECK		:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWANeck6;
		}break;
	case WEAR_SHOES		:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWAShoe7;
		}break;
	case WEAR_UNIQUE	:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWAUnique8;
		}break;
	case WEAR_TWO_HAND	:
		{
			iRareKind = m_aItemLegend[iGroupNo].aSub[iGroupIndex].iWATwoHand10;
		}break;
	default:
		{
		}break;
	}
	return iRareKind;
}

