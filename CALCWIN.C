/*
		Project:		Calculator
		Module:			CALCWIN.C
		Description:	UI for MS-Windows
		Author:			Martin Gäckler
		Date:			21. 7. 1993

		Address:		Hofmannsthalweg 14, A-4030 Linz
		Web:			https://www.gaeckler.at/

		Copyright:		(c) 1988-2024 Martin Gäckler

		This program is free software: you can redistribute it and/or modify  
		it under the terms of the GNU General Public License as published by  
		the Free Software Foundation, version 3.

		You should have received a copy of the GNU General Public License 
		along with this program. If not, see <http://www.gnu.org/licenses/>.

		THIS SOFTWARE IS PROVIDED BY Martin Gäckler, Austria, Linz ``AS IS''
		AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
		TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
		PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR
		CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
		SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
		LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
		USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
		ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
		OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
		OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
		SUCH DAMAGE.
*/

/* --------------------------------------------------------------------- */
/* ----- includes ------------------------------------------------------ */
/* --------------------------------------------------------------------- */

#include <stdlib.h>
#include <string.h>

#define STRICT

#include <windows.h>

#undef ABSOLUTE

#include "calculat.h"
#include "buttons.h"
#include "menu.h"

#define APP_NAME	"WinCalc"
#define START_PANEL	"StartPanel"

#ifdef _MSC_VER
#	pragma warning( disable: 4996 )
#endif

typedef struct
{
	char	*name;
	int		menuId;
} PANEL;

typedef struct
{
	int		callerType;
	union
	{
		HANDLE	instance;
		HWND	window;
	} caller;
	int		type,
			radix;
	char	value[64];
} CALCMSG;

/* --------------------------------------------------------------------- */
/* ----- module statics ------------------------------------------------ */
/* --------------------------------------------------------------------- */

static HWND		calc;

static HICON		icon;
static HINSTANCE	calcInstance;
static HANDLE		calledMsg;
static HMENU		menuHandle;
static PANEL		*last, panels[] =
{
	{ NULL,               0        },
	{ "LARGE_PANEL",      LARGEPAN },
	{ "SMALL_PANEL",      SMALLPAN },
	{ "COMPUTER_PANEL",   COMPPAN  },
	{ "SCIENTIFIC_PANEL", SNCEPAN  },
};

/* --------------------------------------------------------------------- */
/* --------------------------------------------------------------------- */

int		maxMant,	maxExpo;
char	*mantDisp,	*expoDisp;


BOOL CALLBACK CalculatorFunction(
	HWND box, UINT message,
	WPARAM wParam, LPARAM lParam 
);


/* --------------------------------------------------------------------- */
/* ----- entry points test-------------------------------------------------- */
/* --------------------------------------------------------------------- */

static void savePanelPos( void )
{
		char	panelPos[128], value[32], *posPos;
		RECT	pos;
		int		x, y;

		/* Get the position */
		GetWindowRect( calc, &pos );
		x   = pos.left;
		y   = pos.top;

		/* initiate the profile entry */
		strcpy( panelPos, last->name );
		posPos = panelPos + strlen( panelPos );

		/* Write the X-Pos */
		strcpy( posPos, "_X" );
		itoa( x, value, 10 );
		WriteProfileString( APP_NAME, panelPos, value );

		/* Write the Y-Pos */
		strcpy( posPos, "_Y" );
		itoa( y, value, 10 );
		WriteProfileString( APP_NAME, panelPos, value );
}

static void openPanel( PANEL *panel, int iconShow )
{
		char	panelPos[128], *posPos;
		int		x, y;

	if( last == panel )
		return;

	if( calc )
	{
		savePanelPos();

		/* uncheck the menu item */
		CheckMenuItem( menuHandle, last->menuId, MF_UNCHECKED|MF_BYCOMMAND );

		/* kill the old window */
		DestroyWindow( calc );
	}

	calc = CreateDialog( calcInstance, panel->name, (HWND)NULL, CalculatorFunction );
	if( calc )
	{
		/* initiate the profile entry */
		strcpy( panelPos, panel->name );
		posPos = panelPos + strlen( panelPos );

		/* read the x pos */
		strcpy( posPos, "_X" );
		x = GetProfileInt( APP_NAME, panelPos, -1 );

		/* read the y pos */
		strcpy( posPos, "_Y" );
		y = GetProfileInt( APP_NAME, panelPos, -1 );

		/* Now set the position */
		if( x >= 0 && y >= 0 )
			SetWindowPos( calc, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE );

		/* check the menu item */
		menuHandle = GetMenu( calc );
		CheckMenuItem( menuHandle, panel->menuId, MF_CHECKED|MF_BYCOMMAND );

#if defined( __WIN32__ )
		SetClassLong( calc, GCL_HICON, (LONG)icon );
#elif defined( __WIN64__ )
		SetClassLongPtr( calc, GCLP_HICON, (LONG_PTR)icon );
#else
	#error "What Windows"
#endif

		ShowWindow( calc, iconShow );

		WriteProfileString( APP_NAME, START_PANEL, panel->name );
		last = panel;
	}
}

static void openNamedWindow( const char *name, int iconShow )
{
	PANELTYPE	i;
	PANEL		*curPanel;

	curPanel = panels+1;
	forAllPanels( i )
	{
		if( !strcmp( name, curPanel->name ) )
		{
			openPanel( curPanel, iconShow );
			break;
		}
		curPanel++;
	}
}

static void openMenuWindow( int menuId )
{
	PANELTYPE	i;
	PANEL		*curPanel;

	curPanel = panels+1;
	forAllPanels( i )
	{
		if( curPanel->menuId == menuId )
		{
			openPanel( curPanel, SW_SHOWNORMAL );
			break;
		}
		curPanel++;
	}
}

static void copyToClipboard( void )
{
	HANDLE	memHandle;
	LPSTR	ptr;
	short	dummy;
	char	buffer[64];
	char	decimal;    

	if( OpenClipboard( calc ) )
	{
		memHandle = GlobalAlloc( GMEM_MOVEABLE, 128 );
		if( memHandle )
		{
			ptr = GlobalLock( memHandle );
			if( ptr )
			{
				EmptyClipboard();

				lstrcpy( ptr, GetResult( &dummy, &dummy ) );

				GetProfileString( "intl", "sDecimal",
								  ".", buffer, sizeof(buffer) );
				decimal = *buffer;
				if( decimal != '.' )
                {
					while( *ptr && *ptr != '.' )
						ptr++;
					if( *ptr == '.' )
						*ptr = decimal;
                }

				GlobalUnlock( memHandle );
				SetClipboardData( CF_TEXT, memHandle );
			}
			else
				GlobalFree( memHandle );
		}
		CloseClipboard();
	}
}


static void copyFromClipboard( void )
{
	HANDLE	clipHandle;
	LPSTR	ptr;
	char	value[128], *cp;
	char	buffer[64];
	char	decimal;    

	if( OpenClipboard( calc ) )
	{
		clipHandle = GetClipboardData( CF_TEXT );
		if( clipHandle )
		{
			ptr = GlobalLock( clipHandle );
			if( ptr )
			{
				strncpy( value, ptr, sizeof( value ) -1 );
				value[sizeof( value ) -1] = 0;

				GetProfileString( "intl", "sDecimal",
								  ".", buffer, sizeof(buffer) );
				decimal = *buffer;
				if( decimal != '.' )
				{
					for( cp = value; *cp && *cp != decimal; cp++ );
					if( *cp == decimal )
						*cp = '.';
                }

				SetValue( value );
				GlobalUnlock( clipHandle );
			}
		}
		CloseClipboard();
	}
}

static int handleMenu( int item )
{
	switch( item )
	{
		/* The Edit Popup */
		/* -------------- */
		case COPY:
			copyToClipboard();
			break;
		case PASTE:
			copyFromClipboard();
			break;

		case QUIT:
			savePanelPos();
			DestroyWindow( calc );
			PostQuitMessage( 0 );
			break;

		/* The Panel Popup */
		/* --------------- */
		case LARGEPAN:
		case SNCEPAN:
		case COMPPAN:
		case SMALLPAN:
			openMenuWindow( item );
			break;

		/* The INFO Popup */
		/* -------------- */
		case MENU_CONTENTS:
			WinHelp( calc, "CALC.HLP", HELP_CONTENTS, 0 );
			break;
		case MENU_HELP:
		case PANEL_HELP:
		case BUTTON_HELP:
		case KEYS_HELP:
			WinHelp( calc, "CALC.HLP", HELP_CONTEXT, item );
			break;

		case ABOUT:
			MessageBox( calc, "Calculator\n"
							 "-----------------\n"
							 "\n"
							 "Version 1.6 for Windows\n"
							 "© 1991-2024 by Martin Gäckler\n"
							 "https://www.gaeckler.at\n",
							 "About",
				MB_ICONINFORMATION|MB_APPLMODAL|MB_OK );
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

static void calcMessage( HANDLE msgBuff )
{
	CALCMSG		*calcMsg;
	char		value[128];

	calcMsg = (CALCMSG *)GlobalLock( msgBuff );
	if( calcMsg )
	{
		strcpy( value, calcMsg->value );
		RestartCalc( value, calcMsg->type, calcMsg->radix );
		calledMsg = msgBuff;
		ShowWindow( calc, SW_SHOWNORMAL );
		GlobalUnlock( msgBuff );
	}
}

static void queryMessage( HANDLE msgBuff )
{
	struct
	{
		int	type;
		union
		{
			HANDLE	instance;
			HWND	window;
		} id;
	} caller;

	CALCMSG			*calcMsg;
	char			*value;
	short			type, radix;

	calcMsg = (CALCMSG *)GlobalLock( msgBuff );
	if( calcMsg )
	{
		value = GetResult( &type, &radix );

		strcpy( calcMsg->value, value );
		calcMsg->type  = type;
		calcMsg->radix = radix;

		caller.type = calcMsg->callerType;
		if( caller.type > 0 )
		{
			caller.id.window = calcMsg->caller.window;
		}
		else
		{
			caller.id.instance = calcMsg->caller.instance;
		}

		GlobalUnlock( msgBuff );

		if( caller.type > 0 )
			SendMessage( caller.id.window, WM_USER+2, 0, (DWORD)msgBuff );
		else
			PostAppMessage(caller.id.instance, WM_USER+2, 0, (DWORD)msgBuff);
	}
}

static void replyMessage( void )
{
	if( calledMsg )
	{
		queryMessage( calledMsg );
		calledMsg = NULL;
	}
}

static HANDLE initWindow( HINSTANCE hInstance, int iconShow )
{
	HANDLE	accel;
	char	startPanel[32];

	accel = LoadAccelerators( hInstance, "ACCEL" );
	if( !accel )
	{
		MessageBox( NULL, "Can't read Accelerator", "Error",
			MB_APPLMODAL|MB_ICONSTOP|MB_OK );
		return NULL;
	}

	icon = LoadIcon( hInstance, "CALC_ICON" );
	if( !icon )
	{
		MessageBox( NULL, "Can't read Icon \"CALC_ICON\"", "Error",
			MB_APPLMODAL|MB_ICONSTOP|MB_OK );
		return NULL;
	}

	calcInstance = hInstance;

	GetProfileString( APP_NAME, START_PANEL, "LARGE_PANEL",
						startPanel, 32 );
	openNamedWindow( startPanel, iconShow );

	return accel;
}

static void messageLoop( HACCEL accel )
{
	MSG		msg;

	while( GetMessage( &msg, 0,0,0 ) )
	{
		if( !msg.hwnd )
		{
			SendMessage( calc, msg.message, msg.wParam, msg.lParam );
		}
		else if( TranslateAccelerator( calc, accel, &msg ) )
			;
		else
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
	}
}

/* --------------------------------------------------------------------- */
/* ----- entry points -------------------------------------------------- */
/* --------------------------------------------------------------------- */

/*
	called from calculator module
	*****************************
*/

void setDspString( short object, const char *string )
{
	if( GetDlgItem( calc, object ) )
		SetDlgItemText( calc, object, (char *)string );
}

void RefreshDispl(	const char *r_mantDisp, const char *r_expoDisp,
					const char *i_mantDisp, const char *i_expoDisp )
{
	SetDlgItemText( calc, MANTDSP, r_mantDisp );
	SetDlgItemText( calc, EXPODSP, r_expoDisp );
	SetDlgItemText( calc, C_MANTDSP, i_mantDisp );
	SetDlgItemText( calc, C_EXPODSP, i_expoDisp );
}

void DispOn( short object )
{
	HWND	dlgWin;

	dlgWin = GetDlgItem( calc, object );
	if( dlgWin )
		ShowWindow( dlgWin, SW_SHOW );
}

void DispOff( short object )
{
	HWND	dlgWin;

	dlgWin = GetDlgItem( calc, object );
	if( dlgWin )
		ShowWindow( dlgWin, SW_HIDE );
}

void EnableButton( short object )
{
	HWND	control;

	if( (control = GetDlgItem( calc, object )) != NULL )
		EnableWindow( control, 1 );
}

void DisableButton( short object )
{
	HWND	control;

	if( (control = GetDlgItem( calc, object )) != NULL )
		EnableWindow( control, 0 );
}

void EnablePanel( PANELTYPE panel )
{
	EnableMenuItem(menuHandle, panels[panel].menuId, MF_ENABLED|MF_BYCOMMAND);
}

void DisablePanel( PANELTYPE panel )
{
	EnableMenuItem(menuHandle, panels[panel].menuId, MF_GRAYED|MF_BYCOMMAND);
}

/*
	called from Windows
	*******************
*/


BOOL CALLBACK CalculatorFunction( 
	HWND box, UINT message,
	WPARAM wParam, LPARAM lParam 
)
{
	int		retVal = TRUE;
	HWND	button;

	EnableMenuItem( menuHandle, PASTE,
		IsClipboardFormatAvailable( CF_TEXT ) ? MF_ENABLED | MF_BYCOMMAND
											  : MF_GRAYED  | MF_BYCOMMAND
	);

	switch( message )
	{

	case WM_INITDIALOG:
		calc = box;
		SetDlgItemText( box, MEDIUM,   "\xF8"  );
		SetDlgItemText( box, DIVISION, "\xF7"  );
		SetDlgItemText( box, ANGLE,    "° \' \"" );
		ChangePanel();
		break;

	case WM_COMMAND:
		wParam = LOWORD( wParam );

		if( wParam > 255 )
			retVal = handleMenu( (int)wParam );
		else
		{
			button = GetDlgItem( calc, (int)wParam );
			if( button && IsWindowEnabled( button ) )
			{
				HandleObject( (short)wParam );
			}
		}
		break;

	case WM_CLOSE:
		savePanelPos();
		DestroyWindow( box );
		PostQuitMessage( 0 );
		break;

	case WM_KILLFOCUS:
		replyMessage();
		break;

	case WM_USER:
		calcMessage( (HANDLE)lParam );
		break;

	case WM_USER+1:
		queryMessage( (HANDLE)lParam );
		break;

	case WM_QUERYENDSESSION:
		savePanelPos();

	default:
		retVal = FALSE;

	}

	return retVal;
}


/*
	**** main *************************************************************
*/
#ifdef __BORLANDC__
#pragma argsused
#endif

int PASCAL WinMain( 
	HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int iconShow 
)
{
	HANDLE	accel;

	accel = initWindow( hInstance, iconShow );

	if( calc )
		messageLoop( accel );

	return 0;
}