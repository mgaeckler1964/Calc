/*
		Project:		Calculator
		Module:			CALCGEM.C
		Description:	UI for atari ST
		Author:			Martin G„ckler
		Date:			15. 1. 1992

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
/* ----- switches ------------------------------------------------------ */
/* --------------------------------------------------------------------- */

#ifdef __TOS__
#define INTERNAL_RSC 0
#else
#define INTERNAL_RSC 1
#endif

/* --------------------------------------------------------------------- */
/* ----- includes ------------------------------------------------------ */
/* --------------------------------------------------------------------- */

#include <limits.h>
#include <string.h>
#include <ctype.h>

#if INTERNAL_RSC
#	include <portab.h>
#endif

#include <aes.h>
#include <tos.h>

#ifdef __TOS__
#include <screen.h>
#else
#define Bell() /* nixis */
#endif

#include <gak\gaklib.h>

#undef SMALL

#include "buttons.h"

#include "calc.h"
#include "calculat.h"

#if INTERNAL_RSC
#	pragma warn -rpt
#	include "calc.rsh"
#	pragma warn .rpt
#endif

#ifdef __MSDOS__
#pragma  warn -ccc
#pragma  warn -rch
#endif
/* --------------------------------------------------------------------- */
/* ----- constants ----------------------------------------------------- */
/* --------------------------------------------------------------------- */

#define RSC_FILE	"CALC.RSC"
#define CALC_WIND	NAME|CLOSER|MOVER
#define EVENTS		MU_KEYBD|MU_BUTTON|MU_MESAG

#define NUM_PANELS	5			/* incl. the virtual Button				*/

#define P_VERSION	0x0100		/* version number of profile - must be	*/
								/* changed if parts of this structure	*/
								/* are changeed without effect to the	*/
								/* size!!!!								*/

/* --------------------------------------------------------------------- */
/* ----- macros -------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#define getVirtualButton( button ) \
	(translation[findLine( curPanel, (button) )][0])

#define translateButton( button, src, dest ) \
	(translation[findLine( (src), (button) )][(dest)])

/* --------------------------------------------------------------------- */
/* ----- type definitions ---------------------------------------------- */
/* --------------------------------------------------------------------- */

typedef struct
{
	OBJECT	*tree;
	int		menu,
			resId;
	bool	opened;
} PANELINFO;

typedef struct
{
	int			version;
#ifdef __TOS__
	int			resolution;
#endif

	PANELTYPE	curPanel;
	struct
	{
		int	x, y;
	} position[NUM_PANELS];
} PROFILE;

/* --------------------------------------------------------------------- */
/* ----- prototypes ---------------------------------------------------- */
/* --------------------------------------------------------------------- */

static bool handleKeybd( int );
static void invalidatePopUp( void );

/* --------------------------------------------------------------------- */
/* ----- module statics ------------------------------------------------ */
/* --------------------------------------------------------------------- */

static PROFILE		profile;
static char			profileName[128];
static OBJECT		*calculator, *menu;
static int			windId=-1;
static bool			visible=FALSE;
static int			popUp, curTitle, curEntry;
static int			aesId, caller = -1;
static PANELTYPE	curPanel=NO_PANEL;
static GRECT		popUpRect, entryRect;

static PANELINFO panInfos[NUM_PANELS] =
{
	{ NULL, -1,       -1,		FALSE },	/* the virtual    panel		*/
	{ NULL, LARGEPAN, LARGE,	FALSE },	/* the large      panel		*/
	{ NULL, SMALLPAN, SMALL,	FALSE },	/* the small      panel		*/
	{ NULL, COMPPAN,  COMPUTER,	FALSE },	/* the computer   panel		*/
	{ NULL, SNCEPAN,  SCIENCE,	FALSE }		/* the scientific panel		*/
};

static int		translation[][NUM_PANELS] =
{
/*    virtual   large     small     computer  scientific	*/
/*  ------------------------------------------------------	*/
	{ DISPLAY,  LDISPLAY, SDISPLAY, CDISPLAY, EDISPLAY },
	{ INVDSP,   LINVDSP,  -1,       CINVDSP,  EINVDSP  },
	{ HYPDSP,   LHYPDSP,  -1,       -1,       EHYPDSP  },
	{ MEMDSP,   LMEMDSP,  SMEMDSP,  CMEMDSP,  EMEMDSP  },
	{ STATDSP,  LSDSP,    -1,       -1,       ESDSP    },
	{ CNSTDSP,  LCONST,   SCONST,   CCONST,   ECONST   },
	{ TRGDISP,  LTRGDSP,  -1,       -1,       ETRGDSP  },
	{ RADIXDSP, LRADXDSP, -1,       CRADXDSP, -1       },
	{ TYPEDISP, LTYPEDSP, -1,       CTYPEDSP, -1       },
	{ MANTDSP,  LMANTDSP, SMANTDSP, CMANTDSP, EMANTDSP },
	{ EXPODSP,  LEXPODSP, SEXPODSP, CEXPODSP, EEXPODSP },
	{ REGDSP,   -1,       -1,       -1,       EREGDSP  },

	{ ZERO,     LZERO,    SZERO,    CZERO,    EZERO    },
	{ ONE,      LONE,     SONE,     CONE,     EONE     },
	{ TWO,      LTWO,     STWO,     CTWO,     ETWO     },
	{ THREE,    LTHREE,   STHREE,   CTHREE,   ETHREE   },
	{ FOUR,     LFOUR,    SFOUR,    CFOUR,    EFOUR    },
	{ FIVE,     LFIVE,    SFIVE,    CFIVE,    EFIVE    },
	{ SIX,      LSIX,     SSIX,     CSIX,     ESIX     },
	{ SEVEN,    LSEVEN,   SSEVEN,   CSEVEN,   ESEVEN   },
	{ EIGHT,    LEIGHT,   SEIGHT,   CEIGHT,   EEIGHT   },
	{ NINE,     LNINE,    SNINE,    CNINE,    ENINE    },
	{ DIGITA,   LDIGITA,  -1,       CDIGITA,  -1       },
	{ DIGITB,   LDIGITB,  -1,       CDIGITB,  -1       },
	{ DIGITC,   LDIGITC,  -1,       CDIGITC,  -1       },
	{ DIGITD,   LDIGITD,  -1,       CDIGITD,  -1       },
	{ DIGITE,   LDIGITE,  -1,       CDIGITE,  -1       },
	{ DIGITF,   LDIGITF,  -1,       CDIGITF,  -1       },
	{ POINT,    LPOINT,   SPOINT,   CPOINT,   EPOINT   },
	{ EXPONENT, LEXPO,    SEXPO,    CEXPO,    EEXPO    },
	{ SIGN,     LSIGN,    SSIGN,    CSIGN,    ESIGN    },

	{ ALLCLR,   LALLCLR,  SALLCLR,  CALLCLR,  EALLCLR  },
	{ CLEAR,    LCLEAR,   SCLEAR,   CCLEAR,   ECLEAR   },

	{ OPNBRACK, LOPENBRK, SOPENBRK, COPENBRK, EOPENBRK },
	{ CLSBRACK, LCLOSBRK, SCLOSBRK, CCLOSBRK, ECLOSBRK },
	{ SWAPXY,   LSWAP,    SSWAP,    CSWAP,    ESWAP    },
	{ SWAPXM,   LSWAPM,   SSWAPM,   CSWAPM,   ESWAPM   },

	{ MINP,     LMINP,    SMINP,    CMINP,    EMINP    },
	{ MR,       LMR,      SMR,      CMR,      EMR      },
	{ MPLUS,    LMPLUS,   SMPLUS,   CMPLUS,   EMPLUS   },
	{ MMINUS,   LMMINUS,  SMMINUS,  CMMINUS,  EMMINUS  },

	{ INVERS,   LINVERS,  -1,       CINVERS,  EINVERS  },
	{ HYPERBEL, LHYPER,   -1,       -1,       EHYPER   },

	{ EQUAL,    LEQUAL,   SEQUAL,   CEQUAL,   EEQUAL   },
	{ PLUS,     LPLUS,    SPLUS,    CPLUS,    EPLUS    },
	{ MINUS,    LMINUS,   SMINUS,   CMINUS,   EMINUS   },
	{ MULTIPLY, LMULTY,   SMULTY,   CMULTY,   EMULTY   },
	{ DIVISION, LDIVISIO, SDIVISIO, CDIVISIO, EDIVISIO },
	{ POWER,    LPOWER,   SPOWER,   CPOWER,   EPOWER   },
	{ ROOTOPER, LROOT,    SROOT,    CROOT,    EROOT    },
	{ MODULOOP, LMOD,     -1,       CMOD,     -1       },
	{ ANDOPER,  LAND,     -1,       CAND,     -1       },
	{ XOROPER,  LXOR,     -1,       CXOR,     -1       },
	{ OROPER,   LOR,      -1,       COR,      -1       },
	{ BINOM,    -1,       -1,       -1,       EBINOM   },

	{ SQRROOT,  LSQR,     -1,       -1,       ESQR     },
	{ REZIPROK, LREZI,    -1,       -1,       EREZI    },
	{ FACULTAT, LFAC,     -1,       -1,       EFAC     },

	{ FIXPOINT, LFIX,     SFIX,     CFIX,     EFIX     },
	{ ROUND,    LRND,     SRND,     CRND,     ERND     },
	{ ENGINEER, LENG,     -1,       -1,       EENG     },
	{ ANGLE,    LANGLE,   -1,       -1,       EANGLE   },

	{ LINEREG,  -1,       -1,       -1,       ELINREG  },
	{ LNREG,    -1,       -1,       -1,       ELNREG   },
	{ EXPOREG,  -1,       -1,       -1,       EEXPREG  },
	{ POWEREG,  -1,       -1,       -1,       EPOWREG  },
	{ BESTMTCH, -1,       -1,       -1,       EBSTMTCH },
	{ RANDOM,   LRAN,     -1,       -1,       ERAN     },
	{ XINPUT,   LXINP,    -1,       -1,       EXINP    },
	{ SUMX,     LSUMX,    -1,       -1,       ESUMX    },
	{ DNMIN1,   LDNMIN1,  -1,       -1,       EDNMIN1  },
	{ SUMX2,    LSUMSQR,  -1,       -1,       ESUMX2   },
	{ DN,       LDN,      -1,       -1,       EDN      },
	{ NUMBER,   LNUM,     -1,       -1,       ENUM     },
	{ MEDIUM,   LMED,     -1,       -1,       EMED     },
	{ YINPUT,   -1,       -1,       -1,       EYINP    },
	{ SUMY,     -1,       -1,       -1,       ESUMY    },
	{ SUMXY,    -1,       -1,       -1,       ESUMXY   },
	{ SUMY2,    -1,       -1,       -1,       ESUMY2   },
	{ CONSTVAL, -1,       -1,       -1,       ECNSTVAL },
	{ REGKOEF,  -1,       -1,       -1,       EREGKOEF },
	{ KORKOEF,  -1,       -1,       -1,       EKORKOEF },
	{ XOUT,     -1,       -1,       -1,       EXOUT    },
	{ YOUT,     -1,       -1,       -1,       EYOUT    },

	{ DECLOG,   LLOG,     -1,       -1,       ELOG     },
	{ NATLOG,   LLN,      -1,       -1,       ELN      },

	{ SINUS,    LSIN,     -1,       -1,       ESIN     },
	{ COSINUS,  LCOS,     -1,       -1,       ECOS     },
	{ TANGENS,  LTAN,     -1,       -1,       ETAN     },

	{ ABSOLUTE, LABS,     -1,       CABS,      EABS    },
	{ NOTFUNC,  LNOT,     -1,       CNOT,      -1      },
	{ FRAC,     LFRC,     -1,       CFRC,      EFRC    },
	{ INTEGER,  LINT,     -1,       CINT,      EINT    },

	{ DEGREE,   LDEG,     -1,       -1,        EDEG    },
	{ RADIAL,   LRAD,     -1,       -1,        ERAD    },
	{ GRADIAL,  LGRA,     -1,       -1,        EGRA    },

	{ FLOATTYP, LFLOAT,   -1,       CFLOAT,   -1       },
	{ BYTETYPE, LBYTE,    -1,       CBYTE,    -1       },
	{ WORDTYPE, LWORD,    -1,       CWORD,    -1       },
	{ LONGTYPE, LLONG,    -1,       CLONG,    -1       },

	{ DUALSYS,  LDUAL,    -1,       CDUAL,    -1       },
	{ OKTALSYS, LOCT,     -1,       COCT,     -1       },
	{ DEZIMAL,  LDEC,     -1,       CDEC,     -1       },
	{ HEXSYS,   LHEX,     -1,       CHEX,     -1       },

	{ -1,       -1,       -1,       -1,       -1       }
};

/* --------------------------------------------------------------------- */
/* ----- module functions ---------------------------------------------- */
/* --------------------------------------------------------------------- */

/*
	find the line from a panel button in the translation table
*/
static int findLine( PANELTYPE panel, int obj )
{
	int	i;

	for( i=0; translation[i][0] >= 0; i++ )
		if( translation[i][panel] == obj )
			break;

	return i;
}

static void initObjectTree( void )
{
	PANELTYPE i;
	
	forAllPanels( i )
	{
		rsrc_gaddr( ROOT, panInfos[i].resId, &panInfos[i].tree );
	}
		
	rsrc_gaddr( ROOT, GEMMENU, &menu );
		
	if( _app )
		menu_bar( menu, 1 );
	else
	{
		objc_delete( menu, DESK2 );
		objc_delete( menu, DESK3 );
		objc_delete( menu, DESK4 );
		objc_delete( menu, DESK5 );
		objc_delete( menu, DESK6 );
		menu[CALCPOP].ob_height = menu[DESK1].ob_y + menu[DESK1].ob_height;
		menu[DESK1].ob_spec.free_string = "  No Accessories ";
		menu[DESK1].ob_state = DISABLED;
	}
	
	DispOff( INVDSP );
	DispOff( HYPDSP );
	DispOff( MEMDSP );
	DispOff( STATDSP );
	DispOff( CNSTDSP );
}

static void sendReply( int caller )
{
	int	aesBuffer[8];

	aesBuffer[0] = 1026;
	aesBuffer[1] = aesId;
	aesBuffer[2] = 0;
	*(char **)(aesBuffer+3) =
				GetResult( (short *)aesBuffer + 5, (short *)aesBuffer + 6);
	appl_write( caller, 16, aesBuffer );
}

/*
	Profile functions
*/
static void writeProfile( PANELTYPE newPanel )
{
	long	errCode;
	int		fd, offset;

	if( newPanel > VIRTUAL_PANEL )
		profile.curPanel = newPanel;
	else if( curPanel > VIRTUAL_PANEL )
		profile.curPanel = curPanel;

	if( curPanel > VIRTUAL_PANEL )
	{
		offset = _app ? 0 : menu[1].ob_height;
		
		profile.position[curPanel].x = panInfos[curPanel].tree->ob_x;
		profile.position[curPanel].y = panInfos[curPanel].tree->ob_y
			- offset;

		errCode = Fcreate( profileName, 0 );
		if( errCode >= 0 )
		{
			fd = (int)errCode;

			profile.version    = P_VERSION;
#ifdef __TOS__
			profile.resolution = Getrez();
#endif
			Fwrite( fd, sizeof(PROFILE), &profile );
			Fclose( fd );
		}
	}
}

static void readProfile( void )
{
	long		errCode;
	int			fd;
	PROFILE		tmpProfile;
	char		buffer[128];

	strcpy( buffer, "CALC.INF" );
	if( !shel_find( buffer ) )
		strcpy( profileName, "C:\\CALC.INF" );
	else
		fullpath( profileName, buffer );
	
	errCode = Fopen( profileName, 0 );
	if( errCode >= 0 )
	{
		fd = (int)errCode;
		if( Fread(fd, sizeof(PROFILE), &tmpProfile ) == sizeof(PROFILE) 
		&&  tmpProfile.version    == P_VERSION 
#ifdef __TOS__
		&&  tmpProfile.resolution == Getrez()
#endif
		 )
			profile = tmpProfile;
		
		Fclose( fd );
	}
}
		
	
/*
	Clipboard functions
*/
static void copyToClipboard( void )
{
	char	*display;
	short	dummy;
	CLIPHD	clipHd;

	display = GetResult( &dummy, &dummy );
	clipHd = createClipboard( CF_TEXT );
	if( clipHd )
	{
		writeClipboard( clipHd, strlen( display ), display );
		closeClipboard( clipHd );
	}
}

static void copyFromClipboard( void )
{
	char	display[128];
	CLIPHD	clipHd;

	clipHd = openClipboard( CF_TEXT );
	if( clipHd )
	{
		readClipboard( clipHd, sizeof( display ), display );
		SetValue( display );
		closeClipboard( clipHd );
	}
}	

static void checkClipboard( void )
{
	int oldState, newState;
	
	oldState = menu[PASTE].ob_state;
	if( isClipAvail( CF_TEXT ) )
		newState = oldState & ~DISABLED;
	else
		newState = oldState | DISABLED;
	menu[PASTE].ob_state = newState;
}

/*
	panel functions
	---------------
*/
static int openPanel( PANELTYPE panel )
{
	int		x, y, w, h, err=0, offset;

	if( panel <= VIRTUAL_PANEL )
	{
		panel = curPanel;
		if( panel <= VIRTUAL_PANEL )
		{
			panel = profile.curPanel;
			if( panel <= VIRTUAL_PANEL )
				panel = LARGE_PANEL;
		}
	}

	if( panel != curPanel )
	{
		writeProfile( panel );

		calculator = panInfos[panel].tree;

		if( !profile.position[panel].y )  /* x may be 0 but y is never 0 */
		{
			form_center( calculator, &x, &y, &w, &h );
		}
		else
		{
			offset = _app ? 0 : menu[1].ob_height;

			calculator->ob_x = profile.position[panel].x;
			calculator->ob_y = profile.position[panel].y + offset;
		}

		menu_icheck( menu, panInfos[panel].menu, 1 );

		if( curPanel > VIRTUAL_PANEL )
			menu_icheck( menu, panInfos[curPanel].menu, 0 );

		if( !_app )
		{
			menu->ob_width = menu[1].ob_width = calculator->ob_width;
			menu->ob_height = calculator->ob_height + menu[1].ob_height;
			menu->ob_x = calculator->ob_x;
			menu->ob_y = calculator->ob_y - menu[1].ob_height;
		}
	}

	if( windId < 0 || panel != curPanel )
	{
		if( !_app )
			wind_calc( WC_BORDER, CALC_WIND,
				menu->ob_x, menu->ob_y,
				calculator->ob_width,
				calculator->ob_height + menu[1].ob_height,
				&x, &y, &w, &h );
		else
			wind_calc( WC_BORDER, CALC_WIND,
				calculator->ob_x, calculator->ob_y,
				calculator->ob_width, calculator->ob_height,
				&x, &y, &w, &h );

		if( windId < 0 )
		{
			windId = wind_create( CALC_WIND, x, y, w, h );
			if( windId < 0 )
			{
				form_alert( 1, "[3][|No more windows][ Ok ]" );
				err = -1;
			}
			else
			{
#ifdef __TOS__
				wind_set( windId, WF_NAME, " Calculator " );
#else
				char far *name = " Calculator ";
				wind_set( windId, WF_NAME, FP_OFF( name ), FP_SEG( name ),
								  0, 0 );
#endif
				wind_open( windId, x, y, w, h );
			}
		}
		else
		{
			wind_set( windId, WF_CURRXYWH, x, y, w, h );
			form_dial( FMD_START, 0,0,0,0, x, y, w, h );
			form_dial( FMD_FINISH, 0,0,0,0, x, y, w, h );
		}
		visible = FALSE;
		ChangePanel();
	}
	else
		wind_set( windId, WF_TOP, 0, 0, 0, 0 );

	curPanel = panel;
	return err;
}

static void redrawPanel( int aesHandle, GRECT *area )
{
	GRECT clip;
	
	if( aesHandle == windId )
	{
		wind_update( BEG_UPDATE );
	
		wind_get( aesHandle, WF_FIRSTXYWH, 
			&clip.g_x, &clip.g_y, &clip.g_w, &clip.g_h );
		while( clip.g_w && clip.g_h )
		{
			if( rc_intersect( area, &clip ) )
			{
				if( !_app )
				{
					objc_draw( menu, 1, MAX_DEPTH, clip.g_x, clip.g_y,
						clip.g_w, clip.g_h );
				}
				objc_draw( calculator, ROOT, MAX_DEPTH, clip.g_x, clip.g_y,
					clip.g_w, clip.g_h );
				if( popUp > 0 )
				{
					objc_draw( menu, popUp, MAX_DEPTH, 
						clip.g_x, clip.g_y,
						clip.g_w, clip.g_h );
				}
			}
			wind_get( aesHandle, WF_NEXTXYWH, 
					&clip.g_x, &clip.g_y, &clip.g_w, &clip.g_h );
		}
		wind_update( END_UPDATE );
		visible = TRUE;
	}
}

static void topPanel( int aesHandle )
{
	if( aesHandle == windId )
		wind_set( aesHandle, WF_TOP, 0, 0, 0, 0 );
}

static int closePanel( int aesHandle )
{
	if( popUp > 0 )
		invalidatePopUp();

	if( aesHandle == windId )
	{
		writeProfile( DEFAULT_PANEL );
		wind_close( aesHandle );
		wind_delete( aesHandle );
		windId = -1;
		if( caller >= 0 )
		{
			sendReply( caller );
			caller = -1;
		}
		return 1;
	}
	else
		return 0;
}

static void movePanel( int aesHandle, GRECT *newPos )
{
	int		dummy;

	if( aesHandle == windId )
	{
		if( !_app )
		{
			wind_calc( WC_WORK, CALC_WIND,
				newPos->g_x, newPos->g_y, newPos->g_w, newPos->g_h,
				&menu->ob_x, &menu->ob_y, &dummy, &dummy );
			calculator->ob_x = menu->ob_x;
			calculator->ob_y = menu->ob_y + menu[1].ob_height;
		}
		else
			wind_calc( WC_WORK, CALC_WIND,
				newPos->g_x, newPos->g_y, newPos->g_w, newPos->g_h,
				&calculator->ob_x, &calculator->ob_y, 
				&calculator->ob_width, &calculator->ob_height );
		
		wind_set( aesHandle, WF_CURRXYWH, 
			newPos->g_x, newPos->g_y, newPos->g_w, newPos->g_h );
	}
}

/*
	menu functions
*/
static bool handleMenu( int title, int entry )
{
	bool	quit = FALSE;

	switch( entry )
	{

		case	ABOUT:
			form_alert( 1, "[1][Calculator|"
							   "----------|"
#if 1	/* 1: FINAL - 0: BETA */
							   " |"
							   "Version 1.50 for GEM|"
#else
							   __DATE__ "|"
							   "á-Version 1.50 for GEM|"
#endif
							   "½ 1991, 1992 by Martin G„ckler]"
							   "[ OK ]" );
			break;


		/* The Edit Popup */
		/* -------------- */
		case COPY:
			copyToClipboard();
			break;
		case PASTE:
			copyFromClipboard();
			break;

		case	QUIT:
			closePanel( windId );
			quit = TRUE;
			break;


		/* The Panel Popup */
		/* --------------- */
		case	LARGEPAN:
			openPanel( LARGE_PANEL );
			break;
		case	SMALLPAN:
			openPanel( SMALL_PANEL );
			break;
		case	COMPPAN:
			openPanel( COMPUTER_PANEL );
			break;
		case	SNCEPAN:
			openPanel( SCIENCE_PANEL );
			break;
	}

	if( _app && title )
		menu_tnormal( menu, title, 1 );

	return quit;
}

static void invalidatePopUp( void )
{
	if( curEntry > 0 )
	{
		menu[curEntry].ob_state &= ~SELECTED;
		curEntry = -1;
	}
	
	if( popUp )
	{
		menu[curTitle].ob_state = NORMAL;
		popUp = 0;
	}
}

static void removePopUp( void )
{
	wind_update( BEG_UPDATE );
	if( curEntry > 0 )
	{
		menu[curEntry].ob_state &= ~SELECTED;
		curEntry = -1;
	}

	objc_draw( calculator, ROOT, MAX_DEPTH, 
		calculator->ob_x + menu[popUp].ob_x-1, 
		calculator->ob_y + menu[popUp].ob_y,
		menu[popUp].ob_width+2, 
		menu[popUp].ob_height+3 );
	objc_change( menu, curTitle, 0, 
		menu->ob_x, menu->ob_y,
		menu->ob_width, menu->ob_height,
		NORMAL, 1 );
	popUp = 0;
	wind_update( END_UPDATE );
}

static void handleMenuTitle( int titleSel )
{
	int		newPopUp;

	switch( titleSel )
	{
		case CALCMENU:		newPopUp = CALCPOP;		break;
		case EDITMENU:		newPopUp = EDITPOP;		break;
		case PANELMEN:		newPopUp = PANELPOP;	break;
		default:			newPopUp = popUp;		break;
	}
	if( popUp != newPopUp )
	{
		if( popUp > 0 )
			removePopUp();
		if( newPopUp )
		{
			wind_update( BEG_UPDATE );
			objc_offset( menu, newPopUp, &popUpRect.g_x, &popUpRect.g_y );
			popUpRect.g_w = menu[newPopUp].ob_width;
			popUpRect.g_h = menu[newPopUp].ob_height;
			objc_draw( menu, newPopUp, MAX_DEPTH,
				menu->ob_x, menu->ob_y,
				menu->ob_width, menu->ob_height );
			objc_change( menu, titleSel, 0, 
				menu->ob_x, menu->ob_y,
				menu->ob_width, menu->ob_height,
				SELECTED, 1 );
			curTitle = titleSel;
			wind_update( END_UPDATE );
		}
		popUp = newPopUp;
	}
}

static void handleMouseInMove( int mx, int my )
{
	int entry, oldState, newState;
	
	entry = objc_find( menu, popUp, MAX_DEPTH, mx, my );

	if( entry > 0 )
	{
		oldState = menu[entry].ob_state;
		if( !(oldState & DISABLED) )
		{				
			newState = oldState | SELECTED;
			objc_change( menu, entry, 0, 
								popUpRect.g_x, popUpRect.g_y,
								popUpRect.g_w, popUpRect.g_h,
								newState, 1 );
		}

		objc_offset( menu, entry, &entryRect.g_x, &entryRect.g_y );
		entryRect.g_w = menu[entry].ob_width;
		entryRect.g_h = menu[entry].ob_height;

		curEntry  = entry;
	}
}

static void handleMouseOutMove( void )
{
	int newState;
	
	newState = menu[curEntry].ob_state & ~SELECTED;
	objc_change( menu, curEntry, 0, popUpRect.g_x, popUpRect.g_y,
			popUpRect.g_w, popUpRect.g_h, newState, 1 );
	curEntry = -1;
	
}

/*
	handle the messages from event loop
*/
static bool handleMessage( int buffer[8] )
{
	bool	quit=FALSE;
	
	switch( buffer[0] )
	{
		case MN_SELECTED:
			quit = handleMenu( buffer[3], buffer[4] );
			break;
		case WM_REDRAW:
			redrawPanel( buffer[3], (GRECT *)(buffer+4));
			break;
		case WM_TOPPED:
		case WM_NEWTOP:
			topPanel( buffer[3] );
			break;
		case WM_CLOSED:
			quit = closePanel( buffer[3] );
			break;
		case WM_MOVED:
			movePanel( buffer[3], (GRECT *)(buffer+4));
			break;
		case AC_OPEN:
			caller = -1;
			openPanel( DEFAULT_PANEL );
			break;
		case AC_CLOSE:
			invalidatePopUp();
			writeProfile( DEFAULT_PANEL );
			caller = -1;
			windId = -1;
			break;
		case 1024:
			caller = buffer[1];
			if( !buffer[5] 
			|| curPanel == LARGE_PANEL || curPanel == COMPUTER_PANEL )
				openPanel( curPanel );
			else
				openPanel( LARGE_PANEL );

			RestartCalc( *((const char **)(buffer+3)), 
				buffer[5], buffer[6] );
			handleKeybd( buffer[7] );
			break;
		case 1025:
			sendReply( buffer[1] );
			break;
			
	}
	return quit;
}

/*
	handle a button press event in the panel
*/
static void handleCalcButton( int button )
{
	int inButton, virtButton;

	virtButton = getVirtualButton( button );

	if( virtButton > 0 
	&&  (calculator[button].ob_flags & SELECTABLE)
	&&  !(calculator[button].ob_state & DISABLED) )
	{
		inButton = graf_watchbox( calculator, button, SELECTED, NORMAL );
		if( inButton )
		{
			HandleObject( virtButton );
			objc_change( calculator, button, 0, 
				calculator->ob_x, calculator->ob_y,
				calculator->ob_width, calculator->ob_height, 
				NORMAL, 1 );
		}
	}
}

/*
	handle mouse button events
*/
static bool handleButton( int mX, int mY )
{
	int		title, entry, button;
	bool	quit=FALSE;
	
	title = objc_find( menu, 1, MAX_DEPTH, mX, mY );
	if( title >= 0 )
		handleMenuTitle( title );
	else if( popUp > 0 )
	{
		entry = objc_find( menu, popUp, MAX_DEPTH, mX, mY );
		removePopUp();
		if( entry >= 0 && !(menu[entry].ob_state & DISABLED) )
			quit = handleMenu( curTitle, entry );
	}
	else if((button=objc_find( calculator, ROOT, MAX_DEPTH, mX, mY )) >= 0)
	{
		handleCalcButton( button );
	}
	else
		Bell();

	return quit;
}

/*
	handle the keyboard events
*/
static bool handleKeybd( int scanCode )
{
	int		ascii, virtualButton, panButton, i;
	bool	quit = FALSE;

	static struct
	{
		int		ascii, button, menu;
	} table[] =
	{
		/* the button accelerator */
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

		/* the menu accelerator */
		{ 'C'-'A'+1,	0,		COPY  },
		{ 'V'-'A'+1,	0,		PASTE },
		{ 'Q'-'A'+1,	0,		QUIT  },
		{ 'U'-'A'+1,	0,		QUIT  },
		{ -1 }
	};

	ascii = toupper( scanCode & 0xFF );
	
	for( i=0; table[i].ascii >= 0; i++ )
		if( ascii == table[i].ascii )
			break;
	if( table[i].ascii >= 0 )
	{
		if( table[i].button )
		{
			virtualButton = table[i].button;
			panButton = translateButton( virtualButton, VIRTUAL_PANEL, 
										curPanel );
			if( panButton > 0
			&&  !(calculator[panButton].ob_state & DISABLED) )
			{
				objc_change( calculator, panButton, 0, 
					calculator->ob_x, calculator->ob_y,
					calculator->ob_width, calculator->ob_height, 
					SELECTED, 1 );
				HandleObject( virtualButton );
				objc_change( calculator, panButton, 0, 
					calculator->ob_x, calculator->ob_y,
					calculator->ob_width, calculator->ob_height, 
					NORMAL, 1 );
			}
		}
		else
			quit = handleMenu( 0, table[i].menu );
	}

	return quit;
}

/*
	the event loop
*/
static void mainLoop( void )
{
	int		quit = 0, event=0;
	EVENT	buff;

	buff.ev_mm2flags = 0;
	buff.ev_mbclicks = 1;
	buff.ev_bmask    = 1;
	buff.ev_mbstate  = 1;

	while( !_app || !quit )
	{
		if( !(event & MU_M2) )		/* mouse is not in menu_bar */
			checkClipboard();

		if( popUp > 0 )
		{
			buff.ev_mm2x      = menu->ob_x;
			buff.ev_mm2y      = menu->ob_y;
			buff.ev_mm2width  = menu[1].ob_width;
			buff.ev_mm2height = menu[1].ob_height;

			if( curEntry > 0 )
			{
				buff.ev_mm1flags = 1;

				buff.ev_mm1x      = entryRect.g_x;
				buff.ev_mm1y      = entryRect.g_y;
				buff.ev_mm1width  = entryRect.g_w;
				buff.ev_mm1height = entryRect.g_h;
			}
			else
			{
				buff.ev_mm1flags = 0;

				buff.ev_mm1x      = popUpRect.g_x;
				buff.ev_mm1y      = popUpRect.g_y;
				buff.ev_mm1width  = popUpRect.g_w;
				buff.ev_mm1height = popUpRect.g_h;
			}

			buff.ev_mflags    = EVENTS|MU_M1|MU_M2;
		}
		else
			buff.ev_mflags    = EVENTS;

		event = EvntMulti( &buff );			/* evnt_multi */

		if( event & MU_M1 )
		{
			if( curEntry > 0 )
				handleMouseOutMove(); 
			else		
				handleMouseInMove( buff.ev_mmox, buff.ev_mmoy );
		}

		if( event & (MU_BUTTON|MU_M2) )
			quit = handleButton( buff.ev_mmox, buff.ev_mmoy );

		if( event & MU_MESAG )
			quit = handleMessage( buff.ev_mmgpbuf );

		if( event & MU_KEYBD )
			quit = handleKeybd( buff.ev_mkreturn );
	}
}

/* --------------------------------------------------------------------- */
/* ----- entry points -------------------------------------------------- */
/* --------------------------------------------------------------------- */

void setDspString( short obj, const char *string )
{
	int			line;
	PANELTYPE	i;
	
	line = findLine( VIRTUAL_PANEL, obj );

	forAllPanels( i )
		if( translation[line][i] >= 0 )
			panInfos[i].tree[
					translation[line][i]
				].ob_spec.tedinfo->te_ptext = (char *)string ;
}

void RefreshDispl( const char *mantissa, const char *exponent )
{
	setDspString( MANTDSP, mantissa );
	setDspString( EXPODSP, exponent );

	if( visible )
	{
		objc_draw( calculator, translateButton( DISPLAY, VIRTUAL_PANEL,
												curPanel ),
			MAX_DEPTH,
			calculator->ob_x, calculator->ob_y,
			calculator->ob_width, calculator->ob_height );
	}
}

void DispOn( short obj )
{
	int			line1, line2;
	PANELTYPE	i;

	line1 = findLine( VIRTUAL_PANEL, DISPLAY );
	line2 = findLine( VIRTUAL_PANEL, obj );

	forAllPanels( i )
		if( translation[line2][i] > 0 )
			objc_add( panInfos[i].tree,
				translation[line1][i], translation[line2][i] );
}

void DispOff( short obj )
{
	int			line;
	PANELTYPE	i;

	line = findLine( VIRTUAL_PANEL, obj );
	forAllPanels( i )
		if( translation[line][i] >= 0 )
			objc_delete( panInfos[i].tree, translation[line][i] );
}

void EnableButton( short obj )
{
	int			line;
	PANELTYPE	i;

	line = findLine( VIRTUAL_PANEL, obj );

	forAllPanels( i )
		if( translation[line][i] > 0 )
		{
			panInfos[i].tree[translation[line][i]].ob_state &= ~DISABLED;
			if( visible && curPanel == i )
			{
				objc_draw( calculator, translation[line][i], MAX_DEPTH,
							calculator->ob_x,     calculator->ob_y,
							calculator->ob_width, calculator->ob_height );
			}
		}
}

void DisableButton( short obj )
{
	int			line;
	PANELTYPE	i;

	line = findLine( VIRTUAL_PANEL, obj );

	forAllPanels( i )
		if( translation[line][i] > 0 )
		{
			panInfos[i].tree[translation[line][i]].ob_state |= DISABLED;
			if( visible && curPanel == i )
			{
				objc_draw( calculator, translation[line][i], MAX_DEPTH,
							calculator->ob_x,     calculator->ob_y,
							calculator->ob_width, calculator->ob_height );
			}
		}
}

void EnablePanel( PANELTYPE panel )
{
	menu_ienable( menu, panInfos[panel].menu, 1 );
}

void DisablePanel( PANELTYPE panel )
{
	menu_ienable( menu, panInfos[panel].menu, 0 );
}

/*
	********** main *******************************************************
*/

int main( void )
{
	int ap_id, err;

	/* init GEM */
	aesId = ap_id = appl_init();
	if( rsrc_load( RSC_FILE ) )
	{
		initObjectTree();
		readProfile();
		if( !_app )
		{
			err = menu_register( ap_id, "  Calculator" ) < 0;
		}
		else
		{
			err = openPanel( DEFAULT_PANEL ) != 0;
			graf_mouse( ARROW, NULL );
		}
		if( !err )
		{
			mainLoop();
		}
		else
			form_alert( 1, "[3][|InitError][ Ok ]" );

		if( _app )
		{
			menu_bar( menu, 0 );
		}
		rsrc_free();
	}
	else
		form_alert( 1, "[3][|Resource|" RSC_FILE "|not available][ Ok ]" );

	/* ACCs must not terminate */
	while( !_app )
		evnt_timer( INT_MAX, INT_MAX );

	appl_exit();

	return 0;
}