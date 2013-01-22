/*
 * PARSEC HEADER: e_defs.h
 */

#ifndef _E_DEFS_H_
#define _E_DEFS_H_


// ----------------------------------------------------------------------------
// GAMESSERVER SUBSYSTEM (SVG) related definitions                            -
// ----------------------------------------------------------------------------


// default HTML file names ----------------------------------------------------
//
#define list_html_file		"clients.html"
#define header_html_file	"g_header.html"
#define footer_html_file	"g_footer.html"

// default log file names -----------------------------------------------------
//
#define log_file			"gamesrv.log"
#define cfg_file			"server.cfg"


// constants ------------------------------------------------------------------
//

// number of remote event lists
#define NUM_RE_LISTS					5

// size of masterserver queue
#define MAX_MASTERS						3

// maximum number of clients in the banlist
#define MAX_BANNED_CLIENTS				128

// number of players that have to be connected for a game to start
#define MIN_PLAYERS_TO_START			2

#include "net_ports.h"

#endif // _E_DEFS_H_


