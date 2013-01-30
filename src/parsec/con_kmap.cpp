/*
 * PARSEC - Keymapping Commands
 *
 * $Author: uberlinuxguy $ - $Date: 2004/09/26 03:43:34 $
 *
 * Orginally written by:
 *   Copyright (c) Markus Hadwiger     <msh@parsec.org>   1996-1999
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
#include "inp_defs.h"

// local module header
#include "con_kmap.h"

// proprietary module headers
#include "con_ext.h"
#include "con_list.h"
#include "con_main.h"
#include "con_std.h"
#include "keycodes.h"



// generic string paste area --------------------------------------------------
//
#define PASTE_STR_LEN 255
static char paste_str[ PASTE_STR_LEN + 1 ];


// string constants -----------------------------------------------------------
//
static char keymap_syntax[] 	= "syntax: key <keyname> <command>";
static char bind_syntax[]		= "syntax: bind <keyname> <command>";
static char invalid_key[]		= "invalid keyname.";
static char invalid_func[]		= "invalid key function specifier.";
static char invalid_mkc[]		= "invalid make code specifier.";
static char invalid_akc[]		= "invalid key code specifier.";
static char too_many_keyadds[]	= "too many additional key assignments.";


// current keyboard command mappings (console commands) -----------------------
//
keycom_s key_com_mappings[ KEY_ADDITIONS_MAX ];


// key names for additional mappings ------------------------------------------
//
textkeymap_s key_com_names[] = {

	{ AKC_ESCAPE,			"akc_escape"        	},
	{ AKC_SPACE,			"akc_space"         	},
	{ AKC_ENTER,			"akc_enter"         	},
	{ AKC_CURSORUP, 		"akc_cursorup"      	},
	{ AKC_CURSORDOWN,		"akc_cursordown"        },
	{ AKC_CURSORLEFT,		"akc_cursorleft"        },
	{ AKC_CURSORRIGHT,		"akc_cursorright"       },
	{ AKC_BACKSPACE,		"akc_backspace"         },
	{ AKC_INSERT,			"akc_insert"            },
	{ AKC_DELETE,			"akc_delete"            },
	{ AKC_HOME, 			"akc_home"              },
	{ AKC_END,				"akc_end"               },
	{ AKC_PAGEUP,			"akc_pageup"            },
	{ AKC_PAGEDOWN, 		"akc_pagedown"          },
	{ AKC_CAPSLOCK,			"akc_capslock"		    },
	{ AKC_TILDE,	        "akc_tilde"			    },

	{ AKC_MINUS,            "akc_minus"             },
	{ AKC_EQUALS,           "akc_equals"            },
	{ AKC_TAB,              "akc_tab"               },
	{ AKC_LBRACKET,         "akc_lbracket"          },
	{ AKC_RBRACKET,         "akc_rbracket"          },
	{ AKC_SEMICOLON,        "akc_semicolon"         },
	{ AKC_APOSTROPHE,       "akc_apostrophe"        },
	{ AKC_GRAVE,            "akc_grave"             },

	{ AKC_LSHIFT,           "akc_lshift"            },
	{ AKC_RSHIFT,           "akc_rshift"            },
	{ AKC_LCONTROL,         "akc_lcontrol"          },
	{ AKC_RCONTROL,         "akc_rcontrol"          },
	{ AKC_LALT,             "akc_lalt"              },
	{ AKC_RALT,             "akc_ralt"              },

	{ AKC_COMMA,            "akc_comma"             },
	{ AKC_PERIOD,           "akc_period"            },
	{ AKC_SLASH,            "akc_slash"             },

	{ AKC_BACKSLASH,        "akc_backslash"         },
	{ AKC_GERARROWS,		"akc_gerarrows"			},

	{ AKC_NUMPADSLASH,      "akc_numpadslash"		},
	{ AKC_NUMPADSTAR,       "akc_numpadstar"		},
	{ AKC_NUMPADMINUS,      "akc_numpadminus"       },
	{ AKC_NUMPADPLUS,       "akc_numpadplus"        },
	{ AKC_NUMPADPERIOD,     "akc_numpadperiod"      },
	{ AKC_NUMLOCK,          "akc_numlock"         	},
	{ AKC_SCROLL,           "akc_scroll"         	},

	{ AKC_NUMPAD0,          "akc_numpad0"         	},
	{ AKC_NUMPAD1,          "akc_numpad1"         	},
	{ AKC_NUMPAD2,          "akc_numpad2"         	},
	{ AKC_NUMPAD3,          "akc_numpad3"         	},
	{ AKC_NUMPAD4,          "akc_numpad4"         	},
	{ AKC_NUMPAD5,          "akc_numpad5"         	},
	{ AKC_NUMPAD6,          "akc_numpad6"         	},
	{ AKC_NUMPAD7,          "akc_numpad7"         	},
	{ AKC_NUMPAD8,          "akc_numpad8"         	},
	{ AKC_NUMPAD9,          "akc_numpad9"         	},

	{ AKC_LWIN,             "akc_lwin"         		},
	{ AKC_RWIN,             "akc_rwin"         		},
	{ AKC_APPS,             "akc_apps"         		},

	{ AKC_ENTER_GRAY,		"akc_enter_gray"        },
	{ AKC_CURSORUP_GRAY, 	"akc_cursorup_gray"     },
	{ AKC_CURSORDOWN_GRAY,	"akc_cursordown_gray"   },
	{ AKC_CURSORLEFT_GRAY,	"akc_cursorleft_gray"   },
	{ AKC_CURSORRIGHT_GRAY,	"akc_cursorright_gray"  },
	{ AKC_BACKSPACE_GRAY,	"akc_backspace_gray"    },
	{ AKC_INSERT_GRAY,		"akc_insert_gray"       },
	{ AKC_DELETE_GRAY,		"akc_delete_gray"       },
	{ AKC_HOME_GRAY, 		"akc_home_gray"         },
	{ AKC_END_GRAY,			"akc_end_gray"          },
	{ AKC_PAGEUP_GRAY,		"akc_pageup_gray"       },
	{ AKC_PAGEDOWN_GRAY, 	"akc_pagedown_gray"     },

	{ AKC_A,				"akc_a"                 },
	{ AKC_B,				"akc_b"                 },
	{ AKC_C,				"akc_c"                 },
	{ AKC_D,				"akc_d"                 },
	{ AKC_E,				"akc_e"                 },
	{ AKC_F,				"akc_f"                 },
	{ AKC_G,				"akc_g"                 },
	{ AKC_H,				"akc_h"                 },
	{ AKC_I,				"akc_i"                 },
	{ AKC_J,				"akc_j"                 },
	{ AKC_K,				"akc_k"                 },
	{ AKC_L,				"akc_l"                 },
	{ AKC_M,				"akc_m"                 },
	{ AKC_N,				"akc_n"                 },
	{ AKC_O,				"akc_o"                 },
	{ AKC_P,				"akc_p"                 },
	{ AKC_Q,				"akc_q"                 },
	{ AKC_R,				"akc_r"                 },
	{ AKC_S,				"akc_s"                 },
	{ AKC_T,				"akc_t"                 },
	{ AKC_U,				"akc_u"                 },
	{ AKC_V,				"akc_v"                 },
	{ AKC_W,				"akc_w"                 },
	{ AKC_X,				"akc_x"                 },
	{ AKC_Y,				"akc_y"                 },
	{ AKC_Z,				"akc_z"                 },

	{ AKC_A_SHIFTED,		"akc_a_shifted"         },
	{ AKC_B_SHIFTED,		"akc_b_shifted"         },
	{ AKC_C_SHIFTED,		"akc_c_shifted"         },
	{ AKC_D_SHIFTED,		"akc_d_shifted"         },
	{ AKC_E_SHIFTED,		"akc_e_shifted"         },
	{ AKC_F_SHIFTED,		"akc_f_shifted"         },
	{ AKC_G_SHIFTED,		"akc_g_shifted"         },
	{ AKC_H_SHIFTED,		"akc_h_shifted"         },
	{ AKC_I_SHIFTED,		"akc_i_shifted"         },
	{ AKC_J_SHIFTED,		"akc_j_shifted"         },
	{ AKC_K_SHIFTED,		"akc_k_shifted"         },
	{ AKC_L_SHIFTED,		"akc_l_shifted"         },
	{ AKC_M_SHIFTED,		"akc_m_shifted"         },
	{ AKC_N_SHIFTED,		"akc_n_shifted"         },
	{ AKC_O_SHIFTED,		"akc_o_shifted"         },
	{ AKC_P_SHIFTED,		"akc_p_shifted"         },
	{ AKC_Q_SHIFTED,		"akc_q_shifted"         },
	{ AKC_R_SHIFTED,		"akc_r_shifted"         },
	{ AKC_S_SHIFTED,		"akc_s_shifted"         },
	{ AKC_T_SHIFTED,		"akc_t_shifted"         },
	{ AKC_U_SHIFTED,		"akc_u_shifted"         },
	{ AKC_V_SHIFTED,		"akc_v_shifted"         },
	{ AKC_W_SHIFTED,		"akc_w_shifted"         },
	{ AKC_X_SHIFTED,		"akc_x_shifted"         },
	{ AKC_Y_SHIFTED,		"akc_y_shifted"         },
	{ AKC_Z_SHIFTED,		"akc_z_shifted"         },

	{ AKC_0,				"akc_0"                 },
	{ AKC_1,				"akc_1"                 },
	{ AKC_2,				"akc_2"                 },
	{ AKC_3,				"akc_3"                 },
	{ AKC_4,				"akc_4"                 },
	{ AKC_5,				"akc_5"                 },
	{ AKC_6,				"akc_6"                 },
	{ AKC_7,				"akc_7"                 },
	{ AKC_8,				"akc_8"                 },
	{ AKC_9,				"akc_9"                 },

	{ AKC_0_SHIFTED,		"akc_0_shifted"         },
	{ AKC_1_SHIFTED,		"akc_1_shifted"         },
	{ AKC_2_SHIFTED,		"akc_2_shifted"         },
	{ AKC_3_SHIFTED,		"akc_3_shifted"         },
	{ AKC_4_SHIFTED,		"akc_4_shifted"         },
	{ AKC_5_SHIFTED,		"akc_5_shifted"         },
	{ AKC_6_SHIFTED,		"akc_6_shifted"         },
	{ AKC_7_SHIFTED,		"akc_7_shifted"         },
	{ AKC_8_SHIFTED,		"akc_8_shifted"         },
	{ AKC_9_SHIFTED,		"akc_9_shifted"         },

	{ AKC_F1,				"akc_f1"                },
	{ AKC_F2,				"akc_f2"                },
	{ AKC_F3,				"akc_f3"                },
	{ AKC_F4,				"akc_f4"                },
	{ AKC_F5,				"akc_f5"                },
	{ AKC_F6,				"akc_f6"                },
	{ AKC_F7,				"akc_f7"                },
	{ AKC_F8,				"akc_f8"                },
	{ AKC_F9,				"akc_f9"                },
	{ AKC_F10,				"akc_f10"               },
	{ AKC_F11,				"akc_f11"               },
	{ AKC_F12,				"akc_f12"               },

	{ AKC_F1_SHIFTED,		"akc_f1_shifted"        },
	{ AKC_F2_SHIFTED,		"akc_f2_shifted"        },
	{ AKC_F3_SHIFTED,		"akc_f3_shifted"        },
	{ AKC_F4_SHIFTED,		"akc_f4_shifted"        },
	{ AKC_F5_SHIFTED,		"akc_f5_shifted"        },
	{ AKC_F6_SHIFTED,		"akc_f6_shifted"        },
	{ AKC_F7_SHIFTED,		"akc_f7_shifted"        },
	{ AKC_F8_SHIFTED,		"akc_f8_shifted"        },
	{ AKC_F9_SHIFTED,		"akc_f9_shifted"        },
	{ AKC_F10_SHIFTED,		"akc_f10_shifted"       },
	{ AKC_F11_SHIFTED,		"akc_f11_shifted"       },
	{ AKC_F12_SHIFTED,		"akc_f12_shifted"       },

	{ AKC_JOY_BUTTON1,		"akc_joy_button1"		},
	{ AKC_JOY_BUTTON2,		"akc_joy_button2"		},
	{ AKC_JOY_BUTTON3,		"akc_joy_button3"		},
	{ AKC_JOY_BUTTON4,		"akc_joy_button4"		},
	{ AKC_JOY_BUTTON5,		"akc_joy_button5"		},
	{ AKC_JOY_BUTTON6,		"akc_joy_button6"		},
	{ AKC_JOY_BUTTON7,		"akc_joy_button7"		},
	{ AKC_JOY_BUTTON8,		"akc_joy_button8"		},

};

#define AKC_SIGNATURE		"akc_"
#define AKC_SIGNATURE_LEN	4
#define NUM_AKC_CODES		CALC_NUM_ARRAY_ENTRIES( key_com_names )


// key names for functional mappings (make codes) -----------------------------
//
textkeymap_s make_codes[] = {

	{ MKC_ESCAPE,			"mkc_escape"            },
	{ MKC_SPACE,			"mkc_space"             },
	{ MKC_ENTER,			"mkc_enter"             },
	{ MKC_CURSORUP, 		"mkc_cursorup"          },
	{ MKC_CURSORDOWN,		"mkc_cursordown"        },
	{ MKC_CURSORLEFT,		"mkc_cursorleft"        },
	{ MKC_CURSORRIGHT,		"mkc_cursorright"       },
	{ MKC_BACKSPACE,		"mkc_backspace"         },
	{ MKC_INSERT,			"mkc_insert"            },
	{ MKC_DELETE,			"mkc_delete"            },
	{ MKC_HOME, 			"mkc_home"              },
	{ MKC_END,				"mkc_end"               },
	{ MKC_PAGEUP,			"mkc_pageup"            },
	{ MKC_PAGEDOWN, 		"mkc_pagedown"          },
	{ MKC_CAPSLOCK, 		"mkc_capslock"          },
	{ MKC_TILDE,			"mkc_tilde"             },

	{ MKC_MINUS,            "mkc_minus"             },
	{ MKC_EQUALS,           "mkc_equals"            },
	{ MKC_TAB,              "mkc_tab"               },
	{ MKC_LBRACKET,         "mkc_lbracket"          },
	{ MKC_RBRACKET,         "mkc_rbracket"          },
	{ MKC_SEMICOLON,        "mkc_semicolon"         },
	{ MKC_APOSTROPHE,       "mkc_apostrophe"        },
	{ MKC_GRAVE,            "mkc_grave"             },

	{ MKC_LSHIFT,           "mkc_lshift"            },
	{ MKC_RSHIFT,           "mkc_rshift"            },
	{ MKC_LCONTROL,         "mkc_lcontrol"          },
	{ MKC_RCONTROL,         "mkc_rcontrol"          },
	{ MKC_LALT,             "mkc_lalt"              },
	{ MKC_RALT,             "mkc_ralt"              },

	{ MKC_COMMA,            "mkc_comma"             },
	{ MKC_PERIOD,           "mkc_period"            },
	{ MKC_SLASH,            "mkc_slash"             },

	{ MKC_BACKSLASH,        "mkc_backslash"         },
	{ MKC_GERARROWS,		"mkc_gerarrows"			},

	{ MKC_NUMPADSLASH,      "mkc_numpadslash"		},
	{ MKC_NUMPADSTAR,       "mkc_numpadstar"		},
	{ MKC_NUMPADMINUS,      "mkc_numpadminus"       },
	{ MKC_NUMPADPLUS,       "mkc_numpadplus"        },
	{ MKC_NUMPADPERIOD,     "mkc_numpadperiod"      },
	{ MKC_NUMLOCK,          "mkc_numlock"         	},
	{ MKC_SCROLL,           "mkc_scroll"         	},

	{ MKC_NUMPAD0,          "mkc_numpad0"         	},
	{ MKC_NUMPAD1,          "mkc_numpad1"         	},
	{ MKC_NUMPAD2,          "mkc_numpad2"         	},
	{ MKC_NUMPAD3,          "mkc_numpad3"         	},
	{ MKC_NUMPAD4,          "mkc_numpad4"         	},
	{ MKC_NUMPAD5,          "mkc_numpad5"         	},
	{ MKC_NUMPAD6,          "mkc_numpad6"         	},
	{ MKC_NUMPAD7,          "mkc_numpad7"         	},
	{ MKC_NUMPAD8,          "mkc_numpad8"         	},
	{ MKC_NUMPAD9,          "mkc_numpad9"         	},

	{ MKC_LWIN,             "mkc_lwin"         		},
	{ MKC_RWIN,             "mkc_rwin"         		},
	{ MKC_APPS,             "mkc_apps"         		},

	{ MKC_ENTER_GRAY,		"mkc_enter_gray"        },
	{ MKC_CURSORUP_GRAY, 	"mkc_cursorup_gray"     },
	{ MKC_CURSORDOWN_GRAY,	"mkc_cursordown_gray"   },
	{ MKC_CURSORLEFT_GRAY,	"mkc_cursorleft_gray"   },
	{ MKC_CURSORRIGHT_GRAY,	"mkc_cursorright_gray"  },
	{ MKC_BACKSPACE_GRAY,	"mkc_backspace_gray"    },
	{ MKC_INSERT_GRAY,		"mkc_insert_gray"       },
	{ MKC_DELETE_GRAY,		"mkc_delete_gray"       },
	{ MKC_HOME_GRAY, 		"mkc_home_gray"         },
	{ MKC_END_GRAY,			"mkc_end_gray"          },
	{ MKC_PAGEUP_GRAY,		"mkc_pageup_gray"       },
	{ MKC_PAGEDOWN_GRAY, 	"mkc_pagedown_gray"     },

	{ MKC_A,				"mkc_a"                 },
	{ MKC_B,				"mkc_b"                 },
	{ MKC_C,				"mkc_c"                 },
	{ MKC_D,				"mkc_d"                 },
	{ MKC_E,				"mkc_e"                 },
	{ MKC_F,				"mkc_f"                 },
	{ MKC_G,				"mkc_g"                 },
	{ MKC_H,				"mkc_h"                 },
	{ MKC_I,				"mkc_i"                 },
	{ MKC_J,				"mkc_j"                 },
	{ MKC_K,				"mkc_k"                 },
	{ MKC_L,				"mkc_l"                 },
	{ MKC_M,				"mkc_m"                 },
	{ MKC_N,				"mkc_n"                 },
	{ MKC_O,				"mkc_o"                 },
	{ MKC_P,				"mkc_p"                 },
	{ MKC_Q,				"mkc_q"                 },
	{ MKC_R,				"mkc_r"                 },
	{ MKC_S,				"mkc_s"                 },
	{ MKC_T,				"mkc_t"                 },
	{ MKC_U,				"mkc_u"                 },
	{ MKC_V,				"mkc_v"                 },
	{ MKC_W,				"mkc_w"                 },
	{ MKC_X,				"mkc_x"                 },
	{ MKC_Y,				"mkc_y"                 },
	{ MKC_Z,				"mkc_z"                 },

	{ MKC_0,				"mkc_0"                 },
	{ MKC_1,                "mkc_1"                 },
	{ MKC_2,                "mkc_2"                 },
	{ MKC_3,                "mkc_3"                 },
	{ MKC_4,                "mkc_4"                 },
	{ MKC_5,                "mkc_5"                 },
	{ MKC_6,                "mkc_6"                 },
	{ MKC_7,                "mkc_7"                 },
	{ MKC_8,                "mkc_8"                 },
	{ MKC_9,                "mkc_9"                 },

	{ MKC_F1,				"mkc_f1"                },
	{ MKC_F2,				"mkc_f2"                },
	{ MKC_F3,				"mkc_f3"                },
	{ MKC_F4,				"mkc_f4"                },
	{ MKC_F5,				"mkc_f5"                },
	{ MKC_F6,				"mkc_f6"                },
	{ MKC_F7,				"mkc_f7"                },
	{ MKC_F8,				"mkc_f8"                },
	{ MKC_F9,				"mkc_f9"                },
	{ MKC_F10,				"mkc_f10"               },
	{ MKC_F11,				"mkc_f11"               },
	{ MKC_F12,				"mkc_f12"               },

};

#define NUM_MKC_CODES		CALC_NUM_ARRAY_ENTRIES( make_codes )


// functional keyboard function names -----------------------------------------
//
const char *const functional_keys[] = {

	"func_",				// signature (prefix) for all game function specs

	"escape",
	"turnleft",
	"turnright",
	"divedown",
	"pullup",
	"rollleft",
	"rollright",
	"shootweapon",
	"launchmissile",
	"nextweapon",
	"nextmissile",
	"accelerate",
	"decelerate",
	"slideleft",
	"slideright",
	"slideup",
	"slidedown",
	"nexttarget",
	"toggleframerate",
	"toggleobjcamera",
	"togglehelp",
	"toggleconsole",
	"savescreenshot",
	"showkillstats",
	"speedzero",
	"targetspeed",
	"afterburner",
	"fronttarget",
	"select",
	"cursorleft",
	"cursorright",
	"cursorup",
	"cursordown",
};

#define NUM_FUNCTIONAL_KEYS CALC_NUM_ARRAY_ENTRIES( functional_keys )
#define FUNC_SIG_LEN		5 //strlen(functional_keys[0])


// table of keys for which a gray equivalent exists ---------------------------
//
dword dup_gray_keys[] = {

	MKC_ENTER,			MKC_ENTER_GRAY,
	MKC_CURSORUP,		MKC_CURSORUP_GRAY,
	MKC_CURSORDOWN,		MKC_CURSORDOWN_GRAY,
	MKC_CURSORLEFT,		MKC_CURSORLEFT_GRAY,
	MKC_CURSORRIGHT,	MKC_CURSORRIGHT_GRAY,
	MKC_BACKSPACE,		MKC_BACKSPACE_GRAY,
	MKC_INSERT,			MKC_INSERT_GRAY,
	MKC_DELETE,			MKC_DELETE_GRAY,
	MKC_HOME,			MKC_HOME_GRAY,
	MKC_END,			MKC_END_GRAY,
	MKC_PAGEUP,			MKC_PAGEUP_GRAY,
	MKC_PAGEDOWN,		MKC_PAGEDOWN_GRAY,

	MKC_NIL,			MKC_NIL,
};


// get text description for AKC_ code -----------------------------------------
//
char *GetAKCDescription( int num )
{
	ASSERT( num >= 0 );
	ASSERT( ( num < KEY_ADDITIONS_MAX  ) && ( num < KeyAdditional->size ) );

	//NOTE:
	// this function is used by CON_LIST::DispKeyBinding()
	// in order to display a text description of an
	// assignable key-code (AKC_ code).
	// (command "listbindings".)

	keyaddition_s *kap = &KeyAdditional->table[num];

	for ( dword i = 0; i < NUM_AKC_CODES; i++ ) {
		if ( kap->code == (dword)key_com_names[ i ].code ) {
			return (char *) key_com_names[ i ].text;
		}
	}

	// if code not found in table display it in hex
	sprintf( paste_str, "0x%06x", kap->code );
	return paste_str;
}


// get text description of game function --------------------------------------
//
char *GetGameFuncDescription( int num )
{
	ASSERT( ( num >= 0 ) && ( (dword)num < NUM_FUNCTIONAL_KEYS - 1 ) );

	//NOTE:
	// this function is used by CON_LIST::DispKeyFunction()
	// in order to display a text description of a
	// game function.
	// (command "listgamefunckeys".)

	return (char *) functional_keys[ num + 1 ];
}


// get text description for MKC_ codes of function layers 1 and 2 -------------
//
char *GetMKCDescription( int num )
{
	ASSERT( ( num >= 0 ) && ( (dword)num < NUM_GAMEFUNC_KEYS ) );

	//NOTE:
	// this function is used by CON_LIST::DispKeyFunction()
	// in order to display a text description of a key
	// mapped to a game function.
	// (command "listgamefunckeys".)

	int *assigns1 = (int *) KeyAssignments;
	int *assigns2 = (int *)&KeyAssignments[1];

	const char *desc1 = NULL;
	const char *desc2 = NULL;
	
	for ( unsigned int i = 0; i < NUM_MKC_CODES; i++ ) {
		if ( desc1 == NULL && assigns1[ num ] == make_codes[ i ].code )
			desc1 = make_codes[ i ].text; // assume it's the first available name that matches the keycode
		if ( assigns2[ num ] == make_codes[ i ].code )
			desc2 = make_codes[ i ].text; // assume secondary assignment is the last available name that matches the keycode
	}

	//NOTE:
	// if a MKC_ code has been specified as byte rather than
	// a string constant and this byte has no corresponding
	// string constant, then the byte-code will be displayed
	// instead of a key specifier.

	char bytecode1[ 8 ];
	char bytecode2[ 8 ];

	if ( ( desc1 == NULL ) && ( assigns1[ num ] != MKC_NIL ) ) {
		sprintf( bytecode1, "0x%02x", assigns1[ num ] );
		desc1 = bytecode1;
	}
	if ( ( desc2 == NULL ) && ( assigns2[ num ] != MKC_NIL ) ) {
		sprintf( bytecode2, "0x%02x", assigns2[ num ] );
		desc2 = bytecode2;
	}

	// field-width for layer1 description
	#define LAYER1_DESC_PAD_SIZE	14

	if ( ( desc1 != NULL ) && ( desc2 != NULL ) ) {
		sprintf( paste_str, "l1: %s ", desc1 );
		int descpad	= LAYER1_DESC_PAD_SIZE - strlen( desc1 );
		char *ppad	= paste_str + strlen( desc1 ) + 5;
		while ( descpad-- > 0 )
			*ppad++ = ' ';
		sprintf( ppad, "l2: %s", desc2 );
	} else if ( desc1 != NULL ) {
		sprintf( paste_str, "l1: %s ", desc1 );
		int descpad	= LAYER1_DESC_PAD_SIZE - strlen( desc1 );
		char *ppad	= paste_str + strlen( desc1 ) + 5;
		while ( descpad-- > 0 )
			*ppad++ = ' ';
		strcpy( ppad, "l2: unassigned" );
	} else if ( desc2 != NULL ) {
		sprintf( paste_str, "l1: unassigned     l2: %s", desc2 );
	} else {
		sprintf( paste_str, "function unassigned" );
	}

	return paste_str;
}


// get standard key for gray key (or vice versa) ------------------------------
//
PRIVATE
dword GrayEquiv( dword kcode )
{
	//NOTE:
	// e.g., this function would return MKC_ENTER_GRAY
	// for argument MKC_ENTER, and MKC_ENTER for
	// argument MKC_ENTER_GRAY.

	//NOTE:
	// on a pc this function would only need to invert bit 7
	// to toggle between gray/non-gray, but this would
	// not be portable. hence, simple table lookup is used.

	for ( int i = 0; dup_gray_keys[ i ] != MKC_NIL; i+=2 ) {
		if ( dup_gray_keys[ i ] == kcode )
			return dup_gray_keys[ i + 1 ];
		if ( dup_gray_keys[ i + 1 ] == kcode )
			return dup_gray_keys[ i ];
	}

	return MKC_NIL;
}


// get MKC_ code for textually specified MKC_ code identifier -----------------
//
PRIVATE
dword ReadMKC( char *scan )
{
	//NOTE:
	// MKC_ codes may be specified via a string constant
	// like MKC_ESCAPE, or directly via a byte-code as
	// hex-byte, like 0x01.

	if ( scan != NULL ) {

		// if string starts with "0x" convert as byte
		if ( ( scan[ 0 ] == '0' ) && ( scan[ 1 ] == 'x' ) ) {
			char *remptr;
			long code = strtol( scan + 2, &remptr, 16 );
			if ( ( *remptr != 0 ) || ( code < 0 ) ) {
				CON_AddLine( invalid_mkc );
				return MKC_NIL;
			}
			return (dword) code;
		}

		// check if string is valid identifier starting with "MKC_"
		for ( unsigned int i = 0; i < NUM_MKC_CODES; i++ ) {
			if ( strcmp( make_codes[ i ].text, scan ) == 0 )
				return make_codes[ i ].code;
		}

		CON_AddLine( invalid_mkc );

		// if supplied code is invalid deassign mapping
		return MKC_NIL;

	} else {

		// if no code is specified deassign mapping
		return MKC_NIL;
	}
}


// bind console command to specific key (via FUNC_ or AKC_ key specifier) -----
//
PRIVATE
int CheckKeyMappingBase( char *scan, int echo )
{
	ASSERT( scan != NULL );
	ASSERT( ( echo == TRUE ) || ( echo == FALSE ) );

	// fetch keyspec and command
	char *key = scan;
	scan = strtok( NULL, "" );

	// check functional keys (FUNC_, FUNC_1_, FUNC_2_)
	if ( strncmp( key, functional_keys[ 0 ], FUNC_SIG_LEN ) == 0 ) {

		//NOTE:
		// the two functional layers can either be mapped
		// independently, or simultaneously (for gray keys).
		// e.g., the primary keyspec for escape would be
		// FUNC_1_ESCAPE. for the alternate (second) layer
		// it would be FUNC_2_ESCAPE. if the second layer
		// should not be utilized, FUNC_ESCAPE may be
		// used. then, if the specified key has a gray
		// equivalent, this is put into the second layer,
		// otherwise the alternate assignment will be deleted.

		dword *layer  = (dword *) KeyAssignments;
		char *cont   = key + FUNC_SIG_LEN;
		int  setboth = FALSE;

		if ( ( key[ FUNC_SIG_LEN ] == '1' ) && ( key[ FUNC_SIG_LEN + 1 ] == '_' ) ) {
			// select layer 1 and skip 1_ prefix
			cont  += 2;
		} else if ( ( key[ FUNC_SIG_LEN ] == '2' ) && ( key[ FUNC_SIG_LEN + 1 ] == '_' ) ) {
			// select layer 2 and skip 2_ prefix
			layer += NUM_GAMEFUNC_KEYS;
			cont  += 2;
		} else {
			setboth = TRUE;
		}

		//NOTE:
		// the second parameter for mapping functional keys
		// is not a command, but a MKC_ keyspec. also, the
		// echo flag is not used, i.e., the BIND and KEY
		// commands are identical in this case.
		//
		// mapping the primary key for the escape-function
		// to the <escape> key would be:
		// "BIND FUNC_1_ESCAPE MKC_ESCAPE", or, equivalently,
		// "KEY FUNC_1_ESCAPE MKC_ESCAPE".

		// eat whitespace
		while ( *scan == ' ' )
			scan++;

		// cut off trailing whitespace
		char *rscan = scan;
		while ( *rscan != 0 )
			rscan++;
		while ( *--rscan == ' ' )
			{}
		*( rscan + 1 ) = 0;

		int j = -1;
		for ( unsigned int i = 1; i < NUM_FUNCTIONAL_KEYS; i++ ) {
			if ( strcmp( functional_keys[ i ], cont ) == 0 ) {
				// set first layer (may be 1 or 2)
				layer[ j = i - 1 ] = ReadMKC( scan );
				// also set second layer if FUNC_ was used (always 2)
				if ( setboth )
					layer[ NUM_GAMEFUNC_KEYS + j ] = GrayEquiv( layer[ j ] );
				break;
			}
		}
		if ( j == -1 )
			CON_AddLine( invalid_func );

		return 1;
	}

	// check additional keys (AKC_xx)
	if ( strncmp( key, AKC_SIGNATURE, AKC_SIGNATURE_LEN ) == 0 ) {
		for ( dword i = 0; i < NUM_AKC_CODES; i++ ) {
			if ( strcmp( key_com_names[ i ].text, key ) == 0 ) {

				// get code and alloc command string mem
				dword code = key_com_names[ i ].code;
				char *com = scan ? (char *) ALLOCMEM( strlen( scan ) + 1 ) : NULL;

				// copy command into floating string
				if ( com != NULL )
					strcpy( com, scan );

				// search for already existing entry
				int entryindx = -1;
				keyaddition_s *kap = &KeyAdditional->table[0];
				for ( i = 0; i < (unsigned int)KeyAdditional->size; i++, kap++ ) {
					if ( kap->code == code ) {
						entryindx = i;
						break;
					}
				}
				if ( entryindx == -1 ) {
					if ( KeyAdditional->size == KEY_ADDITIONS_MAX ) {
						// don't allow more additional key assignments than
						// the static table can accommodate
						CON_AddLine( too_many_keyadds );
						return 1;
					}
					// create new entry
					entryindx  = KeyAdditional->size++;
					kap->code  = code;
					kap->state = 0;
				} else if ( key_com_mappings[ entryindx ].command != NULL ) {
					// free command string assigned previously
					FREEMEM( key_com_mappings[ entryindx ].command );
				}

				//NOTE:
				// the echo flag is stored along with the command and
				// will be used by ExecBoundKeyCommands() to determine
				// whether the command should be displayed and entered
				// into the history list, or not.

				key_com_mappings[ entryindx ].echo    = echo;
				key_com_mappings[ entryindx ].command = com;

				return 1;
			}
		}

		CON_AddLine( invalid_akc );
		return 1;
	}

	CON_AddLine( invalid_key );
	return 1;
}


// exec key map command (command "key") ---------------------------------------
//
int CheckKeyMappingEcho( char *scan )
{
	ASSERT( scan != NULL );

	//NOTE:
	//CONCOM:
	// key_command ::= 'key' ( <key_spec> <command> | <func_spec> <mkc_code> )
	// key_spec    ::= 'akc_*'
	// command     ::= "arbitrary string, may contain whitespace"
	// func_spec   ::= 'func_*' | 'func_1_*' | 'func_2_*'
	// mkc_code    ::= 'mkc_*' | '0x' <hex-digit> <hex-digit> | <nil>
	// hex-digit   ::= [0-9a-f]
	// nil         ::= "if no mkc_code is supplied mapping will be deassigned."
	//
	// * in 'akc_*' means any of the strings in key_com_names[].
	// * in 'func_*', 'func_1_*', and 'func_2_*' means any of the
	// strings in functional_keys[].
	// * in 'mkc_*' means any of the strings in make_codes[].
	//
	// if the mkc_code is specified as hex-byte it will be
	// interpreted directly as make-code. i.e., this is highly
	// system-dependent!

	//NOTE:
	// commands mapped to keys via the KEY command
	// act just like the command were typed at the
	// console prompt. i.e., the command itself will
	// be displayed in the console along with its
	// text-output, if any.
	//
	// these commands will also be inserted into
	// the history list.

	// check if keymap command
	if ( strcmp( scan, CMSTR( CM_KEY ) ) != 0 )
		return FALSE;

	// keyspec + command must be supplied as parameters
	if ( ( scan = strtok( NULL, " " ) ) == NULL ) {
		CON_AddLine( keymap_syntax );
		return TRUE;
	}

	// store binding with echo flag on
	return CheckKeyMappingBase( scan, TRUE );
}


// exec silent key map command (command "bind") -------------------------------
//
int CheckKeyMappingSilent( char *scan )
{
	ASSERT( scan != NULL );

	//NOTE:
	//CONCOM:
	// bind_command ::= 'bind' ( <key_spec> <command> | <func_spec> <mkc_code> )
	// key_spec    ::= 'akc_*'
	// command     ::= "arbitrary string, may contain whitespace"
	// func_spec   ::= 'func_*' | 'func_1_*' | 'func_2_*'
	// mkc_code    ::= 'mkc_*' | '0x' <hex-digit> <hex-digit> | <nil>
	// hex-digit   ::= [0-9a-f]
	// nil         ::= "if no mkc_code is supplied mapping will be deassigned."
	//
	// * in 'akc_*' means any of the strings in key_com_names[].
	// * in 'func_*', 'func_1_*', and 'func_2_*' means any of the
	// strings in functional_keys[].
	// * in 'mkc_*' means any of the strings in make_codes[].
	//
	// if the mkc_code is specified as hex-byte it will be
	// interpreted directly as make-code. i.e., this is highly
	// system-dependent!

	//NOTE:
	// commands mapped to keys via the BIND command
	// will be executed silently. i.e., the command
	// will not be displayed in the console if the
	// key gets pressed and the command is executed.
	// if the command causes any text-output, this
	// will still be displayed, however.
	//
	// these commands will NOT be inserted into
	// the history list.

	// check if bind command
	if ( strcmp( scan, CMSTR( CM_BIND ) ) != 0 )
		return FALSE;

	// keyspec + command must be supplied as parameters
	if ( ( scan = strtok( NULL, " " ) ) == NULL ) {
		CON_AddLine( bind_syntax );
		return TRUE;
	}

	// store binding with echo flag off
	return CheckKeyMappingBase( scan, FALSE );
}


// exec console commands mapped to additional keys ----------------------------
//
void ExecBoundKeyCommands()
{
	//NOTE:
	// this function is called every frame by
	// the gameloop (G_MAIN.C) to check
	// for depression of additionally mapped keys.

	//NOTE:
	// this function will do nothing if a list command
	// is currently in progress in the console.

	// if standard prompt active execute commands mapped to AKC_xx keys
	if ( !await_keypress ) {

		// test all additional keys
		keyaddition_s *kap = &KeyAdditional->table[0];
		for ( int aid = 0; aid < KeyAdditional->size; aid++, kap++ ) {

			// execute command if key pressed
			if ( kap->state ) {
				kap->state	= 0;

				if ( key_com_mappings[ aid ].command == NULL )
					continue;

				int echo	= key_com_mappings[ aid ].echo;
				int oldbutt = con_bottom;

				// remember old login check state
				int oldcheckstate = con_no_login_check;

				if ( echo ) {

					// insert prompt and command if echo on
					EraseConLine( con_bottom );
					strcpy( paste_str, con_prompt );
					strcat( paste_str, key_com_mappings[ aid ].command );
					strcpy( con_lines[ con_bottom ], paste_str );
					cursor_x = 0;
					InsertCommandLog( key_com_mappings[ aid ].command );

				} else {

					// turn off login check for command if echo off
					con_no_login_check = 1;
				}

				// execute command
				ExecConsoleCommand( key_com_mappings[ aid ].command, echo );

				// restore login check state
				con_no_login_check = oldcheckstate;

				// check for list command
				if ( await_keypress ) {
					CON_ListCtdPrompt();
				} else if ( echo || ( con_bottom != oldbutt ) ) {

					//TODO:
					//CAVEAT:
					// if a command outputs exactly as many lines in one
					// run as the console can contain, con_bottom will
					// wrap around to itself and the output will not
					// be detected!

					CON_ListEndPrompt();
				}
			}
		}
	}
}



