/*
 * PARSEC HEADER: s_emp.h
 */

#ifndef _S_EMP_H_
#define _S_EMP_H_


// use emp as duration weapon
#define EMP_FIRE_CONTINUOUSLY

// external functions
#ifndef PARSEC_SERVER
void	WFX_EmpBlast( ShipObject *shippo );
void	WFX_RemoteEmpBlast( ShipObject *shippo, int curupgrade );
void	WFX_CreateEmpWaves( ShipObject *shippo );
int     WFX_ActivateEmp( ShipObject *shippo );
void    WFX_DeactivateEmp( ShipObject *shippo );
void    WFX_RemoteActivateEmp( int playerid );
void    WFX_RemoteDeactivateEmp( int playerid );

#endif


#define EMP_MAX_TEX_NAME 128


// number of upgrade levels ---------------------------------------------------
//
#define EMP_UPGRADES		3


// preset emp type property values --------------------------------------------
//
#define EMP_MAX_LIFETIME		6000
#define EMP_MIN_LIFETIME		60
#define EMP_MAX_WAVES			100
#define EMP_MIN_WAVES			1
#define EMP_MAX_DELAY			2400
#define EMP_MIN_DELAY			0

// emp standard property values
#define EMP_TEXNAME				"in01_00a.3df"
#define EMP_LOD					16
#define EMP_MAX_WIDTH			200.0f
#define EMP_LAT					0x2000		// 45 deg
#define EMP_RED           		220
#define EMP_GREEN				220
#define EMP_BLUE				220
#define EMP_ALPHA				255
#define EMP_LIFETIME			400
#define EMP_FADEOUT       		400
#define EMP_LAMBDA				0.2
#define EMP_ROT					32
#define EMP_DELAY				200
#define EMP_WAVES				1
#define EMP_ENERGY				0
#define EMP_DAMAGE				2000

// emp upgrade level 1 property values
#define EMP_UP1_TEXNAME			"in01_00a.3df"
#define EMP_UP1_LOD				16
#define EMP_UP1_MAX_WIDTH		200.0f
#define EMP_UP1_LAT				0x2000		// 45 deg
#define EMP_UP1_RED           	220
#define EMP_UP1_GREEN			220
#define EMP_UP1_BLUE			220
#define EMP_UP1_ALPHA			180
#define EMP_UP1_LIFETIME		200
#define EMP_UP1_FADEOUT       	200
#define EMP_UP1_LAMBDA			2.0
#define EMP_UP1_ROT				( BAMS_DEG45 / 16 )
#define EMP_UP1_DELAY			40
#define EMP_UP1_WAVES			6
#define EMP_UP1_ENERGY			6
#define EMP_UP1_DAMAGE			1000

// emp upgrade level 2 property values
#define EMP_UP2_TEXNAME			"in01_00a.3df"
#define EMP_UP2_LOD				16
#define EMP_UP2_MAX_WIDTH		200.0f
#define EMP_UP2_LAT				0x2000		// 45 deg
#define EMP_UP2_RED           	220
#define EMP_UP2_GREEN			200
#define EMP_UP2_BLUE			0
#define EMP_UP2_ALPHA			90
#define EMP_UP2_LIFETIME		800
#define EMP_UP2_FADEOUT       	800
#define EMP_UP2_LAMBDA			4.0
#define EMP_UP2_ROT				16
#define EMP_UP2_DELAY			80
#define EMP_UP2_WAVES			6
#define EMP_UP2_ENERGY			40
#define EMP_UP2_DAMAGE			3000


// emp custom type structure --------------------------------------------------
//
struct Emp : CustomObject {

	int			upgradelevel;
	Xmatrx		WorldXmatrx;	// object- to worldspace matrix
	Vertex3*	ObjVtxs;
	Vertex3*	WorldVtxs;		// vertices in worldspace
	TextureMap*	texmap;
	char		texname[ EMP_MAX_TEX_NAME + 1 ];
	dword		OwnerHostObjno;		// needed for animation calculation
	GenObject*	ownerpo;
	int			Owner;
	int			vtxsnr;
	int			lod;
	bams_t		lat;
	bams_t		rot;
	long		alive;
	int			delay;
	int			red;
	int			green;
	int			blue;
	int			alpha;
	dword		damage;			// hitpoints fractional per refframe
};

#define OFS_TEXNAME			offsetof( Emp, texname )
#define OFS_LOD				offsetof( Emp, lod )
#define OFS_LAT				offsetof( Emp, lat )
#define OFS_ROT				offsetof( Emp, rot )
#define OFS_RED				offsetof( Emp, red )
#define OFS_GREEN			offsetof( Emp, green )
#define OFS_BLUE			offsetof( Emp, blue )
#define OFS_ALPHA			offsetof( Emp, alpha )
#define OFS_DAMAGE			offsetof( Emp, damage )


// assigned type id for emp type ----------------------------------------------
//
static dword emp_type_id[ EMP_UPGRADES ];


// full table for emp expansion -----------------------------------------------
//
static float *emp_expansion_tab[ EMP_UPGRADES ] = {
	NULL, NULL, NULL,
};


// emp behaviour properties ---------------------------------------------------
//
static int			emp_lifetime[ EMP_UPGRADES ]	= {
		EMP_LIFETIME,
		EMP_UP1_LIFETIME,
		EMP_UP2_LIFETIME,
};
static geomv_t		emp_max_width[ EMP_UPGRADES ]	= {
		FLOAT_TO_GEOMV( EMP_MAX_WIDTH ),
		FLOAT_TO_GEOMV( EMP_UP1_MAX_WIDTH ),
		FLOAT_TO_GEOMV( EMP_UP2_MAX_WIDTH ),
};

static float		emp_lambda[ EMP_UPGRADES ]		= {
		EMP_LAMBDA,
		EMP_UP1_LAMBDA,
		EMP_UP2_LAMBDA,
};

static int          emp_fadeout[ EMP_UPGRADES ]		= {
		EMP_FADEOUT,
		EMP_UP1_FADEOUT,
		EMP_UP2_FADEOUT,
};

static int          emp_waves[ EMP_UPGRADES ]		= {
		EMP_WAVES,
		EMP_UP1_WAVES,
		EMP_UP2_WAVES,
};

static int          emp_delay[ EMP_UPGRADES ]		= {
		EMP_DELAY,
		EMP_UP1_DELAY,
		EMP_UP2_DELAY,
};

static int          emp_energy[ EMP_UPGRADES ]		= {
		EMP_ENERGY,
		EMP_UP1_ENERGY,
		EMP_UP2_ENERGY,
};


// macro to set the properties of a itervertex --------------------------------
//
#define SET_ITER_VTX( itvtx, vtx, u, v, r, g, b, a ) \
	(itvtx)->X = (vtx)->X;	\
	(itvtx)->Y = (vtx)->Y;	\
	(itvtx)->Z = (vtx)->Z;	\
	(itvtx)->W = GEOMV_1;	\
	(itvtx)->U = (u);		\
	(itvtx)->V = (v);		\
	(itvtx)->R = (r);		\
	(itvtx)->G = (g);		\
	(itvtx)->B = (b);		\
	(itvtx)->A = (a);


// color combination macro ----------------------------------------------------
//
#define COLOR_MUL(t,a,b)	{ \
	int tmp; \
	tmp = ( (int)(a) * (int)(b) ) / 255; \
	if ( tmp > 255 ) \
		tmp = 255; \
	(t) = tmp; \
}
#endif // _S_EMP_H_


