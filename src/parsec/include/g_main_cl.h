#ifndef _G_MAIN_CL_H_
#define _G_MAIN_CL_H_

// ----------------------------------------------------------------------------
//
class G_Extracted
{
protected:
	
	// remember time last kill was achieved, needed for medals 
	refframe_t last_kill_time;
	
protected:
	
	// check kill announcements and medals ----------------------------------------
	//
	void _CheckKillAnnouncements( int nKiller )
	{
		if ( !AUX_DISABLE_KILLS_LEFT_ANNOUNCEMENT ) {
			
			// check if somebody is approaching the kill limit
			int leaderkills = 0;
			
			for ( int pid = 0; pid < MAX_NET_PROTO_PLAYERS; pid++ ) {
				if ( Player_Status[ pid ] == PLAYER_INACTIVE )
					continue;
				if ( Player_KillStat[ pid ] > leaderkills )
					leaderkills = Player_KillStat[ pid ];
			}
			
			if ( AUX_KILL_LIMIT_FOR_GAME_END > 0 ) {
				if ( AUX_KILL_LIMIT_FOR_GAME_END >= leaderkills ) {
					int killsleft = AUX_KILL_LIMIT_FOR_GAME_END - leaderkills;
					AUD_KillsLeft( killsleft );
				}
			}
		}
		
		// check if we downed that ship ourselves
		if ( nKiller == LocalPlayerId ) {
			
			refframe_t timesincekill = SYSs_GetRefFrameCount() - last_kill_time;
			
			if ( timesincekill < AUXDATA_SKILL_AMAZING_TIME ) {
				
				AUD_SkillAmazing();
				
			} else if ( timesincekill < AUXDATA_SKILL_BRILLIANT_TIME ) {
				
				AUD_SkillBrilliant();
				
			} else {
				
				AUD_JimCommenting( JIM_COMMENT_EATTHIS );
			}
			
			last_kill_time = SYSs_GetRefFrameCount();
		}
	}
	
public:

	G_Extracted()
	{
		last_kill_time = 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// UI Code
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	
	
	// UI feedback for player has joined --------------------------------------
	//
	bool_t UI_PlayerJoinedFeedback( RE_PlayerStatus* playerstatus )
	{
		ASSERT( playerstatus != NULL );
		
		// nothing todo for local client
		if ( playerstatus->senderid == LocalPlayerId ) {
			return false;
		}

		// show join message
		if ( !EntryMode ) {
			CopyRemoteName( paste_str, Player_Name[ playerstatus->senderid ] );
			strcat( paste_str, " has joined" );
			ShowMessage( paste_str );
			
			// play sound "player has joined"
			AUD_NewPlayerJoined();
		}
		
		return true;
	}
	
	// UI feedback when player has unjoined -----------------------------------
	//
	bool_t UI_PlayerUnjoinedFeedback( RE_PlayerStatus* playerstatus )
	{
		ASSERT( playerstatus != NULL );

		// nothing todo for local client
		if ( playerstatus->senderid == LocalPlayerId ) {
			return false;
		}

		// show appropriate message
		if ( !EntryMode ) {
			CopyRemoteName( paste_str, Player_Name[ playerstatus->senderid ] );
			if ( playerstatus->params[ 0 ] == USER_EXIT ) {
				
				strcat( paste_str, " has left" );
				
			} else if ( playerstatus->params[ 0 ] == SHIP_DOWNED ) {
				
				strcat( paste_str, " was killed by " );
				strcat( paste_str, Player_Name[ playerstatus->params[ 2 ] - KILLERID_BIAS ] );
			}
			ShowMessage( paste_str );
		}
		
		return true;
	}

	// UI feedback when player connects ---------------------------------------
	//
	bool_t UI_PlayerConnectedFeedback( int playerid )
	{
		return true;
	}

	// UI feedback when player disconnects ------------------------------------
	//
	bool_t UI_PlayerDisconnectedFeedback( int playerid )
	{
		return true;
	}

	// UI feedback when player is removed ( timeout ) -------------------------
	//
	bool_t UI_PlayerWasRemovedFeedback( int playerid )
	{
		// show appropriate message
		if ( !EntryMode ) {
			CopyRemoteName( paste_str, Player_Name[ playerid ] );
			strcat( paste_str, " has been removed" );
			ShowMessage( paste_str );
		}

		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	// GAMECODE
	//////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// the local player is killed -------------------------------------------------
	//
	void GC_LocalPlayerKill( int nClientID_Killer, int weapon )
	{
		// remember the last killer
		CurKiller = nClientID_Killer;
		
		ShipDowned = TRUE;
		// this causes the correct case to be taken in G_MAIN::Gm_CancelGameLoop()
		ExitGameLoop = 2;

		KillDurationWeapons( MyShip );
		OBJ_CreateShipExtras( MyShip );

		//FIXME: different texts for different ways the local player can get killed
		switch ( weapon ) {
			case 0:
			default:
				strcpy( paste_str, "killed by " );
				strcat( paste_str, NET_FetchPlayerName( nClientID_Killer ) );
				break;
		}
		ShowMessage( paste_str );

		// play sample
		AUD_PlayerKilled();
	}

	
	// GameCode called when player unjoins due to ship downed -----------------
	//
	bool_t GC_UnjoinPlayer_ShipDowned( int nClientID_Downed, int nClientID_Killer, int nWeapon )
	{
		ASSERT( ( nClientID_Downed >= 0 ) && ( nClientID_Downed < MAX_NET_PROTO_PLAYERS ) );
		ASSERT( ( nClientID_Killer >= 0 ) && ( nClientID_Killer < MAX_NET_PROTO_PLAYERS ) );
		ASSERT( nClientID_Downed != LocalPlayerId );

		// local player downing MUST be handled differently
		ASSERT( nClientID_Downed != LocalPlayerId );

		ShipObject *shippo = (ShipObject *) Player_Ship[ nClientID_Downed ];
		ASSERT( shippo != NULL );

		if ( shippo != NULL ) {
		
			//NOTE:
			// the ship will be deleted automatically by the
			// explosion (in a certain animation frame).
			
			// start explosion by setting count greater than zero
			if ( shippo->ExplosionCount == 0 ) {
				shippo->ExplosionCount = MAX_EXPLOSION_COUNT;
				
				AUD_ShipDestroyed( shippo );
			}
		}
		
		_CheckKillAnnouncements( nClientID_Killer  );
		
		return true;
	}
	
	// GameCode called when player unjoins due to user exit -------------------
	//
	bool_t GC_UnjoinPlayer_UserExit( RE_PlayerStatus* playerstatus )
	{
		ASSERT( playerstatus != NULL );
		ASSERT( playerstatus->params[ 0 ] == USER_EXIT );
		
		ShipObject *shippo = (ShipObject *) Player_Ship[ playerstatus->senderid ];
		ASSERT( shippo != NULL );
		
		// open stargate
		SFX_CreateStargate( shippo );
		
		//FIXME: [1/27/2002] shouldn't we move this to the engine core
		// kill ship object
		KillSpecificObject( Player_ShipId[ playerstatus->senderid ], PShipObjects );
		
		return true;
	}

};

// UI/GameCode ----------------------------------------------------------------
//
extern G_Extracted TheGameExtraction;
#define GAMECODE( x )	( TheGameExtraction.x )

#endif // _G_MAIN_CL_H_






