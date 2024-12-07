/*
		Project:		Calculator
		Module:			CALCMAC.C
		Description:	UI for Mac OS X
		Author:			Martin GŠckler
		Date:			24. 11. 2001

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

#include <Carbon/Carbon.h>
#include "calculat.h"
#include "buttons.h"

static PANELTYPE	lastPanel;
static WindowRef	actPanel = NULL;

static pascal OSStatus CalculatorEventHandler( EventHandlerCallRef handlerRef, EventRef event, void *userData );

static void ShowAbout( void )
{
	WindowRef		aboutWindow;
	IBNibRef 		nibRef;
	OSStatus		err;

	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("calc"), &nibRef);
	require_noerr( err, CantGetNibRef );
	
	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib(nibRef, CFSTR( "About"), &aboutWindow );
	require_noerr( err, CantCreateWindow );
	
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
	
	// The window was created hidden so show it.
	ShowWindow( aboutWindow );

CantCreateWindow:
CantGetNibRef:

	return;
}

static OSStatus CopyToScrap( void )
{
	OSStatus	err;
	ScrapRef	theScrap;
	char		buffer[64];
	short		dummy;
	
	err = GetCurrentScrap( &theScrap );
	if( err == noErr )
		err = ClearCurrentScrap();

	if( err == noErr )
		err = GetCurrentScrap( &theScrap );

	if( err == noErr )
	{
		strcpy( buffer, GetResult(&dummy, &dummy));
		err = PutScrapFlavor( theScrap, kScrapFlavorTypeText, 0, strlen(buffer), buffer );
	}
	return err;
}

static OSStatus CopyFromScrap( void )
{
	char		*cp, buffer[64];
	Size		counter;
	OSStatus	err;
	ScrapRef	theScrap;
	
	err = GetCurrentScrap( &theScrap );

	if( err == noErr )
		err = GetScrapFlavorSize( theScrap, kScrapFlavorTypeText, &counter );
	if( err == noErr && counter > 0 )
	{
		counter = sizeof( buffer );
		err = GetScrapFlavorData( theScrap, kScrapFlavorTypeText, &counter, buffer );
		buffer[counter]=0;

		for( cp = buffer; *cp; cp++ );
		if( *cp == ',' )
			*cp = '.';
			
		SetValue( buffer );
	}

	return err;
}

static OSStatus OpenPanel( PANELTYPE panel )
{
	const char		*name = NULL;
	IBNibRef 		nibRef;
	OSStatus		err;

	MenuRef			theMenu = GetMenuHandle(2);

	EventTypeSpec commSpec[] = 
	{
		{ kEventClassCommand, kEventProcessCommand },
		{ kEventClassKeyboard, kEventRawKeyDown },
	};

	char		buffer[32];
	CFStringRef LastPanelValue;
	CFStringRef lastPanelKey = CFSTR("LastPanel");
	
	switch( panel )
	{
		case LARGE_PANEL:
			name = "LargePanel";
			break;
		case SMALL_PANEL:
			name = "SmallPanel";
			break;
		case COMPUTER_PANEL:
			name = "ComputerPanel";
			break;
		case SCIENCE_PANEL:
			name = "ScientificPanel";
			break;
		default:
			name = NULL;
	}
	if( !name )
		return noErr;
		
	if( actPanel != NULL )
	{
		HideWindow( actPanel );
		DisposeWindow( actPanel );
		actPanel = 0;
		CheckMenuItem( theMenu, (MenuItemIndex)lastPanel, false );
	}
	
	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("calc"), &nibRef);
	require_noerr( err, CantGetNibRef );
	
	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	err = CreateWindowFromNib(nibRef, CFStringCreateWithCString( NULL, name, 0), &actPanel);
	require_noerr( err, CantCreateWindow );
	ChangePanel();
	
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);
	
	// The window was created hidden so show it.
	ShowWindow( actPanel );
	
	InstallWindowEventHandler( actPanel, NewEventHandlerUPP(CalculatorEventHandler), 2, commSpec, NULL, NULL );
	
	CheckMenuItem( theMenu, (MenuItemIndex)panel, true );
	lastPanel = panel;

	{
		sprintf( buffer, "%d", (int)panel );
		LastPanelValue = CFStringCreateWithCString( NULL, buffer, 0 );

		// Set up the preference.
		CFPreferencesSetAppValue(lastPanelKey, LastPanelValue, kCFPreferencesCurrentApplication );
		CFPreferencesAppSynchronize( kCFPreferencesCurrentApplication );
	}

CantCreateWindow:
CantGetNibRef:

	return err;
}

static OSStatus CalculatorKeyboardHandler( EventRef event )
{
	OSStatus	err = eventNotHandledErr;
	char		keyCode;
	int			i;
	static struct
	{
		int		ascii, button, menu;
	} table[] =
	{
		/* the button accelerator borrowed from GEM implementation */
		{ '0',		ZERO,		0 },
		{ '1',		ONE,		0 },
		{ '2',		TWO,		0 },
		{ '3',		THREE,		0 },
		{ '4',		FOUR,		0 },
		{ '5',		FIVE,		0 },
		{ '6',		SIX,		0 },
		{ '7',		SEVEN,		0 },
		{ '8',		EIGHT,		0 },
		{ '9',		NINE,		0 },
		{ 'A',		DIGITA,		0 },
		{ 'B',		DIGITB,		0 },
		{ 'C',		DIGITC,		0 },
		{ 'D',		DIGITD,		0 },
		{ 'E',		DIGITE,		0 },
		{ 'F',		DIGITF,		0 },
		{ '.',		POINT,		0 },
		{ ',',		POINT,		0 },
		{ '\t',		EXPONENT,	0 },
		{ '|',		OROPER,		0 },
		{ '^',		XOROPER,	0 },
		{ '&',		ANDOPER,	0 },
		{ '~',		NOTFUNC,	0 },
		{ '%',		MODULOOP,	0 },
		{ '+',		PLUS,		0 },
		{ '-',		MINUS,		0 },
		{ '*',		MULTIPLY,	0 },
		{ '/',		DIVISION,	0 },
		{ '\r',		EQUAL,		0 },
		{ '\b',		CLEAR,		0 },
		{ '(',		OPNBRACK,	0 },
		{ ')',		CLSBRACK,	0 },
		{ '\033',	ALLCLR,		0 },
		{ '!',		FACULTAT,	0 },
		{ 'N',		NUMBER,		0 },
		{ 'X',		XINPUT,		0 },
		{ 'Y',		YINPUT,		0 },
#if 0
		/* the menu accelerator */
		{ 'C'-'A'+1,	0,		COPY  },
		{ 'V'-'A'+1,	0,		PASTE },
		{ 'Q'-'A'+1,	0,		QUIT  },
		{ 'U'-'A'+1,	0,		QUIT  },
#endif
		{ -1 }
	};

	GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(keyCode), NULL, &keyCode );

	for( i=0; table[i].ascii != -1; i++ )
	{
		if( table[i].ascii == toupper(keyCode) )
		{
			HandleObject( table[i].button );
			err = noErr;
			break;
		}
	}
	return err;
}

static OSStatus CalculatorCommandHandler( EventRef event )
{
	char		tmpBuffer[5];
	int			objectID;
	HICommand	command;
	OSStatus	err = noErr;

	GetEventParameter( event, kEventParamDirectObject, typeHICommand, NULL, sizeof(command), NULL, &command );

	switch( command.commandID )
	{
		case 'cpLP':
			OpenPanel( LARGE_PANEL );
			break;
		case 'cpSP':
			OpenPanel( SMALL_PANEL );
			break;
		case 'cpCP':
			OpenPanel( COMPUTER_PANEL );
			break;
		case 'cpWP':
			OpenPanel( SCIENCE_PANEL );
			break;
		case 'abou':
			ShowAbout();
			break;
		case 'copy':
			CopyToScrap();
			break;
		case 'past':
			CopyFromScrap();
			break;
		default:
			*((long *)tmpBuffer) = command.commandID;
			tmpBuffer[4] = 0;
			objectID = 0;
			sscanf( tmpBuffer, "%d", &objectID );
			if( objectID == 0 || !HandleObject( objectID ) )
				err = eventNotHandledErr;
	}

	return err;
}

/*
	Event Handler called from Mac OS
*/

static pascal OSStatus CalculatorEventHandler( EventHandlerCallRef handlerRef, EventRef event, void *userData )
{
	if( GetEventClass( event ) == kEventClassCommand )
		return CalculatorCommandHandler( event );
	else
		return CalculatorKeyboardHandler( event );
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

	OSStatus		status;
	CFStringRef		text;

	ControlHandle	objHandle;
	ControlID		objID	= { 'CALC', 128 };
	
	objID.id = object;

	status = GetControlByID( actPanel, &objID, &objHandle );
	if( status == noErr )
	{
		text = CFStringCreateWithFormat( NULL, NULL, CFSTR("%s"), string );
		SetControlData( objHandle, 0, kControlEditTextCFStringTag, sizeof(CFStringRef), &text );
		CFRelease( text );
		DrawOneControl( objHandle );
	}
}

void RefreshDispl(	const char *r_mantDisp, const char *r_expoDisp,
					const char *i_mantDisp, const char *i_expoDisp )
{
	setDspString( MANTDSP, r_mantDisp );
	setDspString( EXPODSP, r_expoDisp );
	setDspString( C_MANTDSP, i_mantDisp );
	setDspString( C_EXPODSP, i_expoDisp );
}

void DispOn( short object )
{
	OSStatus		status;

	ControlHandle	objHandle;
	ControlID		objID	= { 'CALC', 128 };
	
	objID.id = object;

	status = GetControlByID( actPanel, &objID, &objHandle );
	if( status == noErr )
	{
		SetControlVisibility( objHandle, true, true );
	}
}

void DispOff( short object )
{
	OSStatus		status;

	ControlHandle	objHandle;
	ControlID		objID	= { 'CALC', 128 };
	
	objID.id = object;

	status = GetControlByID( actPanel, &objID, &objHandle );
	if( status == noErr )
	{
		SetControlVisibility( objHandle, false, true );
	}
}

void EnableButton( short object )
{
	OSStatus		status;

	ControlHandle	objHandle;
	ControlID		objID	= { 'CALC', 128 };
	
	objID.id = object;

	status = GetControlByID( actPanel, &objID, &objHandle );
	if( status == noErr )
	{
		ActivateControl( objHandle );
	}
}

void DisableButton( short object )
{
	OSStatus		status;

	ControlHandle	objHandle;
	ControlID		objID	= { 'CALC', 128 };
	
	objID.id = object;

	status = GetControlByID( actPanel, &objID, &objHandle );
	if( status == noErr )
	{
		DeactivateControl( objHandle );
	}
}

void EnablePanel( PANELTYPE panel )
{
	MenuRef	theMenu = GetMenuHandle(2);
	EnableMenuItem( theMenu, (MenuItemIndex)panel );
}

void DisablePanel( PANELTYPE panel )
{
	MenuRef	theMenu = GetMenuHandle(2);
	DisableMenuItem( theMenu, (MenuItemIndex)panel );
}

/*
	the main entry point
*/

int main(int argc, char* argv[])
{
	IBNibRef 		nibRef;

	OSStatus		err;

	Boolean		isOK;
	PANELTYPE	lastPanel;
	CFStringRef	lastPanelKey = CFSTR("LastPanel");

	// Create a Nib reference passing the name of the nib file (without the .nib extension)
	// CreateNibReference only searches into the application bundle.
	err = CreateNibReference(CFSTR("calc"), &nibRef);
	require_noerr( err, CantGetNibRef );
	
	// Once the nib reference is created, set the menu bar. "MainMenu" is the name of the menu bar
	// object. This name is set in InterfaceBuilder when the nib is created.
	
	err = SetMenuBarFromNib(nibRef, CFSTR("MenuBar"));
	require_noerr( err, CantSetMenuBar );
	
	// We don't need the nib reference anymore.
	DisposeNibReference(nibRef);

	// Then create a window. "MainWindow" is the name of the window object. This name is set in 
	// InterfaceBuilder when the nib is created.
	
	lastPanel = (PANELTYPE)CFPreferencesGetAppIntegerValue( 
		lastPanelKey,  kCFPreferencesCurrentApplication, &isOK 
	);
	if( isOK )
		OpenPanel( lastPanel );
	else
		OpenPanel( LARGE_PANEL );
	
#if 1
	/* Register help book */
	{
		CFBundleRef mainBundle = CFBundleGetMainBundle();
		if (mainBundle)
		{
			CFURLRef bundleURL = NULL;
			CFRetain(mainBundle);
			bundleURL = CFBundleCopyBundleURL(mainBundle);
			if (bundleURL)
			{
					FSRef bundleFSRef;
					if (CFURLGetFSRef(bundleURL, &bundleFSRef))
						AHRegisterHelpBook(&bundleFSRef);
					CFRelease(bundleURL);
			}
			CFRelease(mainBundle);
		}
	}
#endif
					
	// Call the event loop
	RunApplicationEventLoop();
	
CantSetMenuBar:
CantGetNibRef:

	return err;
}