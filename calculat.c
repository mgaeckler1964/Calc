/*
		Project:		Calculator
		Module:			CALCULAT.C
		Description:	this is the calculating module
		Author:			Martin Gaeckler
		Date:			20. 1. 1992

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <float.h>


#ifndef __TOS__			/* TOS does not need signal handling */
#include <signal.h>
#endif

#ifndef __WINDOWS__
#include <setjmp.h>
#endif

#if !defined( __BORLANDC__ ) && !defined( _MSC_VER )
char *itoa( int i, char *buffer, int radix );
char *ltoa( long i, char *buffer, int radix );
char *ultoa( unsigned long i, char *buffer, int radix );
char *strupr( char *buffer );
#endif

#if !defined( __BORLANDC__ )
double pow10( double x );
#endif

#include "calculat.h"
#include "buttons.h"

#ifdef _MSC_VER
#	pragma warning( disable: 4996 )
#endif

/* --------------------------------------------------------------------- */
/* ----- constants ----------------------------------------------------- */
/* --------------------------------------------------------------------- */

#define LOWEST		OROPER	/* lowest level operator is |			*/
#define MAXLEVEL	7		/* number of different operator levels	*/
#define MAXBRACK	100		/* number of parenteses levels 			*/
#define MAXSTACK	MAXLEVEL*MAXBRACK

#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif


/*
	System dependencies
	===================
 */

#if defined( __WINDOWS__ )		/* the MS-Windows-Version	*/

	#define CIRCLE		176
	#define MAX_FLOAT	15
	#define MAX_EXPO	3

	/*
		Catch() and Throw()
		stupid windows doesn't allow set-/longjmp

	*/
	typedef int         CATCHBUF[9];
	typedef int far     *LPCATCHBUF;
	typedef CATCHBUF	jmp_buf;

	int  far pascal		Catch(LPCATCHBUF);
	void far pascal		Throw(LPCATCHBUF, int);

	#define setjmp( jmpBuf )			Catch( jmpBuf )
	#define longjmp( jmpBuf, errCode )	Throw( jmpBuf, errCode )

#elif defined( __TOS__ )		/* the ATARI-GEM-Version	*/

	#define CIRCLE		'\xF8'
	#define MAX_FLOAT	20
	#define MAX_EXPO	4

#else							/* All others				*/

	#define CIRCLE		'o'
	#define MAX_FLOAT	20
	#define MAX_EXPO	4

#endif

/* --------------------------------------------------------------------- */
/* ----- type definitions ---------------------------------------------- */
/* --------------------------------------------------------------------- */

typedef enum { FALSE, TRUE } bool;
typedef unsigned long ulong;

typedef enum
{
	WARNING = -1,		/* There is a warning 					*/
	ERROR,				/* Error condition						*/
	ROUNDWAIT, 			/* Waiting for rounding precision		*/
	FIXPOINTWAIT, 		/* Waiting for fix point				*/
	OPWAIT,				/* Waiting for an operator				*/
	WAIT, 				/* Waiting for the first digit			*/
	R_NUMPART,			/* Waiting for integer part of real 	*/
	R_DECPART, 			/* Waiting for decimal part of real		*/
	R_EXPPART,			/* Waiting for exponent of real			*/
	I_NUMPART,			/* Waiting for integer part of imagin	*/
	I_DECPART, 			/* Waiting for decimal part of imagin	*/
	I_EXPPART,			/* Waiting for exponent of imagin		*/
	MINWAIT,			/* Waiting for minute					*/
	MINPART,			/* Entering for minute					*/
	SECWAIT,			/* Waiting for second					*/
	SECPART,			/* Entering for integerpart of second	*/
	SECDECPART,			/* Waiting for decimalpart of second	*/
} ENTRSTATE;

typedef enum
{
	COMPLEX,
	FLOAT,
	BYTE	= 11,		/* must be an odd number	*/
	UBYTE,				/* must be an even number	*/
	WORD,
	UWORD,
	LONG,
	ULONG
} DATATYPES;

typedef enum
{
	LINEREGRESSION, LNREGRESSION, EXPOREGRESSION, POWEREGRESSION
} REGRESSION;

typedef struct
{
	short	button;
	char	*display;
} REGTABLE;

typedef struct
{
	double	real;
	double	imagin;
} COMPLEX_NUMBER;

typedef union
{
	double			real;
	COMPLEX_NUMBER	complex;
	long			integer;
} VALUE;

typedef struct
{
	VALUE	operand;
	short	operator, level;
} OPERATION;

typedef struct
{
	double	sumX,
			sumY,
			sumX2,
			sumY2,
			sumXY;
} REGDATA;

/* --------------------------------------------------------------------- */
/* ----- macros -------------------------------------------------------- */
/* --------------------------------------------------------------------- */

#define forAllRegTypes( i ) \
	for( i = POWEREGRESSION; i>=LINEREGRESSION; i-- )

/* --------------------------------------------------------------------- */
/* ----- module statics ------------------------------------------------ */
/* --------------------------------------------------------------------- */

static char			r_mantissa[34]			= " 0.",
					r_exponent[MAX_EXPO+2]	= "";	/* incl sign and 0-Byte */

static char			i_mantissa[34]			= "",
					i_exponent[MAX_EXPO+2]	= "";	/* incl sign and 0-Byte */

static jmp_buf		jumpBuffer;

static bool 		invers    = FALSE,
					hyperbel  = FALSE,
					memory    = FALSE;

static short		constOper = -1;				/* Constant operator	*/
static VALUE		constVal;					/* Constant operand		*/

static short		lastObj   = -1;

static short		fixPoint  = -1,
					roundPrec = -1,
					engineerPrecision = 0;
static bool			engineerMode = FALSE;

static short		trgMode   = DEGREE;
static VALUE		x_reg     = { 0.0 },
					m_reg     = { 0.0 };

static REGTABLE		regTable[POWEREGRESSION+1] =
{
	{ LINEREG, "Line"  },
	{ LNREG,   "Log"   },
	{ EXPOREG, "Expo"  },
	{ POWEREG, "Power" }
};

static REGDATA		regressionData[POWEREGRESSION+1];
static REGRESSION	regressionType=LINEREGRESSION;
static char			*regDsp=NULL;

static double		lastX;
static long			numX      = 0;
static long			statCount;
static bool			statistic = FALSE,
					regresion = FALSE,
					yExpected = FALSE;

static DATATYPES	dataType  = FLOAT;
static short		curRadix  = 10;
static short		stackPos  = 0,
					curBrack  = 0;
static ENTRSTATE	entrstate = WAIT;

static const char	*radixDsp = NULL,
					*typeDsp  = NULL,
					*trgDsp   = NULL;

static OPERATION	stack[MAXSTACK];

static short		curPos;

struct	/* the operator levels */
{
	short operator, level,
		/* 0: doesn't matter, 1: requires FLOAT, 2: requires integer */
		type;
} levels[] =
{
	{ OROPER,	0, 2 },
	{ XOROPER,	1, 2 },
	{ ANDOPER,	2, 2 },
	{ PLUS,		3, 0 },
	{ MINUS,	3, 0 },
	{ MULTIPLY,	4, 0 },
	{ DIVISION,	4, 0 },
	{ MODULOOP, 4, 2 },
	{ POWER,	5, 1 },
	{ ROOTOPER,	5, 1 },
	{ BINOM,	6, 1 },
	{ -1, -1 }
};

static short maxMantCol=2, maxMantLine=0;

static short maxMantSizes[5][4] =
{
	/* BIN			OCT 		DEZ			HEX	*/
	{	0,			0,			MAX_FLOAT,	0 },		/* FLOAT	*/
	{   32,			11,			10,			8 },		/* LONG		*/
	{	16,			6,			5,			4 },		/* WORD		*/
	{	8,			3,			3,			2 }			/* BYTE		*/
};

static short digits[]=
{
	ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE,
	DIGITA, DIGITB, DIGITC, DIGITD, DIGITE, DIGITF
};

static short maxMant = MAX_FLOAT;

/* --------------------------------------------------------------------- */
/* ----- module functions ---------------------------------------------- */
/* --------------------------------------------------------------------- */

static void getError( const char *errMsg, ENTRSTATE newState )
{
	*r_exponent = 0;
	*i_exponent = 0;
	*i_mantissa = 0;
	strcpy( r_mantissa, errMsg );
	entrstate = newState;
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
	longjmp( jumpBuffer, 1 );
}

#if defined( __WINDOWS__ )
static void FPUsignal( int sig, int type )
{
	const char	*errTxt;

	switch( type )
	{
		case FPE_ZERODIVIDE:	errTxt = "Division by zero";	break;
		case FPE_OVERFLOW:		errTxt = "+Inf";				break;
		case FPE_UNDERFLOW:		errTxt = "-Inf";				break;
		case FPE_INEXACT:		errTxt = "Inexact result";		break;
		default:				errTxt = "General error";		break;
	}
	getError( errTxt, ERROR );
}
#elif !defined( __TOS__ )

#ifdef __BORLANDC__
#pragma argsused
#endif

void FPUsignal( int sig )
{
	getError( "FPU error", ERROR );
}
#	endif

#if defined( __WINDOWS__ )
/*
 *	BC++ needs this error handling too. This function is called by the
 *  floating point library.
 */
int matherr(struct exception *a)
{
   const char	*errTxt;

   switch( a->type )
   {
		case DOMAIN:
		case SING:		errTxt = "Nan";				break;
		case OVERFLOW:	errTxt = "+Inf";			break;
		case UNDERFLOW:	errTxt = "-Inf";			break;
		case TLOSS:		errTxt = "Inexact result";	break;
		default:		errTxt = "General Error";	break;
   }
   getError( errTxt, ERROR );
   return 1;
}
#endif

static short getLevel( short oper )
{
	short i;

	for(i=0; levels[i].operator != oper; i++ );

	if( (levels[i].type == 1 && dataType != FLOAT && dataType != COMPLEX)
	||  (levels[i].type == 2 && dataType == FLOAT) )
		getError( "Illegal operator", ERROR );

	return levels[i].level + curBrack*MAXLEVEL;
}

static void pushStack( VALUE *operand, short operator, short level )
{
	if( stackPos < MAXSTACK )
	{
		stack[stackPos].operand = *operand;
		stack[stackPos].level = level;
		stack[stackPos++].operator = operator;
	}
	else
		getError( "Stack overflow", ERROR );
}

static void initDispl( short digit, char show )
{
	engineerMode = FALSE;

	if( dataType == FLOAT )
		x_reg.real = digit;
	else if( dataType == COMPLEX )
		x_reg.complex.real = digit;
	else
		x_reg.integer = digit;

	r_mantissa[0] = ' ';
	r_mantissa[1] = show;

	if( dataType == FLOAT || dataType == COMPLEX )
	{
		r_mantissa[2] = '.';
		r_mantissa[3] = '\0';
	}
	else
		r_mantissa[2] = '\0';

	if( digit )
	{
		curPos = 2;
		entrstate = R_NUMPART;
	}
	else
		curPos = 1;

	if( dataType == COMPLEX )
	{
		i_mantissa[0] = ' ';
		i_mantissa[1] = '0';
		i_mantissa[2] = '.';
		i_mantissa[3] = '\0';
	}
	else
		*i_mantissa = 0;

	*r_exponent = 0;
	*i_exponent = 0;
}

static COMPLEX_NUMBER scanComplex( const char *str )
{
	COMPLEX_NUMBER	x;
	short			expo;

	x.real = atof( str );

	if( r_exponent[0] )
	{
		expo = atoi( r_exponent );
		if( expo > DBL_MAX_10_EXP )
			getError( "Out of range", WARNING );
		x.real *= pow10( expo );
	}
	if( i_mantissa[0] )
	{
		x.imagin = atof( i_mantissa );
		if( i_exponent[0] )
		{
			expo = atoi( i_exponent );
			if( expo > DBL_MAX_10_EXP )
				getError( "Out of range", WARNING );
			x.imagin *= pow10( expo );
		}
	}

	return x;
}

static double scanFloat( const char *str )
{
	double	x;
	short	expo;

	x = atof( str );

	if( r_exponent[0] )
	{
		expo = atoi( r_exponent );
		if( expo > DBL_MAX_10_EXP )
			getError( "Out of range", WARNING );
		x *= pow10( expo );
	}

	return x;
}

static double scanAngle( const char *str )
{
	char		buffer[128], *cp1, *cp2;
	short		degree=0;
	unsigned	minute=0;
	double		second=0,
				result;

	strcpy( buffer, str );

	cp1 = strchr( buffer, CIRCLE );
	if( cp1 ) *cp1 = 0;
	degree = atoi( buffer );
	if( cp1 )
	{
		cp2 = strchr( ++cp1, '\'' );
		if( cp2 )
			*cp2 = 0;
		minute = atoi( cp1 );
		if( cp2 )
			second = atof( ++cp2 );
	}
	result = (double)degree + (double)minute/60.0 + second/3600.0;

	return result;
}

static void ScanX( const char *str )
{
	if( entrstate > WAIT || str )
	{
		if( !str )
			str = r_mantissa;
		else
			r_exponent[0]=0;

		if( dataType == FLOAT )
			x_reg.real = entrstate < MINWAIT ? scanFloat( str )
										 : scanAngle( str );
		else if( dataType == COMPLEX )
			x_reg.complex = scanComplex( str );
		else if( dataType & 0x1 && curRadix == 10 )
			x_reg.integer = strtol( str, NULL, curRadix );
		else
			x_reg.integer = strtoul( str, NULL, curRadix );
		entrstate = OPWAIT;
	}
}

static void FloatToDisplay( double fltValue, char *mantissa, char *exponent )
{
	char	buffer[128], *expo;
	size_t	len;

	if( fltValue > HUGE_VAL )
		strcpy( buffer, "+Inf" );
	else if( fltValue < -HUGE_VAL )
		strcpy( buffer, "-Inf" );
	else if( fixPoint >= 0 )
		sprintf( buffer, "%.*lf", fixPoint, fltValue );
	else if( roundPrec >= 0 )
		sprintf( buffer, "%.*le", roundPrec, fltValue );
	else if( engineerMode )
		sprintf( buffer, "%.*lge%+d", maxMant, fltValue/pow10( engineerPrecision ), engineerPrecision );
	else if( fltValue )
		sprintf( buffer, "%.*lg", maxMant, fltValue );
	else
	{
		buffer[0]='0';
		buffer[1]=0;
	}

	expo = strchr( buffer, 'e' );
	if( expo )
		*expo++ = 0;

	if( *buffer == '-' || *buffer == '+' )
		strncpy( mantissa, buffer, maxMant+1 );
	else
	{
		strncpy( mantissa+1, buffer, maxMant+1 );
		*mantissa = ' ';
	}
	mantissa[maxMant+1]=0;

#ifdef __TOS__
	if( !strcmp( buffer+1, "Inf" ) || !strcmp( buffer,"Nan" ))
		getError( buffer, ERROR );
#endif

	if( !strchr( mantissa, '.' ) )
	{
		len = (short)strlen( mantissa );
		mantissa[len]='.';
		mantissa[len+1]='\0';
	}
	if( expo )
	{
		*exponent = *expo++ == '-' ? '-' : ' ';
		while( *expo == '0' )
			expo++;
		strcpy( exponent+1, expo );
	}
	else
		*exponent = 0;
}

static void ComplexToDisplay(	COMPLEX_NUMBER	cpxValue,
								char *r_mantissa, char *r_exponent,
								char *i_mantissa, char *i_exponent )
{
	FloatToDisplay( cpxValue.real,   r_mantissa, r_exponent );
	FloatToDisplay( cpxValue.imagin, i_mantissa, i_exponent );
}

static void IntToDisplay( long intValue, char *mantissa )
{
	char	buffer[64], c=0;
	long	sIntValue;
	ulong	uIntValue;

	if( dataType & 0x1 && curRadix == 10 )
	{
		if( dataType == BYTE )
			sIntValue = c = (char)intValue;
		else if( dataType == WORD )
			sIntValue = (short)intValue;
		else
			sIntValue = intValue;
		ltoa( sIntValue, buffer, curRadix );
	}
	else
	{
		uIntValue = (ulong)intValue;
		if( dataType == UBYTE || dataType == BYTE )
		{
			uIntValue &= 0xFFUL;
			c = (char)uIntValue;
		}
		else if( dataType == UWORD || dataType == WORD )
			uIntValue &= 0xFFFFUL;
		ultoa( uIntValue, buffer, curRadix );
	}
	strupr( buffer );

	if( *buffer == '-' || *buffer == '+' )
		strncpy( mantissa, buffer, maxMant+1 );
	else
	{
		strncpy( mantissa+1, buffer, maxMant+1 );
		*mantissa = ' ';
	}
	mantissa[maxMant+1]=0;

	if( c )
	{
		sprintf( buffer, " (\'%c\')", c );
		strcat( mantissa, buffer );
	}
}

static void Display( void )
{
#if 0
	if( entrstate <= ERROR )
		return;
#endif

	if( dataType > FLOAT )
	{
		*r_exponent = 0;
		*i_mantissa = 0;
		*i_exponent = 0;
		IntToDisplay( x_reg.integer, r_mantissa );
	}
	else if( dataType == FLOAT )
	{
		*i_mantissa = 0;
		*i_exponent = 0;
		FloatToDisplay( x_reg.real, r_mantissa, r_exponent );
	}
	else if( dataType == COMPLEX )
	{
		ComplexToDisplay(	x_reg.complex,
							r_mantissa, r_exponent,
							i_mantissa, i_exponent );
	}


	if( invers )
	{
		invers = FALSE;
		DispOff( INVDSP );
	}
	if( hyperbel )
	{
		hyperbel = FALSE;
		DispOff( HYPDSP );
	}

	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

/*
	Berechnung des Binomialkoeffizienten:
*/

static double binom( double x, double y )
{
	double	result, quotExit;

	unsigned long	yL;
	bool			running;


	yL = (unsigned long)y;
#ifdef __TOS__		/* there is a bug in Turbo C for TOS */
	if( y != 0.0 && (y < 0 || yL != y) )
#else
	if( y < 0 || yL != y )
#endif
		getError( "Illegal operand", ERROR );

	result = 1.0;

	/* get some special fast results */
	if( x == y )
		;				/* result is 1.0 */
	else if( !yL )
		;
	else if( y == 1 || y == x-1 )
		result = x;
	else /* now we have to do it */
	{
		quotExit = x - y + 1;

	/*
		Dies sind praktisch zwei Schleifen, die gleichzeitig berechnet
		werden. Einmal die Produktfolge x * (x-1) * ... * (x-y+1) und
		gleichzeitig wird das Ergebnis durch die Produktfolge 1 * 2 ...	y
		dividiert. Dadurch wird die Gefahr eines šberlaufs minimiert.
	*/

		do
		{
			running = FALSE;

			/* loop # 1 */
			if( x >= quotExit  )
			{
				running = TRUE;
				result *= x--;
			}

			/* loop # 2 */
			if( yL )
			{
				running = TRUE;
				result /= (double)yL--;
			}

		} while( running );
	}

	return result;
}

static COMPLEX_NUMBER doComplexOperation( COMPLEX_NUMBER x, short oper, COMPLEX_NUMBER y )
{
	COMPLEX_NUMBER value;

	switch( oper )
	{
		case PLUS:
				value.real = x.real + y.real;
				value.imagin = x.imagin + y.imagin;
				break;
		case MINUS:
				value.real = x.real - y.real;
				value.imagin = x.imagin - y.imagin;
				break;

		case MULTIPLY:
				value.real = (x.real * y.real) - (x.imagin * y.imagin);
				value.imagin = (x.real * y.imagin) + (x.imagin * y.real);
				break;
		case DIVISION:
#ifdef __TOS__
			if( !y.real && !y.imagin )
				getError( "Division by zero", ERROR );
#endif
			value.real = ((x.real*y.real)+(x.imagin*y.imagin)) /
						 ((y.real*y.real)+(y.imagin*y.imagin));
			value.imagin = (-(x.real*y.imagin)+(y.real*x.imagin)) /
						 ((y.real*y.real)+(y.imagin*y.imagin));
			break;
	}

	return value;
}

static double doFloatOperation( double x, short oper, double y )
{
	double value;
	switch( oper )
	{
		case PLUS:		value = x + y;			break;
		case MINUS:		value = x - y;			break;
		case MULTIPLY:	value = x * y;			break;
		case DIVISION:
#ifdef __TOS__
			if( !y )
				getError( "Division by zero", ERROR );
#endif
			value = x / y ;
			break;
		case POWER:		value = pow( x, y );	break;
		case ROOTOPER:
#ifdef __TOS__
			if( !y )
				getError( "Division by zero", ERROR );
#endif
			value = pow( x, 1/y );				break;
		case BINOM:		value = binom( x, y );	break;
	}

	return value;
}

static long doIntegerOperation( long x, short oper, long y )
{
	long value;

	switch( oper )
	{
		case PLUS:		value = x + y;			break;
		case MINUS:		value = x - y;			break;
		case MULTIPLY:	value = x * y;			break;
		case ANDOPER:	value = x & y;			break;
		case OROPER:	value = x | y;			break;
		case XOROPER:	value = x ^ y;			break;
		case MODULOOP:
		case DIVISION:
			if( !y )
				getError( "Division by zero", ERROR );
			value = (oper == DIVISION) ? (x / y) : (x % y);
			break;
	}

	return value;
}

static void doOperation( const VALUE *source, short oper, VALUE *dest )
{
	if( dataType == FLOAT )
		dest->real = doFloatOperation( source->real, oper, dest->real );
	else if( dataType == COMPLEX )
		dest->complex = doComplexOperation( source->complex, oper, dest->complex );
	else
		dest->integer = doIntegerOperation( source->integer,
											oper, dest->integer );
}

static void HandleOperator( short oper )
{
	short level;

	level = getLevel( oper );

	if( level >= 0 )
	{
		if( entrstate == WAIT && stackPos )
		{
			if( oper == lastObj )
			{
				if( constOper < 0 )
				{
					constVal = x_reg;
					constOper = oper;
					DispOn( CNSTDSP );
				}
				else
				{
					constOper = -1;
					DispOff( CNSTDSP );
				}
			}
			else
			{
				stack[stackPos-1].operator = oper;
				stack[stackPos-1].level = level;
			}
		}
		else if( entrstate >= OPWAIT )
		{
			if( constOper >= 0 )
			{
				constOper = -1;
				DispOff( CNSTDSP );
			}
			ScanX( NULL );
			while( stackPos )
			{
				if( stack[stackPos-1].level >= level )
				{
					doOperation( &stack[stackPos-1].operand,
						stack[stackPos-1].operator, &x_reg );
					stackPos--;
				}
				else
					break;
			}
			pushStack( &x_reg, oper, level );
			entrstate = WAIT;
		}
		else
			getError( "No operand expected", ERROR );
		Display();
	}
}

#ifndef __TOS__
static long Random( void )
{
	unsigned long rnd1, rnd2;

	rnd1 = rand();
	rnd2 = rand();

	return (rnd1 << 16) | rnd2;
}
#else
extern long Random( void );
#endif

#ifndef __APPLE__
static double asinh( double x )
{
#ifdef __TOS__
	if( x < 0 )
		getError( "Illegal Argument", ERROR );
#endif
	return log( x + sqrt( x*x + 1 ));
}

static double acosh( double x )
{
#ifdef __TOS__
	if( x < 1 )
		getError( "Illegal argument", ERROR );
#endif
	return log( x + sqrt( x*x -1 ));
}

#ifndef __TOS__
static double atanh( double x )
{
	return 0.5 * log( (1+x) / (1-x) );
}
#endif

#endif /* ifndef __APPLE__ */

static double facult( double x )
{
	double	fac;
	long	run;

	if( x < 0 || modf( x, &fac ) )
		getError( "Illegal argument", ERROR );

	fac = 1.0;
	for( run=2; run <= x; run++ )
	{
		fac *= run;
		if( fac >= HUGE_VAL || run > 10000 )
			break;
	}

	return( fac );
}

static double declog( double x )
{
	if( invers )
		x = pow( 10, x );
	else
	{
#ifdef __TOS__
		if( x < 0 )
			getError( "Illegal Argument", ERROR );
#endif
		x = log10( x );
	}

	return x;
}

static double natlog( double x )
{
	if( invers )
		x = exp( x );
	else
	{
#ifdef __TOS__
		if( x < 0 )
			getError( "Illegal Argument", ERROR );
#endif
		x = log( x );
	}
	return( x );
}

static double FromRad( double x )
{
	if( trgMode == DEGREE )
		x = x * (360.0/(2*M_PI));
	else if( trgMode == GRADIAL )
		x = x * (400.0/(2*M_PI));
	return x;
}

static double ToRad( double x )
{
#ifdef __TOS__
	if( x >= 1e18 )
		getError( "Inexact result", ERROR );
#endif
	if( trgMode == DEGREE )
		x = x * (2*M_PI/360.0);
	else if( trgMode == GRADIAL )
		x = x * (2*M_PI/400.0);
	return x;
}

static double sinus( double x )
{
	if( invers )
	{
		if( hyperbel )
			x = asinh( x );
		else
			x = FromRad( asin( x ));
	}
	else
	{
		if( hyperbel )
			x = sinh( x );
		else
			x = sin( ToRad( x ) );
	}
	return x;
}

static double cosinus( double x )
{
	if( invers )
	{
		if( hyperbel )
			x = acosh( x );
		else
			x = FromRad( acos( x ));
	}
	else
	{
		if( hyperbel )
			x = cosh( x );
		else
			x = cos( ToRad( x ) );
	}
	return x;
}

static double tangens( double x )
{
	if( invers )
	{
		if( hyperbel )
			x = atanh( x );
		else
			x = FromRad( atan( x ));
	}
	else
	{
		if( hyperbel )
			x = tanh( x );
		else
			x = tan( ToRad( x ) );
	}
	return x;
}

static double sqrRoot( double x )
{
	if( invers )
		x = x*x;
	else
	{
#ifdef __TOS__
		if( x < 0 )
			getError( "Illegal Argument", ERROR );
#endif
		x = sqrt( x );
	}

	return x;
}

static double getRandom( void )
{
	return (double)(Random() & 0xFFFFFFL)/(double)(0x1000000L);
}

static void MinpCpx( COMPLEX_NUMBER x )
{
	if( !x.real && !x.imagin )
	{
		memory = FALSE;
		DispOff( MEMDSP );
	}
	else
	{
		memory = TRUE;
		DispOn( MEMDSP );
	}
	m_reg.complex = x;
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void MinpFlt( double x )
{
	if( m_reg.real && !x )
	{
		memory = FALSE;
		DispOff( MEMDSP );
	}
	else if( !m_reg.real && x )
	{
		memory = TRUE;
		DispOn( MEMDSP );
	}
	m_reg.real = x;
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void MinpInt( long x )
{
	if( m_reg.integer && !x )
	{
		memory = FALSE;
		DispOff( MEMDSP );
	}
	else if( !m_reg.integer && x )
	{
		memory = TRUE;
		DispOn( MEMDSP );
	}
	m_reg.integer = x;
}

static COMPLEX_NUMBER SwapXMCpx( COMPLEX_NUMBER x )
{
	COMPLEX_NUMBER oldM;

	oldM = m_reg.complex;
	MinpCpx( x );
	return oldM;
}

static double SwapXMFlt( double x )
{
	double oldM;

	oldM = m_reg.real;
	MinpFlt( x );
	return oldM;
}

static COMPLEX_NUMBER SwapXYCpx( COMPLEX_NUMBER x )
{
	COMPLEX_NUMBER oldY;

	if( stackPos )
	{
		oldY = stack[stackPos-1].operand.complex;
		stack[stackPos-1].operand.complex = x;
	}
	else
		oldY = x;

	return oldY;
}

static double SwapXYFlt( double x )
{
	double oldY;

	if( stackPos )
	{
		oldY = stack[stackPos-1].operand.real;
		stack[stackPos-1].operand.real = x;
	}
	else
		oldY = x;

	return oldY;
}

static long SwapXMInt( long x )
{
	long oldM;

	oldM = m_reg.integer;
	MinpInt( x );
	return oldM;
}

static long SwapXYInt( long x )
{
	long oldY;

	if( stackPos )
	{
		oldY = stack[stackPos-1].operand.integer;
		stack[stackPos-1].operand.integer = x;
	}
	else
		oldY = x;

	return oldY;
}

/*
---------------------------------------------------------------------------
	Statistic & Regression
---------------------------------------------------------------------------
*/
static void HandleRegression( short object )
{
	REGRESSION	regType;

	forAllRegTypes( regType )
		if( object == regTable[regType].button )
		{
			regDsp			= regTable[regType].display;
			regressionType	= regType;
			setDspString( REGDSP, regDsp );
			RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
			break;
		}
}
	
/*
	input new x value for statistic and regression
*/
static void Xinp( double x )
{
	double lnX;

	if( yExpected && regresion )
		getError( "X not expected", ERROR );

/*
	check we have a multiply operator on stack
*/
	if( stackPos && stack[stackPos-1].operator == MULTIPLY )
	{
		stackPos--;
		statCount = (long)stack[stackPos].operand.real;
	}
	else
		statCount = 1;

	if( !statCount )
		return;			/* nothing to do */

	if( invers )
		statCount = -statCount;

	if( x > 0 )
		lnX = log( x );

	if( statCount < 0 )
	{
		if( !statistic )
			getError( "Buffer empty", ERROR );

		if( numX < -statCount )
			getError( "Too many values", ERROR );
	}
	
	if( !regresion && (numX += statCount) == 0 )
	{
		memset( &regressionData, 0, sizeof( regressionData ) );
		statistic = FALSE;

		DispOff( STATDSP );

		return;
	}

	regressionData[LINEREGRESSION].sumX  += x   *       statCount;
	regressionData[LINEREGRESSION].sumX2 += x   * x   * statCount;

	regressionData[ LNREGRESSION ].sumX  += lnX *       statCount;
	regressionData[ LNREGRESSION ].sumX2 += lnX * lnX * statCount;

	regressionData[EXPOREGRESSION].sumX  += x   *       statCount;
	regressionData[EXPOREGRESSION].sumX2 += x   * x   * statCount;

	regressionData[POWEREGRESSION].sumX  += lnX *       statCount;
	regressionData[POWEREGRESSION].sumX2 += lnX * lnX * statCount;

	if( !statistic )
	{
		statistic = TRUE;
		DispOn( STATDSP );
		setDspString( STATDSP, "S" );
	}

	lastX = x;
	yExpected = TRUE;
}

/*
	input new y value for statistic and regression
*/
static void Yinp( double y )
{
	double	x, lnX, lnY;

	if( !yExpected || (!regresion && numX != statCount) )
		getError( "Y not expected", ERROR );

	if( statCount < 0 )
	{
		if( !regresion )
			getError( "Buffer empty", ERROR );

		if( numX < -statCount )
			getError( "Too many values", ERROR );
	}

	if( regresion )	/* not the first time called */
		numX += statCount;

	if( !numX )
	{
		memset( &regressionData, 0, sizeof( regressionData ) );
		statistic = FALSE;
		regresion = FALSE;
		DispOff( STATDSP );
		return;
	}

	x     = lastX;
	if( y > 0 )
		lnY   = log( y );
	if( x > 0 )
		lnX   = log( x );

	regressionData[LINEREGRESSION].sumY  += y   *       statCount;
	regressionData[LINEREGRESSION].sumY2 += y   * y   * statCount;
	regressionData[LINEREGRESSION].sumXY += y   * x   * statCount;

	regressionData[ LNREGRESSION ].sumY  += y   *       statCount;
	regressionData[ LNREGRESSION ].sumY2 += y   * y   * statCount;
	regressionData[ LNREGRESSION ].sumXY += y   * lnX * statCount;

	regressionData[EXPOREGRESSION].sumY  += lnY *       statCount;
	regressionData[EXPOREGRESSION].sumY2 += lnY * lnY * statCount;
	regressionData[EXPOREGRESSION].sumXY += lnY * x   * statCount;

	regressionData[POWEREGRESSION].sumY  += lnY *       statCount;
	regressionData[POWEREGRESSION].sumY2 += lnY * lnY * statCount;
	regressionData[POWEREGRESSION].sumXY += lnY * lnX * statCount;


	if( !regresion )
	{
		regresion = TRUE;
		setDspString( STATDSP, "R" );
	}
	yExpected = FALSE;
}

static double StdDev( void )
{
	double sumX, sumX2;

	if( !statistic )
		getError( "No statistic data", ERROR );

	sumX  = regressionData[LINEREGRESSION].sumX;
	sumX2 = regressionData[LINEREGRESSION].sumX2;

	return sqrt( (sumX2 - sumX * sumX / numX)/numX );
}

static double StdDev1( void )
{
	double sumX, sumX2;

	if( numX < 2 )
		getError( "Too few statistic data", ERROR );

	sumX  = regressionData[LINEREGRESSION].sumX;
	sumX2 = regressionData[LINEREGRESSION].sumX2;

	return sqrt( (sumX2 - sumX * sumX / numX)/(numX-1) );
}

/*
	Korleationskoeffizient
*/
static double KorKoef( REGRESSION regType )
{
	double	dividend, divisor, factorX, factorY, result,
			sumX, sumY, sumXY, sumX2, sumY2;

	if( !regresion )
		getError( "No regression data", ERROR );
	if( yExpected )
		getError( "Y missing", ERROR );
	if( numX < 2 )
		getError( "Too few regression data", ERROR );

	sumX  = regressionData[regType].sumX;
	sumY  = regressionData[regType].sumY;
	sumXY = regressionData[regType].sumXY;
	sumX2 = regressionData[regType].sumX2;
	sumY2 = regressionData[regType].sumY2;

	factorX = numX*sumX2 - sumX*sumX;
	if( !factorX )
		getError( "Bad regression data", ERROR );

	factorY = numX*sumY2 - sumY*sumY;
	if( !factorY )
		result = (sumY >= 0) ? 1.0 : -1.0;
	else
	{
		dividend = numX*sumXY - sumX*sumY;
		divisor  = sqrt( factorX * factorY );
		result = dividend / divisor;
	}

	return result;
}

static double BestMatch( void )
{
	REGRESSION	regType, bestType;
	double		bestKorKoef=0.0, korKoef;

	forAllRegTypes( regType )
	{
		korKoef = fabs( KorKoef( regType ) );
		if( korKoef >= bestKorKoef )
		{
			bestKorKoef = korKoef;
			bestType    = regType;
		}
	}

	bestKorKoef    *= 100.0;
	regressionType  = bestType;
	regDsp          = regTable[bestType].display;
	setDspString( REGDSP, regDsp );

	return bestKorKoef;
}

/*
	Regressionskoefizient
*/
static double RegKoef( REGRESSION regType )
{
	double sumX, sumY, sumXY, sumX2, factorX;

	if( !regresion )
		getError( "No regression data", ERROR );
	if( yExpected )
		getError( "Y missing", ERROR );
	if( numX < 2 )
		getError( "Too few regression data", ERROR );

	sumX  = regressionData[regType].sumX;
	sumY  = regressionData[regType].sumY;
	sumXY = regressionData[regType].sumXY;
	sumX2 = regressionData[regType].sumX2;

	factorX = numX*sumX2 - sumX*sumX;
	if( !factorX )
		getError( "Bad regression data", ERROR );

	return (numX*sumXY - sumX*sumY) / factorX;
}

/*
	Constant Value
*/
static double ConstVal( REGRESSION regType, bool handleRegType )
{
	double regKoef, result, sumX, sumY;

	regKoef = RegKoef( regType );

	sumX = regressionData[regType].sumX;
	sumY = regressionData[regType].sumY;

	result = (sumY - regKoef * sumX) / numX;

	if( handleRegType
	&& ( regType==EXPOREGRESSION
	  || regType==POWEREGRESSION )
	)
		result = exp( result );

	return result;
}

static double yVal( REGRESSION regType, double x )
{
	double m, k, result;

	k = ConstVal( regType, FALSE );
	if( regType == LNREGRESSION
	||  regType == POWEREGRESSION )
	{
#ifdef __TOS__
		if( x < 0 )
			getError( "Illegal Argument", ERROR );
#endif
		x = log( x );
	}

	m = RegKoef( regType );
	result = m * x + k;

	if( regType == EXPOREGRESSION
	||  regType == POWEREGRESSION )
		result = exp( result );

	return result;
}

static double xVal( REGRESSION regType, double y )
{
	double m, k, result;

	k = ConstVal( regType, FALSE );
	if( regType == EXPOREGRESSION
	||  regType == POWEREGRESSION )
	{
#ifdef __TOS__
		if( y < 0 )
			getError( "Illegal Argument", ERROR );
#endif
		y = log( y );
	}

	m = RegKoef( regType );

	if( !m )
		getError( "Horizontal line", ERROR );

	result = (y-k) / m;

	if( regType == LNREGRESSION
	||  regType == POWEREGRESSION )
		result = exp( result );

	return result;
}

/*
---------------------------------------------------------------------------
	Types und Radices
---------------------------------------------------------------------------
*/
static void HandleRadix( short radix );

static void enableDisableRadix( short oldRadix, short newRadix )
{
	short	i;

	if( oldRadix < newRadix )
		for( i=oldRadix; i<newRadix; i++ )
			EnableButton( digits[i] );
	else
		for( i=newRadix; i<oldRadix; i++ )
			DisableButton( digits[i] );
}

static void enableDisableType( short newType )
{
static short complexTable[] =
{
	EXPONENT, REZIPROK, POINT
};
static short floatTable[] =
{
	SINUS, COSINUS, TANGENS, RANDOM, DECLOG, NATLOG, REZIPROK, SQRROOT,
	INTEGER, FRAC, FACULTAT,
	XINPUT, SUMX, SUMX2, MEDIUM, DN, DNMIN1,
	NUMBER, SUMXY, CONSTVAL, REGKOEF, KORKOEF, XOUT, YOUT,
	YINPUT, SUMY, SUMY2,
	ROUND, FIXPOINT, ANGLE,
	BINOM, POWER, ROOTOPER,
	HYPERBEL, POINT, EXPONENT, DEGREE, RADIAL, GRADIAL,
	ENGINEER
};
static short intTable[] =
{
	NOTFUNC, MODULOOP, ANDOPER, XOROPER, OROPER
};
	short	i;

	DisablePanel( SCIENCE_PANEL );
	DisablePanel( SMALL_PANEL );
	DisablePanel( COMPUTER_PANEL );

	for( i=(short)(sizeof(floatTable)/sizeof(floatTable[0]))-1;
		i>=0; i-- )
		DisableButton( floatTable[i] );

	for( i=(short)(sizeof(complexTable)/sizeof(complexTable[0]))-1;
		i>=0; i-- )
		DisableButton( complexTable[i] );

	for( i=(short)(sizeof(intTable)/sizeof(intTable[0])-1); i>=0; i-- )
		DisableButton( intTable[i] );

	if( newType == FLOAT )
	{
		for( i=(short)(sizeof(floatTable)/sizeof(floatTable[0]))-1;
			i>=0; i-- )
			EnableButton( floatTable[i] );

		EnablePanel( SCIENCE_PANEL );
		EnablePanel( SMALL_PANEL );
		EnablePanel( COMPUTER_PANEL );
	}
	else if( newType == COMPLEX )
	{
		for( i=(short)(sizeof(complexTable)/sizeof(complexTable[0]))-1;
			i>=0; i-- )
			EnableButton( complexTable[i] );
	}
	else
	{
		for( i=(short)(sizeof(intTable)/sizeof(intTable[0]))-1; i>=0; i-- )
			EnableButton( intTable[i] );

		EnablePanel( COMPUTER_PANEL );
	}
}

static void HandleDataType( short type )
{
	static struct _table
	{
		short		button;
		DATATYPES	normType, invType;
		char		*normString, *invString;
	} table[] =
	{
		{ FLOATTYP, FLOAT, COMPLEX, "Float", "Complex" },
		{ LONGTYPE, LONG,  ULONG,   "Long",  "U-Long"  },
		{ WORDTYPE, WORD,  UWORD,   "Word",  "U-Word"  },
		{ BYTETYPE, BYTE,  UBYTE,   "Byte",  "U-Byte"  },
	};
	short		i;
	DATATYPES	newType;
	const
	char		*newStr;

	if( type <= 0 )
	{
		/*
			we allready know the type, but we need the display string
		*/
		type = -type;
		for( i=(short)(sizeof(table)/sizeof(struct _table))-1; i>=0 ; i-- )
		{
			if( table[i].normType == type )
			{
				enableDisableType( type );
				typeDsp     = table[i].normString;
				dataType    = type;
				maxMantLine = i;
				maxMant     = maxMantSizes[maxMantLine][maxMantCol];

				setDspString( TYPEDISP, typeDsp );
				return;
			}
			else if( table[i].invType == type )
			{
				enableDisableType( type );
				typeDsp     = table[i].invString;
				dataType    = type;
				maxMantLine = i;
				maxMant     = maxMantSizes[maxMantLine][maxMantCol];
				setDspString( TYPEDISP, typeDsp );
				return;
			}
		}
		return;
	}

	for( i=(short)(sizeof(table)/sizeof(table[0]))-1; i>=0 ; i-- )
		if( table[i].button == type )
			break;

	if( i>= 0 )
	{
		if( invers )
		{
			newType = table[i].invType;
			newStr = table[i].invString;
			invers = FALSE;
			DispOff( INVDSP );
		}
		else
		{
			newType = table[i].normType;
			newStr = table[i].normString;
		}
		if( newType != dataType )
		{
			if( newType == FLOAT )
			{
				if( dataType == COMPLEX )
				{
					x_reg.real = x_reg.complex.real;
					m_reg.real = m_reg.complex.real;
				}
				else
				{
					x_reg.real = x_reg.integer;
					m_reg.real = m_reg.integer;
				}
			}
			else if( newType == COMPLEX )
			{
				if( dataType == FLOAT )
				{
					x_reg.complex.real = x_reg.real;
					m_reg.complex.real = m_reg.real;
				}
				else
				{
					x_reg.complex.real = x_reg.integer;
					m_reg.complex.real = m_reg.integer;
				}
			}
			else
			{
				if( dataType == COMPLEX )
				{
					x_reg.integer = (long)x_reg.complex.real;
					m_reg.integer = (long)m_reg.complex.real;
				}
				else if( dataType == FLOAT )
				{
					x_reg.integer = (long)x_reg.real;
					m_reg.integer = (long)m_reg.real;
				}
			}

			setDspString( TYPEDISP, newStr );
			enableDisableType( newType );

			typeDsp     = newStr;
			maxMantLine = i;
			maxMant     = maxMantSizes[maxMantLine][maxMantCol];
			dataType    = newType;
			stackPos    = 0;

			if( (newType == FLOAT || newType == COMPLEX) && curRadix != 10 )
				HandleRadix( DEZIMAL );
		}
	}
}

static void HandleRadix( short radix )
{
	static struct _table
	{
		short		button, radix;
		char		*string;
	} table[] =
	{
		{ DUALSYS,  2, "Bin" },
		{ OKTALSYS, 8, "Oct" },
		{ DEZIMAL, 10, "Dec" },
		{ HEXSYS,  16, "Hex" }
	};
	short i, newRadix;

	if( radix < 0 )
	{
		radix = -radix;
		for( i=(short)(sizeof(table)/sizeof(table[0]))-1; i>= 0; i-- )
			if( table[i].radix == radix )
				break;
	}
	else
		for( i=(short)(sizeof(table)/sizeof(table[0]))-1; i>= 0; i-- )
			if( table[i].button == radix )
				break;

	if( i>= 0 && (newRadix = table[i].radix) != curRadix )
	{
		if( invers )
		{
			invers = FALSE;
			DispOff( INVDSP );
		}
		if( newRadix != 10 && (dataType == FLOAT || dataType == COMPLEX))
			HandleDataType( LONGTYPE );


		enableDisableRadix( curRadix, newRadix );
		curRadix   = newRadix;
		radixDsp   = table[i].string;
		maxMantCol = i;
		maxMant    = maxMantSizes[maxMantLine][maxMantCol];

		setDspString( RADIXDSP, radixDsp );
	}
}

/*
---------------------------------------------------------------------------
	Function dispatcher
---------------------------------------------------------------------------
*/
static COMPLEX_NUMBER HandleComplexFunction( short function, COMPLEX_NUMBER x, int *handled )
{
	COMPLEX_NUMBER	tmp;
	*handled = 1;
	
	switch( function )
	{
		case ABSOLUTE:
			x.real = fabs( x.real );
			x.imagin = fabs( x.imagin );
			break;
		case MPLUS:
			MinpCpx( doComplexOperation( m_reg.complex, PLUS, x ));
			break;
		case MMINUS:
			MinpCpx( doComplexOperation( m_reg.complex, MINUS, x ));
			break;
		case MINP:
			MinpCpx( x );
			break;
		case MR:
			x = m_reg.complex;
			break;
		case SWAPXM:
			x = SwapXMCpx( x );
			break;
		case SWAPXY:
			x = SwapXYCpx( x );
			break;
		case REZIPROK:
			tmp.real = 1;
			tmp.imagin = 0;
			x = doComplexOperation( tmp, DIVISION, x );
			break;
		default:
			*handled = 0;
			break;
	}
	return x;
}

static double HandleFloatFunction( short function, double x, int *handled )
{
	double dummy, newX;
	*handled = 1;

	switch( function )
	{
		case ABSOLUTE:	x = fabs( x );								break;
		case FRAC:		x = modf( x, &dummy );						break;
		case INTEGER:	modf( x, &newX );
						x = newX;									break;
		case REZIPROK:
#ifdef __TOS__
				if( !x )
					getError( "Division by zero", ERROR );
#endif
				x = 1/x;
				break;
		case FACULTAT:	x = facult( x );							break;
		case DECLOG:	x = declog( x );							break;
		case NATLOG:	x = natlog( x );							break;
		case SINUS:		x = sinus( x );								break;
		case COSINUS:	x = cosinus( x );							break;
		case TANGENS:	x = tangens( x );							break;
		case SQRROOT:	x = sqrRoot( x );							break;
		case RANDOM:	x = getRandom();							break;
		case XINPUT:	Xinp( x );									break;
		case YINPUT:	Yinp( x );									break;
		case SUMX:		x = regressionData[LINEREGRESSION].sumX;	break;
		case SUMX2:		x = regressionData[LINEREGRESSION].sumX2;	break;
		case SUMY:		x = regressionData[LINEREGRESSION].sumY;	break;
		case SUMY2:		x = regressionData[LINEREGRESSION].sumY2;	break;
		case SUMXY:		x = regressionData[LINEREGRESSION].sumXY;	break;
		case NUMBER:	x = (double)numX;							break;
		case MEDIUM:
				if( !numX )
					getError( "No statistic data", ERROR );
				x = regressionData[LINEREGRESSION].sumX/numX;
				break;
		case DN:		x = StdDev();								break;
		case DNMIN1:	x = StdDev1();								break;
		case CONSTVAL:	x = ConstVal( regressionType, TRUE );		break;
		case REGKOEF:   x = RegKoef( regressionType );				break;
		case BESTMTCH:	x = BestMatch();							break;
		case KORKOEF:   x = KorKoef( regressionType );				break;
		case XOUT:      x = xVal( regressionType, x );				break;
		case YOUT:      x = yVal( regressionType, x );				break;

		case MPLUS:		MinpFlt( m_reg.real+x );					break;
		case MMINUS:	MinpFlt( m_reg.real-x );					break;
		case MINP:		MinpFlt( x );								break;
		case MR:		x = m_reg.real;								break;
		case SWAPXM:	x = SwapXMFlt( x );							break;
		case SWAPXY:	x = SwapXYFlt( x );							break;
		default:		*handled = 0;								break;
	}

	return x;
}

static long HandleIntegerFunction( short function, long x, int *handled )
{
	*handled = 1;
	switch( function )
	{
		case ABSOLUTE:	if( dataType & 0x1 ) x = labs( x );			break;
		case SIGN:		if( dataType & 0x1 ) x = -x;				break;
		case NOTFUNC:	x = ~x;										break;
		case MPLUS:		MinpInt( m_reg.integer+x );					break;
		case MMINUS:	MinpFlt( m_reg.integer-x );					break;
		case MINP:		MinpFlt( x );								break;
		case MR:		x = m_reg.integer;								break;
		case SWAPXM:	x = SwapXMInt( x );							break;
		case SWAPXY:	x = SwapXYInt( x );							break;
		default:		*handled = 0;								break;
	}

	return x;
}

static void HandleEngineer()
{
	if( engineerMode )
	{
		int		newPrecision = engineerPrecision;
		int		theEngPrecision;
		
		if( invers )
			newPrecision -= 3;
		else
			newPrecision += 3;
		theEngPrecision = (int)log10(x_reg.real/pow10( newPrecision ));
		if( theEngPrecision > -4 && theEngPrecision < maxMant )
			engineerPrecision = newPrecision;
	}
	else
	{
		engineerMode = TRUE;
		
		engineerPrecision = (short)log10( x_reg.real );
		if( engineerPrecision % 3 )
			engineerPrecision += 3 - engineerPrecision % 3;
	}
}

static int HandleFunction( short object )
{
	int		handled = 1;
	
	ScanX( NULL );

	switch( object )	/* functions for all datatypes */
	{
		case FLOATTYP:
		case BYTETYPE:
		case WORDTYPE:
		case LONGTYPE:
			HandleDataType( object );
			break;

		case DUALSYS:
		case OKTALSYS:
		case DEZIMAL:
		case HEXSYS:
			HandleRadix( object );
			break;
		case ENGINEER:
			HandleEngineer();
			break;
		default:
			if( dataType == FLOAT )
				x_reg.real = HandleFloatFunction( object, x_reg.real, &handled );
			else if( dataType == COMPLEX )
				x_reg.complex = HandleComplexFunction( object, x_reg.complex, &handled );
			else
				x_reg.integer = HandleIntegerFunction( object, x_reg.integer, &handled );
			break;
	}

	if( handled )
	{
		entrstate = OPWAIT;
		Display();
	}
	
	return handled;
}

static void HandleDigit( short object )
{
	short	digit;
	char	show;

	for( digit = entrstate >= OPWAIT ? curRadix-1 : 9; digit>=0; digit-- )
		if( digits[digit] == object )
			break;

	if( digit < 0 )
		return;

	show = (digit >= 10 ? 'A'-10 : '0')+digit;

	switch( entrstate )
	{
	case WAIT:
	case OPWAIT:
		initDispl( digit, show );
		break;
	case R_NUMPART:
	case SECWAIT:
	case SECPART:
		if( curPos <= maxMant )
		{
			r_mantissa[curPos++] = show;
			if( dataType == FLOAT || dataType == COMPLEX )
			{
				r_mantissa[curPos]   = '.';
				r_mantissa[curPos+1] = '\0';
				if( entrstate == SECWAIT )
					entrstate = SECPART;
			}
			else
				r_mantissa[curPos] = '\0';
		}
		break;
	case I_NUMPART:
		if( curPos <= maxMant )
		{
			i_mantissa[curPos++] = show;
			i_mantissa[curPos]   = '.';
			i_mantissa[curPos+1] = '\0';
		}
		break;

	case MINWAIT:
	case MINPART:
	case R_DECPART:
	case SECDECPART:
		if( curPos <= maxMant )
		{
			r_mantissa[curPos++] = digit+'0';
			r_mantissa[curPos]   = '\0';
			if( entrstate == MINWAIT )
				entrstate = MINPART;
		}
		break;
	case I_DECPART:
		if( curPos <= maxMant )
		{
			i_mantissa[curPos++] = digit+'0';
			i_mantissa[curPos]   = '\0';
		}
		break;

	case R_EXPPART:
		if( curPos < MAX_EXPO+1 )
		{
			r_exponent[curPos++] = digit+'0';
			r_exponent[curPos]   = '\0';
		}
		break;

	case I_EXPPART:
		if( curPos < MAX_EXPO+1 )
		{
			i_exponent[curPos++] = digit+'0';
			i_exponent[curPos]   = '\0';
		}
		break;

	case ROUNDWAIT:
	case FIXPOINTWAIT:
		if( digit >= 10 )
			return;
		if( entrstate == ROUNDWAIT )
		{
			fixPoint = -1;
			roundPrec = digit-1;
		}
		else
		{
			fixPoint = digit;
			roundPrec = -1;
		}
		entrstate = OPWAIT;
		Display();
		return;		/* we do not need to refresh the display */
	}
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void HandleDecPoint( void )
{
	switch( entrstate )
	{
	case WAIT:
	case OPWAIT:
		initDispl( 0, '0' );
		entrstate = R_DECPART;
		curPos += 2;
		RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
		break;
	case R_NUMPART:
		entrstate = R_DECPART;
		curPos++;
		break;
	case I_NUMPART:
		entrstate = I_DECPART;
		curPos++;
		break;
	case SECWAIT:
		r_mantissa[curPos++] = '0';
		r_mantissa[curPos++] = '.';
		r_mantissa[curPos] = 0;
		entrstate = SECDECPART;
		RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
		break;
	case SECPART:
		entrstate = SECDECPART;
		curPos++;
		break;
	}
}

static void HandleExponent( void )
{
	if( dataType != FLOAT && dataType != COMPLEX )
		;
	else
	{
		if( entrstate == WAIT || entrstate == OPWAIT )
		{
			if( dataType == FLOAT )
				x_reg.real = M_PI;
			else if( dataType == COMPLEX )
				x_reg.complex.real = M_PI;

			Display();
		}
		else if( entrstate > WAIT && entrstate < R_EXPPART )
		{
			r_exponent[0] = ' ';
			r_exponent[1] = '0';
			r_exponent[2] = '\0';
			entrstate = R_EXPPART;
			curPos = 1;
			RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
		}
		else if( dataType == COMPLEX )
		{
			if( entrstate == R_EXPPART )
			{
				i_mantissa[0] = ' ';
				i_mantissa[1] = '0';
				i_mantissa[2] = '.';
				i_mantissa[3] = '\0';
				curPos = 1;
				entrstate = I_NUMPART;
				RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
			}
			else if( entrstate == I_NUMPART || entrstate == I_DECPART )
			{
				i_exponent[0] = ' ';
				i_exponent[1] = '0';
				i_exponent[2] = '\0';
				entrstate = I_EXPPART;
				curPos = 1;
				RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
			}
		}
	}
}

static void HandleSign( void )
{
	char *what = NULL;

	if( curRadix == 10 )
	{
		if( (dataType == COMPLEX || dataType == FLOAT || (dataType > FLOAT && dataType & 0x1))
/*
		&&
			(entrstate > WAIT
			|| ( x_reg.real && (entrstate == WAIT || entrstate == OPWAIT))
			)
*/
		)
		{
			if( entrstate == R_NUMPART || entrstate == R_DECPART )
				what = r_mantissa;
			else if( entrstate == R_EXPPART )
				what = r_exponent;
			else if( entrstate == I_NUMPART || entrstate == I_DECPART )
				what = i_mantissa;
			else if( entrstate == I_EXPPART )
				what = i_exponent;

			if( what )
			{
				*what = *what == '-' ? ' ' : '-';
				RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
			}
		}
	}
	else
		HandleFunction( SIGN );
}

static void DisplayAngle( double x )
{
	short		degree;
	unsigned	minute;
	double		second, ipart;
	char		*curPos;

	curPos=r_mantissa;
	*curPos++ = x < 0 ? '-' : ' ';
	x = fabs( x );

	x = modf( x, &ipart ) * 60;
	degree = (short)ipart;
	if( ipart >	(double)INT_MAX )
		Display();
	else
	{
		second = modf( x, &ipart ) * 60;
		minute = (unsigned)ipart;

		/* due to some inexact results */
		while( second >= 59.9999999999 )
		{
			second -= 60.0;
			minute++;
		}
		while( minute >= 60 )
		{
			minute -= 60;
			degree++;
		}
	
		itoa( degree, curPos, 10 );
		curPos +=strlen( curPos );
		*curPos++ = CIRCLE;
		itoa( minute, curPos, 10 );
		curPos +=strlen( curPos );
		*curPos++ = '\'';

		sprintf( curPos, "%.*g", 10, second );
		if( strchr( curPos, 'e' ) || strchr( curPos, '?' ) )
			*curPos = 0;	/* it is too small -> forget it */
	
		*r_exponent = 0;
	}
}

static void HandleAngle( void )
{
	if( dataType != FLOAT )
		return;

	if( invers )
	{
		invers = FALSE;
		DispOff( INVDSP );
		ScanX( NULL );
		DisplayAngle( x_reg.real );
	}
	else switch( entrstate )
	{
	case WAIT:
	case OPWAIT:
		r_mantissa[0] = ' ';
		r_mantissa[1] = '0';
		r_mantissa[2] = CIRCLE;
		r_mantissa[3] = 0;
		curPos = 3;
		*r_exponent = 0;
		entrstate = MINWAIT;
		break;
	case R_NUMPART:
		r_mantissa[curPos++] = CIRCLE;
		r_mantissa[curPos] = 0;
		entrstate = MINWAIT;
		break;
	case MINWAIT:
		r_mantissa[curPos++] = '0';
	case MINPART:
		r_mantissa[curPos++] = '\'';
		r_mantissa[curPos] = 0;
		entrstate = SECWAIT;
		break;
	}
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void HandleClear( void )
{
	initDispl( 0, '0' );
	entrstate = WAIT;
	if( dataType == FLOAT )
		x_reg.real = 0.0;
	else if( dataType == COMPLEX )
	{
		x_reg.complex.real = 0.0;
		x_reg.complex.imagin = 0.0;
	}
	else
		x_reg.integer = 0;

	if( invers )
	{
		invers = FALSE;
		DispOff( INVDSP );
	}
	if( hyperbel )
	{
		hyperbel = FALSE;
		DispOff( HYPDSP );
	}

	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void HandleAllClear( void )
{
	if( invers )
	{
		if( numX )
			DispOff( STATDSP );
		numX   = 0;
		memset( &regressionData, 0, sizeof( regressionData ) );
		regresion = statistic = FALSE;
	}
	stackPos=0;
	curBrack=0;
	if( constOper >= 0 )
	{
		constOper = -1;
		DispOff( CNSTDSP );
	}
	HandleClear();
}

static void HandleOpenBracket( void )
{

	if( curBrack >= MAXBRACK )
		getError( "() Overflow", WARNING );

	initDispl( 0, '0' );
	r_mantissa[0] = '(';
	r_mantissa[1] = ' ';
	itoa( ++curBrack, r_mantissa+2, 10 );

	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void HandleCloseBracket( void )
{
	if( curBrack )
	{
		ScanX( NULL );
		while( stackPos )
		{
			if( stack[stackPos-1].level >= curBrack*MAXLEVEL )
			{
				doOperation( &stack[stackPos-1].operand,
					stack[stackPos-1].operator, &x_reg );
				stackPos--;
			}
			else
				break;
		}
		Display();
		curBrack--;
	}
}

static void HandleEqual( void )
{
	ScanX( NULL );
	if( constOper >= 0 )
	{
		doOperation( &constVal, constOper, &x_reg );
		stackPos = 0;
	}
	else while( stackPos )
	{
		doOperation( &stack[stackPos-1].operand, 
			stack[stackPos-1].operator, &x_reg );
		stackPos--;
	}
	Display();
	curBrack = 0;
}

static void HandleTrgMode( short mode )
{
	if( mode != trgMode )
	{
		char *str;
		switch( mode )
		{
			case DEGREE:	str = "Deg" ;	break ;
			case RADIAL:	str = "Rad" ;	break ;
			case GRADIAL:	str = "Gra" ;	break ;
		}

		trgDsp = str;
		setDspString( TRGDISP, str );
		RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
		trgMode = mode;
	}
}

static void HandleInvers( void )
{
	invers = !invers;
	if( invers )
		DispOn( INVDSP );
	else
		DispOff( INVDSP );
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void HandleHyperbel( void )
{
	hyperbel = !hyperbel;
	if( hyperbel )
		DispOn( HYPDSP );
	else
		DispOff( HYPDSP );
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

static void enterRoundFix( short mode )
{
	ScanX( NULL );
	if( mode == ROUND )
	{
		strcpy( r_mantissa, " RND_" );
		entrstate = ROUNDWAIT;
	}
	else if( mode == FIXPOINT )
	{
		strcpy( r_mantissa, " FIX_" );
		entrstate = FIXPOINTWAIT;
	}
	*r_exponent = 0;
	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

/* --------------------------------------------------------------------- */
/* ----- entry points -------------------------------------------------- */
/* --------------------------------------------------------------------- */

int HandleObject( short object )
{
	int		handled = 1;

#ifndef __TOS__
	signal( SIGFPE, FPUsignal );
#endif

	setjmp( jumpBuffer );

	if( object == ALLCLR
	||  entrstate > ERROR
	||  (entrstate == WARNING && object == CLEAR) )
	{
		switch( object )
		{
			case ZERO:
			case ONE:
			case TWO:
			case THREE:
			case FOUR:
			case FIVE:
			case SIX:
			case SEVEN:
			case EIGHT:
			case NINE:
			case DIGITA:
			case DIGITB:
			case DIGITC:
			case DIGITD:
			case DIGITE:
			case DIGITF:
				HandleDigit( object );
				break;
			case POINT:
				HandleDecPoint();
				break;
			case EXPONENT:
				HandleExponent();
				break;
			case SIGN:
				HandleSign();
				break;
			case ANGLE:
				HandleAngle();
				break;
			case CLEAR:
				HandleClear();
				break;
			case ALLCLR:
				HandleAllClear();
				break;
			case OPNBRACK:
				HandleOpenBracket();
				break;
			case CLSBRACK:
				HandleCloseBracket();
				break;
			case EQUAL:
				HandleEqual();
				break;
			case OROPER:
			case XOROPER:
			case ANDOPER:
			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVISION:
			case MODULOOP:
			case POWER:
			case ROOTOPER:
			case BINOM:
				HandleOperator( object );
				break;
			case DEGREE:
			case RADIAL:
			case GRADIAL:
				HandleTrgMode( object );
				break;
			case LINEREG:
			case LNREG:
			case EXPOREG:
			case POWEREG:
				HandleRegression( object );
				break;
			case INVERS:
				HandleInvers();
				break;
			case HYPERBEL:
				HandleHyperbel();
				break;
			case ROUND:
			case FIXPOINT:
				enterRoundFix( object );
				break;
			default:
				handled = HandleFunction( object );
				break;
		}
		lastObj = object;
	}
	
	return handled;
}

void ChangePanel( void )
{
	if( trgDsp )
		setDspString( TRGDISP, trgDsp );

	if( radixDsp )
		setDspString( RADIXDSP, radixDsp );

	if( typeDsp )
		setDspString( TYPEDISP, typeDsp );

#ifndef	__GEM__		/*
					 *	the TOS-Version crashes at that point, on the other
					 *	hand, DispOn(Off) does it's job for all PANELs
					 */
	if( invers )
		DispOn( INVDSP );
	else
		DispOff( INVDSP );

	if( hyperbel )
		DispOn( HYPDSP );
	else
		DispOff( HYPDSP );

	if( memory )
		DispOn( MEMDSP );
	else
		DispOff( MEMDSP );

	if( statistic )
	{
		setDspString( STATDSP, regresion ? "R" : "S" );
		DispOn( STATDSP );
	}
	else
		DispOff( STATDSP );

	if( regDsp )
		setDspString( REGDSP, regDsp );

	if( constOper >= 0 )
		DispOn( CNSTDSP );
	else
		DispOff( CNSTDSP );

	enableDisableRadix( 16, curRadix );
	enableDisableType( dataType );
#endif

	RefreshDispl( r_mantissa, r_exponent, i_mantissa, i_exponent );
}

void SetValue( const char *display )
{
	if( !setjmp( jumpBuffer ) )
	{
		ScanX( display );
		Display();
	}
}

/*
------------------------------------------------------------------------------
	Restart calculator after a request by another applikation
	(not yet implemented for windows)
------------------------------------------------------------------------------
*/
void RestartCalc( const char *value, short type, short radix )
{
	HandleAllClear();
	HandleDataType( type );
	HandleRadix( -radix );

	SetValue( value );
}

/*
------------------------------------------------------------------------------
	Get current display
	(as a reply or to copy to clipboard)
------------------------------------------------------------------------------
*/
char *GetResult( short *type, short *radix )
{
	static char resultBuffer[64];

	strcpy( resultBuffer, r_mantissa );
	if( *r_exponent )
	{
		strcat( resultBuffer, "e" );
		strcat( resultBuffer, *r_exponent == ' ' ? r_exponent + 1 : r_exponent );
	}

	*type = dataType;
	*radix = curRadix;
	return resultBuffer;
}

/*
	some borland functions not suplied by other compilers
*/
#if !defined( __BORLANDC__ ) && !defined( _MSC_VER )

char *itoa( int i, char *buffer, int radix )
{
	return ltoa( i, buffer, radix );
}

char *ltoa( long i, char *buffer, int radix )
{
	char	*cp, *result, tmpBuffer[64];
	long	modulo, tmpMod;

	result = cp = tmpBuffer;

	if( i<0 && radix==10 )
		*buffer++ = '-';

	while( 1 )
	{
		tmpMod = i % radix;
		modulo = abs( tmpMod );
		if( modulo >= 0 && modulo <= 9 )
			*cp++ = '0' + modulo;
		else if( modulo >= 10 )
			*cp++ = 'A' + modulo-10;

		i /= radix;
		if( !i )
/*v*/		break;
	}

	while( --cp >= tmpBuffer )
		*buffer++ = *cp;

	*buffer = 0;

	return result;
}

char *ultoa( unsigned long i, char *buffer, int radix )
{
	char	*cp, *result, tmpBuffer[64];
	long	modulo;

	result = cp = tmpBuffer;

	while( 1 )
	{
		modulo = i % radix;
		if( modulo >= 0 && modulo <= 9 )
			*cp++ = '0' + modulo;
		else
			*cp++ = 'A' + modulo-10;

		i /= radix;
		if( !i )
/*v*/		break;
	}
	while( --cp >= tmpBuffer )
		*buffer++ = *cp;
		
	*buffer = 0;

	return result;
}

char *strupr( char *buffer )
{
	char	*cp = buffer;
	int		c;

	while( 1 )
	{
		c = *cp;
		if( !c )
			break;
		*cp = toupper( c );
		cp++;
	}

	return buffer;
}
#endif

#if !defined( __BORLANDC__ )
double pow10( double x )
{
	return pow( 10, x );
}
#endif
