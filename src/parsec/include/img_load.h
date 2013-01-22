/*
 * PARSEC HEADER: img_load.h
 */

#ifndef _IMG_LOAD_H_
#define _IMG_LOAD_H_


// forward declarations
struct format_3df_s;
struct imgreadinfo_s;
struct imgtotexdata_s;


// image reading info

struct imgreadinfo_s {

	int				mapwidth;		// -1 == use default
	int				mapheight;		// -1 == use default

	int				lodlarge;		// -1 == use default
	int				lodsmall;		// -1 == use default

	int				retilewidth;	// -1 == use default
	int				retileheight;	// -1 == use default

	int				spacing;

	format_3df_s*	texfmtgr;
};


// external functions

int		IMG_ParseLoadingParameters( const char *loaderparams, imgreadinfo_s *imgreadinfo );
int		IMG_CheckMapGeometry( imgreadinfo_s *imgreadinfo, int width, int height );
void	IMG_CheckLodSpecs( imgreadinfo_s *imgreadinfo, int lgside );
void	IMG_CreateTexture( int insertindex, imgtotexdata_s *imgtotexdata, dword texgeometry, int inputwidth, int inputheight, texfont_s *texfont );
int		IMG_LoadGenericTexture(const char *fname, int insertindex, char *loaderparams, texfont_s *texfont);


#endif // _IMG_LOAD_H_


