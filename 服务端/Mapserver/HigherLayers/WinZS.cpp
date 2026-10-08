/* ����������������������������������������������������������������������������
 �ۼ���   : 1995�� 3�� 27��
 �ۼ���   : �� ����
 Compile  : bcc -ms
 ȭ�� ����: �� ȭ���� ��ũ��Ʈ ������ ���������� �ؼ��ϰ� �̰��� ����Ⱑ
            ���� �ִ� ������ ���� ȭ�Ϸ� �����Ѵ�.
            * ��� �Լ����� 1����Ʈ�� ��ȯ�ȴ�. �׷��Ƿ� �ִ� ���ɾ��� ��
              ���� ����, �ܺ� ���ɾ� ���ļ� 255���̴�.
            * ��� ������ �������� 0x8000�� OR�ؼ� ������Ų��.
			* ��� ��ġ���� �������� ������. ( 0 ~ 32767 )
            * ��� GOTO ������ ���� �ּҴ� 2����Ʈ�̴�.
            * switch������ case���� �ִ� 50���̴�.


date of Modification : 1997�� 10�� 30��
who modified : Kim Hongseok
contents of modification : porting of windows
���������������������������������������������������������������������������� */
#include "..\stdafx.h"
#include "DefaultHeader.h"



#define __WINZS_H__
#include "WinZs.h"

#include "Scrp_int.h"
#include "RajaSystem.h"




#ifndef __TYPEDEF__
    typedef unsigned char  BYTE;
    typedef unsigned short WORD;
    typedef unsigned long  DWORD;
#endif


#define MAX_SCR_ARRAY     10000


// �� ���ɾ �ش��ϴ� ���ڿ��� ���� �����Ѵ�.
// ������ ���ǹ��� �����ϵ� ����ȭ�Ͽ� ������ �������� ���̴�.
/*
enum {
    goto_     =  0xe0,
    if_       =  0xe1,
    call_     =  0xe2,
    add_      =  0xe3,
    sub_      =  0xe4,
    mul_      =  0xe5,
    div_      =  0xe6,
    rest_     =  0xe7,
    equal_    =  0xe8,
    end_      =  0xe9,
    ifequal_  =  0xea,
    or_       =  0xeb,
    and_      =  0xec,
    variable_ =  0xed,
    value_    =  0xee,
    add2_     =  0xef,
    sub2_     =  0xf0,
    mul2_     =  0xf1,
    div2_     =  0xf2,
    rest2_    =  0xf3,
    or2_      =  0xf4,
    and2_     =  0xf5,
};
*/
// ������ ���ǹ��� �������Ҷ��� �䱸�Ǵ� ���̴�.
enum {
    else_     =  0xd0,
    case_     =  0xd1,
    endswitch_=  0xd2,
    default_  =  0xd3,
    switch_   =  0xd4,
    break_    =  0xd5,
    include_  =  0xd6,
};


#define LABEL_LENGTH 20        // ���� �ִ� ���̸� ��Ÿ��

enum {
    END = 0,
    DELIMITER,
    VARIABLE,
    NUMERIC,
    FUNCTION,
    STRING,
    LABEL,
    COMMAND,
    USERFUNC,
};


typedef struct {
    char s[ 50 + 1 ];
    int n;
} COMMSET;


typedef struct {        // if_else ���� ������ �����ϱ� ���� ����ü
    int off;            // if�� else���� ��ġ�� ���� �ɼ�
    BYTE d;             // ���̸� ��Ÿ��
    BYTE t;             // 0 = if, 1 = else
} IF_ELSE;


typedef struct {        // ����ڰ� ������ �Լ����� ������ ����ü
    char s[ 50 + 1 ];       // �Լ����� ������ ����
    char format[ 31 ];  // �Լ��� �ش��ϴ� �μ��� format�� ������ ����
    int n;             // �Լ����� �ִ°��� ��ġ
} USER_FUNC;


typedef struct {        // switch ���� ������ ������ ����ü
    int total;          // ���α׷� ó���� case�� ������ ������ ����
    int count;
    int def;
    int numoff[ 50 ];
} SWITCHCASE;


// ������ �ش��ϴ� ���ɾ�� �� ���ɾ �ش��ϴ� ���ڸ� ������ ����ü
static COMMSET comm[ 255 ] = {
    { "goto",           goto_ },
    { "if",             if_ },
    { "call",           call_ },
    { "end",            end_ },
    { "else",           else_ },
    { "switch",         switch_ },
    { "case",           case_ },
    { "endswitch",      endswitch_ },
    { "default",        default_ },
    { "break",          break_ },
    { "include",        include_ },
};


static COMMSET variable[ MAX_SCR_ARRAY ];             // ����ڰ� ������ ������ ������ ����
static COMMSET label1[ MAX_SCR_ARRAY ];               // ���� ���� ������ �����ϴ� ����
static COMMSET label2[ MAX_SCR_ARRAY ];               // goto�� ������ ���� ���� ����
static IF_ELSE ifelse[ MAX_SCR_ARRAY ];               // if, else���� ������ ����
static COMMSET userfunc1[ MAX_SCR_ARRAY ];            // ���� �Լ����� ������ ����
static COMMSET userfunc2[ MAX_SCR_ARRAY ];            // call�� ������ ���� �Լ��� ����
static USER_FUNC g_function[ MAX_SCR_ARRAY ];           // ������ ������ �Լ��� ����
static SWITCHCASE swit[ 50 ];               // �ϳ��� switch������ ������
                                     // case��������

static char TOKEN[ MAX_SCR_ARRAY ];                    // �Ҹ��س� ��ū�� ������ ����
static char TOKENTYPE;                      // ����� ��ū�� Ÿ��
static int  TOK;                            // ��ū�� ��ȯ�� ���� ���� ����

static int ifsp;                            // if, else�� ���ð�
static int switsp;                          // switch�� ������
static int label1_n, label2_n;              // ������ �󺧰��� ������ �󺧰��� ����
static int user1_n, user2_n;                // ������ �Լ����� ������ �Լ����� ����
static int funclabelsp;						// GetLabel () �Լ����� ���

static int LINE = 0;                        // �ҽ�ȭ���� ���ΰ��� ī��Ʈ

BYTE *wzsprog,                          // �ҽ� ���α׷��� �о���� ����
	 *oldwzsprog;                       // ó������ wzsprog�� ���� ���� ������
									 // ���α׷� ����� free�Լ����� ���
static BYTE *outprog,                       // ��� ���۸� ����Ŵ
			*oldoutprog;                    // ó������ oldoutprog�� ���� ���� ������
                                     // ���α׷� ����� free�Լ����� ���
static int SOURCEFILESIZE;                  // �ҽ�ȭ���� ũ�⸦ ��Ÿ��
static char TARGETFILENAME[ FILENAME_MAX];             // ����� ȭ���� �̸��� ��Ÿ��
static FILE *outfile;                       // ����� ȭ���� ����Ű�� ȭ�� ������
static int INCLUDEFILESIZE;                 // ��Ŭ���� ȭ���� ũ�⸦ ����Ŵ

static int ErrorCount;


static char Wz_scriptfilename[FILENAME_MAX];


// Scrp_int.cpp
extern int func_off[100];

//////////////////////////////////////////////////////////////////////////
// �������� �Լ����� ����
//////////////////////////////////////////////////////////////////////////




//------------------------------------------------------------------------

enum {
    ZS_FALSE,
    ZS_TRUE,
};

#define ZSAlloc(x)      ZS_Alloc( (x), __FILE__, __LINE__ )

void ZSError( char *s, ... )
{
    va_list arg;
	char temp[ FILENAME_MAX];

    va_start( arg, s );
    vsprintf( temp, s, arg );
    va_end( arg );

	MyLog( LOG_FATAL, "ZSerror: %s", temp );
}








char *ZS_Alloc( int size, const char *file, int line )
{
    char *memorytemp;
	memorytemp = (char *)GlobalAlloc( GMEM_FIXED, size );
    //memorytemp = (char *)malloc( size );

    if( !memorytemp )
        ZSError( "%s(%d) : \n %d alloc Err\n", file, line, size );

    return( memorytemp );
}


void ZSfree( void *t )
{
	GlobalFree( t );
//	free( t );
}



FILE *ZS_Fopen( const char *file, const char *mode )
{
    FILE *tempfile;

    tempfile = fopen( file, mode );
    if( !tempfile )
        ZSError( "%s FILE PROCESS ERROR.\n", file );

    return( tempfile );
}

//------------------------------------------------------------------------





/* ������������������������������������
 �Լ��� : Incode
 ��  �� : s = ��ȣȭ�� ���ڿ�
 ���ϰ� : ����
 ��  �� : s���ڿ��� �޾Ƽ� ���ο� ���ǵ� ��ġ�� ������ �����ؼ� ��ȣȭ��
������������������������������������ */
void WinZsIncode( char *s )
{
//    static BYTE t[ 20 ] = {
//        101, 255, 168, 210, 184, 47, 176,  54,  58, 63,
//         78,   1,  34,  69, 200,  88,  77, 111, 48, 121
//    };
    BYTE t = 0xff;
    int i = 0;

    while( *s ) {
        (*s++) ^= 0xff;
        if( ++i > 19 ) i = 0;
    }
}


/* ������������������������������������
 �Լ��� : SearchFunction
 ��  �� : s = ã���� �ϴ� �Լ��� ���ڿ�
 ���ϰ� : ã�����ϸ� -1, ã���� ��ȣ�� ����
 ��  �� : s���ڿ��� �ش��ϴ� ������ ������ �Լ����� function�迭���� ã�´�.
������������������������������������ */
int SearchFunction( char *s )
{
    int i;

	for( i = 0; g_function[i].s[0] != 0; i++ )
        if( !strcmp( g_function[i].s, s ) ) return( i );

    return( -1 );
}


/* ������������������������������������
 �Լ��� : SearchCommand
 ��  �� : s = ã�����ϴ� ���ɾ��� ���ڿ�
 ���ϰ� : ã�����ϸ� -1, ã���� ��ȣ�� ����
 ��  �� : s���ڿ��� �ش��ϴ� ���ɾ ã��
������������������������������������ */
int SearchCommand( char *s )
{
	int i;

    for( i = 0; comm[i].s[0] != 0; i++ )
        if( !strcmp( comm[i].s, s ) ) return( i );

    return( -1 );
}


/* ������������������������������������
 �Լ��� : SearchVariable
 ��  �� : s = ã���� �ϴ� ������ ���ڿ�
 ���ϰ� : ã�����ϸ� -1, ã���� ��ȣ�� ����
 ��  �� : s���ڿ��� �ش��ϴ� �������� ã��
������������������������������������ */
int SearchVariable( char *s )
{
    int i;

    for( i = 0; variable[i].s[0] != 0; i++ )
		if( !strcmp( variable[i].s, s ) ) return( i );

    return( -1 );
}


/* ������������������������������������
 �Լ��� : SearchLabel
 ��  �� : s = ã���� �ϴ� ���� ���ڿ�
 ���ϰ� : ã�����ϸ� -1, ã���� ��ȣ�� ����
 ��  �� : s���ڿ��� �ش��ϴ� ���� ã��
������������������������������������ */
int SearchLabel( char *s )
{
    int i;

    for( i = 0; label1[i].s[0] != 0; i++ )
        if( !strcmp( label1[i].s, s ) ) 
			return( i );

    return( -1 );
}



/* ������������������������������������
 �Լ��� : SearchUserFunc
 ��  �� : s = ã���� �ϴ� ������ ������ �Լ��� ���ڿ�
 ���ϰ� : ã�����ϸ� -1, ã���� ��ȣ�� ����
 ��  �� : �ҽ����� ������ s���ڿ��� �ش��ϴ� �Լ����� ã��
          �Լ����� �ݵ�� '_'�� ���۵Ǿ�� ��
������������������������������������ */
int SearchUserFunc( char *s )
{
    int i;

    for( i = 0; userfunc1[i].s[0] != 0; i++ )
        if( !strcmp( userfunc1[i].s, s ) ) return( i );

    return( -1 );
}




/* ������������������������������������
 �Լ��� : Serror
 ��  �� : x = ����ϰ��� �ϴ� ���� �޽����� ��ȣ
 ���ϰ� : ����
 ��  �� : x�� �ش��ϴ� ���� �޽����� ����ϰ� ���α׷��� �����Ѵ�.
������������������������������������ */
void Serror( int x )
{
    char *s[] = {
        "Compile completed.",             // 0
        "Systax error",                   // 1
        "Unbalanced ()",                  // 2
        "Unbalanced {}",                  // 3
        "Too few patameter",              // 4
        "Mismatch patameter",             // 5
        "Invalid patameter",              // 6
        "Invalid Label",                  // 7
		"Too long label name",            // 8
        "Label name is not found",        // 9
        "Lvalue required",                // 10
        "Stack overflow",                 // 11
        "Mismatch if - else",             // 12
        "Mismatch function - end",        // 13
        "Name already exist",             // 14
        "Function name not found",        // 15
        "Symbol not found",               // 16
        "Data file (zs.dat) open error",  // 17
        "Numeric is too big",             // 18
        "file create error",              // 19
        "Not found variable",             // 20
        "Mismatch Switch - endswitch",    // 21
        "Variable not found",             // 22
        "File not found",                 // 23
    };

//    ZSfree( oldwzsprog );
//    ZSfree( oldoutprog );

	ErrorCount++;

	remove( TARGETFILENAME );


	char nn[ FILENAME_MAX];
	strcpy( nn, Wz_scriptfilename );
	for( int i = strlen( nn ) - 1  ; i > 0  ; i -- )	   if( *(nn + i ) == '\\' ) break;
	
//	sprintf( temp, "%s: %d line: '%s'\n\n ' %s '�� �����?",s[x], LINE, TOKEN,  strupr( nn+i+1) );
//	if( YesOrNo( temp, "Script Compile Error") == IDYES )	NotePad( Wz_scriptfilename );



#ifdef MAKE_EXE_FILE	
    if( x == 0 ) printf( "\n%s\n", s[0] );
    else {
        printf( "\n\aError:\n%5d LINE: '%s' %s\n", LINE, TOKEN, s[x] );
    }
	printf( "Anyway, Process?( y/N ) ");
	t = _getch();
	if( t == 'y' || t== 'Y' ) return;
	else exit(1);
#endif
 }



/* ������������������������������������
 �Լ��� : Isdeli
 ��  �� : i = �˻��� ����
 ���ϰ� : ���й����̸� 1, �ƴϸ� 0
 ��  �� : i���ڰ� ���й��������� �˻��Ѵ�.
������������������������������������ */
int Isdeli( char i )
{
    if( strchr( " ';,+-*/%#=<>(){}[]:!@\t&|", i ) )
        return( 1 );

    return( 0 );
}



/* ������������������������������������
 �Լ��� : Isspace
 ��  �� : i = �˻��� ����
 ���ϰ� : ����, ��, ����, ���� �ǵ幮���̸� 1, �ƴϸ� 0
 ��  �� : i���ڰ� ����, ��, ����, ���� �ǵ幮�������� �˻��Ѵ�.
����������������������������������� */
int Isspace( char i )
{
    switch( i ) {
        case 0x0a:        // ���� �ǵ� �����̸� LINE������ �ϳ� �����Ѵ�.
            LINE++;
        case 0x0d:
		case '\t':
        case ' ':
            return( 1 );
        default:
            return( 0 );
    }
}



/* ������������������������������������
 �Լ��� : PutToken
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : TOKEN�迭�� ������ wzsprog�� �ٽþ���.
������������������������������������ */
void PutToken( void )
{
    int i;

	i = strlen( TOKEN );

    while( i ) {
        *--wzsprog = TOKEN[ i - 1 ];
        i--;
    }
    if( TOKENTYPE == LABEL ) {
        *--wzsprog = ':';
    }
}



/* ������������������������������������
 �Լ��� : GetToken
 ��  �� : ����
 ���ϰ� : ��ū Ÿ���� �����Ѵ�.
 ��  �� : wzsprog���۸� �м��ؼ� ��ū�� ��´�.
������������������������������������ */
int GetToken( void )
{
    register char *temp;

    // ��ū ���۸� �ʱ�ȭ �Ѵ�.
    temp = TOKEN;
    TOKENTYPE = 0;
	TOK = 0;

start:
    // ȭ���� ���� �ٴٶ�����
    if( *wzsprog == 0 ) return( TOKENTYPE = END );

    // ����, �ǹ���, ����, �����ǵ� ���ڸ� ���ش�.
    while( Isspace( *wzsprog ) ) ++wzsprog;
    if( *wzsprog == 0 ) goto start;        // ȭ���� ���̸�

    // �󺧸��̸� TOKEN������ �̸��� �����ϰ� ����
    if( *wzsprog == ':' ) {
        if( Isdeli( *++wzsprog ) ) { Serror( 7 ); return -1; } // �󺧸��� �����ڿ� ���ڷ� ���� 
        while( !Isdeli(*wzsprog) && *wzsprog != '\r' && *wzsprog != 0 )
			*temp++ = *wzsprog++;
        *temp = 0;
        return( TOKENTYPE = LABEL );
    }

    // �ּ��̸� ������ �������� ��ŵ�ϰ� �ٽ� ó������ ���ư���.
    if( *wzsprog == ';' ) {
        ++wzsprog;
        while( *wzsprog != '\n' && *wzsprog++ != 0 );
        goto start;
    }

    // ����� ���� �Լ��̸�
    if( *wzsprog == '@' ) {
        ++wzsprog;
        while( !Isdeli(*wzsprog) && *wzsprog != '\r' && *wzsprog != 0 )
            *temp++ = *wzsprog++;
        *temp = 0;
        TOK = SearchUserFunc( TOKEN );
        return( TOKENTYPE = USERFUNC );
	}

    // ���й����̸�
    if( Isdeli( *wzsprog ) ) {
        *temp++ = *wzsprog++;
        *temp = 0;
        return( TOKENTYPE = DELIMITER );
    }

    // ���ڿ��̸�
    if( *wzsprog == '"' ) {
        ++wzsprog;
        while( *wzsprog != '"' && *wzsprog != '\r' && *wzsprog != 0 )
            *temp++ = *wzsprog++;
        // ������ ���ڰ� �����ǵ峪 ȭ���� ���̸� ����
        if( *wzsprog == '\r' || *wzsprog == 0 ) { Serror( 1 ); return -1; }
        *temp = 0;
        wzsprog++;        // '"'���ڸ� ��ŵ
        return( TOKENTYPE = STRING );
    }

    // �����̸�
    if( isdigit(*wzsprog) ) {
        while( isdigit(*wzsprog) && !Isdeli(*wzsprog) &&
               *wzsprog != '\r' && *wzsprog != 0 )
            *temp++ = *wzsprog++;
        *temp = 0;
        return( TOKENTYPE = NUMERIC );
    }

    // ���ɾ ��, ������ üũ
    if( isalpha(*wzsprog) ) {
        while( !Isdeli(*wzsprog) && *wzsprog != '\r' && *wzsprog != 0 )
            *temp++ = *wzsprog++;
        *temp = 0;

        // ���� ����� ���ڿ��� ���ɾ���
        TOK = SearchCommand( TOKEN );
        if( TOK != -1 ) return( TOKENTYPE = COMMAND );

		// ���� ����� ���ڿ��� �������
        TOK = SearchVariable( TOKEN );
        if( TOK != -1 ) return( TOKENTYPE = VARIABLE );

        // ���� ����� ���ڿ��� �󺧸��̶��
        TOK = SearchLabel( TOKEN );
        if( TOK != -1 ) return( TOKENTYPE = LABEL );

        // ���� ����� ���ڿ��� ����� ���� �Լ����̶��
        TOK = SearchFunction( TOKEN );
        if( TOK != -1 ) return( TOKENTYPE = FUNCTION );

        // �׿ܴ� ���ڿ��� ����
        return( TOKENTYPE = STRING );
    }

	{ Serror( 1 ); return -1; }

     return( TOKENTYPE );
}



/* ������������������������������������
 �Լ��� : WriteInt
 ��  �� : x = outprog�� ����ϰ��� �ϴ� ��
 ���ϰ� : ����
 ��  �� : x�� �ش��ϴ� ���� outprog���ۿ� ����Ѵ�.
������������������������������������ */
void WriteInt( int x )
{
	static int count=0;
	count++;
    *((int *)outprog) = x;
    outprog += sizeof( int );
}



/* ������������������������������������
 �Լ��� : WriteBYTE
 ��  �� : x = outprog���ۿ� ����ϰ��� �ϴ� ��
 ���ϰ� : ����
 ��  �� : x�� �ش��ϴ� ���� outprog���ۿ� ����Ѵ�.
������������������������������������ */
void WriteBYTE( char x )
{
    *outprog++ = x;
}



/* ������������������������������������
 �Լ��� : First
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : '}' ��ȣ�� �߰��ϸ� ifelse ������ ���ϰ� ����Ÿ�� ó���Ѵ�.
������������������������������������ */
int First( void )
{
    --ifsp;

    if( ifsp < 0 ){ Serror( 3 );  return -1; }       // ifelse ������ �����÷ο� �������
    if( ifelse[ ifsp ].d != ifsp ) { Serror( 3 ); return -1; }  // ������ ���̰� �������� ���

    *((int *)(oldoutprog + ifelse[ifsp].off )) =
        outprog - oldoutprog;

	return 1;
}



/* ������������������������������������
 �Լ��� : Inc
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ��Ģ����, %, |, &, ~ ������ �����Ѵ�.
������������������������������������ */
int Inc( void )
{
    int jj[ 3 ][ 2 ];               // ù��° ÷�ڴ� �������� ���������� ��Ÿ��
    int i;                          // �����̸� 0, �����̸� 1
    BYTE j;

    jj[0][0] = 0;                   // ù��° ���ڸ� ����, ù��° ���ڴ� ������ ����
    jj[0][1] = variable[TOK].n;

    if( GetToken() == -1 ) return -1;

    if( strchr( "+-/*%|&", *TOKEN ) )      // ���������ڵ��� ���
    {
        j = *TOKEN;

        if( GetToken() == -1 ) return -1;
        if( *TOKEN != '=' ){ Serror( 1 );  return -1; }

        switch( j )
        {
            case '+': j = add2_;  break;
            case '-': j = sub2_;  break;
            case '*': j = mul2_;  break;
            case '/': j = div2_;  break;
            case '%': j = rest2_; break;
            case '|': j = or2_;   break;
            case '&': j = and2_;  break;
        }
        WriteBYTE( j );

        WriteBYTE( (char)variable_ );
        WriteInt( jj[0][1] );

        if( GetToken() == -1 ) return -1;

        switch( TOKENTYPE )
        {
            case VARIABLE:
                WriteBYTE( (char)variable_ );
                WriteInt( variable[TOK].n );
                break;

            case NUMERIC:
                WriteBYTE( (char)value_ );
                WriteInt( atoi( TOKEN ) );
                break;

            default:
                {Serror( 1 );  return -1; }
        }
    }
    else
    {
        if( *TOKEN != '=' ) { Serror( 1 );  return -1; }

        if( GetToken() == -1 ) return -1;

        switch( TOKENTYPE )             // �ι�° ���ڰ� �������� ���������� �˻�
        {
            case VARIABLE:              // ������ ��� ó��
                jj[1][0] = 0;
                jj[1][1] = variable[TOK].n;
                break;

            case NUMERIC:               // ������ ��� ó��
                jj[1][0] = 1;
                jj[1][1] = atoi( TOKEN );
                break;

            default:                    // �׿��� TYPE�̸� ����
                { Serror( 1 ); return -1; }
        }

        if( GetToken() == -1 ) return -1;
        if( !strchr( "+-/*%|&", *TOKEN ) )          // ���� ���Թ��̶��
        {
            PutToken ();
            WriteBYTE( (char)equal_ );

            for( i = 0; i < 2; i++ )
            {
                if( jj[i][0] == 0 )                  // �������
                    WriteBYTE( (char)variable_ );
                else                                 // ���ڶ��
                    WriteBYTE( (char)value_ );

                WriteInt( jj[i][1] );
            }
        }
        else                                         // ���Թ��� �ƴ϶��
        {
            switch( *TOKEN )
            {
                case '+': j = add_;  break;
                case '-': j = sub_;  break;
                case '*': j = mul_;  break;
                case '/': j = div_;  break;
                case '%': j = rest_; break;
                case '|': j = or_;   break;
                case '&': j = and_;  break;
                default : { Serror( 1 ); return -1; }
            }
            WriteBYTE( j );

            if( GetToken() == -1 ) return -1;               // ����° ���ڸ� ����

            switch( TOKENTYPE )
            {
                case VARIABLE:
                    jj[2][0] = 0;
                    jj[2][1] = variable[TOK].n;
                    break;

                case NUMERIC:
                    jj[2][0] = 1;
                    jj[2][1] = atoi( TOKEN );
                    break;

                default: { Serror( 1 ); return -1; }
            }

            for( i = 0; i < 3; i++ )
            {
                if( jj[i][0] == 0 )        // �����̸�
                    WriteBYTE( (char)variable_ );
                else                       // �����̸�
                    WriteBYTE( (char)value_ );

                WriteInt( jj[i][1] );
            }
        }
    } // end if

	return 1;
}




/* ������������������������������������
 �Լ��� : GetLabel
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : �󺧸��� �ִ°��� ��ġ�� �����Ͽ� label1������ �����Ѵ�.
������������������������������������ */
void GetLabel( void )
{
//    static int i;

    label1[funclabelsp++].n = (int)(outprog - oldoutprog);
}



/* ������������������������������������
 �Լ��� : GetCommand
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : �⺻���� ������ ó���Ѵ�.
������������������������������������ */
int GetCommand( void )
{
    int i, j;

    switch( comm[TOK].n )
    {
        // if���� ó���Ѵ�.
        case if_:
            WriteBYTE( (char)if_ );

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != '(' ) { Serror( 1 ); return -1; }

            if( GetToken() == -1 ) return -1;
            // ���� ���� ��ū�� ������ �ƴ϶��
            if( TOKENTYPE != VARIABLE ) { Serror( 10 ); return -1; }

            WriteBYTE( (char)variable_ );                             // ������ ���� ����
            WriteInt( variable[TOK].n );

            if( GetToken() == -1 ) return -1;
            if( !strchr( "<>!=", *TOKEN ) ) { Serror( 1 ); return -1; } 

            // �� ��ȣ�� ã�Ƽ� ������ ó���� �Ѵ�.
            // < - 0, <= - 1, > - 2, >= - 3, != - 4, == - 5
            switch( *TOKEN )
            {
                case '<':
                    if( GetToken() == -1 ) return -1;
                    if( *TOKEN != '=' )          // '<='�����ڰ� �ƴ϶��
                    {
                        PutToken();
                        WriteBYTE( 0 );
                    }
                    else
                    {
                        WriteBYTE( 1 );
                    }
                    break;

                case '>':
                    if( GetToken() == -1 ) return -1;
                    if( *TOKEN != '=' )          // '>='�����ڰ� �ƴ϶��
                    {
                        PutToken();
                        WriteBYTE( 2 );
                    }
                    else {
                        WriteBYTE( 3 );
                    }
                    break;

                case '!':
                    if( GetToken() == -1 ) return -1;
                    if( *TOKEN != '=' ) { Serror( 1 ); return -1; }
                    WriteBYTE( 4 );
                    break;

                case '=':
                    if( GetToken() == -1 ) return -1;
                    if( *TOKEN != '=' ) { Serror( 1 ); return -1; }
                    WriteBYTE( 5 );
                    break;
            }

            // ���� ����� �캯���� ó���Ѵ�. �캯���� �����̰ų� ��ġ�̴�.
            if( GetToken() == -1 ) return -1;
            switch( TOKENTYPE )
            {
                case NUMERIC:
                    WriteBYTE( (char)value_ );
                    WriteInt( atoi(TOKEN) );
                    break;

                case VARIABLE:
                    WriteBYTE( (char)variable_ );
                    WriteInt( variable[TOK].n );
                    break;

                default: { Serror( 1 ); return -1; }
            }

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != ')' ) { Serror( 1 ); return -1; }

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != '{' ) { Serror( 1 ); return -1; }

            // ������ �����ϰ�� ������ �ּҸ� �������� ����� 0���� ����
            WriteBYTE( (char)goto_ );
            WriteInt( 0 );

            ifelse[ifsp].off = outprog - oldoutprog - sizeof( int );
            ifelse[ifsp].d = ifsp;
            ifelse[ifsp].t = 0;
            ++ifsp;
            break;

        // else���� ó���Ѵ�.
        case else_:
            // goto���� �ش��ϴ� ���� ����
            WriteBYTE( (char)goto_ );
            WriteInt( 0 );

            // if���� ������ ��� ������ �ּҸ� ������ �ּҷ� �����Ѵ�.
            *((int *)( oldoutprog + ifelse[ifsp].off)) += (sizeof( int ) + 1);

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != '{' ) { Serror( 1 ); return -1; }
            if( ifelse[ifsp].d != ifsp ) { Serror( 1 ); return -1; }

            ifelse[ifsp].off = outprog - oldoutprog - sizeof( int );
            ifelse[ifsp].d = ifsp;
            ifelse[ifsp].t = 1;
            ++ifsp;
            break;

        // goto���� ó��
        case goto_:
            WriteBYTE( (char)goto_ );
            if( GetToken() == -1 ) return -1;
            if( TOKENTYPE != LABEL ) { Serror( 9 ); return -1; }
			// �󺧸��� label2�� ����. �� ���� ���߿� label1�� ���Ҷ� ����
			label2[label2_n].n = (int)(outprog - oldoutprog);
			strcpy( label2[label2_n++].s, TOKEN );
			WriteInt( 0 );        // ������ �ּҸ� �ӽ÷� ����
		break;

        // call���� ó��
        case call_:
            WriteBYTE( (char)call_ );

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != '(' ) { Serror( 1 ); return -1; }

            if( GetToken() == -1 ) return -1;
            if( TOKENTYPE != USERFUNC ) { Serror( 15 ); return -1; }
            strcpy( userfunc2[user2_n].s, TOKEN );
            // call������ ������ ��������� Ȯ���ϰ� 0�� ����
            userfunc2[user2_n++].n = outprog - oldoutprog;
            WriteInt( 0 );

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != ')' ) Serror( 1 );
            break;

        // end���� ó��
        case end_:
            WriteBYTE( (char)end_ );
            if( ifsp > 0 ) { Serror( 3 ); return -1; }
            break;

        // switch���� ó��
        case switch_:
            ++switsp;

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != '(' ) { Serror( 1 ); return -1; }

            if( GetToken() == -1 ) return -1;
            if( TOKENTYPE != VARIABLE ) { Serror( 22 ); return -1; }
            j = variable[TOK].n;

            if( GetToken() == -1 ) return -1;
            if( *TOKEN != ')' ) { Serror( 1 ); return -1; }

            // ���α׷� ó���� ������ case���� ������ŭ ������ Ȯ��
            for( i = 0; i < swit[switsp].total; i++ )
            {
                WriteBYTE( (char)ifequal_ );

                WriteBYTE( (char)variable_ );
                WriteInt( j );

                // ���� ����� ���� ������ Ȯ��
                swit[switsp].numoff[i] = outprog - oldoutprog;
                WriteBYTE( 0 );
                WriteInt( 0 );

                // �󺧸��� ���� ����ġ�� ���� �����ؼ� ������ ���Ҷ�
                // ����Ѵ�.
                sprintf( label2[label2_n].s, "case%02d_%02d", switsp, i );
                label2[label2_n++].n = outprog - oldoutprog;
                WriteInt( 0 );
            }

            // ����Ʈ���� ������ �󺧸��� ���ڿ��� �߰��Ͽ� default���� ����
            WriteBYTE( (char)goto_ );
            sprintf( label2[label2_n].s, "defa%02d", switsp );
            label2[label2_n++].n = outprog - oldoutprog;
            WriteInt( 0 );
            break;

        // case���� ó��
        case case_:
            // �󺧸��� case���� �ش��ϴ� ���ڿ��� �߰��Ͽ� ���߿� ó���Ѵ�.
            sprintf( label1[label1_n].s, "case%02d_%02d", switsp,
                swit[switsp].count );
            label1[label1_n++].n = outprog - oldoutprog;

            if( GetToken() == -1 ) return -1;
            switch( TOKENTYPE )
            {
                case NUMERIC:
                    *((char *)(oldoutprog +
                        swit[switsp].numoff[swit[switsp].count])) = (char)value_;
                    j = atoi( TOKEN );
                    break;

                case VARIABLE:
                    *((char *)(oldoutprog +
                        swit[switsp].numoff[swit[switsp].count])) = (char)variable_;
                    j = variable[TOK].n;
                    break;

                default: { Serror( 1 ); return -1; }
            }

            *((int *)(oldoutprog + 1 +
                swit[switsp].numoff[swit[switsp].count++])) = j;
            break;

        // default���� ó��
        case default_:
            // �󺧸��� default���� ���ڿ��� �Է��Ͽ� ���߿� ó��
            sprintf( label1[label1_n].s, "defa%02d", switsp );
            label1[label1_n++].n = outprog - oldoutprog;
            break;

        // endswitch���� ó��
		case endswitch_:
            // switch���� ������ �ǹ��Ѵ�.
            if( swit[switsp].def != 1 ) {
                sprintf( label1[label1_n].s, "defa%02d", switsp );
                label1[label1_n++].n = outprog - oldoutprog;
            }
            sprintf( label1[label1_n].s, "defabreak%02d", switsp );
            label1[label1_n++].n = outprog - oldoutprog;

            swit[switsp].count = 0;
            break;

        // break���� ó��
        case break_:
            WriteBYTE( (char)goto_ );
            sprintf( label2[label2_n].s, "defabreak%02d", switsp );
            label2[label2_n++].n = outprog - oldoutprog;
            WriteInt( 0 );
            break;
    }

	return 1;
}




/* ������������������������������������
 �Լ��� : GetFunction
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : zs.dat���� ������ �Լ����� �м��Ͽ� �˸��� �۾��� �Ѵ�.
          i�� int�� �ǹ��ϸ�, s�� ���ڿ��� �ǹ��Ѵ�.
������������������������������������ */
int GetFunction( void )
{
    int i, slen, len, *p;
    int tok = TOK;

    WriteBYTE( (char)g_function[tok].n );          // �Լ��� ��ȣ�� ����

    len = strlen( g_function[tok].format );  // �Ű������� ���̸� ����

    if( GetToken() == -1 ) return -1;
    if( *TOKEN != '(' ){ Serror( 1 );  return -1; }

    for( i = 0; i < len; i++ ) {
        switch( g_function[tok].format[i] ) {
            case 'i':                      // int���� ó��
                if( GetToken() == -1 ) return -1;
                switch( TOKENTYPE )
                {
                    case NUMERIC:
                        WriteBYTE( (char)value_ );
                        WriteInt( atoi( TOKEN ) );
                        break;

                    case VARIABLE:
                        WriteBYTE( (char)variable_ );
//                        WriteInt( atoi( TOKEN ) );
                        WriteInt( variable[TOK].n );
                        break;

                    default:
                        { Serror( 6 ); return -1; }
                }
                break;

            case 's':                      // ���ڿ��� ó��
                if( GetToken() == -1 ) return -1;
                if( TOKENTYPE != STRING ) {Serror( 6 ); return -1; }
                slen = strlen( TOKEN );
                WinZsIncode( (char *)TOKEN );        // ���ڿ��� ��ȣȭ��
//                strcpy( outprog, (char *)TOKEN );
                WriteBYTE( (char)value_ );
                p = (int*)outprog;
                WriteInt( slen );                       // ���ڿ��� ���̸����
                memcpy( outprog, (char*)TOKEN, slen );    // ���ڿ��� ����
                outprog += slen;
REPEA:
                if( GetToken() == -1 ) return -1;
                if( TOKENTYPE == STRING )
                {
                    slen = strlen( TOKEN );
                    WinZsIncode( (char *)TOKEN );        // ���ڿ��� ��ȣȭ��
                    (*p) += slen;
                    memcpy( outprog, (char*)TOKEN, slen );
                    outprog += slen;
                    goto REPEA;
                }
                else
                {
                    PutToken();
                }

                break;

            default: { Serror( 6 ); return -1; }
        }

        // �Ű� ���������� ','�� ó���Ѵ�.
        if( len > 1 && i != (len-1) ) {
            if( GetToken() == -1 ) return -1;
            if( *TOKEN != ',' ) { Serror( 4 ); return -1; }
        }
    }
    if( GetToken() == -1 ) return -1;
    if( *TOKEN != ')' ) {Serror( 1 );  return -1; }

	return 1;
}



/* ������������������������������������
 �Լ��� : GetUserFunc
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ������ ��¹����� ��ġ�� ���� �Լ� ���ۿ� ����Ѵ�.
������������������������������������ */
void GetUserFunc( void )
{
	userfunc1[user1_n++].n = outprog - oldoutprog;
}



/* ������������������������������������
 �Լ��� : Delimeter
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : �����ڸ� �޾��� ��� ó���Ѵ�.
          '#'�� ��Ŭ��幮�� �ǹ��Ѵ�. ��Ŭ��� ���̶�� ���� ��¹�����
          outprog�� ��Ŭ��� ȭ���� ������ �о ����.
������������������������������������ */
int Delimeter( void )
{
    FILE *fp;
    char temp[1024];
    int i;

    if( *TOKEN != '#' ) { Serror( 1 ); return -1; }

    if( GetToken() == -1 ) return -1;
	if( TOKENTYPE != COMMAND || comm[TOK].n != include_ ) Serror( 1 );

    if( GetToken() == -1 ) return -1;
    if( TOKENTYPE != STRING ) { Serror( 1 ); return -1; }

    fp = fopen( TOKEN, "rb" );
    if( fp == NULL ) { Serror( 23 ); return -1; }

    while( 1 ) {
        i = fread( temp, 1, 1024, fp );
        fwrite( temp, 1, i, outfile );
        INCLUDEFILESIZE += i;
        SOURCEFILESIZE += i;
        if( i != 1024 ) break;

    }
    fclose( fp );

	return 1;
}




/* ������������������������������������
 �Լ��� : Run
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : �� ���ɿ� �´� �Լ��� ȣ���Ѵ�.
������������������������������������ */
int  Run( void )
{
    while( 1 ) {
        if( GetToken() == -1 ) return -1;

        if( TOKENTYPE == END ) break;        // ȭ���� ���̶��

        else if( *TOKEN == '}' ) {                // if,else���� �����ڶ��
            if( First() == -1 ) return -1;
            continue;
        }

        else if( TOKENTYPE == NUMERIC )       // ������ ó������ ���ڰ� �ü�����.
		{ Serror( 1 ); return -1; }

        else if( TOKENTYPE == DELIMITER )     // ������ ������ ó��
        {
            if( Delimeter() == -1 ) return -1;;
            continue;
        }

        else if( TOKENTYPE == VARIABLE ) {
            if( Inc() == -1 ) return -1;
            continue;
        }

        else if( TOKENTYPE == LABEL ) {
            GetLabel();
            continue;
        }

        else if( TOKENTYPE == COMMAND ) {
            if( GetCommand() == -1 ) return -1;
            continue;
        }

        else if( TOKENTYPE == FUNCTION ) {
            if( GetFunction() == -1 ) return -1;
            continue;
        }

        else if( TOKENTYPE == USERFUNC ) {
            GetUserFunc();
            continue;
        }

        { Serror( 1 ); return -1; }
    }
    if( ifsp > 0 ) { Serror( 3 ); return -1; }

	return 1;
}



/* ������������������������������������
 �Լ��� : LoadData
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : zs.dat�� �����ؼ� �˸��� �ؼ��� �ϰ� ������ �̸��� ��ǥ��, �Լ���
          �̸��� ��ǥ���� ������
������������������������������������ */
int  LoadData( char *szdat )
{
    FILE *fp;
    char temp[ FILENAME_MAX];
    DWORD size;
    int i = 0, j;
    BYTE *p;

	sprintf( temp, "%s/script/%s", GameServerDataPath, szdat );

    fp = fopen( temp, "rb" );
    if( fp == NULL ) { return 0; }

    size = filelength( fileno(fp) );

    wzsprog = p = (BYTE *)ZSAlloc( size + 1 );
	fread( wzsprog, (size_t)size, 1, fp );
    fclose( fp );

    if( *(wzsprog + size -1)  == 0x1a )
        *(wzsprog+size-1) = 0;
    else
        *(wzsprog+size) = 0;


    //printf( "\nZS.DAT file compile" );

	while( 1 ) {
		if( GetToken() == -1 ) return -1;
		if( TOKENTYPE == END ) break;

        // ������ ǥ���ϴ� ���ڿ��� ã�´�.
		if( strcmp( TOKEN, "variable" ) != 0 ) { Serror( 1 ); return -1; }


		// ������ �ش��ϴ� ��ǥ���� �� ���� variable�迭�� �����Ѵ�.
		while( 1 ) {
			if( GetToken() == -1 ) return -1;
			if( !strcmp( TOKEN, "end" ) ) break;        // ���̸� ��������

			if( TOKENTYPE != STRING ) Serror( 1 );
			j = SearchVariable( TOKEN );                // ���� ������ �ִ��� �˻�
			if( j != -1 ) Serror( 14 );
			strcpy( variable[i].s, TOKEN );

			if( GetToken() == -1 ) return -1;
			if( TOKENTYPE != NUMERIC ) Serror( 1 );
			j = atoi( TOKEN );
			if( j > 30000 ) { Serror( 18 ); return -1; }
			variable[i++].n = j;
		}

		i = 0;

		if( GetToken() == -1 ) return -1;
		if( strcmp( TOKEN, "function" ) != 0 ) { Serror( 1 ); return -1; }

		// �Լ��� �ش��ϴ� ��ǥ���� �� ���� function�迭�� �����Ѵ�.
		while( 1 ) {
			if( GetToken() == -1 ) return -1;
			if( !strcmp( "end", TOKEN )) break;

			if( TOKENTYPE != STRING ) { Serror( 1 ); return -1; }
			j = SearchFunction( TOKEN );
			if( j != -1 ) Serror( 14 );                 // ���� �Լ��� �ִ��� �˻�
			strcpy( g_function[i].s, TOKEN );

			GetToken ();
			if( TOKENTYPE != STRING ) { Serror( 1 ); return -1; }
			if( !strcmp( TOKEN, "null" ) )              // �Ű������� ������
				g_function[i].format[0] = 0;
			else
				strcpy( g_function[i].format, TOKEN );    // �Ű������� ������

			if( GetToken() == -1 ) return -1;
			if( TOKENTYPE != NUMERIC ) { Serror( 1 ); return -1; }
			j = atoi( TOKEN );
			if( j > 0xcf ) { Serror( 18 ); return -1; }
			g_function[i++].n = j;
		}

		if( i == 0 ) { Serror( 1 ); return -1; }
	}
	LINE = 0;
    ZSfree( p );

	return 1; 
    //printf( " --------> completed.\n\n" );
}




/* ������������������������������������
 �Լ��� : InitProgram
 ��  �� : s = �������ϰ��� �ϴ� ȭ���� �̸�
 ���ϰ� : ����
 ��  �� : �ҽ��� �޸𸮷� �о������, ����� ȭ���� �̸��� ������Ų��.
������������������������������������ */
int  InitProgram( char *s )
{
    FILE *fp;
    char *p;

    fp = fopen( s, "rb" );
    if( fp == NULL ) 
	{	
		char mapname[ FILENAME_MAX];
		sprintf( mapname, "Script File Not Found : '%s'", s );
		MyLog( LOG_FATAL, "%s", mapname );
        return -1;
    }	
	
    SOURCEFILESIZE = filelength( fileno(fp) );


    fread( wzsprog, SOURCEFILESIZE, 1, fp );
    fclose( fp );

    if( *(wzsprog + (int)SOURCEFILESIZE -1) == 0x1a )
        *(wzsprog + (int)SOURCEFILESIZE-1) = 0;
    else
        *(wzsprog + (int)SOURCEFILESIZE) = 0;

    strcpy( TARGETFILENAME, s );
    p = strchr( TARGETFILENAME, '.' );
    strcpy( p, ".ZSC" );

/*	if( GameMakeFlag == ON )
	{
		outfile = Fopen( TARGETFILENAME, "wb" );
		if( fp == NULL ) { 
	        exit( 1 );
		}
	}
*/
	return TRUE;
}



/* ������������������������������������
 �Լ��� : Path1
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : �ҽ��� �ѹ� �����鼭 ������ ������ üũ�Ѵ�.
������������������������������������ */
int Path1( void )
{
    BYTE *p = wzsprog;
    int j;
    int func = 0;
    int funcdepth = 0;
    int if_else_count = 0;
    int switch_case = 0;
    int switch_count = 0;

    while( 1 ) {
        if( GetToken() == -1 ) return -1;
        if( TOKENTYPE == END ) break;

        // if, else�� ������ �˻��ؼ� ������ üũ�Ѵ�.
        if( *TOKEN == '{' ) ++if_else_count;
        if( *TOKEN == '}' ) --if_else_count;

        // �󺧸��� ��쿡�� ���� ã�ƺ���, ������ �� �ߺ� ������ ����ϰ�
        // ������ ����Ѵ�.
        if( TOKENTYPE == LABEL )
        {
            if( strlen( TOKEN ) > LABEL_LENGTH ) { Serror( 8 ); return -1; }
            j = SearchLabel( TOKEN );
            if( j != -1 ) Serror( 14 );
            strcpy( label1[label1_n++].s, TOKEN );
        }

        // ������ ������ �Լ��̸� �Լ��� ã�ƺ���, ������ �Լ� �ߺ� ������
        // ����ϰ� ������ ����� �Ѵ�.
        if( TOKENTYPE == USERFUNC )
        {
            if( TOK != -1 ) { Serror( 15 ); return -1; }
            strcpy( userfunc1[funcdepth++].s, TOKEN );
            if( func++ != 0 ) { Serror( 13 ); return -1; }
        }

        if( TOKENTYPE == COMMAND ) {
            switch( comm[TOK].n ) {
                // switch���� case�� default���� ������ Ȯ���ϰ� ������ �˻�
                case switch_:
                    ++switch_case;
                    ++switch_count;
                    break;

                case endswitch_:
                    --switch_case; break;

                case default_:
                    swit[switch_count].def = 1; break;

                case case_:
                    ++swit[switch_count].total; break;

                // call���ɹ� �������� �󺧸��� ���⶧���� �̰��� ���Ѵ�.
                case call_:
                    if( GetToken() == -1 ) return -1;
                    if( GetToken() == -1 ) return -1;
                    if( GetToken() == -1 ) return -1; break;

                // end���� ó���ϸ� if-else, switch-endswitch, �׸��� �Լ���
                // �������ΰ��� �Ǵ��Ѵ�.
                case end_:
                    if( func-- != 1 )  { Serror( 13 ); return -1; }
                    if( if_else_count != 0 ) { Serror( 12 ); return -1; }
                    if( switch_case != 0 ) { Serror( 21 ); return -1; }
                    break;

                case goto_:
                    if( GetToken() == -1 ) return -1;
                    break;
            }
        }
    }
    if( func > 0 ) { Serror( 13 ); return -1; }

    ifsp = 0;
    wzsprog = p;
    LINE = 0;

	return TRUE; 
}




/* ������������������������������������
 �Լ��� : EndProgram
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ���α׷��� �� �������� ����Ǹ�, �󺧸��� ��ġ�� ���,
          �Լ����� ��ġ�� ���, �Լ��� ������ ���ȭ���� ó�� �κп� ����Ѵ�.
������������������������������������*/
int EndProgram( void )
{
    int i, j;
    int size = 0;

    // ������ �󺧸��� ������ ����� �󺧸��� ���� ���ؼ� �װ��� ����Ѵ�.
    for( i = 0; i < label2_n; i++ ) {
        j = SearchLabel( label2[i].s );
        if( j == -1 ) 
		{
            strcpy( TOKEN, label2[i].s );
            { Serror( 9 ); return -1; }
        }

        *((int *)(oldoutprog+label2[i].n)) = label1[j].n;
    }


    // ������ �Լ����� ������ ����� �Լ����� ���� ���ؼ� �װ��� ����Ѵ�.
    for( i = 0; i < user2_n; i++ ) {
        j = SearchUserFunc( userfunc2[i].s );
        if( j == -1 ) {
            strcpy( TOKEN, userfunc2[i].s );
            Serror( 15 );
        }

        *((int *)(oldoutprog+userfunc2[i].n)) = userfunc1[j].n;
    }

	for( i = 0; userfunc1[i].s[0]; i++ ) func_off[i] = userfunc1[i].n;
	TotalFunction = i;


/*
	if( GameMakeFlag == ON )
	{
		// ��� �Լ��� ������ ȭ���� ó���� ����� �ϰ� �Լ��� �ɼ°��� ����Ѵ�.
		temp = ftell( outfile );
		fseek( outfile, 8, SEEK_CUR );        // ����� ����ϱ����� ����

		for( i = 0; userfunc1[i].s[0]; i++ )
		{
			fwrite( &userfunc1[i].n, 1, sizeof( int ), outfile );
			size += sizeof( int );
		}

		// �����ϵ� ����� ȭ�Ϸ� ����Ѵ�.
		size += (outprog - oldoutprog - ( i * sizeof( int )) );
		fwrite( oldoutprog, outprog - oldoutprog, 1, outfile );

		fseek( outfile, temp, SEEK_SET );
		fwrite( &size, 1, sizeof( int ), outfile );        // ȭ���� ũ�⸦ ���
		fwrite( &i, 1, sizeof( int ), outfile );           // �Լ��� ������ ���
		fclose( outfile );
	}
*/
#ifdef MAKE_EXE_FILE
    printf( "\nTotal compile line     : %10d lines\n", --LINE );
    printf( "\nSource file size       : %10d BYTEs\n", SOURCEFILESIZE );
    printf( "\nTarget file size       : %10d BYTEs\n\n",        size + INCLUDEFILESIZE + i * sizeof( int ) );
#endif

	return TRUE;
}



static char *Filename;
static int Totalcount;


/*===========================================================================
   ��������Ʈ�� ���� ��� ȭ���� �˻��Ѵ�.
===========================================================================*/
int SearchFile( char *fn )
{	
    char temp[FILENAME_MAX], *p, (*f)[FILENAME_MAX];
    //struct find_t ff;
	struct	_finddata_t ff;
	int handle, c;
	
    strcpy( temp, fn );
    p = strchr( temp, '.' );
    if( p )
        strcpy( p, ".ZS" );
    else
        strcat( temp, ".ZS" );
	
    f = (char (*)[FILENAME_MAX])Filename;
	
	handle =  _findfirst( temp, &ff);
	if( handle == -1 ) return 0 ;
	c = 0;
    while( !c )
    {	
        strcpy( (char *)f++, ff.name );
        Totalcount++;
		
        c = _findnext( handle, &ff );
    }	
	
	return Totalcount;
}	
	
	
#ifdef MAKE_EXE_FILE	
	void main( int argc, char *argv[] )
	{	
		int i;
		char temp[FILENAME_MAX], (*f)[FILENAME_MAX];
		
		printf( "\n   ZS Script Compiler For Win95  Version 2.0\n" );
		printf( "----------------------------------------------------------------\n");
		
		wzsprog = oldwzsprog = (BYTE *)ZSAlloc( 800000 + 1 );
		outprog = oldoutprog = (BYTE *)ZSAlloc( 400000 );
		Filename = ZSAlloc( FILENAME_MAX * 2000 );
		
		if( argc == 1 )	strcpy( temp, "*.*" );
		else			strcpy( temp, argv[1] );
		
		if( SearchFile( temp ) ) LoadData();
		
		f = (char (*)[FILENAME_MAX])Filename;
		for( i = 0; i < Totalcount; i++, f++ )
		{
			wzsprog = oldwzsprog;
			outprog = oldoutprog;
			memset( wzsprog, 0, SOURCEFILESIZE + 1 );

			memset( label1,		0,	sizeof( COMMSET )	*	MAX_SCR_ARRAY );            // ���� ���� ������ �����ϴ� ����
			memset( label2,		0,	sizeof( COMMSET )	*	MAX_SCR_ARRAY );            // goto�� ������ ���� ���� ����
			memset( ifelse,		0,	sizeof( IF_ELSE )	*	MAX_SCR_ARRAY );            // if, else���� ������ ����
			memset( userfunc1,	0,	sizeof( COMMSET )	*	MAX_SCR_ARRAY );            // ���� �Լ����� ������ ����
			memset( userfunc2,	0,	sizeof( COMMSET )	*	MAX_SCR_ARRAY );            // call�� ������ ���� �Լ��� ����
			memset( swit,		0,	sizeof( SWITCHCASE )*	 50 );            // �ϳ��� switch������ ������ case��������

			printf( " %3d/%3d     : %13s File Compiling...........\n", i+1, Totalcount, (char*)f );
			

			ifsp = 0;                            // if, else�� ���ð�
			switsp = 0;                          // switch�� ������
			label1_n=0, label2_n = 0;              // ������ �󺧰��� ������ �󺧰��� ����
			user1_n = 0, user2_n = 0;                // ������ �Լ����� 
			
			
			InitProgram( (char*)f );
			Path1();
			Run();
			EndProgram();
		}

		printf( "\n    %d ZS Files Compiled ..\n", Totalcount );
		printf(   "    %d Error occured...\n", ErrorCount );
		printf( "-------------------------------------------Copyright(c),1995 F.E.W \n");
		printf( "-------------------------------------------------- Coded by Phyneix  \n");
		
		ZSfree( oldwzsprog );
		ZSfree( oldoutprog );
		ZSfree( Filename );
		
		_getch();
	}

#else
	
	
void InitTextScript( void )
{	
	oldwzsprog = (BYTE *)ZSAlloc( 800000 + 1 );
	oldoutprog = (BYTE *)ZSAlloc( 400000 );
}	
	

	
	
void EndTextScript( void )
{	
	if( oldwzsprog != NULL ) { ZSfree( oldwzsprog ); oldwzsprog = NULL; }
	if( oldoutprog != NULL ) { ZSfree( oldoutprog ); oldoutprog = NULL; }
}	
	
	
	
void NotePad( char *name )
{	
	//"NotePad.exe"
//	ShellExecute(NULL, NULL, SysInfo.notepad, name, NULL, SW_SHOWNORMAL);
}	
	
	
BYTE *LoadTextScript( char *name )
{	

	LINE = 0;
	
	TOK = 0; // ^^^^ 0308
	memset( variable, 0, sizeof( COMMSET ) *  2000 );	//MAX_SCR_ARRAY );             // ����ڰ� ������ ������ ������ ����
	memset( g_function, 0, sizeof( USER_FUNC ) * 1000 ); //MAX_SCR_ARRAY );           // ������ ������ �Լ��� ����
	
	LoadData( "Zs.Dat" );	

	wzsprog = oldwzsprog;
	outprog = oldoutprog;

	strcpy( Wz_scriptfilename, name );

	memset( wzsprog, 0, 100000 + 1 );
	memset( (char*)label1,		0,	sizeof( COMMSET )	*	100 ); // MAX_SCR_ARRAY );            // ���� ���� ������ �����ϴ� ����
	memset( (char*)label2,		0,	sizeof( COMMSET )	*	100 ); // MAX_SCR_ARRAY );            // goto�� ������ ���� ���� ����
	memset( (char*)ifelse,		0,	sizeof( IF_ELSE )	*	500 ); // MAX_SCR_ARRAY );            // if, else���� ������ ����
	memset( (char*)userfunc1,	0,	sizeof( COMMSET )	*	1000 );//MAX_SCR_ARRAY );            // ���� �Լ����� ������ ����
	memset( (char*)userfunc2,	0,	sizeof( COMMSET )	*	1000 ); //MAX_SCR_ARRAY );            // call�� ������ ���� �Լ��� ����
	memset( (char*)swit,		0,	sizeof( SWITCHCASE )*	 50 );						// �ϳ��� switch������ ������ case��������

	ifsp		= 0;                   // if, else�� ���ð�
	switsp		= 0;                   // switch�� ������
	label1_n	= 0, label2_n	= 0;   // ������ �󺧰��� ������ �󺧰��� ����
    user1_n		= 0, user2_n	= 0;   // ������ �Լ����� 
	funclabelsp = 0;

	if( InitProgram( name ) == -1 ) goto ERROR_;
	if( Path1()				== -1 ) goto ERROR_;
	if( Run()				== -1 ) goto ERROR_;
	if( EndProgram()		== -1 ) goto ERROR_;
	
  CLoadScript = SCRIPT_NEW;
	return oldoutprog;
ERROR_:

	char nn[ FILENAME_MAX];
	strcpy( nn, Wz_scriptfilename );

	for( int i = strlen( nn ) - 1  ; i > 0  ; i -- )	   if( *(nn + i ) == '\\' ) break;

//	sprintf( temp, "'%s'�� �ٽ� �ε� �մϴ�.", strupr(nn+ i +1)  );
//	if( YesOrNo( temp, "Ȯ��") == IDYES ) goto AGAIN_;

	return NULL;
}


#endif







