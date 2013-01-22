/*
 * PARSEC - Packet Byte-Order Swapping
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:42 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2001-2002
 *   Copyright (c) Andreas Varga       <sid@parsec.org>   1998-2000
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1998-2000
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
#include <stddef.h>
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

// network code config
#include "net_conf.h"

// local module header
#include "net_swap.h"

#ifdef PARSEC_CLIENT
	#include "net_rmev.h"
#endif // PARSEC_CLIENT



// converts a network geomv_t (little-endian float) into native geomv_t -------
//
void Geomv_in( geomv_t *value )
{
	//NOTE:
	// geomv_t vars are sent over the net as floats.
	// thus, incoming floats have to be converted to geomv_t's.

	dword tmp 		= NET_SWAP_32( DW32( *value ) );
	geomv_t tmp2 	= FLOAT_TO_GEOMV( *(float *)&tmp );
	*(dword *)value = DW32( tmp2 );
}


// converts a native geomv_t into network geomv_t (little-endian float) -------
//
void Geomv_out( geomv_t *value )
{
	//NOTE:
	// geomv_t vars are sent over the net as floats.
	// thus, outgoing geomv_t's have to be converted to floats.

	float tmp 	= GEOMV_TO_FLOAT( *value );
	dword tmp2 		= NET_SWAP_32( DW32( tmp ) );
	*(dword *)value = tmp2;
}


// swap endianness of incoming vector -----------------------------------------
//
void Vertex3_in( Vertex3 *vec )
{
	Geomv_in( &vec->X );
	Geomv_in( &vec->Y );
	Geomv_in( &vec->Z );
	vec->VisibleFrame = 0;
}


// swap endianness of outgoing vector -----------------------------------------
//
void Vertex3_out( Vertex3 *vec )
{
	Geomv_out( &vec->X );
	Geomv_out( &vec->Y );
	Geomv_out( &vec->Z );
	vec->VisibleFrame = 0;
}


// swap endianness of incoming matrix -----------------------------------------
//
void Xmatrx_in( Xmatrx matrix )
{
	Geomv_in( &matrix[ 0 ][ 0 ] );
	Geomv_in( &matrix[ 0 ][ 1 ] );
	Geomv_in( &matrix[ 0 ][ 2 ] );
	Geomv_in( &matrix[ 0 ][ 3 ] );
	Geomv_in( &matrix[ 1 ][ 0 ] );
	Geomv_in( &matrix[ 1 ][ 1 ] );
	Geomv_in( &matrix[ 1 ][ 2 ] );
	Geomv_in( &matrix[ 1 ][ 3 ] );
	Geomv_in( &matrix[ 2 ][ 0 ] );
	Geomv_in( &matrix[ 2 ][ 1 ] );
	Geomv_in( &matrix[ 2 ][ 2 ] );
	Geomv_in( &matrix[ 2 ][ 3 ] );
}


// swap endianness of outgoing matrix -----------------------------------------
//
void Xmatrx_out( Xmatrx matrix )
{
	Geomv_out( &matrix[ 0 ][ 0 ] );
	Geomv_out( &matrix[ 0 ][ 1 ] );
	Geomv_out( &matrix[ 0 ][ 2 ] );
	Geomv_out( &matrix[ 0 ][ 3 ] );
	Geomv_out( &matrix[ 1 ][ 0 ] );
	Geomv_out( &matrix[ 1 ][ 1 ] );
	Geomv_out( &matrix[ 1 ][ 2 ] );
	Geomv_out( &matrix[ 1 ][ 3 ] );
	Geomv_out( &matrix[ 2 ][ 0 ] );
	Geomv_out( &matrix[ 2 ][ 1 ] );
	Geomv_out( &matrix[ 2 ][ 2 ] );
	Geomv_out( &matrix[ 2 ][ 3 ] );
}


// swap endianness of incoming ShipCreateInfo structure -----------------------
//
void ShipCreateInfo_in( ShipCreateInfo *createinfo )
{
	createinfo->ShipIndex = NET_SWAP_32( createinfo->ShipIndex );
	Xmatrx_in( createinfo->ObjPosition );
}


// swap endianness of outgoing ShipCreateInfo structure -----------------------
//
void ShipCreateInfo_out( ShipCreateInfo *createinfo )
{
	createinfo->ShipIndex = NET_SWAP_32( createinfo->ShipIndex );
	Xmatrx_out( createinfo->ObjPosition );
}


// swap endianness of incoming ShipRemInfo structure --------------------------
//
void SWAP_ShipRemInfo_in( ShipRemInfo *reminfo )
{
	ASSERT( reminfo != NULL );

	Xmatrx_in( reminfo->ObjPosition );

	reminfo->CurDamage	= NET_SWAP_16( reminfo->CurDamage );
	reminfo->CurShield	= NET_SWAP_16( reminfo->CurShield );
	reminfo->CurSpeed	= NET_SWAP_32( reminfo->CurSpeed );
	reminfo->CurYaw		= NET_SWAP_32( reminfo->CurYaw );
	reminfo->CurPitch	= NET_SWAP_32( reminfo->CurPitch );
	reminfo->CurRoll	= NET_SWAP_32( reminfo->CurRoll );

	Geomv_in( &reminfo->CurSlideHorz );
	Geomv_in( &reminfo->CurSlideVert );
}


// swap endianness of outgoing ShipRemInfo structure --------------------------
//
void SWAP_ShipRemInfo_out( ShipRemInfo *reminfo )
{
	ASSERT( reminfo != NULL );

	Xmatrx_out( reminfo->ObjPosition );

	reminfo->CurDamage	= NET_SWAP_16( reminfo->CurDamage );
	reminfo->CurShield	= NET_SWAP_16( reminfo->CurShield );
	reminfo->CurSpeed	= NET_SWAP_32( reminfo->CurSpeed );
	reminfo->CurYaw		= NET_SWAP_32( reminfo->CurYaw );
	reminfo->CurPitch	= NET_SWAP_32( reminfo->CurPitch );
	reminfo->CurRoll	= NET_SWAP_32( reminfo->CurRoll );

	Geomv_out( &reminfo->CurSlideHorz );
	Geomv_out( &reminfo->CurSlideVert );
}


// swap endianness of incoming ServerInfo structure ---------------------------
//
void ServerInfo_in( ServerInfo *svinfo )
{
	ASSERT( svinfo != NULL );
}


// swap endianness of outgoing ServerInfo structure ---------------------------
//
void ServerInfo_out( ServerInfo *svinfo )
{
	ASSERT( svinfo != NULL );
}

// swap endianness of all REs in RE list --------------------------------------
//
void NET_RmEvList_Swap( RE_Header* relist, int incoming )
{
	ASSERT( relist != NULL );

	// change byte order of remote event list
	while ( relist->RE_Type != RE_EMPTY ) {

		switch ( relist->RE_Type ) {

			case RE_DELETED:
				// ignore
				break;

			case RE_CREATEOBJECT:
				{
					RE_CreateObject *re_co = (RE_CreateObject *) relist;
					re_co->ObjectClass 	= NET_SWAP_16( re_co->ObjectClass );
					re_co->HostObjId 	= NET_SWAP_32( re_co->HostObjId );
					if ( incoming ) {
						Xmatrx_in( re_co->ObjPosition );
					} else {
						Xmatrx_out( re_co->ObjPosition );
					}
					re_co->Flags = NET_SWAP_32( re_co->Flags );
				}
				break;

			case RE_CREATELASER:
				{
					RE_CreateLaser *re_cl  = (RE_CreateLaser *) relist;
					re_cl->ObjectClass 	= NET_SWAP_16( re_cl->ObjectClass );
					re_cl->HostObjId 	= NET_SWAP_32( re_cl->HostObjId );
					if ( incoming ) {
						Xmatrx_in( re_cl->ObjPosition );
						Vertex3_in( &re_cl->DirectionVec );
					} else {
						Xmatrx_out( re_cl->ObjPosition );
						Vertex3_out( &re_cl->DirectionVec );
					}
				}
				break;

			case RE_CREATEMISSILE:
				{
					RE_CreateMissile *re_cm  = (RE_CreateMissile *) relist;
					re_cm->ObjectClass 	= NET_SWAP_16( re_cm->ObjectClass );
					re_cm->HostObjId 	= NET_SWAP_32( re_cm->HostObjId );
					if ( incoming ) {
						Xmatrx_in( re_cm->ObjPosition );
						Vertex3_in( &re_cm->DirectionVec );
					} else {
						Xmatrx_out( re_cm->ObjPosition );
						Vertex3_out( &re_cm->DirectionVec );
					}
					re_cm->TargetHostObjId = NET_SWAP_32( re_cm->TargetHostObjId );
				}
				break;

			case RE_PARTICLEOBJECT:
				{
					RE_ParticleObject *re_po = (RE_ParticleObject *) relist;
					re_po->ObjectType = NET_SWAP_16( re_po->ObjectType );
					if ( incoming ) {
						Vertex3_in( &re_po->Origin );
					} else {
						Vertex3_out( &re_po->Origin );
					}
				}
				break;
                        case RE_CREATEMINE:
                                {
                                       RE_CreateMine *re_ce  = (RE_CreateMine *) relist;
                                       re_ce->ExtraIndex       = NET_SWAP_16( re_ce->ExtraIndex );
                                       re_ce->HostObjId        = NET_SWAP_32( re_ce->HostObjId );
                                       if ( incoming ) {
                                               Xmatrx_in( re_ce->ObjPosition );
                                       } else {
                                               Xmatrx_out( re_ce->ObjPosition );
                                       }
                                
                                }
			case RE_CREATEEXTRA:
				{
					RE_CreateExtra *re_ce  = (RE_CreateExtra *) relist;
					re_ce->ExtraIndex	= NET_SWAP_16( re_ce->ExtraIndex );
					re_ce->HostObjId 	= NET_SWAP_32( re_ce->HostObjId );
					if ( incoming ) {
						Xmatrx_in( re_ce->ObjPosition );
					} else {
						Xmatrx_out( re_ce->ObjPosition );
					}
				}
				break;

			case RE_KILLOBJECT:
				{
					RE_KillObject *re_ko = (RE_KillObject *) relist;
					re_ko->HostObjId = NET_SWAP_32( re_ko->HostObjId );
				}
				break;

			case RE_SENDTEXT:
				// nothing to swap
				break;

			case RE_PLAYERNAME:
				// nothing to swap
				break;

			case RE_PLAYERLIST:
				{
					RE_PlayerList *re_pl = (RE_PlayerList *) relist;
					
					for ( int pid = 0; pid < MAX_NET_UDP_PEER_PLAYERS; pid++ )
						if ( incoming )
							ShipCreateInfo_in( &re_pl->ShipInfoTable[ pid ] );
						else
							ShipCreateInfo_out( &re_pl->ShipInfoTable[ pid ] );
					}
				break;

			case RE_CONNECTQUEUE:
				{
					RE_ConnectQueue *re_cq = (RE_ConnectQueue *) relist;
					re_cq->NumRequests = NET_SWAP_16( re_cq->NumRequests );
				}
				break;

			case RE_WEAPONSTATE:
				{
					RE_WeaponState *re_ws = (RE_WeaponState *) relist;
					re_ws->WeaponMask = NET_SWAP_32( re_ws->WeaponMask );
	//				re_ws->Specials   = NET_SWAP_32( re_ws->Specials );
					re_ws->CurEnergy  = NET_SWAP_32( re_ws->CurEnergy );
				}
				break;

			case RE_STATESYNC:
				{
					RE_StateSync *re_ss = (RE_StateSync *) relist;
					re_ss->StateKey   = NET_SWAP_16( re_ss->StateKey );
					re_ss->StateValue = NET_SWAP_32( re_ss->StateValue );
				}
				break;

			case RE_CREATESWARM:
				{
					RE_CreateSwarm *re_cs = (RE_CreateSwarm *) relist;
					if ( incoming )
						Vertex3_in( &re_cs->Origin );
					else
						Vertex3_out( &re_cs->Origin );
					re_cs->TargetHostObjId	= NET_SWAP_32( re_cs->TargetHostObjId );
					re_cs->RandSeed 		= NET_SWAP_32( re_cs->RandSeed );
				}
				break;

			case RE_CREATEEMP:
				{
					RE_CreateEmp *re_ce = (RE_CreateEmp *) relist;
					re_ce->Upgradelevel	= NET_SWAP_32( re_ce->Upgradelevel );
					re_ce->SenderId = NET_SWAP_32( re_ce->SenderId);
				}
				break;
			case RE_OWNERSECTION:
				{
					RE_OwnerSection* re_os = (RE_OwnerSection*) relist;
					re_os->owner = NET_SWAP_32( re_os->owner );
				}
				break;

			case RE_PLAYERSTATUS:
				{
					RE_PlayerStatus* playerstatus = (RE_PlayerStatus*) relist;

					playerstatus->player_status = NET_SWAP_16( playerstatus->player_status );
					playerstatus->senderid 		= NET_SWAP_32( playerstatus->senderid );
					playerstatus->objectindex   = NET_SWAP_32( playerstatus->objectindex );
					playerstatus->RefFrame 		= NET_SWAP_32( playerstatus->RefFrame );
				}
				break;

			case RE_PLAYERANDSHIPSTATUS:
				{
					RE_PlayerAndShipStatus* pas_status = (RE_PlayerAndShipStatus*) relist;

					pas_status->player_status = NET_SWAP_16( pas_status->player_status );
					pas_status->senderid 	  = NET_SWAP_32( pas_status->senderid );
					pas_status->objectindex   = NET_SWAP_32( pas_status->objectindex );
					pas_status->RefFrame 	  = NET_SWAP_32( pas_status->RefFrame );

					pas_status->CurDamage	= NET_SWAP_16( pas_status->CurDamage );
					pas_status->CurShield	= NET_SWAP_16( pas_status->CurShield );
					pas_status->CurSpeed	= NET_SWAP_32( pas_status->CurSpeed );
					pas_status->CurYaw		= NET_SWAP_32( pas_status->CurYaw );
					pas_status->CurPitch	= NET_SWAP_32( pas_status->CurPitch );
					pas_status->CurRoll		= NET_SWAP_32( pas_status->CurRoll );

					// swap endianness of ShipRemInfo 
					if ( incoming ) {
						Xmatrx_in( pas_status->ObjPosition );
						Geomv_in ( &pas_status->CurSlideHorz );
						Geomv_in ( &pas_status->CurSlideVert );
					} else {
						Xmatrx_out( pas_status->ObjPosition );
						Geomv_out ( &pas_status->CurSlideHorz );
						Geomv_out ( &pas_status->CurSlideVert );
					}

					pas_status->CurEnergy 	  = NET_SWAP_32( pas_status->CurEnergy );
				}
				break;
			case RE_KILLSTATS:
				// nothing to swap
				break;
			case RE_GAMESTATE:
				{
					RE_GameState* gamestate = (RE_GameState*) relist;
					gamestate->GameTime = NET_SWAP_32( gamestate->GameTime );
				}
				break;

			case RE_COMMANDINFO:
				// nothing to swap
				break;

			case RE_CLIENTINFO:
				// nothing to swap
				break;

			case RE_CREATEEXTRA2:
				{
					RE_CreateExtra2* re_ce2  = (RE_CreateExtra2 *) relist;
					re_ce2->ExtraIndex	= NET_SWAP_16( re_ce2->ExtraIndex );
					re_ce2->HostObjId 	= NET_SWAP_32( re_ce2->HostObjId );
					if ( incoming ) {
						Xmatrx_in( re_ce2->ObjPosition );
						Vertex3_in( (Vertex3*)&re_ce2->DriftVec );
					} else {
						Xmatrx_out( re_ce2->ObjPosition );
						Vertex3_out( (Vertex3*)&re_ce2->DriftVec );
					}
					re_ce2->DriftTimeout = NET_SWAP_32( re_ce2->DriftTimeout );
				}
				break;

			case RE_IPV4SERVERINFO:
				{
					RE_IPv4ServerInfo* re_si = (RE_IPv4ServerInfo*) relist;
					// for re_si->node NO SWAPPING is needed
					re_si->serverid = NET_SWAP_32( re_si->serverid );
					re_si->xpos		= NET_SWAP_32( re_si->xpos  );
					re_si->ypos		= NET_SWAP_32( re_si->ypos  );
				}
				break;

			case RE_SERVERLINKINFO:
				{
					RE_ServerLinkInfo* re_sli = (RE_ServerLinkInfo*) relist;
					re_sli->flags     = NET_SWAP_16( re_sli->flags );
					re_sli->serverid1 = NET_SWAP_16( re_sli->serverid1 );
					re_sli->serverid2 = NET_SWAP_16( re_sli->serverid2 );
				}
				break;

			case RE_MAPOBJECT:
				{
					RE_MapObject* re_mo = (RE_MapObject*) relist;
					re_mo->map_objectid = NET_SWAP_16( re_mo->map_objectid );
					re_mo->xpos			= NET_SWAP_32( re_mo->xpos );
					re_mo->ypos			= NET_SWAP_32( re_mo->ypos );
					re_mo->w			= NET_SWAP_32( re_mo->w );
					re_mo->h			= NET_SWAP_32( re_mo->h );
				}
				break;
			case RE_STARGATE:
				{
					RE_Stargate* re_stg = (RE_Stargate*) relist;

					// for re_stg->destination_node NO SWAPPING is needed
					re_stg->serverid		= NET_SWAP_16( re_stg->serverid );			
					re_stg->rotspeed		= NET_SWAP_32( re_stg->rotspeed );			
					re_stg->radius			= NET_SWAP_32( re_stg->radius );				
					re_stg->actdistance		= NET_SWAP_32( re_stg->actdistance );		
					re_stg->partvel			= NET_SWAP_32( re_stg->partvel );			
					re_stg->modulrad1		= NET_SWAP_32( re_stg->modulrad1 );			
					re_stg->modulrad2		= NET_SWAP_32( re_stg->modulrad2 );			
					re_stg->numpartactive	= NET_SWAP_16( re_stg->numpartactive );		
					re_stg->actcyllen		= NET_SWAP_16( re_stg->actcyllen );			
					re_stg->modulspeed		= NET_SWAP_16( re_stg->modulspeed );			

					if ( incoming ) {
						Geomv_in( &re_stg->pos[ 0 ] );
						Geomv_in( &re_stg->pos[ 1 ] );
						Geomv_in( &re_stg->pos[ 2 ] );

						Geomv_in( &re_stg->dir[ 0 ] );
						Geomv_in( &re_stg->dir[ 1 ] );
						Geomv_in( &re_stg->dir[ 2 ] );
					} else {
						Geomv_out( &re_stg->pos[ 0 ] );
						Geomv_out( &re_stg->pos[ 1 ] );
						Geomv_out( &re_stg->pos[ 2 ] );

						Geomv_out( &re_stg->dir[ 0 ] );
						Geomv_out( &re_stg->dir[ 1 ] );
						Geomv_out( &re_stg->dir[ 2 ] );
					}
				}
				break;
				//break;
				//break;
				//break;
				//break;
			default:
				MSGOUT( "NET_RmEvList_Swap(): unknown remote event (%d).", relist->RE_Type );
		}

		// advance to next event in list
		//MSGOUT("%d, %d ", NET_RmEvGetSize( relist ), relist->RE_BlockSize);
		ASSERT( ( relist->RE_BlockSize == RE_BLOCKSIZE_INVALID ) ||
				( relist->RE_BlockSize == NET_RmEvGetSize( relist ) ) );
		relist = (RE_Header *) ( (char *) relist + NET_RmEvGetSize( relist ) );
	}
}

