/*
 * inp_keyn.h
 *
 *  Created on: Nov 3, 2012
 *      Author: jasonw
 */

#ifndef INP_KEYN_H_
#define INP_KEYN_H_

// C++ STL map
#include <map>

#include "keycodes.h"

// "constructor" for the key_names hash table
// (STL maps can only use initializer lists in C++11)
static std::map<int, const char *> create_key_names()
{
	std::map<int, const char *> m;
	
	m[MKC_NIL]			= "None";
	m[MKC_ESCAPE]		= "Escape";
	m[MKC_1]			= "1";
	m[MKC_2]			= "2";
	m[MKC_3]			= "3";
	m[MKC_4]			= "4";
	m[MKC_5]			= "5";
	m[MKC_6]			= "6";
	m[MKC_7]			= "7";
	m[MKC_8]			= "8";
	m[MKC_9]			= "9";
	m[MKC_0]			= "0";
	m[MKC_MINUS]		= "-";
	m[MKC_EQUALS]		= "=";
	m[MKC_BACKSPACE]	= "Backspace";
	m[MKC_TAB]			= "Tab";
	m[MKC_Q]			= "Q";
	m[MKC_W]			= "W";
	m[MKC_E]			= "E";
	m[MKC_R]			= "R";
	m[MKC_T]			= "T";
	m[MKC_U]			= "U";
	m[MKC_I]			= "I";
	m[MKC_O]			= "O";
	m[MKC_P]			= "P";
	m[MKC_LBRACKET]		= "[";
	m[MKC_RBRACKET]		= "]";
	m[MKC_BACKSLASH]	= "\\";
	m[MKC_A]			= "A";
	m[MKC_S]			= "S";
	m[MKC_D]			= "D";
	m[MKC_F]			= "F";
	m[MKC_G]			= "G";
	m[MKC_H]			= "H";
	m[MKC_J]			= "J";
	m[MKC_K]			= "K";
	m[MKC_L]			= "L";
	m[MKC_SEMICOLON]	= ";";
	m[MKC_APOSTROPHE]	= "'";
	m[MKC_ENTER]		= "Enter";
	m[MKC_LSHIFT]		= "Left Shift";
	m[MKC_RSHIFT]		= "Right Shift";
	m[MKC_Z]			= "Z";
	m[MKC_X]			= "X";
	m[MKC_C]			= "C";
	m[MKC_V]			= "V";
	m[MKC_B]			= "B";
	m[MKC_N]			= "N";
	m[MKC_M]			= "M";
	m[MKC_COMMA]		= ",";
	m[MKC_PERIOD]		= ".";
	m[MKC_SLASH]		= "/";
	m[MKC_LCONTROL]		= "Left Control";
	m[MKC_RCONTROL]		= "Right Control";
	m[MKC_LALT]			= "Left Alt";
	m[MKC_RALT]			= "Right Alt";
	m[MKC_CURSORUP]		= "Up Arrow";
	m[MKC_CURSORLEFT]	= "Left Arrow";
	m[MKC_CURSORDOWN]	= "Down Arrow";
	m[MKC_CURSORRIGHT]	= "Right Arrow";
	m[MKC_SPACE]		= "Spacebar";
	m[MKC_F1]			= "F1";
	m[MKC_F2]			= "F2";
	m[MKC_F3]			= "F3";
	m[MKC_F4]			= "F4";
	m[MKC_F5]			= "F5";
	m[MKC_F6]			= "F6";
	m[MKC_F7]			= "F7";
	m[MKC_F8]			= "F8";
	m[MKC_F9]			= "F9";
	m[MKC_F10]			= "F10";
	m[MKC_F11]			= "F11";
	m[MKC_F12]			= "F12";
	m[MKC_TILDE]		= "Tilde";
	m[MKC_HOME]			= "Home";
	m[MKC_INSERT]		= "Insert";
	m[MKC_DELETE]		= "Delete";
	m[MKC_PAGEUP]		= "Page Up";
	m[MKC_PAGEDOWN]		= "Page Down";
	
	return m;
}

// create the hash table
static std::map<int, const char*> key_names = create_key_names();


#endif /* INP_KEYN_H_ */
