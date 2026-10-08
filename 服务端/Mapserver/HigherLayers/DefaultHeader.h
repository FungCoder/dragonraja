#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sqlext.h>
#include <winerror.h>
#include <crtdbg.h>

#define _NATION_APPLY_

#define ATTACKPOWER_PRO_	140
#define GET_EXP_PC_AND_PC_	30// PC간에는 경험치를 NPC에 의해 얻는 수치에 아래수치롤 나누어 준다. ( 되게 쪼금준다. )
#define ULTRA_PK_NUM_		100// 극악PK인경우 아래 수치를 넘게되면 name_status.ultra_pk == 1이되고 공격을 못하게 된다. 

///////////////////////////////////////////////////////////////////////////////
//< CSD-030517
const int Num_Of_Effect = 700;
const int Num_Of_Magic = 251;
const int Num_Of_NPC_Type = 100;
const int Num_Of_NPC_Lev = 256;//256
const int Num_Of_NPC_Name = 7932;
const int Num_Of_NPC_Generation	= 398;

const int MAX_LOG_TYPE = 4;     // CSD-TW-030622
const int MAX_COMBAT_LEVEL = 3; // CSD-TW-030622
const int MAX_NATION = 7;
const int MAX_AREA_BUFFER =	1000;
const int MAX_MAUL = 10;
const int MAX_MAP_IN_MAUL = 20;
const int MAX_GAMBLE_ITEM_KIND = 8;
const int MAX_GAMBLE_ITEM_TYPE  = 7;
const int MAX_GUARDIAN_GUILD = 3;
const int MAX_MAXIMUM_NUMBER = 24; // CSD-TW-030704
const int MAX_CDUNGEONBASIC_INT = 6; // CSD-030723
const int MAX_PARTY_MEMBER = 6; // CSD-030723
const int MAX_SKILL_NO	= 200; // 200 add by leo
const int MAX_DIR_TABLE = 256; // CSD-030723
const int MAX_SHORTPATH = 100; // CSD-030723 // LTS AI
const int MAX_LEARN_ITEM = 1000; // CSD-030723
const int MAX_PC_CONTROL_NPC = 32; // CSD-030723
const int MAX_CLASS = 5; //5蘆斂撚
const int MAX_RESIST_TYPE = 6; // CSD-030723
const int MAX_GENERATE_POSITION_ = 100; // CSD-031106
const DWORD MAX_BANK_MONEY = 4000000000; //陵契닸운離댕令
//< CSD-030723
const double HT_NORMAL_CHANGE_VALUE = 0.003;	//가만히 있을때 깎일 값
const double HT_WALK_CHANGE_VALUE = 0.013;	//피스모드 걸을때
const double HT_RUN_CHANGE_VALUE = 0.030;	//피스모드 뛸때
const double HT_BATTLE_WALK_CHANGE_VALUE = 0.020;	//배틀모드 걸을때
const double HT_BATTLE_RUN_CHANGE_VALUE = 0.040;	//배틀모드 뛸때
const double HT_ATTACK_CHANGE_VALUE = 0.030;	//물리공격시
const double HT_MAGIC_CHANGE_VALUE = 0.030;	//마법사용시
const double HT_SKILL_CHANGE_VALUE = 0.050;	//직업시
// 죽었을때 약 5초마다 Check한다. 
const int DEAD_CHARACTER_CHECK_TIME = 5;
// 죽은다음 사라시거나, Ghost로 바뀌는 시간을 정한다. 
// 단위는 DEAD_CHARACTER_CHECK_TIME(초)이다. 
const int DEAD_PC_COUNT_ = 65;
const int DEAD_NPC_COUNT_BOTTOM_ = 5; // NPC : ((DEAD_PC_COUNT_ - DEAD_NPC_COUNT_BOTTOM_ ) * DEAD_CHARACTER_CHECK_TIME )초간 시체인상태가 지속된다. 
const int DEAD_PC_COUNT_BOTTOM_ = DEAD_PC_COUNT_ - 5;	// PC는 시체인상태를 (5 * DEAD_CHARACTER_CHECK_TIME)초간만 지속된다. 
const int DEAD_NPC_ANIMAL_COUNT_ = 25;
const int DEAD_NPC_HUNT_MONSTER_COUNT_ = 6; // CSD-040407
const int DEAD_NPC_MONSTER_COUNT_ = 15;
const int DEAD_SEALSTONE_COUNT_ = (2*60*60)/5;		// LTS NEW LOCALWAR
//> CSD-030723
const int CIB_POTION_BOX = 70; // CSD-030723
const int LOTTO_PAY = 20000; // CSD-TW-030704 : 로또 구입 비용
const int GUILD_WEAR01	   = 102; // 지키미 길드복

const int PS_EXP		= 10;  // 그냥 경험치
const int PS_TACTIC	= 20;  // 택틱 경험치
const int PS_MAGIC_TAC	= 30;  // 마법 택틱 경험치
const int NO_TAC		= 13;  // 전투에 사용되는 택틱수
const int ULTRA_NK     = 100; // Ultra NK 수치
const int ENABLE_DUMMY_LEVEL = 10; // CSD-030806
const int ENABLE_DUEL_LEVEL = 10; // CSD-030806
const int ENABLE_GM_LEVEL = 20; // CSD-030806
const int ENABLE_NATIONWAR_LEVEL = 10; // CSD-030806
const int ENABLE_PARTY_LEVEL = 35; // CSD-030806
const int ENABLE_FREE_LEVEL = 50; // CSD-030806
const int ENABLE_HOMETOWN_LEVEL = 20; // CSD-030806
const int BUILD_GUILD_LEVEL = 40; // 쉔접묏삔離됴된섬
const int CROSSING_CLASS_LEVEL = 101; // 쏵쌓된섬

enum LOG_TYPE 
{
	LT_ACCELATER = 0,
	LT_BATTLE = 1,
	LT_CHANGE = 2,
	LT_CHECK = 3,
	LT_LEVEL_UP = 4,
	LT_CHAT = 5,
	LT_SCRIPT	= 6,	// 030826 kyo
	LT_AUCTION	= 7,	// BBD 040226
	LT_EVENTITEM = 8,	// BBD 040308
	LT_LOCAL_WAR = 9    // CSD-040407
};

//<! BBD 040308 이벤트 아이템 로그 타입
enum EVENTITEM_LOG_TYPE
{
	// EILT = Event Item Log Type
	EILT_INVALID_CONNECTION = 0,		// 콘넥션이 끊어졌다
	EILT_INVALID_NAME = 1,				// 이름이 다르다
	EILT_NOTENOUGH_INVENTORY = 2,		// 요청한 사이 인벤을 채웠다.
};
//> BBD 040308 이벤트 아이템 로그 타입

//<! BBD 040226 옥션로그 타입
enum AUCTION_LOG_TYPE
{
	// 거래가 완전히 종료될때에 로그가 생겨야 함
	ACLT_SELLER_REGIST = 0,			// 파는 놈이 등록할때 생기는 로그
	ACLT_SELLER_CANCLE = 1,			// 파는 놈이 취소할때 생기는 로그
	ACLT_BUYER_TAKE = 2,			// 사는 놈이 물건을 가져감으로 거래가 종료될때 생기는 로그
	ACLT_SELLER_TAKE = 3,			// 파는 놈이 대금을 회수함으로 거래가 종료될때 생기는 로그 
};
//> BBD 040226 옥션로그 타입

enum DAY_TYPE
{	//< CSD-030804
	SUNDAY = 0,
	MONDAY = 1,
	TUESDAY = 2,
	WEDNESDAY = 3,
	THURSDAY = 4,
	FRIDAY = 5,
	SATURDAY = 6
};	//> CSD-030804

enum NATION_TYPE
{	
	N_NEUTRAL = 0,
	N_NONSTER = 2,
	N_VYSEUS = 3,
	N_ZYPERN = 4,
	N_HEGEMONIA = 5,
	N_YILSE = 6,
	N_NATION_ALL = 7
};

enum HOUSE_TYPE
{	//< CSD-030723
	BLACKSMITH = 0,	 // 대장간
	FARM = 1,		 // 농장
	RESTAURANT = 2,	 // 음식점
	HERBSHOP = 3,	 // 약초점
	SHAMBLES = 4,	 // 푸주간
	CANDLE = 5,		 // 양초점
	SLOPSHOP = 6,	 // 옷가게 
	ALCHEMY = 7,	 // 활 화살
	WOODWORKING = 8, // 목공소
	BOWCRAFT = 9,	 // 활 화살
	SHIPYARD = 10,	 // 조선소
	PUB = 11,	     // 주점
	HOSPITAL = 12,	 // 병원
	STORE = 13,		 // 잡화점
	INN = 14,		 // 여관
	MAGIC_GUILD = 15,
	STORE_TOTAL = 17
};	//> CSD-030723

enum LIFE_TYPE
{	//< CSD-030723
	REMOVE_ = 0,
	ALIVE_ = 1,	
	DEAD_ = 2,
	BUFE_ = 3
};	//> CSD-030723

enum NK_TYPE
{	
	NK_TYPE_STEAL_ = 0,
	NK_TYPE_KILL_ = 1,
	NK_TYPE_NO_KILL_ = 2
};

enum CLASS_TYPE
{	//< CSD-030723
	WARRIOR	= 0,
	THIEF = 1,
	ARCHER = 2,
	WIZARD = 3,
	PRIEST = 4,
	CREATURE = 5
};	//> CSD-030723

enum CHAR_STATUS_TYPE
{	//< CSD-030723
	CS_NOMAL = 0,	// 정상 상태
	CS_REST = 1,	// 휴식 상태
	CS_SLEEP = 2	// 낮에 자기 상태
};	//> CSD-030723

enum VIEW_TYPE
{	
	VIEWTYPE_NORMAL_ = 0,
	VIEWTYPE_CAMELEON_ = 1,
	VIEWTYPE_GHOST_	= 2,
	VIEWTYPE_TREE_	= 3,
	VIEWTYPE_OBSERVE_ = 4,
	VIEWTYPE_CONFUSION_	= 5,
	VIEWTYPE_POISON_ = 6,
	VIEWTYPE_STONE_	= 7,
	VIEWTYPE_SLOW_ = 8,
	VIEWTYPE_FROZEN_ = 9,
	VIEWTYPE_CURSE_	= 10,
	VIEWTYPE_TRANSPARENCY_ = 11,
	VIEWTYPE_SELECT_ = 12,
	VIEWTYPE_STAND_UP_ = 13
};

enum CONDITION_TYPE
{	//< CSD-030723
	CON_NORMAL = 0x00,	// 정상의 상태	 00000000
	CON_CURSE = 0x01,	// 저주의 상태	 00000001
	CON_FROZEN = 0x02,	// 냉동의 상태   00000010
	CON_SLOW = 0x04,	// 감속의 상태   00000100
	CON_DAZE = 0x08,	// 혼란의 상태	 00001000
	CON_POISON = 0x10,	// 중독의 상태	 00010000
	CON_STONE = 0x20,	// 석화의 상태	 00100000
	CON_DEATH = 0x40,	// 죽음의 상태	 01000000
	CON_DARKNESS = 0x80 // 어둠의 상태   10000000
};	//> CSD-030723

enum HUNGRY_TYPE
{	//< CSD-030723
	HT_NORMAL = 0, // (-(HT_WALK * 40.))
	HT_WALK = 1,
	HT_RUN = 2,
	HT_BATTLE_WALK = 3,
	HT_BATTLE_RUN = 4,
	HT_ATTACK = 5,
	HT_MAGIC = 6,
	HT_SKILL = 7,
	HT_COMBAT = 8
};	//> CSD-030723

enum DIRECTION
{	//< CSD-030723
	DIRECTION_SAME = 0,
	DIRECTION_UP = 4,
	DIRECTION_RIGHTUP = 5,
	DIRECTION_RIGHT	= 6,
	DIRECTION_RIGHTDOWN	= 7,
	DIRECTION_DOWN = 0,
	DIRECTION_LEFTDOWN = 1,
	DIRECTION_LEFT = 2,
	DIRECTION_LEFTUP = 3	
};	//> CSD-030723

enum HIT_TYPE
{ 
	HIT_FAILED = 0,
	HIT_AND_DEAD = 1,
	HIT_AND_NOTDEAD = 2,
	HIT_AND_ALIVE = 3,
	HIT_GHOSTRESURRECTION = 4,
	HIT_SUMMONING = 5
};

enum RESIST_TYPE
{	//< CSD-030723
	RESIST_UNKNOWN = 0,
	RESIST_FIRE = 1,
	RESIST_ICE = 2,
	RESIST_ELECT = 3,
	RESIST_POISON = 4,
	RESIST_CURSE = 5,
	RESIST_HOLY = 6,
	RESIST_IGNORE = 7
};	//> CSD-030723

enum eRESIST_TYPE
{
	RT_POISON	=0,
	RT_CURSE	=1,
	RT_HOLY 	=2,
	RT_FIRE		=3,
	RT_ICE		=4,
	RT_ELECT	=5
};

enum TACTIC_TYPE
{ 
	CRAPPLE_ = 0,
	SWORD_ = 1,
	ARCHERY_ = 2,
	FENCING_ = 3,
	MACE_ = 4,
	PIERCE_ = 5,
	WHIRL_ = 6,		
	HURL_ = 7,
	PARRYING_ = 8,
	D_SWORD_ = 9,
	D_MACE_ = 10,
	MAGERY_ = 11,
	ORISON_ = 12
};

enum JOB_TYPE
{	//< CSD-030723
	J_FAMER = 20,
	J_MINNER = 21,
	J_FISHER = 22,
	J_LUMBERJACK = 23,
	J_BUTCHER = 24,
	J_HERB_DIGGER = 25,
	J_HERDSMAN = 26,
	J_CARPENTER = 27,
	J_BLACKSMITH = 28,
	J_COOKER = 29,
	J_TAILOR = 30,
	J_BOWCRAFT = 31,
	J_ALCHEMIST = 32,
	J_CANDLEMAKER = 33,
	J_MERCHANT = 34
};	//> CSD-030723

enum MAGIC_TYPE
{ 
	MT_PROTECTION = 2,
	MT_ASSISTANCE = 3,
	MT_CURSE = 4
};

enum CHAR_STEP_TYPE
{	//< CSD-TW-030620
	CLS_STEP = 0,  // 듀얼 클래스 단계
	DUAL_CLS = 1,  // 듀얼 클래스 종류
	EXP_STEP = 2,  // 경험치 단계
	T01_STEP = 3,  // 택틱 경험치 단계
	T02_STEP = 4,
	T03_STEP = 5,
	T04_STEP = 6,
	T05_STEP = 7,
	T06_STEP = 8,
	T07_STEP = 9,
	T08_STEP = 10,
	T09_STEP = 11,
	T10_STEP = 12,
	T11_STEP = 13,
	T12_STEP = 14,
	T13_STEP = 15,
	CSP_STEP = 16, // 전투스킬 포인터
	MAX_STEP = 17  // 최대 데이타수
};	//> CSD-TW-030620

enum WEAPON_RANGE_TYPE
{	//< CSD-031007
	WR_SHORT = 0,
	WR_MIDDLE = 1,
	WR_LONG1 = 2,
	WR_LONG2 = 3,
	WR_LONG3 = 4,
	WR_EFFECT = 5
};	//> CSD-031007

enum BATTLE_TYPE
{	//< CSD-031007
	// 물리적 공격 
	PHYSICAL = 0,                  
	// 마법
	LIFE_UP = 1,                   // 체력회복 
	PROTECTION = 2,                // 보호
	SHIELD = 3,                    // 보호막
	DECIPHER = 4,                  // 해독 마법
	HEAL_DISEASE = 5,              // 질병치료
	GREAT_HEAL = 6,                // 대체력회복
	SLOW = 10,                     // 속도저하
	CONFUSION = 11,                // 혼란
	INTOXICATION = 12,             // 중독
	STONELY =	13,                // 석화
	ABSOLUTE_PARALYSIS = 14,       // 절대마비
	TOXIC_CLOUD = 15,              // 독구름  
	DISEASE_INDUCE = 16,           // 질병유발 
	VAMPIRE_TOUCH = 17,            // 뱀파이어의 손길
	SPIDER_WEB = 19,               // 거미줄  
	MAGICAL_TRAP = 20,             // 마법트랩
	ENCHANT_WEAPON = 21,           // 인첸트웨폰
	TRANSPARENCY = 22,             // 투명주문
	NULLITY_MAGIC = 24,            // 마법무효화  
	MINOR_AMPLIFY = 25,            // 소증폭
	MAGIC_AMPLIFY = 26,            // 마법증폭
	FIRE_RESIST = 28,              // 불에 대한 저항  
	FIRE_ARROW = 29,               // 화염화살 
	FIRE_BALL= 30,                 // 화염구
	SCORCHER = 31,                 // 화염군(화염방사)
	CIRCLE_FLAME= 33,              // 방사화염
	FIRE_WALL = 34,                // 화염벽
	FLAME_WAVE = 35,               // 움직이는 화염벽
	APOCALYPSE = 36,               // 지옥의 불
	ICE_RESIST = 37,               // 빙계 저항
	ICE_ARROW = 38,                // 얼음화살
	ICE_BALL = 39,                 // 얼음구 
	FREEZE_5TILE_RADIUS = 41,      // 반경 5타일 얼림	
	ICE_WALL = 42,                 // 얼음벽  	
	CIRCLE_ICE = 43,               // 원형얼음 공격
	ICE_STORM = 44,                // 얼음폭풍
	FREEZE_9TILE_RADIUS = 45,      // 반경 9타일 얼리기	
	LIGHT = 46,                    // 빛
	ELECT_RESIST = 47,             // 전격에 대한 저항
	LIGHTNING_BOLT = 48,           // 벼락
	CHAIN_LIGHTNING = 50,          // 유도 전격
	CIRCLE_LIGHTNING = 51,         // 원형 전격 
	LIGHTNING = 52,                // 직선 전격
	TWINKLE_STAR = 53,             // 수호성
	LIGHTNING_5TILE_RADIUS = 54,   // 반경 5타일 전격
	LOW_MONSTER_RECALL = 55,       // 저급 몬스터 소환
	MIDDLE_MONSTER_RECALL = 58,    // 중급 몬스터 소환
	SNOWSTORM	= 59,			   // 눈보라의 폭풍
	ICE_BREATH2	= 60,			   // 아이스 브레스2
	BLOOD_LIGHTNING = 61,		   // 블러드 라이트닝
	PHASING = 64,                  // 장소이동
	MEMORIZING_LOCATION = 65,      // 이동장소기억
	TELEPORT = 66,                 // 순간이동
	MULTI_PORT = 67,               // 복수차원이동
	TOWN_PORTAL = 68,              // 차원이동
	MAGIC_DETECT = 75,             // 마법탐지
	REMOVE_CURSE = 78,             // 저주해제
	PROTECT_RANGE_WEAPON = 79,     // 장거리 무기로부터 보호
	PHEONIX = 80,				   // 불새
	METEOR_STRIKE = 81,			   // 운석 // 030509 kyo //030506이전 110
	LOST_SIGHT = 82,               // 시력상실   
	SPEED_UP = 83,                 // 속도증가
	GREAT_THUNDER_BOLT = 84,       // 대전격
	GREAT_METEOR = 85,             // 대운석 
	DRAGON_SCALE = 86,             // 드래곤의 비늘
	DEATH_FIRE = 87,               // 데쓰파이어
	SUMMONING_SKELETON = 88,       // 데쓰나이트 소환
	FLYING_SPARK = 89,             // 플라잉 스파크
	VAMPIRE_POISON = 90,           // 뱀파이어의 독
	VAMPIRE_DODGE = 91,            // 뱀파이어 회피
	SUMMONING_UNDEAD = 92,         // 언데드 소환
	FEATHER_OF_CURSE = 93,         // 마비의 깃털
	CURSE_OF_HELL = 94,            // 헬의 저주
	RECALL_FOLLOW = 95,            // 부하 소환
	FIRE_EXPLOSION = 96,           // 화염폭발
	FIRE_STORM = 97,               // 파이어스톰
	SHADOW_OF_EVIL = 98,           // 악의 그림자
	FREEZING = 99,                 // 주위 얼림
	BLITZ_SPEAR = 100,             // 번개창
	LIGHTNING_CLOUD = 101,         // 전격 구름
	FIRE_BREATH = 102,             // 화염 브레쓰
	ICE_BREATH = 103,              // 얼음 브레쓰
	LIGHTNING_BREATH = 104,        // 전격 브레쓰
	FLAME_PILLAR = 105,            // 화염 기둥
	FLAME_POUR = 106,              // 화염 폭풍 
	OCTA_LIGHTNING = 107,          // 방사 전격
	BLOOD_SPRINKLE = 108,          // 피뿌리기
	GAZER	= 109,				   // 게이저
	FLAME_ARROW = 110,             // 화염화살 

	AREA_BAT_ATTACK = 111,         // 지역 박쥐공격
	AREA_PARALYSIS = 112,          // 지역 마비
	AREA_POISON = 113,             // 지역 중독
	AREA_CONFUSION = 114,          // 지역 혼란
	AREA_SLOW = 115,               // 지역 속도저하

	RESTORE_AUTHORITY = 151,       // 회복의권능  
	PROTECT_DANGER = 152,          // 위험으로부터 보호
	BLESS = 153,                   // 신의 축복
	RECOVER_POISON = 154,          // 해독의 권능
	CURE_DISEASE = 155,            // 질병치료의 권능
	MIRACLE_RECOVER = 156,     	   // 완치의 기적
	PERFECT_PROTECT = 157,         // 완전한 결계  
	GREAT_SHIELD = 158,            // 보호의 기적
	CANCEL_DIVINE = 159,           // 반신력 무효화
	RECURRENCE_DIVINE = 160,       // 회귀의 권능
	PRAY_POWER = 161,              // 기도의 역사
	HOLY_LIGHT = 162,              // 성스러운 빛  
	ANTHEM = 163,                  // 성가
	RELIEF_AUTHORITY = 164,        // 구원의 빛 
	HOLY_FIRE = 165,               // 성스러운 불
	GREAT_PRAY = 166,              // 위대한 기도  
	APPRECIATE = 167,              // 바른 깨달음
	INFLUENCE_DIVINE = 168,        // 신에의 감화
	VOICE_SOUL = 169,              // 영혼의 소리
	HOLY_RAGE = 170,               // 성스런 분노
	TURN_UNDEAD = 171,             // 턴 언데드
	CURSE = 172,                   // 신의 저주 
	AUTHORITY_RECOVER_SIGHT = 173, // 시력회복의 권능	
	DIVINE_POWER = 174,            // 디바인 파워
	AUTHORITY_LUCK = 175,          // 절대 행운의 권능	
	AUTHORITY_LOST_SIGHT = 177,    // 시력상실의 권능
	PRAY = 178,	                   // 기도
	HOLY_CURE = 179,               // 신성한 치유
	AUTHORITY_SIMUNIAN = 181,      // 시무니안의 권능
	AUTHORITY_CHARISNUMEN = 182,   // 카리스 누멘의 권능
	AUTHORITY_LETHY = 183,         // 레티의 권능
	AUTHORITY_OF_ATHYAS	= 185,     // 아샤스의 권능
	AUTHORITY_OF_WHOARENCHA	= 187, // 화렌차의 권능
	RAGE_STONE = 189,              // 대지의 분노
	LIFE_DRAIN = 190,              // 생명력 탈취  
	RAGE_BLAST = 191,              // 화신의 분노
	RAGE_LIGHT_BALL = 192,         // 성령의 분노
	THROW_ARROW = 194,             // 화살
	THROW_KNIFE = 195,             // 비도
	// 번개 속성 전투스킬
	LIGHTNING_BOOM = 215,          
	THUNDER_BLOW = 216,
	LIGHTNING_SHOCK = 217,
	THUNDER_STRIKE = 218,
	LIGHTNING_SHIELD = 219,
	LIGHTNING_EXTREME = 220,
	// 대지 속성 전투스킬
	BLOOD_EARTH = 221,             
	GROUND_ATTACK = 222, 
	BLOOD_WILL = 223,
	STONE_ARMOR = 224,
	SIMUNIAN_BOSOM = 225,
	EARTH_EXTREME = 226,
	// 화염 속성 전투스킬 
	SWORD_N_ROSES = 227,
	MULTIPLE_FIRE = 228, 
	FLEET_MISSILE = 229,
	CONVERTING_ARMOR = 230, 
	BURNING_BLOODY= 231,
	FIRE_EXTREME = 232,
	// 어둠 속성 전투스킬
	POISONING_NOVA = 233,
	CHERROY_SHADE = 234, 
	DARK_BLADE = 235, 
	TYBERN_GIFT = 236,
	ACUQUIRE_DARK = 237,
	DARK_EXTREME = 238,
	// 얼음 속성 전투스킬
	ICING_BLAST = 239,
	CRITICAL_HIDING = 240, 
	SHARK_MISSILE = 241, 
	HORN_OF_ICEBERG = 242, 
	ICE_SHIELD = 243,
	ICE_EXTREME = 244,
	// 바람 속성 전투스킬
	CHARGING = 245,
	DOUBLE_ATTACK = 246, 
	WHILWIND = 247,
	TWISTER = 248, 
	GUST = 249, 
	WIND_EXTREME = 250,
	// 이펙트 아이템
	ITEM_FIRE_EFFECT = 610, // CSD-030916
	ITEM_ICE_EFFECT = 611, // CSD-030916
	ITEM_LIGHTNING_EFFECT = 612, // CSD-030916	
	// 이벤트
	THROW_SNOW = 193,
	THROW_WATER_BALL = 661,
	THROW_BIG_GULP = 663,
	// 아이템 효과
	ITEM_EFFECT_FIRE = 605,
	ITEM_EFFECT_ICE = 606,
	ITEM_EFFECT_LIGHTNING = 607
};	//> CSD-031007

enum COMBAT_RESULT
{
	CSR_NORMAL,    // 정상적인 관계
	CSR_PRIOR,     // 승패관계에서 승
	CSR_POSTERIOR, // 승패관계에서 패
	CSR_CONFLICT,  // 대립적인 관계
	CSR_EQUAL      // 동등한 관계
};

enum VEHICLE_KIND
{
	HORSE = 1,  // 말
	BULL = 2,   // 황소
	PHANTOM = 3 // 팬텀
};

enum NPC_KIND
{	//< CSD-TW-030606
	DUMMY = 0,
	HUMAN = 1,
	GUARD = 2,
	ORC_DUMMY = 3,
	LIFELESS = 4,
	HOBBIT = 5,
	DWARF = 6,
	DRAGON = 7,
	ANIMAL = 8,
	NORM_MON = 9,
	SEALSTONE = 11,
	GUARDSTONE = 12,
	//CASTLEGATEWAY = 12
	//WARARMS = 12
};	//> CSD-TW-030606

enum ATTR_TYPE
{	//< CSD-031027
	AT_NORMAL = 0,
	AT_FIRE = 1,
	AT_ICE = 2,
	AT_LIGHTING = 3,
	AT_UNDEAD = 4,
	AT_PIERCING = 5,
	AT_ANTIMAGIC = 6,
	AT_BOSS = 7,
	AT_SUMMON = 8,
	AT_TAMING = 9,
	AT_SEMIBOSS = 10
};	//> CSD-031027

enum NPC_PARAMETER_TYPE
{	//< CSD-031106 : Client에서 변화된 NPC 파라메터를 Server로 보내준다. 
	NPC_PARAMETER_TARGETID = 0,
	NPC_PARAMETER_TARGETNAME = 1,
	NPC_PARAMETER_BOSSID = 2,
	NPC_PARAMETER_HOSTID = 3,
	NPC_PARAMETER_TAME_CMD = 4,
	NPC_PARAMETER_PATTERNTYPE = 5,
	NPC_PARAMETER_OLDPATTERNTYPE = 6,
	NPC_PARAMETER_ATTRIBUTE = 7
};	//> CSD-031106

enum ARENA_TYPE
{
	AT_UNKNOWN = 0,
	AT_CORE = 1,
	AT_STONE = 2,
	AT_MONSTER = 3
};

enum ARENA_STATE
{
	CS_NOW_PEACE	= 0, // 평화로운 상태(보통맵과 다를게 없는 상태
	CS_NOW_REGISTER	= 1, // 각각 팀에 등록하는 상태. 
	CS_NOW_FIGHT	= 2  // 전투 시작 상태
};

enum ARENA_RESULT
{
	FST_WIN = 0,
	FST_DEFEAT = 1,
	FST_DISCONNECT = 2
};

enum ARENA_ITEM
{
	GT_RESOURCE = 5,
	GT_ARENA_LOW = 6,
	GT_ARENA_HIGH = 7
};
//> CSD-030517
///////////////////////////////////////////////////////////////////////////////

struct AREA
{	
	BOOL	  bAlive;
	short int Type;
	short int ID;
};

///////////////////////////////////////////////////////////////////////////////
//< CSD-030723
enum eBillingType
{	//-값으로 구성 해야 합니다. CheckPW_TotalDB_SQL이 남은 날짜(양수)를 리턴하기 때문입니다.
	BT_WRONG_ID					= -1,//ID 틀림
	BT_WRONG_PW					= -2,//PW 틀림

	BT_NEED_PAY					= -3,//돈 부족

	BT_COMMERCIAL_TIME_REMAIN	= -4,//남은 시간 있는놈
	BT_COMMERCIAL_IP			= -20,//IP 종량제
	BT_FREE						= -100,//공짜
	BT_WAIT_BILLING_MSG			= -200,//대만 같은 곳의 처리 방식 빌링을 기다리도록 처리
};	//DBDemon과 항상 동기화 시키십시요

enum eBankMoneyChangeType
{
	BMCT_ARENA_GAMBLE_DIVIDEND	=	1,
	BMCT_ARENA_GAMBLE_BETTING	=	2,
	BMCT_BANK_DEPOSIT			=	3,
	BMCT_BANK_DEFRAYAL			=	4,
	BMCT_AUCTION_ITEM_BUY		=	5,
	BMCT_AUCTION_ITEM_REGISTER	=	6,
	BMCT_BUY_LOTTO				=	7,
	BMCT_RESET_ABILITY			=	8,
	BMCT_CHAR_LOGIN				=	9,
	BMCT_RECV_MAP_CHANGE		=	10,
	BMCT_RECV_CHAR_INFO_BASIC	=	11,
	BMCT_SKILL_RESET			=	12,
};

enum eSendCharInfoType//021030 lsw 
{
	STR			=0,
	DEX			=1,
	CON			=2,
	WIS			=3,
	INT_		=4,
	CHA			=5,
	MOVP		=6,
	ENDU		=7,
	MOR			=8,
	WSPS		=9,
	LUCK		=10,
	HEALTH		=11,
	MOV			=12,
	AC			=13,
	FAME		=14,
	DAMAGE		=15,
	HIT			=16,
	TACTICS		=17,
	MONEY		=18,
	LVPOINT		=19,
	LEV			=20,
	EXP			=21,
	HP	  		=22,
	MAX_HP		=23,
	MP			=24,
	MAX_MP		=25,
	SP			=26,
	MAX_SP		=27,
	CP      	=28,
	MAX_CP  	=29,
	BP	    	=30,
	MAX_BP  	=31,
	R_FIR		=32,
	R_ICE		=33,
	R_ELE		=34,
	DAM_MIN		=35,
	DAM_MAX		=36,
	HIT_RAT		=37,
	DEF_RAT		=38,
	TAC_L1		=39,
	TAC_L2		=40,
	TAC_L3		=41,
	TAC_L4		=42,
	TAC_L5		=43,
	TAC_L6		=44,
	TAC_L7		=45,
	TAC_L8		=46,
	TAC_L9		=47,
	TAC_L10		=48,
	TAC_L11		=49,
	TAC_L12		=50,
	TAC_L13		=51,
	TAC_E1		=52,
	TAC_E2		=53,
	TAC_E3		=54,
	TAC_E4		=55,
	TAC_E5		=56,
	TAC_E6		=57,
	TAC_E7		=58,
	TAC_E8		=59,
	TAC_E9		=60,
	TAC_E10		=61,
	TAC_E11		=62,
	TAC_E12		=63,
	TAC_E13 	=64,
	BANKMONEY	=65,
	FAITH		=66,
	VIEWGHOST	=67,
	VIEWBOMB	=68,
	VIEWTRAP	=69,
};

enum eMkType//021030 lsw 
{
	MK_NORMAL      =0,	// 일반적인 경우
	MK_WARNING     =1,	// 위험, 경고
	MK_SHORTAGE    =2,	// 부족
	MK_INFORMATION =3,	// 정보
};

enum eSpriteType
{
	SPRITETYPE_NONE			=0,
	SPRITETYPE_CHARACTER	=1,
	SPRITETYPE_NPC			=2,
};
//Skill No
enum eSkillNoIndex
{
	SKILL_UNKNOWN							=0,	// 030117 kyo 
	SKILL_DETECT_GHOST						=1,	
	SKILL_TALK_WITH_GHOST					=2,	
	SKILL_POISONING							=3,	
	SKILL_DETECT_POISON						=4,	
	SKILL_FIND_TRAPS						=5,	
	SKILL_REMOVE_TRAPS						=6,	
	SKILL_ENTRAPMENT						=7,	
	//	SKILL_MAGIC_SCROLLING					=8,	
	//	SKILL_ABILITY_SCROLLING					=9,	
	//	SKILL_EMERGENCY_ALRAM					=10,	
	//	SKILL_FIRST_AID							=11,	
	//	SKILL_FAST_FOOT							=12,	
	//	SKILL_SECRETHEAR						=13,	
	//	SKILL_WALLCLIMBING						=14,	
	//	SKILL_INTUITION							=15,	
	SKILL_DETECT_BOMB						=16,	
	SKILL_BOMB_IN_BOX						=17,	
	SKILL_REMOVE_BOMB						=18,	
	SKILL_STEALING							=19,	
	//	SKILL_LOCKPICKING						=20,	
	SKILL_RECLAIM							=21,	
	SKILL_SOW								=22,	
	//	SKILL_CULTIVATE							=23,	
	SKILL_HAVEST							=24,	
	//	SKILL_DETECT_MINERAL					=25,	
	SKILL_MINING							=26,	
	SKILL_REFINE							=27,	
	SKILL_FISH_HOOKING						=28,	
	//	SKILL_BAITMAKING						=29,	
	SKILL_NETTING							=30,	
	//	SKILL_SAILING							=31,	
	SKILL_CUTTING							=32,	
	SKILL_LUMBERING							=33,	
	SKILL_BUTCHERING						=34,	
	//	SKILL_DETECT_HERB						=35,	
	SKILL_DIGGING							=36,	
	SKILL_MAKEPOWDER						=37,	
	//	SKILL_HERDING							=38,	
	//	SKILL_MILKING							=39,	
	//	SKILL_MIDWIFERY							=40,	
	SKILL_SHEAR								=41,	
	SKILL_CARPENTRY							=42,	
	SKILL_BLACKSMITHY						=43,	
	SKILL_COOKING							=44,	
	SKILL_CLOTHING							=45,	
	//	SKILL_DYEING							=46,	
	SKILL_TAILORING							=47,	
	SKILL_BOWCRAFTING						=48,	
	SKILL_FLETCHING							=49,	
	SKILL_MAKEPOTION						=50,	
	SKILL_MAKEMETAL							=51,	
	SKILL_MAKEBOTTLE						=52,	
	SKILL_CANDLEMAKING						=53,	
	//	SKILL_WAXING							=54,	
	//	SKILL_REPAIRNG							=55,	
	//	SKILL_REUTILIZING						=56,	
	//	SKILL_PLAYING							=57,	
	//	SKILL_PEACEMAKING						=58,	
	//	SKILL_PARALYSIS							=59,	
	//	SKILL_HALLUCINATION						=60,	
	//	SKILL_PROVOCATION						=61,	
	//	SKILL_ENTICEMENT						=62,	
	//	SKILL_NOISE								=63,	
	//	SKILL_TRACKING							=64,	
	SKILL_SNOOPING							=65,	
	//	SKILL_SEARCH							=66,	
	SKILL_MERCHANT							=67,	
	SKILL_BLACKSMITHY_MASTER				=68,	//대장장이 기성	
	SKILL_CARPENTRY_MASTER					=69,	//목수 기성	
	SKILL_BOWCRAFTING_MASTER				=70,	//활제작사 기성	
	SKILL_TAILORING_MASTER					=71,	//재단사 기성
	//	SKILL_HIDE								=72,	
	//	SKILL_CAMPING							=73,	
	SKILL_MAKEFIRE							=74,	
	SKILL_TAMING							=75,	
	//	SKILL_COMMUNICATE_WITH_ANIMAL			=76,	
	//	SKILL_COMMUNICATE_WITH_OTHER_TRIBE		=77,	
	//	SKILL_ANIMAL_LORE						=78,	
	//	SKILL_ARMS_LORE							=79,	
	SKILL_EVALUATE_INTELIGENCE				=80,	
	SKILL_EVALUATE_WISDOM					=81,	
	SKILL_ANATOMY_JUDGEMENT					=82,	
	SKILL_ITEM_IDENTIFICATION				=83,	
	//	SKILL_TASTE_IDENTIFICATION				=84,	
	//	SKILL_APPRAISAL							=85,	
	//	SKILL_HEALING							=86,	
	SKILL_TACTICS							=87,//021113 lsw
	TACTICS_Crapple							=88,
	TACTICS_swordmanship					=89,
	TACTICS_Archery							=90,
	TACTICS_Fencing							=91,	
	TACTICS_Mace_fighting					=92,
	TACTICS_Pierce							=93,
	TACTICS_Whirl							=94,
	TACTICS_Hurl							=95,
	TACTICS_Parrying						=96,
	TACTICS_Double_Swordmanship				=97,
	TACTICS_Double_MaceFighting				=98,	
	TACTICS_Magery							=99,
	TACTICS_Orison							=100,
	//	SKILL_THREAD							=101,	
	//	SKILL_COLORING							=102,	
	//	SKILL_JUMP								=103,	
	//	SKILL_RIDING							=104,	
	SKILL_GHOST_RESURRECTION				=105,	
	SKILL_GM_CARPENTER						=106,	
	SKILL_GM_BLACKSMITH						=107,	
	SKILL_GM_COOK							=108,	
	SKILL_GM_TAILOR							=109,	
	SKILL_GM_BOWCRAFT						=110,	
	SKILL_GM_ALCHEMIST						=111,	
	SKILL_GM_CANDLEMAKER					=112,	
};
// 아이템 kind
enum eItemKindType
{
	IK_NONE							=0,	//	non
	IK_GON							=1,	//	곤			
	IK_PENCING						=2,	//	펜싱검		
	IK_ONE_SWORD					=3,	//	한손검
	IK_ONE_AXE						=4,	//	한손도끼
	IK_ONE_MACE						=5,	//	한손철퇴
	IK_CLUB							=6,	//	몽둥이
	IK_WHIRL						=7,	//	봉
	IK_STAFE						=8,	//	지팡이
	IK_SPEAR						=9,	//	보통창
	IK_FORK							=10,	//	포크
	IK_HURL							=11,	//	비검,던지는무기
	IK_SHORT_BOW					=12,	//	소형활
	IK_TWO_SWORD					=13,	//	양손검
	IK_TWO_AXE						=14,	//	양손도끼
	IK_TWO_MACE						=15,	//	양손 철퇴
	IK_TWO_ARROW					=16,	//	화살
	IK_IRON_ARMOR					=17,	//	쇠갑옷
	IK_LEATHER_ARMOR				=18,	//	가죽갑옷
	IK_RING							=19,	//	반지
	IK_SHIELD						=20,	//	방패
	IK_SHOES						=21,	//	신발
	IK_ROBE							=22,	//	로브
	IK_FULL_HELMET					=23,	//	플헬멧
	IK_HALF_HELMET					=24,	//	하프헬멧
	IK_NORMAL_CLOTHE				=25,	//	보통옷
	IK_AMULET						=26,	//	목거리
	IK_ARMLET						=27,	//	팔찌
	IK_MUSIC						=28,	//	악기
	IK_SCROLL						=29,	//	스크롤
	IK_LONG_BOW						=30,	//	대형활
	IK_BOX							=31,	//	상자
	IK_GRAIN						=32,	//	곡식
	IK_POTION						=33,	//	포션
	IK_FOOD_MASS					=34,	//	덩어리 음식
	IK_PLATE_FOOD					=35,	//	쟁반 음식
	IK_DRINK						=36,	//	음료
	IK_ARROW_POINT					=37,	//	화살 촉
	IK_FEATHER						=38,	//	깃털		
	IK_SEED							=39,	//  씨앗.
	IK_MONEY						=40,	//	돈
	IK_MINERAL						=41,	//	광석
	IK_MINERAL_LUMP					=42,	//	가공한 광석( 철괴, 금괴... )
	IK_HERB							=43,	//	약초					// 0626 YGI
	IK_HERB_POWDER					=44,	//	약초 가루
	IK_WOOD_BEFOR_USE				=45,		//가공전의 나무
	IK_WOOD_AFTER_USE				=46,		//가공후의 나무
	IK_TENT							=47,		//텐트류
	IK_TOOL_RECLAI					=48,		//도구 개간용
	IK_TOOL_HAVEST					=49,		//도구 추수용
	IK_TOOL_MINING					=50,		//도구 채광용
	IK_TOOL_REFINE					=51,		//도구 제련용
	IK_TOOL_FISH_HOOKING			=52,		//도구 낚시용
	IK_TOOL_CUTTING					=53,		//도구 벌목용
	IK_TOOL_LUMBERING				=54,		//도구 목재가공용
	IK_TOOL_BUTCHERING				=55,		//도구 도축용
	IK_TOOL_DIGGING					=56,		//도구 약초채집용
	IK_TOOL_MAKEPOWDER				=57,		//도구 약재만들기용
	IK_TOOL_SHEAR					=58,		//도구 가위류
	IK_TOOL_CARPENTRY				=59,		//도구 목공용
	IK_TOOL_BLACKSMITHY				=60,		//도구 철공용
	IK_TOOL_COOKING_FIRE			=61,		//도구 대우는요리용
	IK_TOOL_COOKING_CUT				=62,		//도구 써는요리용
	IK_TOOL_PIN						=63,		//도구 바늘류
	IK_TOOL_BOWCRAFTING				=64,		//도구 활제작용
	IK_TOOL_FIRE_BOAD				=65,		//도구 불대
	IK_TOOL_CANDLEMAKING			=66,		//도구 양초제작용
	IK_TOOL_OILING					=67,		//도구 밀납가공용
	IK_TOOL_TAMING					=68,		//도구 테이밍용
	IK_TOOL_NETTING					=69,		//도구 그물류
	IK_TOOL_MAKEFIRE				=70,		//도구 부싯돌류
	IK_WORK_BOAD_BLAST_FURNACE 		=71,		//작업대 용광로
	IK_WORK_BOAD_BAETLE				=72,		//작업대 배틀
	IK_WORK_BOAD_BLOCK				=73,		//작업대 선반
	IK_WORK_BOAD_ANVIL				=74,		//작업대 모루
	IK_WORK_BOAD_BRAZIER			=75,		//작업대 화덕
	IK_WORK_BOAD_TEST_TUBE			=76,		//작업대 시험관류
	IK_WORK_BOAD_SPINNING_WHEEL		=77,		//작업대 물레
	IK_WORK_BOAD_FIRE_BOAD			=78,		//작업대 불대
	IK_TOOL_BUCKET					=79,		//도구 양동이
	IK_TOOL_RECYCLE					=80,		//도구 재활용
	IK_TOOL_REPAIR					=81,		//도구 수리용
	IK_IRON_SHIELD					=82,		//쇠방패	
	IK_IRON_SHOES					=83,		//쇠 신발	
	IK_CAP							=84,		//모자	
	IK_GOKAL						=85,		//두건	
	IK_PRIEST_ROBE					=86,		//성직로브	
	IK_CANDLE						=87,		//양초	
	IK_MAN_TO_MAN					=88,		//결투서	
	IK_CRACKER						=89,		// 폭죽			// 010210 YGI
	IK_RAJA_EMBLEM					=90,		// 라자 엠블렘
	IK_POTION_CHANGE_COLOR_CLOTHES	=91,	// 옷 염색약
	IK_POTION_CHANGE_COLOR_HAIR		=92,	// 피부 염색약
	IK_NEW_MONEY					=93,	// 신권돈
	IK_LOCK_STEAL					=94,	// 도둑방지 덫
	IK_MAP							=95,	//	지도
	IK_GAUNTLET						=96,		//건틀렛
	IK_GOOD_FOOD					=97,		//고급음식
	IK_GOOD_POTION					=98,		//고급포션
	IK_GOOD_CANDLE					=99,		//고급양초
	IK_UNIQ_RESOURCE				=100,		//유니크재료
	IK_GENERAL_GAMBLE				=101,		//일반겜블아이템
	IK_GRADE_GAMBLE					=102,		//그레이드겜블아이템
	IK_CLOAK						=103,		//망토
	IK_CHAIR						=104,		//의자			// LTS SITDOWN
	IK_BOMB							=105,		//011212 lsw 
	IK_TRAP							=106,		//011212 lsw 
	IK_THROW_EVENT					=107,		// CSD-030723 : 이벤트용 던지기 물건
	IK_POTION_BOX					=108,		// 포션 주머니		020418 YGI
	IK_FALG							=109,		// 깃발
	IK_SADDLE						=110,		// 말안장
	IK_DUAL_GAUNTLET				=111,	
	IK_DUAL_SWORD					=112,	
	IK_DUAL_BOW						=113,	
	IK_DUAL_FENCING					=114,	
	IK_DUAL_AXE						=115,	
	IK_DUAL_SPEAR					=116,	
	IK_DUAL_KNIFE					=117,	
	IK_DUAL_DOUBLE_SWORD			=118,	
	IK_DUAL_DOUBLE_AXE				=119,	
	IK_DUAL_MAGICIAN_STAFF			=120,	
	IK_DUAL_PRIEST_STAFF			=121,	
	IK_DUAL_SHIELD					=122,	
	IK_DUAL_ARMOR					=123,	
	IK_DUAL_ARCHER_LEATHER			=124,	
	IK_DUAL_MAGICIAN_ROBE			=125,	
	IK_DUAL_HELM					=126,	
	IK_DUAL_ARCHER_CAP				=127,	
	IK_DUAL_MAGICIAN_HAT			=128,	
	IK_DUAL_BOOTS					=129,	
	IK_DUAL_ARCHER_SHOES			=130,	
	IK_DUAL_MAGICIAN_SHOES			=131,	
	IK_DUAL_THIEF_SHOES				=132,	
	IK_DUAL_PRIEST_SHOES			=133,	
	IK_DUAL_THIEF_CAP				=134,	
	IK_DUAL_PRIEST_HAT				=135,	
	IK_DUAL_THIEF_LEATHER			=136,	
	IK_DUAL_PRIEST_ROBE				=137,	
	IK_DUAL_THIEF_SHIELD			=138,	
	IK_DUAL_ARROW					=139,	
	IK_ABYSS_MAZE					=140,	//아비스 미궁용 아이템	
	IK_UNDEAD_DUNGEON				=141,	//언데드 던전용 아이템	
	IK_GREAT_MAZE					=142,	//대미궁 용 아이템	
	IK_TREASURE_MAP					=145,	// 보물지도		// 040105 YGI 보물
	IK_POTION_BAG					=147,	// BBD 040213 내구도 있는 포션
};

enum eFuntionItemIndex
{
	FITEM_MOVABLE			=1,	//마비,석화,혼란 해제	마비계열해제	
	FITEM_RECOVERY			=2,	//속도저하 해제	속도저하 해제	
	FITEM_DETOX				=3,	//해독작용	해독	
	FITEM_ENLIGHT			=4,	//시력 회복	시력상실	
	//FITEM_NULL			=5,	NULL	NULL	
	//FITEM_NULL			=6,	NULL	NULL	
	FITEM_REVIVAL			=7,	//자동 부활	자동 부활	
	FITEM_COME_BACK			=8,	//마을로의 귀환	자신이 속한 국가의 마을로 돌아옴 	
	FITEM_RESIST_UP			=9,	//모든 계열 저항력 상승	모든 계열 마법 저항력 상승	
	FITEM_STEALTH			=10,	//투명 상태 유지	일정 시간 투명 상태 유지	
	FITEM_INVALID_CURSE		=11,	//저주 계열 마법 무효화	상대의 저주 계열 마법 무효화	
	FITEM_INVALID_OFFENCE	=12,	//공격 계열 마법 무효화	상대의 공격 계열 마법 무효화	
	FITEM_POWER_UP			=13,	//물리적 공격력 증가	물리적 공격력 증가(%)	
	FITEM_SPEED_UP			=14,	//이동 속도 증가	이동 속도 (이동력) 증가	
	FITEM_HEAVY_ARMOR		=15,	//AC 증가	AC 수치 증가	
	FITEM_SPELL_UP			=16,	//마법 데미지 증폭	마법 데미지 증폭(%)	
	FITEM_SPELL_TIME_UP		=17,	//마법 지속 시간 증폭	마법 지속 시간 증폭(msec)	
	FITEM_CRITICAL			=18,	//CRITICAL DAMAGE 	크리티컬 데미지로 물리적 공격	
	FITEM_STROKE			=19,	//물리적 공격 성공률 100%	물리적 공격 성공률 100%	
	FITEM_AGILITY			=20,	//물리적 공격 회피율 70%	물리적 공격 회피율 70%	
	FITEM_INVALID_PROTECT	=21,	//상대의 보호 계열 마법 무시	상대의 보호 계열 마법을 무조건 무시하여 공격(물리/마법 모두)	
	FITEM_INVALID_RESIST	=22,	//상대의 마법 저항력 무시	상대의 마법 저항력 무조건 무시 (마법 저항력 0으로 적용)	
	FITEM_GHOST				=23,	//유령을 볼 수 있음	유령을 볼 수 있음	
	FITEM_SAMARITAN			=24,	//NK의 공격을 무시	NK의 물리적/마법 공격 모두 무시	
	FITEM_SLIGHT			=25,	//마법 캐스팅 시간 단축	마법 캐스팅 시간 단축(%)	
};
//< CSD-030723
///////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// 마법 테이블 구조체
struct MAGICTABLE
{
	int	serial_Num;		    // 고유번호 : 마법(1 ~ 200), 전투스킬(215 ~ 250)
	int	order_Type;		    // 계열 : 마법사(1 ~ 12), 성직자(21 ~ 31), 전투스킬(61 ~ 66)
	char Han_Name[30];
	int	appoint_Type;		  // 대상 지정방식 타입(0->나자신/1->대상지정(단일대상)/2->대상지정(복수대상)/3->지역마법)
	int	basic_Level;		  // 사용 기본 레벨(1~9)
	int	point_MS;		      // Magic S point 
	int before_Magic;		  // 전 단계 마법(먼저 익혀야 할 마법)
	int	basic_magery ;		//
	int	cast_Time;		    // 캐스팅 시간(절대시간)
	int	cast_Continue;	  // 캐스팅 지속 시간(절대시간)
	int	continue_Time;	  // 지속시간(0->영원히/1~절대시간)
	int	avail_Type;		    // 효용 타입(0->자신/1->자신 아닌 1명/2~->유효범위내 다수)
	int	avail_Range;	    // 사정거리(타일단위/0->자신/99->무한)
	int	exhaust_MP;		    // 소모 MP
	int	exhaust_MS;		    // 소모 MS
	int	require_IT;		    // 요구 지능
	int	require_WP;		    // 요구 지혜
	int nHDPoint; // CSD-030314 : 마법 데미지 기준값
	int nHDLimit; // CSD-030314 : 마법 데미지 기준값 제한치(절대값)
	int	point_WS;		      // WS 포인트(0->not available/1~)
	int	point_Move;		    // 이동포인트(0->not available/1~)
	int	invalid_TargetIT;	// 마법 사용가능 범위
	int nClass;           // 전투스킬 사용가능 클래스  
	int nTactics;         // 전투스킬 사용가능 무기택틱
	int nAddition;        // 전투스킬 사용시 소모되는 소모량
	int aLevel[4];        // 전투스킬 레벨
	int aRandom[4];       // 전투스킬 레벨 편차
	int Spell_Type ;		  // 0 Wizard, 1~13 priest(God별 type들 때문에 다양
	int	Appoint_Type ;
	int	Excute_Type;
	int	magic_Type;
	int	Price;
	int	Exp;
	int	Learnable;
	int	nSummonMax;   // CSD-030314 : 소환마법시 소환가능 최대 몬스터의 수
	int nCombatCount; // CSD-030314 : 사용가능한 전투스킬 수
	int nResist;
	int nRiseFall;
	int nDualStep;	// 030415 kyo
};
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// 020808 YGI(변수의 위치와 구조를 바꿨음)
class CEffectTbl
{ 
public:
	CEffectTbl()
	{
		effect_Num = 0;
		pattern_Num = 0;
		sprite_Num = 0;
		static_Effect = false;
		sound_Num = 0;
		sound_Volume = 0;
		sound_loop = 0;
		next_Effect = 0;
		link_Frame = 0;
		trans_Type = 0;
		trans_Level = 0;
		sort_Type = 0;
		spr_Speed = 0;
		spr_Height = 0;
		object_Num = 0;
		fallow = false;
		light = 0;
		end_Time = 0;
		impect_Effect_Num = 0;
		send_result = false;
	}
	
public:
	int Check() const 
	{
		int value;
		char *pValue = (char *)&value;
		pValue[0] = Check1()+Check3();
		pValue[1] = Check2()+Check4();
		pValue[2] = 0;
		pValue[3] = 0;
		return (short int)value;
	}
	
	char Check1() const
	{
		return (char)(effect_Num+end_Time);
	}
	char Check2() const
	{
		return (char)(sort_Type+next_Effect+sort_Type+object_Num);
	}
	char Check3() const
	{
		return (char)(spr_Speed + spr_Height + object_Num);
	}
	char Check4() const
	{
		return (char)(pattern_Num + sprite_Num + link_Frame);
	}
	
public:
	short int	end_Time;			//종료까지의 총 프레임수
	short int	light;				//광원효과 넣을까?(0->넣지 않음/100자리수치->0이면 정상적 출력/1이면 높이값 더해서 출력(1~20 사이의 광원 단계)
	short int	sound_Volume;		//이펙트와 연결된 사운드의 볼륨레벨	
	char		static_Effect;		//스프라이트의 스타틱 여부
	short int	sprite_Num;			//스프라이트의 고유 번호
	short int	pattern_Num;		//패턴(함수)의 번호	
	char		send_result;		//효과가 서버에 결과를 요청하는지 여부
	short int	spr_Height;			//이팩트의 높이
	short int	sound_Num;			//이펙트와 연결될 사운드의 고유번호
	short int	spr_Speed;			//이팩트의 이동치
	short int	impect_Effect_Num;	//타격시 연결되는 이팩트의 번호
	short int	sort_Type;			//화면 sorting 방법(9(9+0)->일반적 소팅/10(9+1)->무조건 최상/11(9+2)->무조건 최하/12(9+3)->에니메이션 화일 참조하여 가변소팅/13(9+4)->시전자 앞뒤에 2장의 스프라이트 찍음)
	short int	trans_Level;		//트렌스 정도
	short int	effect_Num;			//이팩트의 고유(일련) 번호
	short int	object_Num;			//이팩트 소스의 동시출력 갯수
	short int	link_Frame;			//다음 이펙트가 시작될 프레임
	char		fallow;				//인물을 따라갈지->1/말지->0
	short int	next_Effect;		//다음 이펙트 번호
	short int	sound_loop;			//사운드 출력 횟수
	short int	trans_Type;			//트렌스 방법
};

class CEffectTbl2
{ 
public:
	int Check() const 
	{
		int value = 0;
		for( int i=0; i<30; i++ )
			value += sprite_Name[i];
		
		return value;
	}
public:
	char sprite_Name[30];
};
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
//
class CMagicTbl
{
public:
	CMagicTbl()
	{
		order_Type = 0;
		serial_Num = 0;
		basic_Level = 0;
		WS = 0;
		before_Magic = 0;
		basic_Magery = 0;
		cast_Time = 0;
		cast_Continue = 0;
		continue_Time = 0;
		avail_Type = 0;
		avail_Range = 0;
		exhaust_MP = 0;
		exhaust_WS = 0;
		require_WP = 0;
		require_IT = 0;
		point_WS = 0;
		invalid_TargetIT = 0;
		spell_Type = 0;
		appoint_Type = 0;
		excute_Type = 0;
		magic_Type = 0;
		Price = 0;
		Exp = 0;
		Learnable = 0;
		nSummonMax = 0;
		nCombatCount = 0;
		nResist = 0;
		nRiseFall = 0;
		nDualStep = 0; // 030415 kyo

	}
	
public:
	int Check() const 
	{
		return serial_Num + basic_Level + cast_Time + cast_Continue + 
			continue_Time + avail_Range + excute_Type + appoint_Type +
			avail_Type + spell_Type + magic_Type + nCombatCount;
	}
	
public:
	int	order_Type;			     // 계열(1->생/2->사/3->일반/4->화염/5->얼음/6->전격/7->소환/8->시공/9->신성/10->정동/11->정념/12->특수)
	int	serial_Num;			     // 고유번호(1~)
	int	basic_Level;		     // 사용 기본 레벨(1~9)
	int	WS;					         // wizwrd spell(or priest spell) point 
	int	before_Magic;		     // 전 단계 마법(먼저 익혀야 할 마법)
	int	basic_Magery ;		   // 기본 마법력
	int	cast_Time;			     // 캐스팅 시간(절대시간)
	int	cast_Continue;		   // 캐스팅 지속 시간(절대시간)
	int	continue_Time;		   // 지속시간(0->순간,영원히/1~절대시간)	
	int	avail_Type;			     // 효용 타입(0->자신/1->자신 아닌 1명/2~->유효범위내 다수)
	int	avail_Range;		     // 사정거리(타일단위/0->자신/99->무한)		
	int	exhaust_MP;			     // 소모 MP
	int	exhaust_WS;			     // 소모 WS
	int	require_WP;			     // 요구 지혜					
	int	require_IT;			     // 요구 지능
	int	point_WS;			       // WS 포인트(0->not available/1~)
	int	invalid_TargetIT;	   // 마법 사용가능 범위
	int	spell_Type ;		     // 0 Wizard, 1~13 priest(God별 type들 때문에 다양)
	int	appoint_Type;		     // 대상 지정방식 타입(0->나자신/1->대상지정(단일/복수대상)/2->특정지점(클릭위치)/3->지역마법)
	int	excute_Type;		     // 마법 캐스팅 유지시 시행될수 있는 횟수(0->여러번/1->한번만)
	int	magic_Type;			     // 마법 대분류(0->비공격마법/1->대상공격마법/2->지역복수공격마법)
	int	Price;		  // 마법을 배우는데 필요한 비용
	int	Exp;		  // 일반마법의 경우 경험피
	int	Learnable;	  // 각 신봉신의 타입에 따라 배울수 있는 경우
	int	nSummonMax;   // CSD-030314 : 소환마법시 소환가능 최대 몬스터의 수
	int nCombatCount; // CSD-030314 : 사용가능한 전투스킬 수
	int nResist;      // 막을 수 있는 저항력
	int nRiseFall;    // 높낮이 적용 범위
	int nDualStep;	  //마법의 듀얼 단계 // 030415 kyo
};
//
/////////////////////////////////////////////////////////////////////////////////////////

struct NPCName_by_Gender
{
	char Female[17] ;
	char Male[17] ;
	char Last[17] ;
};

struct NPCLev_to_Exp
{	//< CSD-030430
	int nMinExp;         // 현재 레벨의 최소 경험치
	int nMaxExp;         // 현재 레벨의 최대 경험치
	int nNeedExp;        // 다음 레벨로 올라가기 위해 획득해야될 경험치
	int nTrainingExp;    // 택틱 수련장에서 얻는 경험치
	float fExpReduction; // 사냥시 죽었을 경우 경험치 삭감률
	float fNKReduction;	 // NK에 죽었을 경우 경험치 삭감률
	int nTacRate;		 // 택틱 보정치
	int nMaxTactic;	     // 레벨에 따른 택틱 수련장에서 올릴 수 있는 최대 택틱 레벨
	int	nDyeingPrice;    // 염색 가격
	int nLessRate;       // 낮은 레벨 보정치  
	int nGreatRate;      // 높은 레벨 보정치  
	int nCspMax;         // 최대 전투스킬 포인터 획득수
	int nBaseCP;         // 전투스킬 기본량
	int bank_loan;	     // 010314 YGI
	int nStep;           // 경험치 단계
	int nWarriorLife;    // 전사의 기본 Life
	int nThiefLife;      // 도둑의 기본 Life
	int nArcherLife;     // 궁수의 기본 Life
	int nWizardLife;     // 마법사의 기본 Life
	int nPriestLife;     // 성직자의 기본 Life
	int nWarriorExpRate; // 전사의 경험치 보정률(단위%)
	int nThiefExpRate;   // 도둑의 경험치 보정률(단위%)
	int nArcherExpRate;  // 궁수의 경험치 보정률(단위%)
	int nWizardExpRate;  // 마법사의 경험치 보정률(단위%)
	int nPriestExpRate;  // 성직자의 경험치 보정률(단위%)
};	//> CSD-030430

struct NPC_Generation
{	//< CSD-030806
	int SprNO;
	//char Han_name[31];
	char Name[31];
	//int Selectable;
	int Sel_gender;
	//char Sel_FirstName[31];
	//char Sel_LastName[31] ;
	int nNpcRace;        // NPC 종족
	int nNpcNK;          // NPC를 죽었을 때 NK처리 여부
	int nStep;           // 클래스 단계
	int nAttr;           // 몬스터 속성
	int nHpMax;          // 몬스터 HP
	int nEpMax;          // 몬스터 EP
	int nStrikeDamage;   // 물리적 공격 데미지
	int nStrikeRandom;   // 물리적 공격 데미지 변화량
	int nMagicDamage;    // 공격 마법 데미지
	int nMagicRandom;    // 공격 마법 데미지 변화량
	int nCriticalRate;   // 크리티칼 확률
	int nCriticalDamage; // 크리티칼 데미지
	int nPiercingStrike; // 물리공격 피어싱 확률
	int nPiercingMagic;  // 마법공격 피어싱 확률
	int Sel_Job_Alchemist;
	int nAIBase;         // 기본적인 Npc AI 패턴 
	int nAppearType;     // 보스몬스터 등장관련 타입
	int nAIType;         // Npc AI 제어 위치
	int nAINo;           // Npc AI 패턴 번호
	int nLvMin;
	int nLvMax;
	int Money_min;						
	int Money_Max;						
	int AttackDelay;	// 0811 NPC KHS	
	int AccessDelay;	// 0811 NPC KHS	
	int BackDelay;		// 0811 NPC KHS	
	int PCfindDelay;	
	int nRvAp;	      // 공격력 보정(단위%)
	int nRvLife;	    // Life 보정(단위%)
	int nRvExp;       // 경험치 보정(단위%) 
	int nAc;          // 일반 방어력(단위%)
	int nDpRate;      // 듀얼 방어력 보정률(단위%)
	int nApRate;      // 듀얼 공격력 보정률(단위%)
	int nPhysical;    // 물리적 공격에 대한 방어력(단위%)
	int nFire;	      // 불계열 공격 마법에 대한 저항력(단위%)
	int nIce;		      // 얼음계열 공격 마법에 대한 저항력(단위%)
	int nElect;	      // 전격계열 공격 마법에 대한 저향력(단위%)
	int nHoly;	      // 신력계열 공격 마법에 대한 저항력(단위%)
	int nStun;        // 마비계열 저주 마법에 대한 저항력(단위%)
	int nSlow;        // 속도저하계열 저주 마법에 대한 저항력(단위%)
	int nPoison;      // 중독계열 저주 마법에 대한 저항력(단위%)
	int nBlind;       // 시력상실계열 저주 마법에 대한 저항력(단위%)
	int nDrain;       // 흡탈계열 저주 마법에 대한 저항력(단위%)
	int nFreeze;      // 냉동계열 저주 마법에 대한 저항력(단위%)
	//int SpiritValue;
	//int SpiritValue_Range;
	//int PS[15];
	//int Sel_Dice;
	//int QuantityperLv;
	//int WS[15] ;
	//int Sel_Dice1;
	//int QuantityperLv1;
	//int Leathal_weapon1;
	//int Leathal_weapon2;
	//int Damage_weapon;
	//int Leathal_attackws[9] ;
	//int Damage_WP;
	//int Leathal_WS;
	//int Success;
	//int Leathal_ps;
	//int DamagePercent_ps;
	int Inventory[15] ;
	//int Amount_Dice;
	//int Righthand;
	//int Lefthand;
	//int Body;
	//int Helmet;
	//int leg;
	//int Leg_Probability;
	//int neck;
	//int neck_Probability	;
	//int unique1;
	//int Unique1_probalbility;
	//int Unique2;
	//int unique2_probability	;
	//int Crapple;
	//int swordmanship;
	//int Archery;
	//int Fencing;
	//int Mace_fighting;
	//int Pierce;
	//int Whirl;
	//int Hurl;
	//int Parrying;
	//int Double_Swordmanship	;
	//int Double_MaceFighting	;
	//int Magery;
	//int Orison;
	//int LV_Str;
	//int Con;
	//int Dex;
	//int Wis;
	//int Int;
	int Movp;
	//int Char;
	//int Endu;
	//int Mor;
	//int Luck;
	//int WS_PS;
	//int Fullpoint;
	//int Suplus_Resist_Death;
	//int Suplus_Resist_Ston;
	//int Suplus_Resist_Magic;
	//int Suplus_Resist_Fire;
	//int Suplus_Resist_Ice;
	//int Suplus_Resist_Light;
	//int DivideLv;
	//int Resist_Range;
	int Tamingable;
	int Taming_min;
	int Taming_max;
	int Talkanimal_min;
	int Talkanimal_max;
	//int Resist;
	//int Resist_range1;
	int ValueHP;
	int ValueSP;
	//int Rely_value;
	//int nor_range;
	//int att_range;
	int offset[20];
	//int neglect_attack;
	//int neglect_avoid;
	//int check_pass;
	//int party;
	//int enemy_no;
	//int levelgap;
	//int apgap;
	//int vpgap;
	//int opendoor;
	//int lockdoor;
	//int closedoor;
	//int breakdoor;
	//int openobj;
	//int lockbj;
	//int closeobj;
	//int breakobj;
	//int act_month;
	//int act_time;
	//int act_bi;
	//int act_chun;
	//int act_nun ;
	int Get_meat;
	int Meat_amount;
	int Get_skin;
	int Skin_amount;
	int Get_Wool;
	int Wool_amount;
	int Get_beewax;
	int BeeWax_amount;
	int mutant;
	
	bool CanUse;
};	//> CSD-030806

///////////////////////////////////////////////////////////////////////////////
//
struct TimeData
{
	int Year;
	int Month;
	int Day;
	int DayofWeek;

	TimeData()
	{
		Year=0;
		Month=0;
		Day=0;
		DayofWeek=0;
	}
};
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//< CSD-030408 
struct REGEN_MONSTER_INFO
{
	int nMonsterNo;
	int nMonsterType;
	int nMonsterCount;
};

struct REGEN_PLACE_INFO
{
	int nNumber;
	int nPosX;
	int nPosY;
	int nScope;
};
//> CSD-030408
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//< CSD-030506
struct k_nation_item_data
{
	short int item_no;
	short int fame;
	int	money;
};
//> CSD-030506
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//< CSD-TW-030704
typedef struct tag_LottoUsers
{
	int		nLottoID;
	char	szID[20];
	TIMESTAMP_STRUCT	date;
	int		aNumbers[10];
}LOTTO_USER;

typedef struct tag_LottoEventInfo
{
	int		nLottoID;
	tm		StartDate;
	tm		LotteryDate;
	int		nWinNumCount;
	int		anWinNumbers[10]; // 10개 지만 현재는 4개만 쓰인다.
}LOTTO_EVENT_INFO;

typedef	struct	tag_DateCheck
{
	int		nDay;
	int		nHour;
	int		nMin;
}DATECHECK;
//> CSD-TW-030704
///////////////////////////////////////////////////////////////////////////////

struct START_UP_INFO
{	//< CSD-030723
	DWORD idIndex;
	int	  nApplyDay;  
	int   nStartHour;
	int   nStartMinute;
	int   nEndHour;
	int   nEndMinute;
};	//> CSD-030723	

struct CharacterParty
{	//< CSD-030723
	short int	On;
	char		Name[31];
	int			Face;
	int			Level;
	int			Gender;
	int			Str;
	int			Class;
};	//> CSD-030723

struct WINLOSEPOINT
{	//< CSD-030723
	DWORD win:13;
	DWORD defeat:12;
	DWORD disconnect:7;
};	//> CSD-030723

struct Event_Join							// 020115 LTS
{	//< CSD-030723
	unsigned char Event1 : 1;				// 이벤트 1의 참여여부	
	unsigned char Event2 : 1;				// 이벤트 2의 참여여부	
	unsigned char Event3 : 1;				// 이벤트 3의 참여여부	
	unsigned char Event4 : 1;				// 이벤트 4의 참여여부	
	unsigned char Event5 : 1;				// 이벤트 5의 참여여부	
	unsigned char Event6 : 1;				// 이벤트 6의 참여여부	
	unsigned char Event7 : 1;				// 이벤트 7의 참여여부	
	unsigned char Event8 : 1;				// 이벤트 8의 참여여부	
};	//> CSD-030723

struct DiseaseTable
{	//< CSD-030723							
	short int	disease_number;
	char		han_name[31];	
	char		name[31];	
	short int	lv;			
	short int	nut1_1;		
	short int	nut2_1;			
	short int	nut3_1;		
	short int	probability_1;
	short int	andor_1;	
	short int	checktime_1;
	short int	contact_char;
	short int	talk_char;	
	short int	fuck_char;	
	short int	contact_obj;
	short int	eat_obj;	
	short int	nut1_2;		
	short int	nut2_2;	
	short int	nut3_2;
	short int	probability_2;
	short int	checktime_2;
	short int	andor_2;
	short int	dec_str;
	short int	dec_dex;
	short int	dec_con;
	short int	dec_wis;
	short int	dec_int;
	short int	dec_movp;
	short int	dec_char;
	short int	dec_endu;
	short int	dec_mor;
	short int	dec_luck;
	short int	dec_ws;
	short int	dec_ps;
	short int	recoverable;
	short int	life	;
	short int	health	;
	short int	mp	;
	short int	dp	;
	short int	max_	;
	short int	damage	;
	short int	att_rate	;
	short int	depen_rate	;
	short int	ac	;
	short int	mad	;
	short int	poison	;
	short int	stony	;
	short int	sluggish	;
	short int	active	;
	short int	paralysis	;
	short int	active_time	;
	short int	maintain_time	;
	short int	natural_recovery	;
	short int	nut1_3	;
	short int	nut2_3	;
	short int	nut3_3	;
	short int	probability_3	;
	short int	checktime_3	;
	short int	andor_3	;
	short int	price_heal	;
	short int	price_prevent	;	
};	//> CSD-030723

struct GodTable
{	//< CSD-030723
	char meet_year_month;
	char meet_year_day;
	char meet_year_hour;
	char meet_month_day;
	char meet_month_hour;
	char meet_day_hour;
};	//> CSD-030723

struct NW_Character	// 010904 LTS		// LTS CHAR		
{	//< CSD-030723
	DWORD WarNo						:	7;	//	전쟁번호
	DWORD WarfieldNo					:	3;	//	전쟁터 번호
	DWORD SquadNo						:	3;	//	부대번호
	DWORD isSquadLeader				:	1;	//	부대장인가?	
	DWORD isCommander					:	1;	//	사령관인가?
	DWORD isCommanderCandidater		:	1;	//  사령관 후보인가?		// 010915 LTS
	DWORD DoWarJoinVote				:	1;	//	전쟁터 참여 투표를 했나?
	DWORD DoCommanderVote				:	1;	//	사령관 투표를 했다
	DWORD YL_JoinNation				:	4;	//	일스인경우 자이펀에했나? 바이서스에 했나?  // 국가번호
	DWORD Reserved					:	10;	//	예약..
};	//> CSD-030723

struct CCharRank
{	//< CSD-030723
	DWORD nation	  :3;  // 속한나라 - 0:없음, 1, 2, 3:바이서스, 4:자이펀, 5:헤게모니아, 6:일스
	DWORD guild_master:3;  // 길드 마스터
	DWORD king		  :1;  // 나라 왕
	DWORD dwReserved  :9;  // 사용되지 않음
	DWORD counselor	  :2;  // 0 : 없음, 1: 회사 운영자, 2: 게임내 운영자
	DWORD ultra_nk3	  :1;  // 극악피케이	1004 YGI
	DWORD ultra_nk4	  :1;  // 극악피케이	1004 YGI
	DWORD ultra_nk6	  :1;  // 극악피케이	1004 YGI
	DWORD reporter	  :1;
	DWORD imsi		  :10; // 1004 YGI
};	//> CSD-030723

struct MAP_MOVE_INFO
{	//< CSD-HK-030829
	int idUser;
	int nMapPort;
	int nX;
	int nY;
};	//> CSD-HK-030829

#define LOG_ABNORMAL_PROTOCOL			0
#define LOG_DISCONNECT_LOGIN_SERVER		1
#define LOG_DISCONNECT_DB_DEMON			2
#define LOG_HANDLE_WRITING				3
#define LOG_EXCEPTION_FAIL				4
#define LOG_HANDLE_COMMAND				5
#define LOG_HANDLE_READING				6
#define LOG_INSERT_LOGINTABLE_ERROR		7
#define LOG_SEND_UPDATING_MESSAGE		8
#define LOG_EXCEPTION_ITEM_ABRASION_ERROR    9
#define LOG_HANDLE_WRITING_INSIDE		10

extern DWORD global_time;
extern DWORD g_alive_time;
extern DWORD g_curr_time;
extern DWORD g_curr_time_with_out_year; // 031009 CI YGI


extern int g_year;
extern int g_mon;
extern int g_yday;
extern int g_wday;
extern int g_day;
extern int g_hour;
extern int g_min;
extern int g_sec;

extern int g_count_ok;
extern int g_weatherflag;

extern bool g_block_attack;
extern bool g_block_magic;
extern bool g_block_steal;//030102 lsw
extern bool g_stop_please_give_life;
// 멀티스레트를 위한 Setting.
extern HANDLE g_hSema;

#include "../LowerLayers/MyLog.h"
#include "CharInfo.h" // CSD-030723
#include "MagicState.h" // CSD-030723
#include "ResistState.h" // CSD-030723
#include "CharList.h" // CSD-030723
#include "nation.h"
#include "ItemMgr.h"
#include "dr_network.h"
#include "MYSQL.H"
#include "ITEMLIST.H"
#include "Hong_Sub.h"
#include "OP_Chr_Status.h"
#include "NPCLIST.H"
#include "Object.h"
#include "NPC_Pattern.h"
#include "AREA.H"
#include "SkillMgr.h" //020214 lsw skill.h 삭제
#include "CrackMgr.h" //020214 lsw skill.h 삭제
#include "Map.h"
#include "RajaSystem.h"
#include <mmsystem.h> //020214 lsw
#include "RareEffectMgr.h"
#include "menuserver.h"
#include "Nationsys_define.h"
#include "LocalizingMgr.h"
#include "LottoSystem.h" // CSD-TW-030704

extern CRareEffectMgr RareEM;
extern void ResetAbility(CHARLIST *ch);//020303 lsw
extern int MAX_EXP_LEVEL;  // CSD-040204
extern int MAX_TAC_LEVEL;  // CSD-040204
