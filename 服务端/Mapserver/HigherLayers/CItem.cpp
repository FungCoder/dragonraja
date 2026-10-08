#include "..\stdafx.h"
#include "DefaultHeader.h"

#define CITEM_CLASS__
#include "CItem.h"
#include "Hong_Sub.h"
#include "teambattle.h"
#include "..\LowerLayers\mylog.h"

CItem_Plant			CPlant		[MAX_ITEM_NUM];
CItem_Mineral		CMineral	[MAX_ITEM_NUM];
CItem_Herb			CHerb		[MAX_ITEM_NUM];
CItem_Cook			CCook		[MAX_ITEM_NUM];
CItem_Potion		CPotion		[MAX_ITEM_NUM];
CItem_Tool			CTool		[MAX_ITEM_NUM];
CItem_Weapon		CWeapon		[MAX_ITEM_NUM];
CItem_Disposable	CDisposable	[MAX_ITEM_NUM];
CItem_Armor			CArmor		[MAX_ITEM_NUM];
CItem_Accessory		CAccessory	[MAX_ITEM_NUM];
CItem_Etc			CEtc		[MAX_ITEM_NUM];

extern DWORD CreateItemDuration( int type, int itemno );
extern void SendCharacterCondition( CHARLIST* lpChar, int change_HP );
extern HENV hEnv ;
extern HDBC hDBC ;
struct CItem_List Item_Ref ;


// 酒捞袍 曼炼 抛捞喉阑 DB俊辑 佬绢客 积己窍绰 窃荐.
// 酒捞袍 曼炼 抛捞喉阑 DB俊辑 佬绢客 积己窍绰 窃荐.
int initItem(void)		// 0705 YGI
{
	int i, c ;
	int check[20] = {0,};
	if(hEnv == NULL || hDBC == NULL) return -1 ;

	c= CPlant[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg( " Fail to Read Plant Item "); return -1; }
	Item_Ref.nItem[0]= c;
	MyLog( LOG_NORMAL, "    .Plant Item		%4d data Loaded", c) ;
	
	c= CMineral[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Mineral Item "); return -1; }
	Item_Ref.nItem[1]= c;
	MyLog( LOG_NORMAL, "    .Mineral Item		%4d data Loaded", c) ;
	
	c= CHerb[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Herb Item "); return -1; }
	Item_Ref.nItem[2]= c;
	MyLog( LOG_NORMAL, "    .Herb Item		%4d data Loaded", c) ;
	
	c= CCook[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Cook Item "); return -1; }
	Item_Ref.nItem[3]= c;
	MyLog( LOG_NORMAL, "    .Cook Item		%4d data Loaded", c) ;
	
	c= CPotion[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Potion Item "); return -1; }
	Item_Ref.nItem[4]= c;
	MyLog( LOG_NORMAL, "    .Potion Item		%4d data Loaded", c) ;
	
	c= CTool[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Tool Item "); return -1; }
	Item_Ref.nItem[5]= c;
	MyLog( LOG_NORMAL, "    .Tool Item		%4d data Loaded", c) ;
	
	c= CWeapon[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Weapon Item "); return -1; }
	Item_Ref.nItem[6]= c;
	MyLog( LOG_NORMAL, "    .Weapon Item		%4d data Loaded", c) ;
	
	c= CDisposable[0].getAllData() - 1 ;
	if( c <= 0  ){ JustMsg("  Fail to Read Disposable Item "); return -1; }
	Item_Ref.nItem[7]= c;
	MyLog( LOG_NORMAL, "    .Disposable Item	%4d data Loaded", c) ;
	
	c= CArmor[0].getAllData() - 1 ;
	if( c <= 0 ){ JustMsg("  Fail to Read Armor Item "); return -1; }
	Item_Ref.nItem[8]= c;
	MyLog( LOG_NORMAL, "    .Armor Item		%4d data Loaded", c) ;
	
	c= CAccessory[0].getAllData() - 1 ;
	if( c <= 0 ){ JustMsg("  Fail to Read Accessory Item "); return -1; }
	Item_Ref.nItem[9]= c;
	MyLog( LOG_NORMAL, "    .Accessory Item	%4d data Loaded", c) ;
	
	c= CEtc[0].getAllData() - 1 ;
	if( c <= 0 ){ JustMsg("  Fail to Read Etc  Item "); return -1; }
	Item_Ref.nItem[10]= c;
	MyLog( LOG_NORMAL, "    .Etc Item		%4d data Loaded", c) ;

	
	// 0锅 牢郸胶绰 荤侩窍瘤 臼澜. 
	// item锅龋啊 1锅何磐 矫累(0锅篮 NULL酒捞袍, 溜 酒捞袍 绝澜狼 惑怕甫 唱鸥辰促)
	for(i= 1 ; i <= Item_Ref.nItem[0] ; i++) Item_Ref.Item_Info[0][i]= &CPlant[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[1] ; i++) Item_Ref.Item_Info[1][i]= &CMineral[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[2] ; i++) Item_Ref.Item_Info[2][i]= &CHerb[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[3] ; i++) Item_Ref.Item_Info[3][i]= &CCook[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[4] ; i++) Item_Ref.Item_Info[4][i]= &CPotion[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[5] ; i++) Item_Ref.Item_Info[5][i]= &CTool[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[6] ; i++) Item_Ref.Item_Info[6][i]= &CWeapon[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[7] ; i++) Item_Ref.Item_Info[7][i]= &CDisposable[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[8] ; i++) Item_Ref.Item_Info[8][i]= &CArmor[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[9] ; i++) Item_Ref.Item_Info[9][i]= &CAccessory[i] ;
	for(i= 1 ; i <= Item_Ref.nItem[10] ; i++) Item_Ref.Item_Info[10][i]= &CEtc[i] ;
	
	return 1;
}


/////////////////////////////////////////////////////////////////////////////
//
// 窃荐汲疙: DB俊辑 秦寸 酒捞袍狼 荐摹甫 佬绢坷绰 窃荐.
//				virtual 窃荐捞促.
//				秦寸 酒捞袍 硅凯狼 1锅何磐 佬绢坷扁 矫累茄促.
//				Num_Of_CItem_XXXXX肺 define等 俺荐父怒 佬绢柯促.
//				DB俊 乐绰 酒捞袍狼 俺荐啊 Num_Of_CItem_XXXXX焊促 累促搁
//				DB俊 乐绰 酒捞袍 犁荐父 佬绢柯促.
// 
// 府畔蔼: DB俊辑 角力肺 佬绢柯 酒捞袍 俺荐
//
/////////////////////////////////////////////////////////////////////////////

int CItem_Plant::getAllData(void)
{
	if(Num_Of_CItem_Plant <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	char col_name[60]= {0, } ;
	//UWORD i ;
	SQLLEN cbValue ;
	int *tmp= NULL ;
	int c= 1 ;
	int i= 4 ;
	//char name[21], hname[21] ;
	
    SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Plant order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;
	//ret= SQLExecDirect(hStmt, (UCHAR *)item_query[index], SQL_NTS) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem PLAnt: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}

	while(ret == SQL_SUCCESS)
	{
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CPlant[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CPlant[c].Name, sizeof(Name), &cbValue) ;

		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CPlant[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CPlant[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

		tmp = &CPlant[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		ret= SQLFetch(hStmt) ;			
		m_Much = 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP) ;
	return c ;
}


int CItem_Mineral::getAllData(void)
{
	if(Num_Of_CItem_Mineral <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Mineral order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;
	//ret= SQLExecDirect(hStmt, (UCHAR *)item_query[index], SQL_NTS) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Mineral: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	//tmp= &Nead2_Level ;
	//tmp= &lv ;
	while( ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CMineral[c].Item_id = c;

		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CMineral[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CMineral[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CMineral[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

//		tmp+= 5 ;
		tmp = &CMineral[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Mineral   (%3d:%3d) reading process complete.\r\n", c,nCols ) ;
	return c ;
}


int CItem_Herb::getAllData(void)
{
	if(Num_Of_CItem_Herb <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Herb order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;
	//ret= SQLExecDirect(hStmt, (UCHAR *)item_query[index], SQL_NTS) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Herb: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}	
		
	while( ret == SQL_SUCCESS)
	{	
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CHerb[c].Item_id = c;

		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CHerb[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CHerb[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CHerb[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}
		
//		tmp+= 5 ;
		tmp = &CHerb[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Herb      (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}

int CItem_Cook::getAllData(void)
{
	if(Num_Of_CItem_Cook <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Cook order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Cook: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CCook[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CCook[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CCook[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CCook[c].Han_Name );
				
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

//		tmp+= 5 ;
		tmp = &CCook[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;					//1215
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Cook      (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}

int CItem_Potion::getAllData(void)
{
	if(Num_Of_CItem_Potion <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Potion order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Potion: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CPotion[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CPotion[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CPotion[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar(CPotion[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

//		tmp+= 5 ;
		tmp = &CPotion[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Potion    (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}


int CItem_Tool::getAllData(void)
{
	if(Num_Of_CItem_Tool <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Tool order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Tool: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while( ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CTool[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CTool[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CTool[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CTool[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

//		tmp+= 5 ;
		tmp = &CTool[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Tool      (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}

int CItem_Weapon::getAllData(void)
{
	if(Num_Of_CItem_Weapon <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Weapon order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Weapon: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CWeapon[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CWeapon[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CWeapon[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CWeapon[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

//		tmp+= 5 ;
		tmp = &CWeapon[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols. %d Rows", nCols, c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Weapon    (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}




int CItem_Disposable::getAllData(void)
{
	if(Num_Of_CItem_Disposable <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Disposable order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Disposable: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CDisposable[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CDisposable[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CDisposable[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CDisposable[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

		tmp = &CDisposable[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Disposable(%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}

int CItem_Armor::getAllData(void)
{
	if(Num_Of_CItem_Armor <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Armor order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Armor: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) 
	{
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CArmor[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CArmor[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CArmor[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CArmor[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}

//		tmp+= 5 ;
		tmp = &CArmor[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Armor     (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}

int CItem_Accessory::getAllData(void)
{
	if(Num_Of_CItem_Accessory <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Accessory order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;

	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Accessory: ExecDirect Error ") ;
		return -1 ;
	}

	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA

		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nError!!! (%d)", ret) ;
			return -1 ;
		}
		CAccessory[c].Item_id = c;

		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CAccessory[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CAccessory[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CAccessory[c].Han_Name );
		if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
			printf("\nString GET Error!!! (%d)", ret) ;
			return -1 ;
		}
	
//		tmp+= 5 ;
		tmp = &CAccessory[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			//fprintf(stdout, "\n [%3d][%8x] %d", i, tmp, *tmp) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;		// 弊 酒捞袍捞 倔付唱 乐唱 檬扁拳 1215
	}
//	printf("\n<<<<<< %d Cols.>>>>>>>>", nCols) ;
//	printf("\n<<<<<< %d Rows.>>>>>>>>", c) ;
	SQLFreeStmt(hStmt, SQL_DROP) ;
//	printf("       Item_Accessory (%3d:%3d) reading process complete.\r\n", c, nCols ) ;
	return c ;
}	
	
int CItem_Etc::getAllData(void)
{	
	if(Num_Of_CItem_Etc <= 0) {
		//fprintf(stdout, "\n	No item's available ") ;
		return -4 ; // No item's available
	}
	HSTMT hStmt= NULL ;
	RETCODE ret ;
	SWORD nCols ;
	char query_stmt[80]= {0, } ;
	SQLLEN cbValue ;
	int *tmp ;
	int c= 1, i ;
	SQLAllocStmt(hDBC, &hStmt);
	
	strcpy(query_stmt, "select * from Item_Etc order by item_id") ;
	ret= SQLExecDirect(hStmt, (UCHAR *)query_stmt, SQL_NTS) ;
	
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		printf("\nItem Etc: ExecDirect Error ") ;
		return -1 ;
	}
	
	SQLNumResultCols(hStmt, &nCols) ;
	
	ret= SQLFetch(hStmt) ;
	if(ret != SQL_SUCCESS_WITH_INFO && ret != SQL_SUCCESS) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,    NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			(LPTSTR) &lpMsgBuf,    0,    NULL );// Process any inserts in lpMsgBuf.
		// ...// Display the string.
		printf("\nLast Error: %s", (LPCTSTR)lpMsgBuf) ;
		// Free the buffer.
		LocalFree( lpMsgBuf );
		printf("\nItem7: Fetch Error ") ;
		return -1 ;
	}
		
	while(ret == SQL_SUCCESS)
	{
		// Accquire the DATA
		ret= SQLGetData(hStmt, 1, SQL_C_SLONG, &c, sizeof(int), &cbValue) ;
		CEtc[c].Item_id = c;
		ret= SQLGetData(hStmt, 2, SQL_C_CHAR, &CEtc[c].Name, sizeof(Name), &cbValue) ;
		ret= SQLGetData(hStmt, 3, SQL_C_CHAR, &CEtc[c].Han_Name, sizeof(Han_Name), &cbValue) ;
		EatRearWhiteChar( CEtc[c].Han_Name );
		tmp = &CEtc[c].lv;
		for(i=4; i <= nCols; i++) {
			ret= SQLGetData(hStmt, i, SQL_C_SLONG, tmp, sizeof(int), &cbValue) ;
			tmp++ ;
		}
		c++ ;
		
		ret= SQLFetch(hStmt) ;
		m_Much  = 0;
	}
	SQLFreeStmt(hStmt, SQL_DROP) ;
	return c ;
}








// *****************************************************************************************
// *****************************************************************************************
// *****************************************************************************************
// *****************************************************************************************
// *****************************************************************************************
//    Item 积己 
//	  Item 积己 
// *****************************************************************************************
// *****************************************************************************************
// *****************************************************************************************
// *****************************************************************************************
// *****************************************************************************************

int ItemGen_successrate( t_connection c[], int cn, int type, int tableno, int itemtype, int itemno )
{
	CHARLIST *ch;

	if( cn >= DRAGON_MAX_CONNECTIONS ) return 0;
	if( cn < DRAGON_CONNECTIONS_START ) return 0;

	ch = &c[cn].chrlst;
	if( ch->bAlive != ALIVE_ ) return 0;
//----------------------------------------------------  Check Variables 裹困...
	return 100;
}

/*
#define IATTR_DURATION			0
#define IATTR_LIMIT				1	
#define IATTR_ATTR				2
#define IATTR_ID				3
#define IATTR_RESERVED0			4
#define IATTR_RESERVED1			5

#define IA2_HIDE			0x00000001
#define IA2_NOTMOVE			0x00000002
#define IA2_POISONED		0x00000004
#define IA2_OPENED			0x00000008
#define IA2_BROKEN			0x00000010
#define IA2_HOUSE			0x00000020
#define IA2_MAGIC			0x00000040
#define IA2_CANNOTUSE		0x00000080
#define IA2_NOIDENTIFIED	0x00000100
#define IA2_DOOR			0x00000200
#define IA2_BOX				0x00000400
#define IA2_CHAIR			0x00000800
#define IA2_TABLE			0x00001000


#define	PLANT			0
#define	MINERAL			1
#define	HERB			2
#define	COOK			3 
#define	POTION			4
#define	TOOL			5
#define	WEAPON			6
#define	DISPOSABLE		7
#define	ARMOR			8
#define	ACCESSORY		9
#define	ETC				10
#define MAP				11	
*/



unsigned int ItemGen_Duration( int type, int no )
{
	return 0;
}

unsigned int ItemGen_Limit( int type, int no )
{
	return 0;
}

unsigned int ItemGen_Attr( int type, int no )
{
	return 0;
}

unsigned int ItemGen_DbId( int type, int no )
{
	return 0;
}




void RecvItemDoorOpenClose( int cn, t_item_door_open_close *p )
{
	LPITEMLIST i;
	int item_id = p->item_id;

	if( item_id < 0 ) return;
	if( item_id >= MAX_ITEM_LIST ) return;

	i = &ItemList[ item_id ];
			
	if( (i->attr[ IATTR_ATTR] & IA2_DOOR) == 0 ) return; // 巩捞 酒聪匙...
			
	if( p->openclose == 0 ) // 凯绢林技夸..
	{		
		if( i->attr[ IATTR_ATTR] & IA2_OPENED ) return; // 凯妨 乐栏搁 府畔.
		//	
		// 咯扁辑 connections[cn].chrslt啊  凯荐 乐绰瘤 八荤茄促.
		//	
		i->attr[ IATTR_ATTR] |= IA2_OPENED;
			
		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, item_id );
	}		
	else // 摧酒林技夸.
	{		
		if( (i->attr[ IATTR_ATTR] & IA2_OPENED) == 0 ) return; // 摧囚 乐栏搁 府畔.
		//
		// 咯扁辑 connections[cn].chrslt啊  摧阑荐 乐绰瘤 八荤茄促.
		//
		i->attr[ IATTR_ATTR] &= (~(IA2_OPENED) );
		
		SetArea( CHANGE_IA2_ATTR_ITEM_AREA, item_id );
	}	
}		
	
	
void SendItemAttrChange( t_connection c[], int cn, int id, int attr )
{	
	t_packet packet;
	
	packet.h.header.type = CMD_ITEM_ATTR_CHANGE;
	{
		packet.u.item_attr_change.item_id = id;	
		packet.u.item_attr_change.array = attr;
		packet.u.item_attr_change.attr  = ItemList[id].attr[ attr];
	}
	packet.h.header.size = sizeof( item_attr_change );
	
	QueuePacket(c, cn, &packet, 1);
}	
	
void SendItemFoot_On_Trap0( t_connection c[], int cn, int item_id ) 
{	
	t_packet packet;

  connections[cn].chrlst.DecLife(ItemList[item_id ].attr[IATTR_DURATION]);

	if (connections[cn].chrlst.IsDead() == true)
	{
		killCharacter( NULL, &connections[cn].chrlst );
		SendCharacterCondition( &connections[cn].chrlst, 0);
	}

	packet.h.header.type = CMD_FOOT_ON_TRAP_0_ITEM;
	packet.h.header.size = sizeof(t_server_foot_on_trap0);
	packet.u.server_foot_on_trap0.idServer = WORD(cn);
	packet.u.server_foot_on_trap0.idItem = WORD(item_id);
	packet.u.server_foot_on_trap0.nLife = connections[cn].chrlst.Hp;
	QueuePacket(c, cn, &packet, 1);
	CastMe2Other( cn, &packet );
}	

void SendItemFoot_On_Trap1( t_connection c[], int cn, int item_id )
{	
	t_packet packet;

  connections[cn].chrlst.DecLife(ItemList[ item_id ].attr[IATTR_DURATION]);

	if (connections[cn].chrlst.IsDead() == true)
	{
		killCharacter( NULL, &connections[cn].chrlst );
		SendCharacterCondition( &connections[cn].chrlst, 0);
	}
	
	packet.h.header.type = CMD_FOOT_ON_TRAP_1_ITEM;
	packet.h.header.size = sizeof(t_server_foot_on_trap1);
	packet.u.server_foot_on_trap1.idServer = WORD(cn);
	packet.u.server_foot_on_trap1.idItem= WORD(item_id);
	packet.u.server_foot_on_trap1.nLife = connections[cn].chrlst.Hp;
	QueuePacket(c, cn, &packet, 1);
	CastMe2Other( cn, &packet );
}	



// 010322 KHS
void CheckItemTimeElapsed( void )
{										
	static int count;								
	int s,e;	
	ITEMLIST *ii;
	DWORD time;

	time = g_curr_time;

	if( count >= MAX_ITEM_LIST ) count = 0;
	s = count;												
	count += ( MAX_ITEM_LIST/ 100 );
	e = count;					
	if( e > MAX_ITEM_LIST ) e = MAX_ITEM_LIST;		// 茄锅俊 MAX_ITEM_LIST/3沥档父 Check茄促. 辑滚 何淬阑 临捞扁 困秦.
								
	for( int m = s ; m < e ; m ++)
	{							
		if( ItemList[m].bAlive == 0 ) continue;

		ii = &ItemList[m];
	
		unsigned int attr2 = ii->attr[ IATTR_ATTR];
		if( GetAttr2( attr2, IA2_BOX ) )
		{
			if( GetAttr2( attr2, IA2_OPENED ) ) 
			{
				if( time > ii->attr[ IATTR_LIMIT] ) // 柄柳第 倔付埃狼 矫埃捞 瘤车促. 
				{
					ii->attr[ IATTR_ATTR] = IA2_NOTMOVE | IA2_BOX;
					SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
					continue;
				}
			}
		}
		else if( attr2 & IA2_FARM0 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM0);
				ii->attr[IATTR_ATTR ] |= IA2_FARM1;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM1 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM1);
				ii->attr[IATTR_ATTR ] |= IA2_FARM2;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM2 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM2);
				ii->attr[IATTR_ATTR ] |= IA2_FARM3;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM2 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM2);
				ii->attr[IATTR_ATTR ] |= IA2_FARM3;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM3 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM3);
				ii->attr[IATTR_ATTR ] |= IA2_FARM4;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM4 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM4);
				ii->attr[IATTR_ATTR ] |= IA2_FARM5;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM5 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM5);
				ii->attr[IATTR_ATTR ] |= IA2_FARM6;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM6 )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				DWORD time = ReturnHowLongTimeGrow( ii );
				time += time;

				ii->attr[IATTR_ATTR ] &= ~(IA2_FARM6);
				ii->attr[IATTR_ATTR ] |= IA2_FARM7;
				ii->attr[IATTR_DURATION ] = g_curr_time + time;
				SetArea( CHANGE_IA2_ATTR_ITEM_AREA, m );
				SetArea( CHANGE_IA2_DURATION_ITEM_AREA, m );
			}
		}else if( attr2 & IA2_FARM7 )	// 荐犬阑 窍瘤 臼绊 矫埃捞 瘤唱搁 荤扼柳促. 
		{
			//001214 zhh		//荐犬窍绰 窜拌父 5窜拌啊 等促.(荐犬窍绰单 矫埃捞 何练窃)
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				RemoveItemList( m );
			}
		}
		else if( attr2 & IA2_FIRE )
		{
			if( ii->attr[IATTR_DURATION] < g_curr_time )
			{
				ii->bAlive = REMOVE_;
				SetArea( REMOVE_ITEM_AREA, m );
			}
		}
		else if( attr2 & IA2_DOOR )
		{
		}
		else if( attr2 & IA2_HOUSE )
		{
		}
		else if( attr2 & IA2_CHAIR )
		{
		}
		else if( attr2 & IA2_TABLE )
		{
		}
		else if( attr2 & IA2_TRAPED0 )
		{
			if( g_curr_time  - ii->birthday > TRAP_DELETE_TIME )//020315 lsw
			{	
				ii->birthday = g_curr_time;

				{
					ii->bAlive = REMOVE_;
					SetArea( REMOVE_ITEM_AREA, m );
				}
			}
		}
		else if( attr2 & IA2_TRAPED1 )
		{		
			if( g_curr_time  - ii->birthday > TRAP_DELETE_TIME )//020215 lsw
			{	
				ii->birthday = g_curr_time;

				{
					ii->bAlive = REMOVE_;
					SetArea( REMOVE_ITEM_AREA, m );
				}
			}
		}
		else if( ( GetAttr2( attr2, IA2_COLOSSUS_STONE ) )
			|| ( GetAttr2( attr2, IA2_SCENARIO_POTAL ) )	)	// 020620 YGI
		{
			// 矫埃捞 瘤唱档 酒公繁 累诀阑 窍瘤 臼绰促.
		}
		else		// 矫埃捞 版苞窍搁 磊悼栏肺 瘤况柳促. 
		{		

			int iLimitTime = 120;
			if(!LocalMgr.IsAbleNation(KOREA))//030102 lsw Local
			{
				iLimitTime = 60;
			}
			if( g_curr_time  - ii->birthday > iLimitTime )
			{	
				ii->birthday = g_curr_time;

				{
					ii->bAlive = REMOVE_;
					SetArea( REMOVE_ITEM_AREA, m );
				}
			}	
		}
	}
}








//-----------------------------------------------------------------------
// 酒捞袍 积己 窃荐 ( 10/24 )
//-----------------------------------------------------------------------
/*
DWORD ItemAttr_Make_First( int type, int itemno, int success_rate )
{
	switch( type )
	{
		case	PLANT			:	return global_time + (21600 * 60);
		case	MINERAL			:
		case	HERB			:	
		case	COOK			:	return global_time + (21600 * 60);
		case	POTION			:
		case	TOOL			:	
		case	WEAPON			:
		case	DISPOSABLE		:
		case	ARMOR			:
		case	ACCESSORY		:
		case	ETC				:
		break;
	}
	return 0L;
}
*/



//////////////////////////////////////////////////////////////////////////
// 某腐磐 牢亥配府俊辑 后镑 茫扁, 绝栏搁 0阑 府畔
bool SearchInv( ItemAttr inv[3][3][8], int &a, int &b, int &c )
{
	for( a=0; a<3; a++)
	for( b=0; b<3; b++)
	for( c=0; c<8; c++)
	{
		if( !inv[a][b][c].item_no ){return true;}
	}

	return false;	
}

int SearchInv_invert( ItemAttr inv[3][3][8], int &a, int &b, int &c )
{
	for( a=2; a >=0; a--)
	for( b=2; a >=0; b--)
	for( c=7; c >=0; c--)
	{
		if( !inv[a][b][c].item_no )
			return 1;
	}

	return 0;	
}


// add money 俊辑 荤侩且 MAX_MONEY焊促 累篮 捣苞 后傍埃阑 茫绰 窃荐		// 001030 YGI
int SearchMoneyItem( ItemAttr inv[3][3][8], int &a, int &b, int &c )		
{
	for( a=0; a<3; a++)
	for( b=0; b<3; b++)
	for( c=0; c<8; c++)
	{
		if( inv[a][b][c].item_no ) 
		{
			CItem *t = ItemUnit( inv[a][b][c] );
			if( !t ) continue;
			
			int iItemKind = IK_NEW_MONEY;
			if(LocalMgr.IsChangeMoney())//030102 lsw
			{
				iItemKind = IK_NEW_MONEY;
			}
			else
			{
				iItemKind = IK_MONEY;
			}

			if( t->GetItemKind() == iItemKind )
			{
				if( inv[a][b][c].attr[IATTR_MUCH] < MAX_MONEY ) 
				{
					return 1;
				}
			}
		}
		else 
		{
			return 1;
		}
	}

	return 0;	
}


///////////////////////////////////////////////////////////////////////////////////
