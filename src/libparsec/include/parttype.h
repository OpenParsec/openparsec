/*
 * PARSEC HEADER
 * Particle System Types and Definitions V1.46
 *
 * Copyright (c) Markus Hadwiger 1997-1999
 * All Rights Reserved.
 */

#ifndef _PARTTYPE_H_
#define _PARTTYPE_H_


// lifetime for objects that don't destroy themselves
#define INFINITE_LIFETIME					2000000000

// number of lightning particles in beam
#define LIGHTNING_LENGTH					256


// properties of generic sphere particles
#define SPHERE_BM_INDX						BM_LIGHTNING1
#define SPHERE_REF_Z						10.0f //9.0f // 30.0f // 150.0f
#define SPHERE_PARTICLE_COLOR				255
#define SPHERE_PARTICLES					250 //512 //200

#define SPHERE_EXPLOSION_DURATION			250    //400
#define SPHERE_EXPLOSION_SPEED				FIXED_TO_GEOMV( 0x2800 ) //0x07000

#define SPHERE_EXPLOSION_BM_INDX			BM_FIREBALL3
#define SPHERE_EXPLOSION_COLOR       		168
#define SPHERE_EXPLOSION_REF_Z	       		20.0f

#define SPHERE_EXPL_PARTICLES				150

// properties of contracting sphere
#define SPHERE_CONTRACT_SPEED				FIXED_TO_GEOMV( 0x0c00 )
#define CONTRACTING_SPHERE_LIFETIME			8000
#define CONTRACTING_SPHERE_EXPANSION_TIME	800


// sphere animation type identifiers
#define SAT_NO_ANIMATION 					0x00000600
#define SAT_ROTATING						0x00000601
#define SAT_EXPLODING						0x00000402
#define SAT_PULSATING						0x00000503
#define SAT_CONTRACTING 					0x00000404
#define SAT_STOCHASTIC_MOTION				0x00000605

// anim type for lightning (no "real" sphere animtype)
#define SAT_LIGHTNING						0x00000006

// anim type for geometry particles (no "real" sphere animtype)
#define SAT_GENOBJECT						0x00000007

// anim type for photon particles (no "real" sphere animtype)
#define SAT_PHOTON                          0x00000008

// types for differently shaped spheres
#define SAT_SPHERETYPE_NORMAL				0x00000000
#define SAT_SPHERETYPE_DISC					0x00010000
#define SAT_SPHERETYPE_DISCWITHCORE			0x00020000

// special sphere flags
#define SAT_RESET_MEGASHIELD_FLAG			0x00100000
#define SAT_DECREMENT_EXTRA_COUNTER			0x00200000
#define SAT_AUTO_DEPLETE_PARTICLES			0x00400000

// special sphere anim types (combined)
#define SAT_MEGASHIELD_SPHERE				( SAT_ROTATING | SAT_RESET_MEGASHIELD_FLAG | SAT_AUTO_DEPLETE_PARTICLES )
#define SAT_ENERGYFIELD_SPHERE				( SAT_CONTRACTING | SAT_DECREMENT_EXTRA_COUNTER )

// sphere animation type identifier masks
#define SAT_NEEDS_REFCOORDS_MASK			0x00000100
#define SAT_VALID_FOR_OBJECTCENTERED_SPHERE	0x00000200
#define SAT_VALID_FOR_PSPHERE_OBJECT		0x00000400
#define SAT_BASIC_ANIM_MASK					0x0000ffff
#define SAT_SPHERE_TYPE_MASK				0x000f0000
#define SAT_SPECIAL_FLAGS_MASK				0xfff00000


// cluster type identifiers (pcluster_s::type)
#define CT_CONSTANT_VELOCITY				0x00000000
#define CT_LIGHTNING						0x00001201
#define CT_OBJECTCENTERED_SPHERE			0x00000602
#define CT_PARTICLE_SPHERE					0x00000103
#define CT_CALLBACK_TRAJECTORY				0x00000004
#define CT_CUSTOMDRAW						0x00000805
#define CT_GENOBJECT_PARTICLES				0x00000606
#define CT_PHOTON_SPHERE                    0x00000207

// cluster type identifier flags/masks (pcluster_s::type)
#define CT_PARTICLE_OBJ_MASK				0x00000100
#define CT_GENOBJECTRELATIVE_OBJ_MASK		0x00000200
#define CT_DONT_DRAW_IN_COCKPIT_MASK		0x00000400
#define CT_DONT_DRAW_AUTOMATICALLY			0x00000800
#define CT_DONT_DRAW_IF_BASE_VISNEVER		0x00001000
#define CT_EXTINFO_STORAGE					0x00010000
#define CT_DONT_CULL_WITH_GENOBJECT			0x00020000
#define CT_CLUSTER_GLOBAL_EXTINFO			0x00040000
#define CT_HINT_PARTICLES_IDENTICAL			0x00100000
#define CT_HINT_PARTICLES_HAVE_EXTINFO		0x00200000
#define CT_HINT_NO_APPEARANCE_ANIMATION		0x00400000
#define CT_HINT_NO_POSITIONAL_ANIMATION		0x00800000
#define CT_HINT_CULL_APPEARANCE_ANIMATION	0x01000000
#define CT_HINT_CULL_POSITIONAL_ANIMATION	0x02000000
#define CT_TYPEMASK							0x0000ffff
#define CT_TYPEENUMERATIONMASK				0x000000ff


// particle flags (field particle_s::flags)
#define PARTICLE_ACTIVE						0x00000001
#define PARTICLE_COLLISION					0x00000002

// linear particle distinguishing flags/masks
#define PARTICLE_IS_HELIX					0x00010000
#define PARTICLE_IS_PHOTON					0x00020000
#define PARTICLE_IS_MASK					0xffff0000


// single texture anim registration info --------------------------------------
//
struct ptexreg_s {

	int			deltatime;		// delta time to next entry (frame)
	char*		texname;		// name of frame texture
};


// single texture trafo registration info (currently same as ptrafo_s) --------
//
struct pxforeg_s {

	int			deltatime;		// delta time to next entry (frame)
	imgtrafo_s*	imgtrafo;		// image transformation for this frame
};


// registration info for particle definition ----------------------------------
//
struct pdefreg_s {

	ptexreg_s*	texinfo;		// info for texture table
	dword		textabsize;		// number of texture table entries
	dword		texstart;		// start index
	dword		texrep;			// repeat index
	dword		texend;			// end index

	pxforeg_s*	xfoinfo;		// info for trafo table
	dword		xfotabsize;		// number of trafo table entries
	dword		xfostart;		// start index
	dword		xfoend;			// end index
	dword		xforep;			// repeat index
};


// particle definition (texture animation) ------------------------------------
//
typedef texanim_s pdef_s;


// registered pdef as used by PART_API::ParticleDefinitions[] -----------------
//
struct pdefref_s {

	char*		defname;		// unique name for particle definition
	pdef_s*		def;			// actual definition
};


// extended definition/state info (to fields in particle_s) -------------------
//
struct pextinfo_s {

// extended definition

	pdef_s*		partdef;		// particle definition (until particle dies)
	pdef_s*		partdef_dest;	// particle definition (during destruction)

// extended state info

	word		tex_pos;		// current position in texture table
	short  		tex_time;		// time left until next advance in table
	word		xfo_pos;		// current position in trafo table
	short  		xfo_time;		// time left until next advance in table
};


// generic particle structure (size is 64 bytes) ------------------------------
//
struct particle_s {

	int			owner;		// particle's owner (used for identification in netgame)
	dword		flags;		// miscellaneous flags
	int			lifetime;	// lifetime after which automatic destruction commences
	pextinfo_s*	extinfo;	// extended definition/state info (may be NULL)
	int			bitmap;		// negative means single pixel; bitmapindex otherwise
	int			color;		// color for pixel particle
	int			sizebound;	// LOD threshold for switch between bitmap and pixel
	float		ref_z;		// reference z (distance for original-size bitmap)
	Vertex3		position;	// current position in 3-space
	Vector3		velocity;	// velocity vector (or current tangent to trajectory)
};

//NOTE:
// if ( extinfo != NULL ) field particle_s::bitmap has
// nothing to do with a bitmap anymore, but rather contains
// the iteration-type (iter_xx) with which the particle
// should be rendered in the loword and additional rendering
// flags in the hiword.

#define PART_REND_MASK_ITER		0x0000ffff
#define PART_REND_MASK_FLAGS	0xffff0000

#define PART_REND_NONE			0x00000000
#define PART_REND_NODEPTHCMP	0x00010000	// disable depth compare
#define PART_REND_NODEPTHSCALE	0x00020000	// no scale with depth coordinate
#define PART_REND_POINTVIS		0x00040000	// check point visibility

// flag to turn sizebound off (always use texture)
#define PRT_NO_SIZEBOUND	-1


// header for auxiliary memblock in cluster (normally accessed via userinfo) --
//
struct pusrinfo_s {

	int			infovalid;	// flag if actually valid
	size_t	 	blocksize;	// size of block in bytes
};


// basic particle cluster (animation/culling/memory allocation efficiency) ----
//
struct pcluster_s {

	pcluster_s*	next;		// [ THE SEQUENCE OF THE FIRST THREE FIELDS ]
	pcluster_s*	prec;		// [ *MUST NOT* BE CHANGED!!                ]
	particle_s*	rep;		// particle storage
	dword   	type;		// type of cluster (CT_xx: anim/struct-type/flags)
	int			numel;		// current number of contained particles
	int			maxnumel;	// maximum number of contained particles
	geomv_t		bdsphere;	// radius of bounding sphere (0 means none)
	pusrinfo_s*	userinfo;	// arbitrary user-defined info pertaining to cluster
};


// particle cluster for linear particles --------------------------------------
//
struct linear_pcluster_s;
typedef void (*linear_pcluster_fpt)( linear_pcluster_s*, int );
struct linear_pcluster_s : pcluster_s {

	linear_pcluster_fpt		callback;
};


// particle cluster with generic callback function for animation --------------
//
struct callback_pcluster_s;
typedef void (*callback_pcluster_fpt)( callback_pcluster_s* );
struct callback_pcluster_s : pcluster_s {

	callback_pcluster_fpt	callback;
};


// particle cluster for customdraw animation ----------------------------------
//
struct customdraw_pcluster_s;
typedef void (*customdraw_pcluster_fpt)( customdraw_pcluster_s* );
struct customdraw_pcluster_s : pcluster_s {

	const GenObject*		baseobject;	// owner object
	customdraw_pcluster_fpt	callback;
};


// particle cluster containing genobject relative particles -------------------
//
struct objectbase_pcluster_s : pcluster_s {

	GenObject*				baseobject;	// pointer to object that owns cluster
	objectbase_pcluster_s*	attachlist;	// pointer to next attached cluster
	int						animtype;	// animation type (SAT_xx)
};


// particle cluster containing position-relative particles --------------------
//
struct particleobj_pcluster_s : pcluster_s {

	Vertex3					origin;		// origin of cluster as a whole
	int						animtype;	// animation type (SAT_xx)
};


// cluster of particles comprising a sphere around a genobject ----------------
//
struct basesphere_pcluster_s : objectbase_pcluster_s {

	int		lifetime;			// current lifetime of entire sphere
	int		max_life;			// initial lifetime of entire sphere

	union {

		struct {				// SAT_ROTATING
			bams_t	pitch;
			bams_t	yaw;
			bams_t	roll;
		} rot;

		struct {				// SAT_STOCHASTIC_MOTION
			geomv_t	radius;
			int		speed;
			int		fcount;
		} rand;
	};
};


// particle cluster containing genobject relative photon particles ------------
//
struct photon_sphere_pcluster_s : objectbase_pcluster_s  {

    geomv_t contraction_speed;
    int     contraction_time;
    int     cur_contraction_time;
    int     max_loading_time;
    int     firing;
    int     numloads;
    Vertex3 center;
    int     alive;                      // current lifetime of entire sphere
    bams_t  pitch;
    bams_t  yaw;
    bams_t  roll;
};


// cluster of particles comprising a sphere at a specific position ------------
//
struct sphereobj_pcluster_s : particleobj_pcluster_s {

	int		lifetime;			// current lifetime of entire sphere
	int		max_life;			// initial lifetime of entire sphere

	union {

		struct {				// SAT_ROTATING
			bams_t	pitch;
			bams_t	yaw;
			bams_t	roll;
		} rot;

		struct {				// SAT_EXPLODING
			geomv_t	speed;
		} expl;

		struct {				// SAT_PULSATING
			geomv_t	amplitude;
			geomv_t	midradius;
			bams_t	frequency;	// actually: angle/timeunit (omega)
			bams_t	current_t;	// actually: current angle (omega*t)
			bams_t	pitch;
			bams_t	yaw;
			bams_t	roll;
		} puls;

		struct {				// SAT_CONTRACTING
			geomv_t	speed;
			int		expandtime;
			bams_t	pitch;
			bams_t	yaw;
			bams_t	roll;
		} cont;
	};
};


// cluster of lightning particles ---------------------------------------------
//
struct lightning_pcluster_s : objectbase_pcluster_s {

	int			sizzlespeed;
	int			framecount;
	Vertex3		beamstart1;
	Vertex3		beamstart2;
};


// cluster of particles that are part of an object's geometry -----------------
//
struct genobject_pcluster_s;
typedef void (*genobject_pcluster_fpt)( genobject_pcluster_s* );
struct genobject_pcluster_s : objectbase_pcluster_s {

	genobject_pcluster_fpt	callback;
};


// info how a particle should be drawn ----------------------------------------
//
struct pdrwinfo_s {

	int			bmindx;		// index of particle's bitmap
	int	   		pcolor;		// particle's color if not drawn as bitmap
	float		ref_z;		// reference z (scaling info)
	int			sizebnd;	// LOD threshold for switch between bitmap and pixel
	pextinfo_s*	extinfo;	// extended definition/state info (may be NULL)
};

//NOTE:
// this structure is used as argument to functions that create a whole cluster
// of particles instead of just a single particle. at the moment these are
// PRT_CreateObjectCenteredSphere() and PRT_CreateParticleSphereObject().
// (functions creating a single particle take a particle_s directly.)


// list of particle clusters --------------------------------------------------
//
extern pcluster_s *Particles;
extern pcluster_s *CurLinearCluster;
extern pcluster_s *CustomDrawCluster;


// typical lower boundary of particle bitmap size -----------------------------
//
extern int partbitmap_size_bound;


#endif // _PARTTYPE_H_


