/*
 * PARSEC - Modulemanager
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/15 12:25:39 $
 *
 * Orginally written by:
 *   Copyright (c) Clemens Beer        <cbx@parsec.org>   2002
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

// proprietary headers
#include "con_arg.h"
#ifdef PARSEC_SERVER
	#include "con_int_sv.h"
	#include "con_com_sv.h"
	#include "con_main_sv.h"
#else // !PARSEC_SERVER
	#include "con_int.h"
	#include "con_com.h"
	#include "con_main.h"
#endif // !PARSEC_SERVER

// standard ctor --------------------------------------------------------------
//
E_Module::E_Module( const char* pszName )
{
	strncpy( m_szName, pszName, 256 );
	m_szName[ 255 ] = 0;
	TheModuleManager->RegisterModule( this );		
}

// register a module ----------------------------------------------------------
//
void E_ModuleManager::RegisterModule( E_Module* pModule )
{
#ifdef PARSEC_DEBUG
	ASSERT( m_Modules.Find( pModule ) == NULL );
#endif //PARSEC_DEBUG
	m_Modules.AppendTail( pModule );
}

// unregister a module --------------------------------------------------------
//
void E_ModuleManager::UnregisterModule( E_Module* pModule )
{
#ifdef PARSEC_DEBUG
	ASSERT( m_Modules.Find( pModule ) != NULL );
#endif //PARSEC_DEBUG
	m_Modules.Remove( pModule );
}

// init all registered modules ( constructor ) --------------------------------
//
void E_ModuleManager::InitAllModules()
{
	for( LE_Module* entry = m_Modules.GetHead(); entry != NULL; ) {
		ASSERT( entry->m_data != NULL );
		entry->m_data->Init();
		entry = entry->m_pNext;
	}	
}

// kill all registered modules ( destructor ) ---------------------------------
//
void E_ModuleManager::KillAllModules()
{
	for( LE_Module* entry = m_Modules.GetHead(); entry != NULL; ) {
		ASSERT( entry->m_data != NULL );
		entry->m_data->Kill();
		entry = entry->m_pNext;
	}	
}

// console command for listing all registered modules -------------------------
//
PRIVATE
int Cmd_MODULES_LIST( char* dummystr )
{
	ASSERT( dummystr != NULL );
	HANDLE_COMMAND_DOMAIN_SEP( dummystr );

	TheModuleManager->ListAll();

	return TRUE;
}

// list all loaded modules in the console -------------------------------------
//
void E_ModuleManager::ListAll() const
{
	MSGOUT( "# of registered modules: %d", m_Modules.GetNumEntries() );
	int nModule = 0;
	for( LE_Module* entry = m_Modules.GetHead(); entry != NULL; ) {
		ASSERT( entry->m_data != NULL );
		MSGOUT( "%d: %s", nModule, entry->m_data->GetName() );
		entry = entry->m_pNext;
		nModule++;
	}	

}

// register the console commands for managing modules -------------------------
//
void E_ModuleManager::_RegisterConsoleCommand() const
{
	user_command_s regcom;
	memset( &regcom, 0, sizeof( user_command_s ) );

	// register "clbot.start" command
	regcom.command	 = "modules.list";
	regcom.numparams = 0;
	regcom.execute	 = Cmd_MODULES_LIST;
	regcom.statedump = NULL;
	CON_RegisterUserCommand( &regcom );
}


