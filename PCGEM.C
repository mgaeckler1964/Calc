/*
		Project:		Calculator
		Module:			pcgem.c
		Description:	UI for GEM on MS-DOS
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

#include <dos.h>

#include <aes.h>

static int		control[5];
static int		global[12];
static int		int_in[16];
static int		int_out[7];
static void far *addr_in[2];
static void far	*addr_out[1];

typedef struct gemblkstr
{

   int    far		 *gb_pcontrol;
   int    far		 *gb_pglobal;
   int    far  		 *gb_pintin;
   int	  far 		 *gb_pintout;
   void   far  * far *gb_padrin;
   void   far  * far *gb_padrout;

} GEMBLK;

static GEMBLK  gb = {

   control,
   global,
   int_in,
   int_out,
   addr_in,
   addr_out

};

#if 0
WORD gem_if(opcode)
WORD opcode;
{
   WORD i;
   BYTE *pctrl;

   control[0] = opcode;

   pctrl = &ctrl_cnts[(opcode - 10) * 3];
   for(i = 1; i <= CTRL_CNT; i++)
	  control[i] = *pctrl++;

   gem((GEMBLK FAR *)&gb);

   return((WORD) RET_CODE );
}
#endif

/*
 * Turbo-C and MS-C allow to interrupt by a C-function.
 * So let's do this, to avoid inexperienced users asking
 * for how to use the assembler-interface.
 */
static void aesCall(GEMBLK far *ad_g)
{
   union REGS regs;
   struct SREGS segregs;

   regs.x.cx = 200;
   segregs.es = FP_SEG(ad_g);
   regs.x.bx =  FP_OFF(ad_g);

   int86x(0xef,&regs,&regs,&segregs);
}

int appl_init( void )
{
	control[0] = 10;
	control[1] = 0;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	aesCall( &gb );

	return( int_out[0] );
}

int appl_write( int id, int length, void *buff )
{
	control[0] = 12;
	control[1] = 2;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = id;
	int_in[1]  = length;

	addr_in[0] = buff;

	aesCall( &gb );

	return( int_out[0] );
}

int appl_exit( void )
{
	control[0] = 19;
	control[1] = 0;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	aesCall( &gb );

	return( int_out[0] );
}

int evnt_timer( int low, int high )
{
	control[0] = 24;
	control[1] = 2;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = low;
	int_in[1]  = high;

	aesCall( &gb );

	return( int_out[0] );
}

int EvntMulti( EVENT *evnt_struct )
{
	GEMBLK  gb;

	gb.gb_pcontrol = control;
	gb.gb_pglobal  = global;
	gb.gb_pintin   = &evnt_struct->ev_mflags;
	gb.gb_pintout  = &evnt_struct->ev_mwich;
	gb.gb_padrin   = addr_in;
	gb.gb_padrout  = addr_out;

	control[0] = 25;
	control[1] = 16;
	control[2] = 7;
	control[3] = 1;
	control[4] = 0;

	addr_in[0] = evnt_struct->ev_mmgpbuf;

	aesCall( &gb );

	return( evnt_struct->ev_mwich );

}



int form_dial( int mode, int lx, int ly, int lw, int lh,
						 int bx, int by, int bw, int bh )
{
	control[0] = 51;
	control[1] = 9;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = mode;
	int_in[1]  = lx;
	int_in[2]  = ly;
	int_in[3]  = lw;
	int_in[4]  = lh;
	int_in[5]  = bx;
	int_in[6]  = by;
	int_in[7]  = bw;
	int_in[8]  = bh;

	aesCall( &gb );

	return( int_out[0] );
}

int form_alert( int defButton, const char *alertText )
{
	control[0] = 52;
	control[1] = 1;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = defButton;

	addr_in[0] = alertText;

	aesCall( &gb );

	return( int_out[0] );
}

int form_center( OBJECT *tree, int *px, int *py, int *pw, int *ph )
{
	control[0] = 54;
	control[1] = 0;
	control[2] = 5;
	control[3] = 1;
	control[4] = 0;

	addr_in[0] = tree;

	aesCall( &gb );

	*px = int_out[1];
	*py = int_out[2];
	*pw = int_out[3];
	*ph = int_out[4];

	return( int_out[0] );
}

int graf_watchbox( OBJECT *tree, int obj, int instate, int outstate )
{
	control[0] = 75;
	control[1] = 4;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = 0;
	int_in[1]  = obj;
	int_in[2]  = instate;
	int_in[3]  = outstate;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
}

int graf_handle( int *wChar, int *hChar, int *wBox, int *hBox )
{
	control[0] = 77;
	control[1] = 0;
	control[2] = 5;
	control[3] = 0;
	control[4] = 0;

	aesCall( &gb );

	*wChar = int_out[1];
	*hChar = int_out[2];
	*wBox  = int_out[3];
	*hBox  = int_out[4];

	return( int_out[0] );
}


int graf_mouse( int num, MFORM *mouse )
{
	control[0] = 78;
	control[1] = 1;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = num;

	addr_in[0] = mouse;

	aesCall( &gb );

	return( int_out[0] );
}


int menu_bar( OBJECT *menuTree, int showIt )
{
	control[0] = 30;
	control[1] = 1;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = showIt;

	addr_in[0] = menuTree;

	aesCall( &gb );

	return( int_out[0] );
}

int menu_icheck( OBJECT *menuTree, int item, int checkIt )
{
	control[0] = 31;
	control[1] = 2;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = item;
	int_in[1]  = checkIt;

	addr_in[0] = menuTree;

	aesCall( &gb );

	return( int_out[0] );
}

int menu_ienable( OBJECT *menuTree, int item, int enableIt )
{
	control[0] = 32;
	control[1] = 2;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = item;
	int_in[1]  = enableIt;

	addr_in[0] = menuTree;

	aesCall( &gb );

	return( int_out[0] );
}

int menu_tnormal( OBJECT *menuTree, int item, int normalIt )
{
	control[0] = 33;
	control[1] = 2;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = item;
	int_in[1]  = normalIt;

	addr_in[0] = menuTree;

	aesCall( &gb );

	return( int_out[0] );
}

int menu_register( int applId, const char *text )
{
	control[0] = 35;
	control[1] = 1;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = applId;

	addr_in[0] = text;

	aesCall( &gb );

	return( int_out[0] );
}

int objc_add( OBJECT *tree, int parent, int child )
{
	control[0] = 40;
	control[1] = 2;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = parent;
	int_in[1]  = child;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
}

int objc_delete( OBJECT *tree, int delob )
{
	control[0] = 41;
	control[1] = 1;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = delob;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
}

int objc_draw( OBJECT *tree, int obj, int depth,
			   int x, int y, int w, int h )
{
	control[0] = 42;
	control[1] = 6;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0] = obj;
	int_in[1] = depth;
	int_in[2] = x;
	int_in[3] = y;
	int_in[4] = w;
	int_in[5] = h;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
}

int objc_find( OBJECT *tree, int obj, int depth, int x, int y )
{
	control[0] = 43;
	control[1] = 4;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = obj;
	int_in[1]  = depth;
	int_in[2]  = x;
	int_in[3]  = y;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
}

int objc_offset( OBJECT *tree, int obj, int *px, int *py )
{
	control[0] = 44;
	control[1] = 1;
	control[2] = 3;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = obj;

	addr_in[0] = tree;

	aesCall( &gb );

	*px = int_out[1];
	*py = int_out[2];

	return( int_out[0] );
}

int objc_change( OBJECT *tree, int obj, int resvd,
				 int x, int y, int w, int h,
				 int newstate, int redraw )
{
	control[0] = 47;
	control[1] = 8;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0] = obj;
	int_in[1] = resvd;
	int_in[2] = x;
	int_in[3] = y;
	int_in[4] = w;
	int_in[5] = h;
	int_in[6] = newstate;
	int_in[7] = redraw;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
}

int rsrc_load( const char *rscFile )
{
	control[0] = 110;
	control[1] = 0;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	addr_in[0] = rscFile;

	aesCall( &gb );

	return( int_out[0] );
}

int rsrc_free( void )
{
	control[0] = 111;
	control[1] = 0;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	aesCall( &gb );

	return( int_out[0] );
}

int rsrc_gaddr( int type, int id, void *addr )
{
	control[0] = 112;
	control[1] = 2;
	control[2] = 1;
	control[3] = 0;
	control[4] = 1;

	int_in[0] = type;
	int_in[1] = id;

	aesCall( &gb );

	*(void **)addr = addr_out[0];

	return( int_out[0] );
}

int rsrc_obfix( OBJECT *tree, int obj )
{
#if 1
	control[0] = 114;
	control[1] = 1;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	int_in[0]  = obj;

	addr_in[0] = tree;

	aesCall( &gb );

	return( int_out[0] );
#else
	static int	hChar=0, wChar=0;
	int			dummy;

	if( !hChar )
	{
    	form_alert( 1, "[3][gh1][Ok]" );
		graf_handle( &wChar, &hChar, &dummy, &dummy );
		form_alert( 1, "[3][gh2][Ok]" );
	}

	tree[obj].ob_x      *= wChar;
	tree[obj].ob_y      *= hChar;
	tree[obj].ob_width  *= wChar;
	tree[obj].ob_height *= hChar;

	return 1;
#endif
}


int scrp_read( char *clipDir )
{
	control[0] = 80;
	control[1] = 0;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	addr_in[0] = clipDir;

	aesCall( &gb );

	return( int_out[0] );
}

int scrp_write( char *clipDir )
{
	control[0] = 81;
	control[1] = 0;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;

	addr_in[0] = clipDir;

	aesCall( &gb );

	return( int_out[0] );
}

int shel_find( char *fileName )
{
	control[0] = 124;
	control[1] = 0;
	control[2] = 1;
	control[3] = 1;
	control[4] = 0;
	addr_in[0] = fileName;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_create( int kind, int x, int y, int w, int h )
{
	control[0] = 100;
	control[1] = 5;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = kind;
	int_in[1]  = x;
	int_in[2]  = y;
	int_in[3]  = w;
	int_in[4]  = h;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_open( int handle, int x, int y, int w, int h )
{
	control[0] = 101;
	control[1] = 5;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = handle;
	int_in[1]  = x;
	int_in[2]  = y;
	int_in[3]  = w;
	int_in[4]  = h;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_close( int handle )
{
	control[0] = 102;
	control[1] = 1;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = handle;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_delete( int handle )
{
	control[0] = 103;
	control[1] = 1;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = handle;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_get( int handle, int kind,
			  int *px, int *py, int *pw, int *ph )
{
	control[0] = 104;
	control[1] = 2;
	control[2] = 5;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = handle;
	int_in[1]  = kind;

	aesCall( &gb );

	*px = int_out[1];
	*py = int_out[2];
	*pw = int_out[3];
	*ph = int_out[4];

	return( int_out[0] );
}

int wind_set( int handle, int field, int w1, int w2, int w3, int w4 )
{
	control[0] = 105;
	control[1] = 6;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = handle;
	int_in[1]  = field;
	int_in[2]  = w1;
	int_in[3]  = w2;
	int_in[4]  = w3;
	int_in[5]  = w4;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_update( int mode )
{
	control[0] = 107;
	control[1] = 1;
	control[2] = 1;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = mode;

	aesCall( &gb );

	return( int_out[0] );
}

int wind_calc( int handle, int kind,
			   int   x, int   y, int   w, int   h,
			   int *px, int *py, int *pw, int *ph )
{
	control[0] = 108;
	control[1] = 6;
	control[2] = 5;
	control[3] = 0;
	control[4] = 0;

	int_in[0]  = handle;
	int_in[1]  = kind;
	int_in[2]  = x;
	int_in[3]  = y;
	int_in[4]  = w;
	int_in[5]  = h;

	aesCall( &gb );

	*px = int_out[1];
	*py = int_out[2];
	*pw = int_out[3];
	*ph = int_out[4];

	return( int_out[0] );
}