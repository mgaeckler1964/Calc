/*
		Project:		Calculator
		Module:			CALCUTAT.H
		Description:	Interface definitions
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


/**************************************************************************
	types
**************************************************************************/

typedef enum
{
	NO_PANEL = -2, DEFAULT_PANEL, VIRTUAL_PANEL,		/* must be 0 */
	LARGE_PANEL, SMALL_PANEL, COMPUTER_PANEL, SCIENCE_PANEL
} PANELTYPE;


/**************************************************************************
	macros
**************************************************************************/

#define forAllPanels( panel )	\
					for( panel=LARGE_PANEL; panel<=SCIENCE_PANEL; panel++ )

/**************************************************************************
	Entry points provided by the User Interface Module
**************************************************************************/

void setDspString( short obj, const char *string );
void RefreshDispl(	const char *r_mantissa, const char *r_exponent,
					const char *i_mantissa, const char *i_exponent );

void DispOn ( short obj );
void DispOff( short obj );

void  EnableButton( short obj );
void DisableButton( short obj );

void  EnablePanel( PANELTYPE panel );
void DisablePanel( PANELTYPE panel );

/**************************************************************************
	Entry points provided by the Calculator Module
**************************************************************************/

int HandleObject( short object );
void SetValue( const char *value );
void RestartCalc( const char *value, short type, short radix );
char *GetResult( short *type, short *radix );
void ChangePanel( void );
