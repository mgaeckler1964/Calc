/*
		Project:		Calculator
		Module:			gemsend.c
		Description:	Special messages for Atari ST
		Author:			Martin Gäckler
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

#include <stdlib.h>
#include <stdio.h>
#include <aes.h>

typedef enum
{
	FLOAT, BYTE, UBYTE, WORD, UWORD, LONG, ULONG
} DATATYPES;


int main( void )
{
	int		acc_id, apId, topWin;
	int		workBuffer[8];
	char	buffer[128];
	double	res;		

	apId = appl_init();
	acc_id = appl_find( "CALC    " );
	workBuffer[0] = 1024;
	workBuffer[1] = apId;
	workBuffer[2] = 0;
	*((const char **)(workBuffer+3)) = "1234.5678";
	workBuffer[5] = FLOAT;
	workBuffer[6] = 10;
	workBuffer[7] = '+';
	appl_write( acc_id, 16, workBuffer );
	
	do
	{
		evnt_mesag( workBuffer );
	}while( workBuffer[0] != 1026 );

	res = strtod( *(char **)(workBuffer + 3), NULL );
	sprintf( buffer, "[1][|Ergebniss|%s|%lg|%d %d][ OK ]", 
		*(char **)(workBuffer + 3), res, workBuffer[5], workBuffer[6] );
	form_alert( 1, buffer );
	return 0;
}