/*
 * PARSEC HEADER: gd_heads.h
 */

#ifndef _GD_HEADS_H_
#define _GD_HEADS_H_


// ----------------------------------------------------------------------------
// DATA FILE HEADER DEFINITIONS                                               -
// ----------------------------------------------------------------------------


// filepack: structure of file package header ---------------------------------
//
struct packageheader_s {

	char	signature[ 16 ];
	dword	numitems;
	dword	headersize;
	dword	datasize;
	dword	packsize;
};


// filepack: structure of single entry in info list in memory -----------------
//
struct pfileinfo_s {

	char	*file;
	dword	foffset;
	dword	flength;
	FILE	*fp;
	dword	fcurpos;
};


// filepack: structure of single entry in info list on storage device ---------
//
struct pfileinfodisk_s {

	char	file[ 16 ];
	dword	foffset;
	dword	flength;
	dword	fp; // dword instead of FILE* because packed .dat file requires it to be 32 bits
	dword	fcurpos;
};


// parpfg format (parsec font info files) -------------------------------------
//
struct PfgHeader {

	char	signature[7];
	byte	version;
	int 	srcwidth;
	short	width;
	short	height;
};

#define PFG_SIGNATURE			"PARPFG"
#define REQUIRED_PFG_VERSION	0x01


// parfnt format (parsec font files) ------------------------------------------
//
struct FntHeader {

	char	signature[7];
	byte	version;
	int 	width;
	int 	height;
};

#define FNT_SIGNATURE			"PARFNT"
#define REQUIRED_FNT_VERSION	0x01
#define FONT_EXTENSION			"fnt"


// partex format (parsec texture data files) ----------------------------------
//
struct TexHeader {

	char	signature[7];
	byte	version;
	int 	width;
	int 	height;
};

#define TEX_SIGNATURE			"PARTEX"
#define REQUIRED_TEX_VERSION	0x01


// parbdt format (parsec bitmap data files) -----------------------------------
//
struct BdtHeader {

	char	signature[7];
	byte	version;
	int 	width;
	int 	height;
};

#define BDT_SIGNATURE			"PARBDT"
#define REQUIRED_BDT_VERSION	0x01


// pardem format (parsec demo data files) --------------------------------------
//
struct DemHeader {

	char	signature[7];
	byte	version;
	dword	headersize;
};

#define DEM_SIGNATURE			"PARDEM"
#define REQUIRED_DEM_VERSION	0x01

enum {

	DEMO_KEY_TITLE			= 0x01,
	DEMO_KEY_DESCRIPTION	= 0x02,
	DEMO_KEY_AUTHOR			= 0x03,

	NUM_DEMO_KEYS			// must be last in list
};


#endif // _GD_HEADS_H_


