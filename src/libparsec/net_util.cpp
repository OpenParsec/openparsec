/*
 * PARSEC - Utility Functions - shared
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:42 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-2000
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
#include <stdarg.h>

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
//#include "vid_defs.h"

// drawing subsystem
//#include "d_bmap.h"
//#include "d_font.h"

// network code config
#include "net_conf.h"

// local module header
#include "net_util.h"

// proprietary module headers
//#include "e_color.h"
//#include "net_swap.h"
//#include "sys_bind.h"


// lookup table for fast CRC32 calculation ------------------------------------
//
static const dword crc32lookup[ 256 ] =
{ 
	0x00000000ul, 0x77073096ul, 0xEE0E612Cul, 0x990951BAul,
	0x076DC419ul, 0x706AF48Ful, 0xE963A535ul, 0x9E6495A3ul,
	0x0EDB8832ul, 0x79DCB8A4ul, 0xE0D5E91Eul, 0x97D2D988ul,
	0x09B64C2Bul, 0x7EB17CBDul, 0xE7B82D07ul, 0x90BF1D91ul,
	0x1DB71064ul, 0x6AB020F2ul, 0xF3B97148ul, 0x84BE41DEul,
	0x1ADAD47Dul, 0x6DDDE4EBul, 0xF4D4B551ul, 0x83D385C7ul,
	0x136C9856ul, 0x646BA8C0ul, 0xFD62F97Aul, 0x8A65C9ECul,
	0x14015C4Ful, 0x63066CD9ul, 0xFA0F3D63ul, 0x8D080DF5ul,
	0x3B6E20C8ul, 0x4C69105Eul, 0xD56041E4ul, 0xA2677172ul,
	0x3C03E4D1ul, 0x4B04D447ul, 0xD20D85FDul, 0xA50AB56Bul,
	0x35B5A8FAul, 0x42B2986Cul, 0xDBBBC9D6ul, 0xACBCF940ul,
	0x32D86CE3ul, 0x45DF5C75ul, 0xDCD60DCFul, 0xABD13D59ul,
	0x26D930ACul, 0x51DE003Aul, 0xC8D75180ul, 0xBFD06116ul,
	0x21B4F4B5ul, 0x56B3C423ul, 0xCFBA9599ul, 0xB8BDA50Ful,
	0x2802B89Eul, 0x5F058808ul, 0xC60CD9B2ul, 0xB10BE924ul,
	0x2F6F7C87ul, 0x58684C11ul, 0xC1611DABul, 0xB6662D3Dul,
	0x76DC4190ul, 0x01DB7106ul, 0x98D220BCul, 0xEFD5102Aul,
	0x71B18589ul, 0x06B6B51Ful, 0x9FBFE4A5ul, 0xE8B8D433ul,
	0x7807C9A2ul, 0x0F00F934ul, 0x9609A88Eul, 0xE10E9818ul,
	0x7F6A0DBBul, 0x086D3D2Dul, 0x91646C97ul, 0xE6635C01ul,
	0x6B6B51F4ul, 0x1C6C6162ul, 0x856530D8ul, 0xF262004Eul,
	0x6C0695EDul, 0x1B01A57Bul, 0x8208F4C1ul, 0xF50FC457ul,
	0x65B0D9C6ul, 0x12B7E950ul, 0x8BBEB8EAul, 0xFCB9887Cul,
	0x62DD1DDFul, 0x15DA2D49ul, 0x8CD37CF3ul, 0xFBD44C65ul,
	0x4DB26158ul, 0x3AB551CEul, 0xA3BC0074ul, 0xD4BB30E2ul,
	0x4ADFA541ul, 0x3DD895D7ul, 0xA4D1C46Dul, 0xD3D6F4FBul,
	0x4369E96Aul, 0x346ED9FCul, 0xAD678846ul, 0xDA60B8D0ul,
	0x44042D73ul, 0x33031DE5ul, 0xAA0A4C5Ful, 0xDD0D7CC9ul,
	0x5005713Cul, 0x270241AAul, 0xBE0B1010ul, 0xC90C2086ul,
	0x5768B525ul, 0x206F85B3ul, 0xB966D409ul, 0xCE61E49Ful,
	0x5EDEF90Eul, 0x29D9C998ul, 0xB0D09822ul, 0xC7D7A8B4ul,
	0x59B33D17ul, 0x2EB40D81ul, 0xB7BD5C3Bul, 0xC0BA6CADul,
	0xEDB88320ul, 0x9ABFB3B6ul, 0x03B6E20Cul, 0x74B1D29Aul,
	0xEAD54739ul, 0x9DD277AFul, 0x04DB2615ul, 0x73DC1683ul,
	0xE3630B12ul, 0x94643B84ul, 0x0D6D6A3Eul, 0x7A6A5AA8ul,
	0xE40ECF0Bul, 0x9309FF9Dul, 0x0A00AE27ul, 0x7D079EB1ul,
	0xF00F9344ul, 0x8708A3D2ul, 0x1E01F268ul, 0x6906C2FEul,
	0xF762575Dul, 0x806567CBul, 0x196C3671ul, 0x6E6B06E7ul,
	0xFED41B76ul, 0x89D32BE0ul, 0x10DA7A5Aul, 0x67DD4ACCul,
	0xF9B9DF6Ful, 0x8EBEEFF9ul, 0x17B7BE43ul, 0x60B08ED5ul,
	0xD6D6A3E8ul, 0xA1D1937Eul, 0x38D8C2C4ul, 0x4FDFF252ul,
	0xD1BB67F1ul, 0xA6BC5767ul, 0x3FB506DDul, 0x48B2364Bul,
	0xD80D2BDAul, 0xAF0A1B4Cul, 0x36034AF6ul, 0x41047A60ul,
	0xDF60EFC3ul, 0xA867DF55ul, 0x316E8EEFul, 0x4669BE79ul,
	0xCB61B38Cul, 0xBC66831Aul, 0x256FD2A0ul, 0x5268E236ul,
	0xCC0C7795ul, 0xBB0B4703ul, 0x220216B9ul, 0x5505262Ful,
	0xC5BA3BBEul, 0xB2BD0B28ul, 0x2BB45A92ul, 0x5CB36A04ul,
	0xC2D7FFA7ul, 0xB5D0CF31ul, 0x2CD99E8Bul, 0x5BDEAE1Dul,
	0x9B64C2B0ul, 0xEC63F226ul, 0x756AA39Cul, 0x026D930Aul,
	0x9C0906A9ul, 0xEB0E363Ful, 0x72076785ul, 0x05005713ul,
	0x95BF4A82ul, 0xE2B87A14ul, 0x7BB12BAEul, 0x0CB61B38ul,
	0x92D28E9Bul, 0xE5D5BE0Dul, 0x7CDCEFB7ul, 0x0BDBDF21ul,
	0x86D3D2D4ul, 0xF1D4E242ul, 0x68DDB3F8ul, 0x1FDA836Eul,
	0x81BE16CDul, 0xF6B9265Bul, 0x6FB077E1ul, 0x18B74777ul,
	0x88085AE6ul, 0xFF0F6A70ul, 0x66063BCAul, 0x11010B5Cul,
	0x8F659EFFul, 0xF862AE69ul, 0x616BFFD3ul, 0x166CCF45ul,
	0xA00AE278ul, 0xD70DD2EEul, 0x4E048354ul, 0x3903B3C2ul,
	0xA7672661ul, 0xD06016F7ul, 0x4969474Dul, 0x3E6E77DBul,
	0xAED16A4Aul, 0xD9D65ADCul, 0x40DF0B66ul, 0x37D83BF0ul,
	0xA9BCAE53ul, 0xDEBB9EC5ul, 0x47B2CF7Ful, 0x30B5FFE9ul,
	0xBDBDF21Cul, 0xCABAC28Aul, 0x53B39330ul, 0x24B4A3A6ul,
	0xBAD03605ul, 0xCDD70693ul, 0x54DE5729ul, 0x23D967BFul,
	0xB3667A2Eul, 0xC4614AB8ul, 0x5D681B02ul, 0x2A6F2B94ul,
	0xB40BBE37ul, 0xC30C8EA1ul, 0x5A05DF1Bul, 0x2D02EF8Dul, 
};


// fast CRC calculation -------------------------------------------------------
//
dword NET_CalcCRC( void* buf, size_t len )
{
	dword crc32 = ~0;

	for( size_t i = 0; i < len; i++ ) { 

		byte lookup = (byte)( (crc32 ^ (((byte *)buf)[ i ])) & 0xff );

		crc32 = ( (crc32 >> 8) & 0x00fffffful ) ^ crc32lookup[ lookup ];
	}
	crc32 = ~crc32;

	return crc32;
}


// encrypt payload section of packet ------------------------------------------
//
void NET_EncryptData( void* packet, size_t psize )
{
	//NOTE:
	// this function is used by NETs_HandleOutPacket{_DEMO} after packing, 
	// swapping and CRC calculation
	
	//FIXME: evt. we change the Protocol field, to indicate, that we have an
	//       encrypted packet
	//strcpy( packet->Signature, net_cryp_signature );
	
	extern float fsin_tab[];
	
	// leave signature unencoded
	byte *block = (byte *) packet;
	
	//FIXME: exclude MessageId from encryption and use it as a basepointer
	//       into the fsin_tab

	for ( size_t epos = 0; epos < psize; epos++ ) {
		
		// lsbs of mantissa have highest frequency
		byte xorkey = (byte)DW32( fsin_tab[ epos ] ) & 0xff;
		block[ epos ] ^= xorkey;
	}
}


// decrypt payload section of packet ------------------------------------------
//
void NET_DecryptData( void* packet, size_t psize )
{
	//NOTE:
	// this function is used by NETs_HandleInPacket{_DEMO} before CRC is checked
	// and packet is swapped and unpacked
	
	//FIXME: evt. we must check, whether this is an encrypted packet at all
	//if ( strcmp( packet->Signature, net_cryp_signature ) != 0 ) {
	//	return;
	//}
	
	// reset signature
	//strcpy( packet->Signature, net_game_signature );
	
	extern float fsin_tab[];
	
	// skip signature
	byte *block = (byte *) packet;
	
	for ( size_t epos = 0; epos < psize; epos++ ) {
		
		// lsbs of mantissa have highest frequency
		byte xorkey = (byte)DW32( fsin_tab[ epos ] ) & 0xff;
		block[ epos ] ^= xorkey;
	}
}

#ifdef PACKETINFO_WRITING_AVAILABLE


// fp to which PrInf_ functions write -----------------------------------------
//
FILE *dest_fp_PacketInfo;


// output wrapper function ----------------------------------------------------
//
int NET_PacketInfo( const char *bstr, ... )
{
	va_list	ap;
	va_start( ap, bstr );

	// simply write to text-file
	int rc = vfprintf( dest_fp_PacketInfo, bstr, ap );

	va_end( ap );
	return rc;
}


// print remote-event info ----------------------------------------------------
//
void PrInf_CREATEOBJECT( RE_Header *relist )
{
	RE_CreateObject *re_co = (RE_CreateObject *) relist;

	NET_PacketInfo( ";-- objclass=%d, hostobjid=%d, flags=%0x\n", re_co->ObjectClass, re_co->HostObjId, re_co->Flags );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_CREATELASER( RE_Header *relist )
{
	RE_CreateLaser *re_cl = (RE_CreateLaser *) relist;

	NET_PacketInfo( ";-- objclass=%d, hostobjid=%d\n", re_cl->ObjectClass, re_cl->HostObjId );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_CREATEMISSILE( RE_Header *relist )
{
	RE_CreateMissile *re_cm = (RE_CreateMissile *) relist;

	NET_PacketInfo( ";-- objclass=%d, hostobjid=%d, target=%d\n", re_cm->ObjectClass, re_cm->HostObjId, re_cm->TargetHostObjId );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_CREATEEXTRA( RE_Header *relist )
{
	RE_CreateExtra *re_ce = (RE_CreateExtra *) relist;

	NET_PacketInfo( ";-- extraindex=%d, hostobjid=%d\n", re_ce->ExtraIndex, re_ce->HostObjId );
}

// print remote-event info ----------------------------------------------------
//
void PrInf_CREATEEXTRA2( RE_Header *relist )
{
	RE_CreateExtra2 *re_ce = (RE_CreateExtra2 *) relist;

	NET_PacketInfo( ";-- extraindex=%d, hostobjid=%d\n", re_ce->ExtraIndex, re_ce->HostObjId );
}

void PrInf_CREATEMINE( RE_Header *relist )
{
        RE_CreateMine *re_ce = (RE_CreateMine *) relist;

        NET_PacketInfo( ";-- extraindex=%d, hostobjid=%d\n", re_ce->ExtraIndex, re_ce->HostObjId );
}

// print remote-event info ----------------------------------------------------
//
void PrInf_IPV4SERVERINFO( RE_Header *relist )
{
	RE_IPv4ServerInfo* re_si = (RE_IPv4ServerInfo *) relist;

	//FIXME: include "net_util_sv.h"
	//NET_PacketInfo( ";-- node: %s\n", NODE_Print( (node_t*)re_ipsi->node ) );
}

// print remote-event info ----------------------------------------------------
//
void PrInf_SERVERLINKINFO( RE_Header *relist )
{
	RE_ServerLinkInfo* re_sli = (RE_ServerLinkInfo *) relist;

	NET_PacketInfo( ";-- index1=%d, index2=%d\n", re_sli->serverid1, re_sli->serverid2 );
}

// print remote-event info ----------------------------------------------------
//
void PrInf_MAPOBJECT( RE_Header *relist )
{
	RE_MapObject* re_mo = (RE_MapObject *) relist;

	NET_PacketInfo( ";-- map_objectid=%d, name=%s, xpos=%d, ypos=%d, w=%d, h=%d, texname=%s\n", 
		re_mo->map_objectid, re_mo->name, 
		re_mo->xpos, re_mo->ypos, 
		re_mo->w, re_mo->h, 
		re_mo->texname );
}

// print remote-event info ----------------------------------------------------
//
void PrInf_STARGATE( RE_Header *relist )
{
	RE_Stargate* re_stg = (RE_Stargate *) relist;

	//FIXME: more information
	NET_PacketInfo( ";-- serverid=%d\n", re_stg->serverid );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_KILLOBJECT( RE_Header *relist )
{
	RE_KillObject *re_ko = (RE_KillObject *) relist;

	NET_PacketInfo( ";-- list=%d, flags=%d, hostobjid=%d\n", re_ko->ListId, re_ko->Flags, re_ko->HostObjId );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_SENDTEXT( RE_Header *relist )
{
	RE_SendText *re_st = (RE_SendText *) relist;

	NET_PacketInfo( ";-- text=\"%s\"\n", re_st->TextStart );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_PLAYERNAME( RE_Header *relist )
{
	RE_PlayerName *re_pn = (RE_PlayerName *) relist;

	NET_PacketInfo( ";-- name=\"%s\"\n", re_pn->PlayerName );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_PARTICLEOBJECT( RE_Header *relist )
{
	RE_ParticleObject *re_po = (RE_ParticleObject *) relist;

	NET_PacketInfo( ";-- pobjtype=%d\n", re_po->ObjectType );
}


// print remote-event info ----------------------------------------------------
//
void PrInf_PLAYERLIST( RE_Header *relist )
{
	//TODO:
}


// print remote-event info ----------------------------------------------------
//
void PrInf_CONNECTQUEUE( RE_Header *relist )
{
	//TODO:
}


// print remote-event info ----------------------------------------------------
//
void PrInf_WEAPONSTATE( RE_Header *relist )
{
	//TODO:
}


// print remote-event info ----------------------------------------------------
//
void PrInf_STATESYNC( RE_Header *relist )
{
	//TODO:
}


// print remote-event info ----------------------------------------------------
//
void PrInf_CREATESWARM( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_CREATEEMP( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_OWNERSECTION( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_PLAYERSTATUS( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_PLAYERANDSHIPSTATUS( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_KILLSTATS( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_GAMESTATE( RE_Header *relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_COMMANDINFO( RE_Header* relist )
{
	//TODO:
}

// print remote-event info ----------------------------------------------------
//
void PrInf_CLIENTINFO( RE_Header* relist )
{
	//TODO:
}

// list of remote-event names -------------------------------------------------
//
static const char *re_names[] = {

	"RE_EMPTY",
	"RE_DELETED",

	"RE_CREATEOBJECT",
	"RE_CREATELASER",
	"RE_CREATEMISSILE",
	"RE_CREATEEXTRA",
	"RE_KILLOBJECT",
	"RE_SENDTEXT",
	"RE_PLAYERNAME",
	"RE_PARTICLEOBJECT",
	"RE_PLAYERLIST",
	"RE_CONNECTQUEUE",
	"RE_WEAPONSTATE",
	"RE_STATESYNC",
	"RE_CREATESWARM",
	"RE_CREATEEMP",
	"RE_OWNERSECTION",
	"RE_PLAYERSTATUS",
	"RE_PLAYERANDSHIPSTATUS",
	"RE_KILLSTATS",
	"RE_GAMESTATE",
	"RE_COMMANDINFO",
	"RE_CLIENTINFO",
	"RE_CREATEEXTRA2",
	"RE_IPV4SERVERINFO",
	"RE_SERVERLINKINFO",
	"RE_MAPOBJECT",
	"RE_STARGATE",
	"RE_CREATEMINE",
};


// list of remote-event info printing functions -------------------------------
//
static void (*re_funcs[])(RE_Header*) = {

	NULL,
	NULL,

	PrInf_CREATEOBJECT,
	PrInf_CREATELASER,
	PrInf_CREATEMISSILE,
	PrInf_CREATEEXTRA,
	PrInf_KILLOBJECT,
	PrInf_SENDTEXT,
	PrInf_PLAYERNAME,
	PrInf_PARTICLEOBJECT,
	PrInf_PLAYERLIST,
	PrInf_CONNECTQUEUE,
	PrInf_WEAPONSTATE,
	PrInf_STATESYNC,
	PrInf_CREATESWARM,
	PrInf_CREATEEMP,
	PrInf_OWNERSECTION,
	PrInf_PLAYERSTATUS,
	PrInf_PLAYERANDSHIPSTATUS,
	PrInf_KILLSTATS,
	PrInf_GAMESTATE,
	PrInf_COMMANDINFO,
	PrInf_CLIENTINFO,
	PrInf_CREATEEXTRA2,
	PrInf_IPV4SERVERINFO,
	PrInf_SERVERLINKINFO, 
	PrInf_MAPOBJECT, 
	PrInf_STARGATE,
};


#ifndef PARSEC_MASTER

// write verbose RE list info as a comment to recording file ------------------
//
void NET_RmEvList_WriteInfo( FILE* fp, RE_Header* relist )
{
	ASSERT( fp		!= NULL );
	ASSERT( relist	!= NULL );

	// process remote event list
	while ( relist->RE_Type != RE_EMPTY ) {
		
		if ( relist->RE_Type < RE_NUMEVENTS ) {
			
			NET_PacketInfo( ";-- rmev=%d (%s)\n", relist->RE_Type, re_names[ relist->RE_Type ] );
			if ( re_funcs[ relist->RE_Type ] != NULL )
				(*re_funcs[ relist->RE_Type ])( relist );
			
		} else {
			NET_PacketInfo( ";** unknown remote-event: %d\n",	relist->RE_Type );
		}
		
		// advance to next event
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == NET_RmEvGetSize( relist ) ) );

		relist = (RE_Header *) ( (char *) relist + NET_RmEvGetSize( relist ) );
	}
}

#endif // !PARSEC_MASTER

#endif // PACKETINFO_WRITING_AVAILABLE



