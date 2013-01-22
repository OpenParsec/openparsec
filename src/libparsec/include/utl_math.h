/*
 * PARSEC HEADER: x_math.h
 */

#ifndef _X_MATH_H_
#define _X_MATH_H_


// ----------------------------------------------------------------------------
// MATHEMATICS SUBSYSTEM                                                      -
// ----------------------------------------------------------------------------


// thou shalt choose wise
#define USE_DOTPRODUCT_MACRO



// matrix that can be used as destination (MAT2)
extern pXmatrx DestXmatrx;

// interpolation info for remote player ---------------------------------------
//
struct playerlerp_s {

	fixed_t		curspeed;
	bams_t		curyaw;
	bams_t		curpitch;
	bams_t		curroll;
	geomv_t		curslidehorz;
	geomv_t		curslidevert;

	Xmatrx		dstposition;
	geomv_t		transvec_x;		// delta vec during
	geomv_t		transvec_y;		// transition
	geomv_t		transvec_z;
	int			transition;		// time countdown

	Quaternion	srcquat;		// start of slerp
	Quaternion	dstquat;		// end of slerp
	float		curalpha;		// current slerp alpha
	float		incalpha;		// delta for slerp alpha
};

// hermite arclen interpolation data ------------------------------------------
//
struct Hermite_ArcLen
{
	int		 num_steps;
	float* table_u;			
	float* table_s;
	Vector3* table_Vecs;
	float	 total_arc_len;
};

// external functions (MATH)

void	AdjointMtx( const Xmatrx smatrx, Xmatrx dmatrx );

void	MtxMtxMUL(  const Xmatrx matrxb, const Xmatrx  matrxa, Xmatrx  dmatrx );
void 	MtxMtxMULt( const Xmatrx matrxb, const Xmatrx  matrxa, Xmatrx  dmatrx );
void 	MtxVctMUL(  const Xmatrx matrx,  const Vector3 *svect, Vector3 *dvect );
void 	MtxVctMULt( const Xmatrx matrx,  const Vector3 *svect, Vector3 *dvect );
void 	DirVctMUL(  const Xmatrx matrx, geomv_t scalar, Vector3 *dvect );
void	RightVctMUL( const Xmatrx matrx, geomv_t scalar, Vector3 *dvect );
void	UpVctMUL( const Xmatrx matrx, geomv_t scalar, Vector3 *dvect );
void	VctReflect( const Vector3 *ivec, const Vector3 *normal, Vector3 *destvec );
void	CalcMovement( Vector3* move_offset, const Xmatrx frame, fixed_t forward, geomv_t horiz, geomv_t vert, refframe_t refframes );

void 	GetSinCos( dword angle, sincosval_s *resultp );

void 	ObjRotX( Xmatrx matrix, bams_t pitch );
void 	ObjRotY( Xmatrx matrix, bams_t yaw );
void 	ObjRotZ( Xmatrx matrix, bams_t roll );
void 	CamRotX( Xmatrx matrix, bams_t pitch );
void 	CamRotY( Xmatrx matrix, bams_t yaw );
void 	CamRotZ( Xmatrx matrix, bams_t roll );

geomv_t	DotProduct( const Vector3 *vect1, const Vector3 *vect2 );
void 	CrossProduct( const Vector3 *vect1, const Vector3 *vect2, Vector3 *cproduct );
void	CrossProduct2( const geomv_t *vect1, const geomv_t *vect2, geomv_t *cproduct );

void 	ReOrthoMtx( Xmatrx matrix );

void 	ProcessObject( GenObject *object );


// external functions (MAT2)

geomv_t	VctLen( Vector3 *vec );
geomv_t	VctLenX( Vector3 *vec );
void	NormVct( Vector3 *vec );
void	NormVctX( Vector3 *vec );
void	NormMtx( Xmatrx matrix );
void	ReOrthoNormMtx( Xmatrx matrix );
void	MakeIdMatrx( Xmatrx matrx );
void	MakeNonTranslationMatrx( const Xmatrx matrx, Xmatrx dmatrx );
void	CalcOrthoInverse( const Xmatrx matrx, Xmatrx dmatrx );
void	CalcObjSpaceCamera( const GenObject *objectp, Vector3 *cameravec );
void	TransformVolume( const Xmatrx matrx, Plane3 *vol_in, Plane3 *vol_out, dword cullmask );
void	BackTransformVolume( const Xmatrx matrx, Plane3 *vol_in, Plane3 *vol_out, dword cullmask );

int		QuaternionIsUnit( const Quaternion *quat );
int		QuaternionIsUnit_f( const Quaternion_f *quat );
void	QuaternionMakeUnit( Quaternion *quat );
void	QuaternionMakeUnit_f( Quaternion_f *quat );
void	QuaternionInvertUnit( Quaternion *quat );
void	QuaternionInvertUnit_f( Quaternion_f *quat );
void	QuaternionInvertGeneral( Quaternion *quat );
void	QuaternionInvertGeneral_f( Quaternion_f *quat );
void	QuaternionFromMatrx( Quaternion *quat, const Xmatrx matrix );
void	QuaternionFromMatrx_f( Quaternion_f *quat, const Xmatrx matrix );
void	MatrxFromQuaternion( Xmatrx matrix, const Quaternion *quat );
void	MatrxFromQuaternion_f( Xmatrx matrix, const Quaternion_f *quat );
void	MatrxFromAngularDisplacement( Xmatrx matrix, bams_t angle, Vertex3 *axis );
void	QuaternionMUL( Quaternion *qd, const Quaternion *qb, const Quaternion *qa );
void	QuaternionMUL_f( Quaternion_f *qd, const Quaternion_f *qb, const Quaternion_f *qa );
void	QuaternionSlerp( Quaternion *qd, const Quaternion *qa, const Quaternion *qb, float alpha );
void	QuaternionSlerp_f( Quaternion_f *qd, const Quaternion_f *qa, const Quaternion_f *qb, float alpha );
void	QuaternionSlerpFrames( Xmatrx slerp_frame, Xmatrx start_frame, Xmatrx end_frame, float t );
void	CalcSlerpedMatrix( Xmatrx matrix, const playerlerp_s *playerlerp );

void	Hermite_Interpolate( Vector3* dest, float t, const Vector3* start, const Vector3* end, const Vector3* start_tan, const Vector3* end_tan );
Hermite_ArcLen* Hermite_ArcLen_InitData( int num_steps, const Vector3* start, const Vector3* end, const Vector3* start_tan, const Vector3* end_tan );
void	Hermite_ArcLen_KillData( Hermite_ArcLen* hermite_data );
void	Hermite_ArcLen_Interpolate( const Hermite_ArcLen* hermite_data, float s, Vector3* interp );
										
void	DumpMatrix( Xmatrx mat );


// geometric value MUL instruction --------------------------------------------
//
#define GEOMV_MUL(a,b)		((a)*(b))


// geometric value DIV instruction --------------------------------------------
//
#define GEOMV_DIV(a,b)		((a)/(b))


// dot product macro/function encapsulation -----------------------------------
//
#ifdef USE_DOTPRODUCT_MACRO
	#define DOT_PRODUCT(u,v) \
		(GEOMV_MUL((u)->X,(v)->X)+GEOMV_MUL((u)->Y,(v)->Y)+GEOMV_MUL((u)->Z,(v)->Z))
#else
	#define DOT_PRODUCT(u,v) DotProduct((u),(v))
#endif


// make vector identity (NULL vector) -----------------------------------------
//
inline
void MakeIdVector( Vector3& vec )
{
	vec.X = vec.Y = vec.Z = GEOMV_0;
}


// fetch x vector of matrix ---------------------------------------------------
//
inline
void FetchXVector( const Xmatrx matrx, Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	vec->X = matrx[ 0 ][ 0 ];
	vec->Y = matrx[ 1 ][ 0 ];
	vec->Z = matrx[ 2 ][ 0 ];
}


// fetch y vector of matrix ---------------------------------------------------
//
inline
void FetchYVector( const Xmatrx matrx, Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	vec->X = matrx[ 0 ][ 1 ];
	vec->Y = matrx[ 1 ][ 1 ];
	vec->Z = matrx[ 2 ][ 1 ];
}


// fetch z vector of matrix ---------------------------------------------------
//
inline
void FetchZVector( const Xmatrx matrx, Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	vec->X = matrx[ 0 ][ 2 ];
	vec->Y = matrx[ 1 ][ 2 ];
	vec->Z = matrx[ 2 ][ 2 ];
}


// fetch t vector of matrix (translation part) --------------------------------
//
inline
void FetchTVector( const Xmatrx matrx, Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	vec->X = matrx[ 0 ][ 3 ];
	vec->Y = matrx[ 1 ][ 3 ];
	vec->Z = matrx[ 2 ][ 3 ];
}


// store x vector of matrix ---------------------------------------------------
//
inline
void StoreXVector( Xmatrx matrx, const Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	matrx[ 0 ][ 0 ] = vec->X;
	matrx[ 1 ][ 0 ] = vec->Y;
	matrx[ 2 ][ 0 ] = vec->Z;
}


// store y vector of matrix ---------------------------------------------------
//
inline
void StoreYVector( Xmatrx matrx, const Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	matrx[ 0 ][ 1 ] = vec->X;
	matrx[ 1 ][ 1 ] = vec->Y;
	matrx[ 2 ][ 1 ] = vec->Z;
}


// store z vector of matrix ---------------------------------------------------
//
inline
void StoreZVector( Xmatrx matrx, const Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	matrx[ 0 ][ 2 ] = vec->X;
	matrx[ 1 ][ 2 ] = vec->Y;
	matrx[ 2 ][ 2 ] = vec->Z;
}


// store t vector of matrix (translation part) --------------------------------
//
inline
void StoreTVector( Xmatrx matrx, const Vector3 *vec )
{
	ASSERT( matrx != NULL );
	ASSERT( vec != NULL );

	matrx[ 0 ][ 3 ] = vec->X;
	matrx[ 1 ][ 3 ] = vec->Y;
	matrx[ 2 ][ 3 ] = vec->Z;
}


// primitive vector arithmetics -----------------------------------------------
//
#define VECADD(c,a,b) { \
	(c)->X = (a)->X+(b)->X; \
	(c)->Y = (a)->Y+(b)->Y; \
	(c)->Z = (a)->Z+(b)->Z; \
}

#define VECADD_UVW(c,a,b) { \
	(c)->X = (a)->X+(b)->X; \
	(c)->Y = (a)->Y+(b)->Y; \
	(c)->Z = (a)->Z+(b)->Z; \
	(c)->W = (a)->W+(b)->W; \
	(c)->U = (a)->U+(b)->U; \
	(c)->V = (a)->V+(b)->V; \
}

#define VECADD_RGBA(c,a,b) { \
	(c)->X = (a)->X+(b)->X; \
	(c)->Y = (a)->Y+(b)->Y; \
	(c)->Z = (a)->Z+(b)->Z; \
	(c)->R = (a)->R+(b)->R; \
	(c)->G = (a)->G+(b)->G; \
	(c)->B = (a)->B+(b)->B; \
	(c)->A = (a)->A+(b)->A; \
}

#define VECADD_UVW_RGBA(c,a,b) { \
	(c)->X = (a)->X+(b)->X; \
	(c)->Y = (a)->Y+(b)->Y; \
	(c)->Z = (a)->Z+(b)->Z; \
	(c)->W = (a)->W+(b)->W; \
	(c)->U = (a)->U+(b)->U; \
	(c)->V = (a)->V+(b)->V; \
	(c)->R = (a)->R+(b)->R; \
	(c)->G = (a)->G+(b)->G; \
	(c)->B = (a)->B+(b)->B; \
	(c)->A = (a)->A+(b)->A; \
}

#define VECSUB(c,a,b) { \
	(c)->X = (a)->X-(b)->X; \
	(c)->Y = (a)->Y-(b)->Y; \
	(c)->Z = (a)->Z-(b)->Z; \
}

#define VECSUB_UVW(c,a,b) { \
	(c)->X = (a)->X-(b)->X; \
	(c)->Y = (a)->Y-(b)->Y; \
	(c)->Z = (a)->Z-(b)->Z; \
	(c)->W = (a)->W-(b)->W; \
	(c)->U = (a)->U-(b)->U; \
	(c)->V = (a)->V-(b)->V; \
}

#define VECSUB_RGBA(c,a,b) { \
	(c)->X = (a)->X-(b)->X; \
	(c)->Y = (a)->Y-(b)->Y; \
	(c)->Z = (a)->Z-(b)->Z; \
	(c)->R = (a)->R-(b)->R; \
	(c)->G = (a)->G-(b)->G; \
	(c)->B = (a)->B-(b)->B; \
	(c)->A = (a)->A-(b)->A; \
}

#define VECSUB_UVW_RGBA(c,a,b) { \
	(c)->X = (a)->X-(b)->X; \
	(c)->Y = (a)->Y-(b)->Y; \
	(c)->Z = (a)->Z-(b)->Z; \
	(c)->W = (a)->W-(b)->W; \
	(c)->U = (a)->U-(b)->U; \
	(c)->V = (a)->V-(b)->V; \
	(c)->R = (a)->R-(b)->R; \
	(c)->G = (a)->G-(b)->G; \
	(c)->B = (a)->B-(b)->B; \
	(c)->A = (a)->A-(b)->A; \
}

// multiply vector with scalar ------------------------------------------------
//
#define VECMULS(c,a,b) {   \
	(c)->X = (b) * (a)->X; \
	(c)->Y = (b) * (a)->Y; \
	(c)->Z = (b) * (a)->Z; \
}


// saxpy vector arithmetics ---------------------------------------------------
//

// y = a*x+y (a is scalar)
#define SAXPY(a,x,y) { \
	(y)->X = GEOMV_MUL((a),(x)->X)+(y)->X; \
	(y)->Y = GEOMV_MUL((a),(x)->Y)+(y)->Y; \
	(y)->Z = GEOMV_MUL((a),(x)->Z)+(y)->Z; \
}

// c = a*x+y (a is scalar)
#define CSAXPY(c,a,x,y) { \
	(c)->X = GEOMV_MUL((a),(x)->X)+(y)->X; \
	(c)->Y = GEOMV_MUL((a),(x)->Y)+(y)->Y; \
	(c)->Z = GEOMV_MUL((a),(x)->Z)+(y)->Z; \
}

// c = a*x+y (a is scalar)
// also calculates [uvw]
#define CSAXPY_UVW(c,a,x,y) { \
	(c)->X = GEOMV_MUL((a),(x)->X)+(y)->X; \
	(c)->Y = GEOMV_MUL((a),(x)->Y)+(y)->Y; \
	(c)->Z = GEOMV_MUL((a),(x)->Z)+(y)->Z; \
	(c)->W = GEOMV_MUL((a),(x)->W)+(y)->W; \
	(c)->U = GEOMV_MUL((a),(x)->U)+(y)->U; \
	(c)->V = GEOMV_MUL((a),(x)->V)+(y)->V; \
}


// accelerated dot product with plane normal (struct Plane3) ------------------
//
#define PLANE_DOT(p,x) ( PLANE_AXIAL(p) ? \
	( PLANE_AXISCOMP(p) * ((geomv_t*)x)[ PLANE_CMPAXIS(p) ] ) : \
	DOT_PRODUCT(PLANE_NORMAL(p),(x)) )


// reject/accept point tests for a cull plane ---------------------------------
//
#define PLANEDIST_REJECTPOINT(p,b) ( \
	GEOMV_MUL( (b)->minmax[ (p)->reacx.reject[ 0 ] ], (p)->plane.X ) + \
	GEOMV_MUL( (b)->minmax[ (p)->reacx.reject[ 1 ] ], (p)->plane.Y ) + \
	GEOMV_MUL( (b)->minmax[ (p)->reacx.reject[ 2 ] ], (p)->plane.Z ) - \
	(p)->plane.D )

#define PLANEDIST_ACCEPTPOINT(p,b) ( \
	GEOMV_MUL( (b)->minmax[ (p)->reacx.accept[ 0 ] ], (p)->plane.X ) + \
	GEOMV_MUL( (b)->minmax[ (p)->reacx.accept[ 1 ] ], (p)->plane.Y ) + \
	GEOMV_MUL( (b)->minmax[ (p)->reacx.accept[ 2 ] ], (p)->plane.Z ) - \
	(p)->plane.D )


// project 3-D vertex onto screen ---------------------------------------------
//
#define PROJECT_TO_SCREEN(v,s) { \
	(s).X = GEOMV_TO_COORD( GEOMV_DIV( (v).X, (v).Z ) ) + Screen_XOfs; \
	(s).Y = GEOMV_TO_COORD( GEOMV_DIV( (v).Y, (v).Z ) ) + Screen_YOfs; \
}


// elementary transformations into screen space -------------------------------
//
#define SCREENSPACE_XOZ(v,z)	( GEOMV_TO_COORD( GEOMV_MUL( (v).X, (z) ) ) + Screen_XOfs )
#define SCREENSPACE_YOZ(v,z)	( GEOMV_TO_COORD( GEOMV_MUL( (v).Y, (z) ) ) + Screen_YOfs )
#define SCREENSPACE_OOZ(v)		( GEOMV_DIV( GEOMV_1, (v).Z ) )


// depth value calculation ----------------------------------------------------
//
#ifdef FRACTIONAL_DEPTH_VALUES
	#define DEPTHBUFF_VALUE(z)	( (depth_t) (z) )
	#define DEPTHBUFF_OOZ(z)	( (depth_t) GEOMV_DIV( GEOMV_1, (z) ) )
#else
	#define DEPTHBUFF_VALUE(z)	( (word)GEOMV_TO_FIXED(z) )
	#define DEPTHBUFF_OOZ(z)	( (word)GEOMV_TO_FIXED( GEOMV_DIV(GEOMV_1,(z)) ) )
#endif


#endif // _X_MATH_H_


