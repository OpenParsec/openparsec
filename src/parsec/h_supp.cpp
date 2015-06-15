/*
 * PARSEC - Supporting Graphics Code
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:37 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2001
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */ 

// C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compilation flags/debug support
#include "config.h"
#include "debug.h"

// general definitions
#include "general.h"
#include "objstruc.h"

// global externals
#include "globals.h"

// subsystem headers
#include "net_defs.h"
#include "sys_defs.h"

// drawing subsystem
#include "d_font.h"

// local module header
#include "h_supp.h"

// proprietary module headers
#include "con_aux.h"
#include "e_supp.h"
#include "h_cockpt.h"
#include "m_button.h"
#include "g_bot_cl.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char	paste_str[ PASTE_STR_LEN + 1 ];

// top y position of message area ---------------------------------------------
//
int Msg_ScreenTop;				// also used by H_DRWHUD.C


// calculate address of hud charset of specific color -------------------------
//
char *CalcHudCharAddress( int no )
{
	ASSERT( ( no >= 0 ) && ( no < 6 ) );

	int cno = HUD_CHARSETNO;

	char * adr = CharsetInfo[ cno ].charsetpointer + no * 2 * 8 * CharsetInfo[ cno ].srcwidth * 4;

	return adr;
}


// currently set color of hud charset -----------------------------------------
//
PUBLIC int		cur_hud_char_color = 0;


// offset table into hud charset for different colors -------------------------
//
PUBLIC char*	Hud_Char_ColOfs[ 6 ];	// set by H_DRWHUD::InitHudDisplay()


// set color of hud charset ---------------------------------------------------
//
void SetHudCharColor( int no )
{
	ASSERT( ( no >= 0 ) && ( no < NUM_HUD_CHARSET_COLORS ) );
	CharsetDataBase = Hud_Char_ColOfs[ no ];
	cur_hud_char_color = no;
}


// default message area properties --------------------------------------------
//
#define MESSAGE_AREA_SIZE		4
#define MESSAGE_LIFETIME		4000

#if ( MESSAGE_AREA_SIZE > MAX_SCREENMESSAGES )
	#error "default MESSAGE_AREA_SIZE invalid!"
#endif

#if ( PASTE_STR_LEN < MAX_MESSAGELEN )
	#error "PASTE_STR_LEN too short!"
#endif

static int message_area_size = MESSAGE_AREA_SIZE;
static int message_lifetime  = MESSAGE_LIFETIME;


// number of currently active texfont messages --------------------------------
//
static int num_texfont_msgs = 0;


// free everything associated with a texfont message and remove from table ----
//
PRIVATE
void FreeTexfontMessage( int msgid )
{
	ASSERT( msgid < num_texfont_msgs );

	FREEMEM( TexfontMsgs[ msgid ].message );
	FREEMEM( TexfontMsgs[ msgid ].itexfont );

	if ( msgid < ( num_texfont_msgs - 1 ) ) {
		memmove( &TexfontMsgs[ msgid ], &TexfontMsgs[ msgid + 1 ],
			sizeof( texfontmsg_s ) * ( num_texfont_msgs - msgid - 1 ) );
	}

	num_texfont_msgs--;
}


// display and maintain texfont messages --------------------------------------
//
PRIVATE
void MaintainTexfontMessages()
{
	for ( int msgid = 0; msgid < num_texfont_msgs; msgid++ ) {

		// get current alpha (manual clamping is mandatory)
		TexfontMsgs[ msgid ].itexfont->Vtxs[ 0 ].A =
			( TexfontMsgs[ msgid ].curalpha < 255.0f ) ?
			(byte)TexfontMsgs[ msgid ].curalpha : 255;

		// display message
		D_TexfontWrite( TexfontMsgs[ msgid ].message, TexfontMsgs[ msgid ].itexfont );

		if ( TexfontMsgs[ msgid ].starttime > 0 ) {
			TexfontMsgs[ msgid ].starttime -= CurScreenRefFrames;
		}

		if ( TexfontMsgs[ msgid ].starttime <= 0 ) {

			// maintain alpha fading
			TexfontMsgs[ msgid ].curalpha -= TexfontMsgs[ msgid ].decalpha * CurScreenRefFrames;
			if ( TexfontMsgs[ msgid ].curalpha < 0.0f ) {
				TexfontMsgs[ msgid ].curalpha = TexfontMsgs[ msgid ].refalpha;
				TexfontMsgs[ msgid ].maxpulses--;
			}

			// maintain lifetime
			TexfontMsgs[ msgid ].lifetime -= CurScreenRefFrames;

			// remove message if lifetime spent or pulse count reached
			if ( ( TexfontMsgs[ msgid ].lifetime < 0 ) ||
				( TexfontMsgs[ msgid ].maxpulses <= 0 ) ) {
				FreeTexfontMessage( msgid );
			}
		}
	}
}


// show (make active) a texfont message ---------------------------------------
//
void ShowTexfontMessage( texfontmsg_s *msg )
{
	ASSERT( msg != NULL );

	//NOTE:
	// messages of the same type will replace each other,
	// whereas messages of different types will coexist.

	// search for msg to replace (according to type)
	int msgid = 0;
	for ( msgid = 0; msgid < num_texfont_msgs; msgid++ ) {
		if ( TexfontMsgs[ msgid ].msgtype == msg->msgtype ) {
			FreeTexfontMessage( msgid );
			msgid = num_texfont_msgs;
			break;
		}
	}

	// guard against table size overflow
	if ( msgid >= MAX_TEXFONT_MESSAGES ) {
		return;
	}

	// copy basic info
	memcpy( &TexfontMsgs[ msgid ], msg, sizeof( texfontmsg_s ) );

	// refalpha may be set explicitly to allow for oversaturated alpha starts
	if ( msg->itexfont->Vtxs[ 0 ].A == 0 ) {
		TexfontMsgs[ msgid ].curalpha = TexfontMsgs[ msgid ].refalpha;
	} else {
		TexfontMsgs[ msgid ].refalpha = msg->itexfont->Vtxs[ 0 ].A;
		TexfontMsgs[ msgid ].curalpha = msg->itexfont->Vtxs[ 0 ].A;
	}

	// must make a copy of message string
	TexfontMsgs[ msgid ].message = (char *) ALLOCMEM( strlen( msg->message ) + 1 );
	if ( TexfontMsgs[ msgid ].message == NULL )
		OUTOFMEM( 0 );
	strcpy( TexfontMsgs[ msgid ].message, msg->message );

	// must make a copy of itertexfont structure
	TexfontMsgs[ msgid ].itexfont = (IterTexfont *) ALLOCMEM( sizeof( IterTexfont ) );
	if ( TexfontMsgs[ msgid ].itexfont == NULL )
		OUTOFMEM( 0 );
	memcpy( TexfontMsgs[ msgid ].itexfont, msg->itexfont, sizeof( IterTexfont ) );

	// one more active message
	num_texfont_msgs++;
}


// message ring buffer control ------------------------------------------------
//
static int head_message = 0;
static int tail_message = -1;


// invalidate all messages (clear message area) -------------------------------
//
inline
void InvalidateMessages()
{
	// set extraordinary indexes to denote "empty"
	head_message = 0;
	tail_message = -1;
}


// reset message area ---------------------------------------------------------
//
void ResetMessageArea()
{
	//NOTE:
	// this is also called from G_BOOT::InitGlobals().

	InvalidateMessages();

	for ( int curmsg = 0; curmsg < MAX_SCREENMESSAGES; curmsg++ ) {
		MsgMaint[ curmsg ].lifetimecount = 0;
	}
}


// write messages and maintain their timing -----------------------------------
//
void MaintainMessages()
{
	// second type of messages (texfont)
	MaintainTexfontMessages();

	// check message area configuration (bounds)
	if ( ( AUXDATA_MESSAGE_AREA_SIZE < 1 ) ||
		 ( AUXDATA_MESSAGE_AREA_SIZE > MAX_SCREENMESSAGES ) )
		AUXDATA_MESSAGE_AREA_SIZE = MESSAGE_AREA_SIZE;
	if ( ( AUXDATA_MESSAGE_LIFETIME < 100 ) ||
		 ( AUXDATA_MESSAGE_LIFETIME > 10000 ) )
		AUXDATA_MESSAGE_LIFETIME = MESSAGE_LIFETIME;

	// ensure config changes work correctly
	if ( message_area_size != AUXDATA_MESSAGE_AREA_SIZE ) {
		message_area_size = AUXDATA_MESSAGE_AREA_SIZE;
		ResetMessageArea();
	}
	if ( message_lifetime != AUXDATA_MESSAGE_LIFETIME  ) {
		message_lifetime = AUXDATA_MESSAGE_LIFETIME;
		ResetMessageArea();
	}

	// select font
	int msgfont, lineadv, yposofs;
	if ( AUX_USE_BIG_FONT_IN_MESSAGE_AREA ) {
		msgfont = MSG_CHARSETNO;
		lineadv = CharsetInfo[ msgfont ].height + 2;
		yposofs = 0;
	} else {
		msgfont = HUD_CHARSETNO;
		lineadv = CharsetInfo[ msgfont ].height + 1;
		yposofs = -5;
	}

	D_SetWStrContext( CharsetInfo[ msgfont ].charsetpointer,
					  CharsetInfo[ msgfont ].geompointer,
					  NULL,
					  CharsetInfo[ msgfont ].width,
					  CharsetInfo[ msgfont ].height );

	// alter font color
	if ( msgfont == HUD_CHARSETNO  )
		SetHudCharColor( 2 );

	// messages should not cover left and right panel if on
	int pwidth = AUX_DONT_DRAW_BLUEPRINTS ?
					16 : BitmapInfo[ BM_SHIP1_BLUEPRINT ].width * 2 + 16;
	int maxlen = 0;
	if(headless_bot) {
		maxlen = 80;
	} else {
		maxlen = ( ( Screen_Width - pwidth ) / CharsetInfo[ msgfont ].width );
	}
	if ( maxlen > MAX_MESSAGELEN )
		maxlen = MAX_MESSAGELEN;
	ASSERT( maxlen > 0 );

	int curmsg = head_message;

	int ypos = 0;
	if ( InFloatingMenu ) {
		ypos = 84 * NUM_MENU_ITEMS;
	} else {
		ypos = ( AUX_DRAW_COCKPIT && !ObjCameraActive &&
				 ( AUX_DRAW_COCKPIT_ICONBAR || AUX_DRAW_COCKPIT_ICONBAR_BACKGROUND ) ) ?
				cockpit_messagearea_yoffs : ( Msg_ScreenTop + yposofs );
	}
	ASSERT( ypos >= 0 );

	// determine whether actual drawing disabled
	int drawingdisabled = InFloatingMenu ?
		AUX_DISABLE_FLOATING_MENU_DRAWING : AUX_HUD_DISABLE_USER_DISPLAYS;
	if ( AUX_DISABLE_MESSAGE_AREA ) {
		drawingdisabled = TRUE;
	}

	// display message area
	for ( int curl = 0; curl < message_area_size; curl++, ypos+=lineadv ) {

		if ( MsgMaint[ curmsg ].lifetimecount > 0 ) {

			if ( !drawingdisabled ) {

				if ( !AUX_DISABLE_MSGAREA_IF_CONSOLE_OPEN || ( ConsoleSliding == 0 ) ) {

					strncpy( paste_str, MsgMaint[ curmsg ].message, maxlen );
					paste_str[ maxlen ] = 0;

					int xpos = 0;
					if ( InFloatingMenu ) {
						xpos = ( 325 - strlen( paste_str ) *
								 CharsetInfo[ msgfont ].width ) / 2;
					} else {
						xpos = ( Screen_Width - strlen( paste_str ) *
								 CharsetInfo[ msgfont ].width ) / 2;
					}
					if ( xpos < 0 ) {
						xpos = 0;
					}

					if ( ypos < ( Screen_Height - CharsetInfo[ msgfont ].height ) ) {
						D_WriteString( paste_str, xpos, ypos );
					}
				}
			}

			MsgMaint[ curmsg ].lifetimecount -= CurScreenRefFrames;
			if ( MsgMaint[ curmsg ].lifetimecount <= 0 ) {

				// [ (head==tail) means only one message ]
				if ( head_message == tail_message ) {

					// set valid "emtpy" state
					InvalidateMessages();

				} else {

					// advance head to next message
					if ( ++head_message >= message_area_size ) {
						head_message = 0;
					}
				}
			}
		}

		if ( ++curmsg >= message_area_size ) {
			curmsg = 0;
		}
	}
}


// add message to message structure -------------------------------------------
//
void ShowMessage( const char *msg )
{
	//NOTE:
	// the message area is a ring-buffer that uses
	// all entries. no entry is used as sentinel in
	// the case when the ring-buffer is full.
	// (when using only head and tail this is only
	// possible if a non-valid index is used as flag.)
	//
	// [ (head==0 tail==-1) means no messages     ]
	// [ (head==tail) means only one message      ]
	// [ tail one pos before head means area full ]

	ASSERT( msg != NULL );

	if ( tail_message != -1 ) {

		// advance tail
		if ( ++tail_message >= message_area_size ) {
			tail_message = 0;
		}

		// [ tail one pos before head means area full ]

		// eat from head if too many entries
		if ( tail_message == head_message ) {
			if ( ++head_message >= message_area_size ) {
				head_message = 0;
			}
		}

	} else {

		// [ (head==0 tail==-1) means no messages ]

		// first message in area (was empty before)
		ASSERT( head_message == 0 );
		tail_message = 0;
	}

	// append message at tail
	strncpy( MsgMaint[ tail_message ].message, msg, MAX_MESSAGELEN );
	MsgMaint[ tail_message ].message[ MAX_MESSAGELEN ]	= 0;
	MsgMaint[ tail_message ].lifetimecount				= message_lifetime;
}


// write frame rate to top left of screen -------------------------------------
//
void WriteFrameRate()
{
	char str[ 10 ];
	int  pos = 9;

	str[ pos ] = 0;

	int value = FrameRate;
	do {
		int digit = value % 10;
		str[ --pos ] = digit + '0';
	} while ( ( value /= 10 ) > 0 );

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );
	SetHudCharColor( 4 );
	D_WriteString( str + pos, 4, 6 );
}


// write game time to bottom left of screen -----------------------------------
//
void WriteGameTime()
{
	//TODO:
	// move this into NET_UTIL.C?

	if ( GAME_NO_SERVER() )
		return;
	if(headless_bot)
		return;


	char str[ 10 ];
	int  pos = 9;
	int ypos = Screen_Height - CharsetInfo[ HUD_CHARSETNO ].height - 6;
	int xpos = 4;

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );
	SetHudCharColor( 4 );

	if ( CurServerName != NULL ) {
		D_WriteString( CurServerName, xpos, ypos );
		xpos += ( strlen( CurServerName ) + 1 ) * CharsetInfo[ HUD_CHARSETNO ].width;
	}

	if ( !GAME_RUNNING() )
		return;

	str[ pos ] = 0;

	int minutes = CurGameTime / 60;
	int seconds = CurGameTime % 60;

	if ( minutes > 99 )
		minutes = 99;

	do {
		int digit = seconds % 10;
		str[ --pos ] = digit + '0';
	} while ( ( seconds /= 10 ) > 0 );

	if ( ( CurGameTime % 60 ) < 10 )
		str[ --pos ] = '0';

	str[ --pos ] = ':';

	do {
		int digit = minutes % 10;
		str[ --pos ] = digit + '0';
	} while ( ( minutes /= 10 ) > 0 );

	if ( ( CurGameTime / 60 ) < 10 )
		str[ --pos ] = '0';

	D_WriteString( str + pos, xpos, ypos );
}


// write current server ping time to top right corner of screen ---------------
//
void WriteServerPing()
{
	if ( GAME_NO_SERVER() )
		return;

	if ( !AUX_DRAW_SERVER_PING )
		return;

	if ( CurServerPing < 0 )
		return;

	char str[ 10 ];

	snprintf( str, 9, "%d", CurServerPing );

	int xpos = Screen_Width - strlen( str ) * CharsetInfo[ HUD_CHARSETNO ].width - 6;
	int ypos = 6;

	D_SetWStrContext( CharsetInfo[ HUD_CHARSETNO ].charsetpointer,
					  CharsetInfo[ HUD_CHARSETNO ].geompointer,
					  NULL,
					  CharsetInfo[ HUD_CHARSETNO ].width,
					  CharsetInfo[ HUD_CHARSETNO ].height );
	SetHudCharColor( 4 );

	D_WriteString( str, xpos, ypos );
}

// write the extra collection message -----------------------------------------
//
void WriteExtraCollectMessage( char* pszText )
{
//	ShowMessage( text );

	texfont_s *texfont = FetchTexfont( "impact" );
	if ( texfont != NULL ) {

		IterTexfont itexfont;
		itexfont.itertype  = iter_texrgba | iter_alphablend;
		itexfont.raststate = rast_texclamp;
		itexfont.rastmask  = rast_nomask;
		itexfont.texfont   = texfont;

		itexfont.Vtxs[ 0 ].U = 1.0f;
		itexfont.Vtxs[ 0 ].V = 1.0f;
		itexfont.Vtxs[ 0 ].R = 255;
		itexfont.Vtxs[ 0 ].G = 255;
		itexfont.Vtxs[ 0 ].B = 255;
		itexfont.Vtxs[ 0 ].A = 180;

		int width = D_TexfontGetWidth( pszText, &itexfont );

		itexfont.Vtxs[ 0 ].X = ( Screen_Width - width ) / 2;
		itexfont.Vtxs[ 0 ].Y = 100;

		texfontmsg_s msg;
		msg.message   = pszText;
		msg.itexfont  = &itexfont;
		msg.msgtype   = 0;

//		msg.starttime = 1500;
//		msg.lifetime  = 5000;
//		msg.maxpulses = 1;
//		msg.decalpha  = 0.5f;

		msg.starttime = 0;
		msg.lifetime  = 5000;
		msg.maxpulses = 4;
		msg.decalpha  = 0.8f;

		ShowTexfontMessage( &msg );
	}
}

