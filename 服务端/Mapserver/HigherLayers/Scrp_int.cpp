#define __SCRIPT_C__

#include "..\stdafx.h"
#include "DefaultHeader.h"

#include "scrp_int.h"
#include "scrp_exe.h"
#include "Winzs.h"

int  CLoadScript;  //LOAD_SCRIPT;
int  TotalFunction;                           // �� �Լ��� ������ ����
static char *prog, *oldprog;                    // ��ũ��Ʈ ����Ÿ�� ������ ����
static int CALLSTACK[ 10 ];                     // call�� ����� �ּҸ� ������ ����
static int CALLSP;                              // call���� ǥ���ϴ� ���� �����ͺ���
int func_count;
int func_off[_SCRIPT_FUNCTION_POINT_]; // CSD-030509


int  NScriptNo;



/*������������������������������������
 �Լ��� : Incode
 ��  �� : s = ��ȣȭ�� ���ڿ�
 ���ϰ� : ����
 ��  �� : s���ڿ��� �޾Ƽ� ���ο� ���ǵ� ��ġ�� ������ �����ؼ� ��ȣȭ��
������������������������������������ */
void Incode( char *s )
{
    int i = 0;
    while( *s )
    {
        *s++ ^= 0xff;	if( ++i > 19 ) i = 0;
    }
}


/* ������������������������������������
 �Լ��� : GetByte
 ��  �� : ����
 ���ϰ� : 1����Ʈ�� ��
 ��  �� : prog�� ��ϵ� ���뿡�� 1����Ʈ�� �����ϰ� prog�����͸� 1����
������������������������������������ */
BYTE GetByte( void )
{
    return( *prog++ );
}


/*������������������������������������
 �Լ��� : GetInt
 ��  �� : ����
 ���ϰ� : 4����Ʈ�� ��
 ��  �� : prog�� ��ϵ� ���뿡�� 4����Ʈ�� �����ϰ� prog�����͸� 4����
������������������������������������ */
int GetInt( void )
{
    int i, j;
	
    i = GetByte();
    j = *((int *)prog);

    prog += sizeof( int );

    if( i == variable_ )
        j = var[EventPC][j];

    return( j );
}

static void ChangeName( char *buf, int size, char *oldname, char *newname )
{
	for( int i = 0 ; i < size - (int)strlen( oldname ) ; i ++)
	{
		if( strncmp( buf + i, oldname, strlen( oldname )) == NULL )
		{
			for( int j = 0 ; j < (int)strlen( oldname ) ; j ++)
			{
				*( buf+i + j ) = newname[j];
			}
		}
	}
}


/* ������������������������������������
 �Լ��� : GetString
 ��  �� :
 ���ϰ� : ���ڿ��� �����ϰ� �ִ� �迭�� ���� ������
 ��  �� : ��ũ��Ʈ ����Ÿ���� ���ڿ��� �о���δ�.
������������������������������������ */
//����7
void GetString( char *buf )
{
    int  size;

    size = GetInt();
    memcpy( buf, prog, size );
    prog += size;
    buf[ size ] = 0;
    Incode( buf );
}

static char GetByte1( void )
{
     return( *prog++ );
}

static int GetInt1( void )
{
    int i;

    i = *((int *)prog);
    prog += sizeof( int );

    return( i );
}

/* ������������������������������������
 �Լ��� : If
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : if���� �ش��ϴ� ������ �����Ѵ�.
������������������������������������ */
void If( void )
{
    int i1, i2;
    BYTE com;
    int check = 0;
	int fuck=0;

    GetByte1();                      // ���� �����ڸ� ����
    i1 = GetInt1();                  // Lvalue�� ���� ��´�
    i1 = var[EventPC][ i1 ];                // var�迭���� �ش��ϴ� ���� ��´�

    com = GetByte1();                // �����ȣ�� ��´�

	fuck = GetByte1() & 0x000000ff;


    if( fuck == variable_ )    // Rvalue�� ���� �������
    {
        i2 = GetInt1();
        i2 = var[EventPC][ i2 ];
    }
    else                            // Rvalue�� ���� ��ġ���
    {
        i2 = GetInt1();
    }

    switch( com )
    {
        case 0:
            if( i1 < i2 ) check = 1;    break;
        case 1:
            if( i1 <= i2 ) check = 1;   break;
        case 2:
            if( i1 > i2 ) check = 1;    break;
        case 3:
            if( i1 >= i2 ) check = 1;   break;
        case 4:
            if( i1 != i2 ) check = 1;   break;
        case 5:
            if( i1 == i2 ) check = 1;   break;
    }

    if( check == 1 )        // ������ ���̶��
        prog += ( sizeof( int ) + 1 );          // ������ goto���� ��ŵ
}




/* ������������������������������������
 �Լ��� : Cal
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ��Ģ����, %, ������ ��Ʈ������� �����Ѵ�
������������������������������������ */
void Cal( int com )
{
    int i1, i2, i3, result;
    int *p2, *p3;
    BYTE op;

    GetByte1();              // ���� �����ڸ� ����
    i1 = GetInt1();          // Lvalue�� ����

    op = GetByte1() & 0x000000ff;
    i2 = GetInt1();
    if( op == variable_ )   // �������
    {
        p2 = &(var[EventPC][ i2 ]);
    }
    else                    // ��ġ���
    {
        p2 = &i2;
    }


    op = GetByte1() & 0x000000ff;
    i3 = GetInt1();
    if( op == variable_ )   // �������
    {
        p3 = &var[EventPC][ i3 ];
    }
    else                    // ��ġ���
    {
        p3 = &i3;
    }

    switch( com )                        // �� �����ڿ� ���� ����
    {
        case add_:  result = (*p2) + (*p3); break;
        case sub_:  result = (*p2) - (*p3); break;
        case mul_:  result = (*p2) * (*p3); break;
        case div_:  result = (*p2) / (*p3); break;
        case rest_: result = (*p2) % (*p3); break;
        case and_:  result = (*p2) & (*p3); break;
        case or_:   result = (*p2) | (*p3); break;
    }

    var[EventPC][i1] = result;                      // ������ ����� �迭�� ����
}

void Cal2( int com )
{
    int i1, i2;
    int *p2;
    BYTE op;

    GetByte1();
    i1 = GetInt1();                          // Lvalue ���� ����

    op = GetByte1() & 0x000000ff;
    i2 = GetInt1();                          // Rvalue ���� ����
    if( op == variable_ )   // �������
    {
        p2 = &var[EventPC][ i2 ];
    }
    else                    // ��ġ���
    {
        p2 = &i2;
    }

    switch( com )
    {
        case add2_: var[EventPC][ i1 ] += (*p2); break;
        case sub2_: var[EventPC][ i1 ] -= (*p2); break;
        case mul2_: var[EventPC][ i1 ] *= (*p2); break;
        case div2_: var[EventPC][ i1 ] /= (*p2); break;
        case rest2_: var[EventPC][ i1 ] %= (*p2); break;
        case or2_ : var[EventPC][ i1 ] |= (*p2); break;
        case and2_: var[EventPC][ i1 ] &= (*p2); break;
    }
}



/* ������������������������������������
 �Լ��� : Equal
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ������ ���Թ��� ó��
������������������������������������ */
void Equal(void)
{
    int i1, i2;
    BYTE op;

    GetByte1();
    i1 = GetInt1();

    op = GetByte1();
    i2 = GetInt1();
    if( op == variable_ )
	{
        i2 = var[EventPC][i2];
	}
	
    var[EventPC][i1] = i2;
}



/* ������������������������������������
 �Լ��� : IfEqual
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ����ġ���� �ش��ϴ� ������ ó��
������������������������������������ */
void IfEqual( void )
{
    int i1, i2;
    char op;

    GetByte1();
    i1 = GetInt1();                         // Lvalue ���� ����

    op = GetByte1();                        // Rvalue ���� ����
    i2 = GetInt1();

    if( op == value_ )
    {
        if( var[EventPC][i1] == i2 )                   // ���� ������ ���̶�� �б�
            prog = oldprog + *((int *)prog);
        else                                  // �����̸� �������� ����
            prog += sizeof( int );
    }
    else
    {
        if( var[EventPC][i1] == var[EventPC][i2] )                   // ���� ������ ���̶�� �б�
            prog = oldprog + *((int *)prog);
        else                                  // �����̸� �������� ����
            prog += sizeof( int );
    }
}



/* ������������������������������������
 �Լ��� : Script
 ��  �� : ����
 ���ϰ� : ����
 ��  �� : ��ũ��Ʈ �� �����
������������������������������������ */
void Script( int num )
{
  BYTE comm;
	int Fuck;
  if (CLoadScript == SCRIPT_OLD && num == 0)	goto RET_; // �����Ѱ��� �����ʿ�� ������...
  if( num >= TotalFunction )  goto RET_;

  NScriptNo = num;
	CLoadScript = SCRIPT_OLD;

    prog = oldprog + func_off[ num ];
    while( 1 )
    {
        comm = GetByte1();

        switch( comm )
        {
            case goto_:
				Fuck = *(int *)prog;
				//prog += 4;
                prog = oldprog + Fuck; 
				break;

            case if_:
                If(); break;

            case call_:
                CALLSTACK[ CALLSP++ ] = prog - oldprog + sizeof( int );
                prog = oldprog + *((int *)prog);
                break;

            case add_:
            case sub_:
            case mul_:
            case div_:
            case rest_:
            case and_:
            case or_:
                Cal( comm ); break;

            case add2_:
            case sub2_:
            case mul2_:
            case div2_:
            case rest2_:
            case and2_:
            case or2_:
                Cal2( comm ); break;

            case equal_:
                Equal(); break;

            case end_:
                if( --CALLSP < 0 )
                {
                    CALLSP = 0;
                    return;
                }
                prog = oldprog + CALLSTACK[ CALLSP ];
                break;

            case ifequal_:
                IfEqual(); break;

            default:        // ����� ���� �Լ��� ����

				// 010531 KHS
				//011023 >
				if( comm >= 0 && comm <= MAX_SCRIPT_FUNCTION )			// 011213 LTS
				//011023 <
				{
	                (*SFunction[ comm ])();
				}
				else 
				{
#ifdef _M_IX86
					_asm int 3
#else
					__debugbreak();
#endif
					CALLSP = 0;	
					goto RET_;	
				}
        
        if (CLoadScript == SCRIPT_NEW)
        {
          CALLSP = 0;
          goto RET_;
        }
      }
    }

RET_:
	return;
}


/*
// ��ũ��Ʈ �ؽ�Ʈ ������ �о���δ�.
int LoadScriptText( char *scrname )
{
	char temp[FILENAME_MAX];
	char cwd[ FILENAME_MAX];

	getcwd( cwd, FILENAME_MAX );

	_chdir( GameStartDirectory );

	strcpy( CurScriptName, scrname );

	MakeFullPathName( temp, SCRIPT_PATH__, scrname);	// Scriptfile�� CD�� �ƴ� �ϵ忡�� �д°��� �⺻���� �Ѵ�. 


	if( GameMakeFlag == ON )	
	{
		prog = oldprog = (char *)LoadTextScript( temp );
	}
	else 
	{
		strcat( temp, "c");
		LoadScript( temp );
	}

	CLoadScript  =  _NEW_;

	_chdir( cwd );

	return 0;
}
*/



/* ������������������������������������
 �Լ��� : LoadScript
 ��  �� :
 ���ϰ� :
 ��  �� : ���������� �о���̸� 1�� �����Ѵ�.
������������������������������������ */

int LoadScriptCompiled( char *scrname )
{	
    FILE *fp;
    int size, t, i;
	
    fp = fopen( scrname, "rb" );

	if( fp == NULL ) 	{
		MessageBox(0, "LoadScriptCompiled() :: File Not Found", "ERROR", MB_OK);
		exit(0);
	}
	
	if( oldprog != NULL )free( oldprog );
	
    fread( &size, 1, sizeof( int ), fp );           // �� ������ ũ�⸦ ����
    fread( &TotalFunction, 1, sizeof( int ), fp );              // �Լ��� ������ ����
	
    for( i = 0; i < TotalFunction; i++ )
    {
        fread( &t, 1, sizeof( int ), fp );          // �Լ��� �ɼ��� ����
        func_off[ i ] = t;
    }
	
	oldprog = (char *)malloc( size );
    fread( oldprog, size, 1, fp );
	prog = oldprog;
    fclose( fp );

    return( 1 );
}

int LoadScript( char *mapname )
{
    char scriptname[ FILENAME_MAX];
	sprintf( scriptname, "%s/script/%s.zs", GameServerDataPath, mapname );
	
	if( 1 )
	{
		prog = oldprog = (char *)LoadTextScript( scriptname );
		if( prog == NULL ) return 0;
	}
	else 
	{
		strcat( scriptname, "c" );
		LoadScriptCompiled( scriptname );
	}
	
	CLoadScript  =  SCRIPT_NEW;
  return( 1 );
}	
	
	
	
	
	
	
	
	
	
