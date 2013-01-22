/*
 * PARSEC HEADER: ro_objmc.h
 */

#ifndef _RO_OBJMC_H_
#define _RO_OBJMC_H_


// --------------------------------------------------
// messy macros for low level object rendering
// --------------------------------------------------


// don't check everything in debug mode --
#ifdef RELAXED_DEBUG_MODE

#undef CHECKHEAPBASEREF
#undef CHECKHEAPREF
#undef CHECKMEMINTEGRITY

#define CHECKHEAPBASEREF( r )	{}
#define CHECKHEAPREF( r )		{}
#define CHECKMEMINTEGRITY()		{}

#endif


// ------------------------------------
#ifdef DEBUG
	#define CHECKOBJECTREF( r ) if((r)==MyShip) {} else CHECKHEAPBASEREF((r))
#else
	#define CHECKOBJECTREF( r ) {}
#endif


// ------------------------------------
#ifndef UPDATE_SOUND
	#define AUDs_MaintainSound() {}
#endif


// ------------------------------------
#ifndef COUNT_RENDERED_POLYGONS
	#define COUNTPOLYGONS(a)	(a)
#else
	#define COUNTPOLYGONS(a)	if ((a)) { NumRenderedPolygons++; } else { }
#endif


// ------------------------------------
//#define BACKFACE_TOL_EPS		600000
#define BACKFACE_TOL_EPS		AUXDATA_BSP_BACKFACE_TOLERANCE_EPS


// ------------------------------------
#ifndef CULL_BACKFACES_IN_BSP

	#define BSPBACKFACETEST(a) (a)

#else

	#ifndef CHECK_BACKFACE_EPS

		#define BSPBACKFACETEST(a) \
			if ( ( bsp_objectp->ObjectType & TYPEBACKFACEMASK ) != 0 ) \
				{ (a); } else { }

	#else

		#define BSPBACKFACETEST(a) \
			if ( ( sprod > -BACKFACE_TOL_EPS ) ||  \
				 ( ( bsp_objectp->ObjectType & TYPEBACKFACEMASK ) != 0 ) ) \
				{ (a); } else { }

	#endif

#endif


// ------------------------------------
#ifndef TWO_SIDES_POSSIBLE

	#define BACKFACINGBSPNODE(n,p,v,f)	{}

#else

	#define BACKFACINGBSPNODE(n,p,v,f)	BSPBACKFACETEST( \
			RO_DrawBSPNode( (n), (p), (v), (f) ) )

#endif


// ------------------------------------
#define CALCTEXTUREGRADIENTS(o,f) { \
	MtxMtxMUL( (o)->CurrentXmatrx, (f)->TexXmatrx, DestXmatrx ); \
	AdjointMtx( DestXmatrx, (f)->CurTexXmatrx ); }

/*
	if ( ( (f)->Shading == persptex_shad ) || \
		 ( (f)->Shading == ipol1tex_shad ) ) { \
		MtxMtxMUL( (o)->CurrentXmatrx, (f)->TexXmatrx, DestXmatrx ); \
		AdjointMtx( DestXmatrx, (f)->CurTexXmatrx ); \
	}
*/


// ------------------------------------
//#define BSP_ERROR_EPS		80000
#define BSP_ERROR_EPS		AUXDATA_BSP_ERROR_EPS


// ------------------------------------
#ifdef ENABLE_BSP_ERROR_TRACING

#define CHECKBSPPLANE(s,n,p,v,f)															\
																							\
	(s) > -BSP_ERROR_EPS && (s) < BSP_ERROR_EPS ) {											\
																							\
		if ( AUX_BSP_DO_EXTENDED_SPROD_CHECK ) {											\
			AUXDATA_BSP_SCALARPRODUCT_VALUE = abs( (s) );									\
																							\
			if ( !AUX_BSP_EXTCHECK_SKIPSUBTREE ) {											\
				if ( AUX_BSP_EXTCHECK_DRAW_NODE )											\
					RO_DrawBSPNode( (n), (p), (v), (f) );									\
																							\
				if ( AUX_BSP_EXTCHECK_BACKNODE_FIRST ) {									\
					if ( bsp_objectp->BSPTree[ bspnode ].BackTree > 0 )						\
						RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].BackTree );		\
					if ( bsp_objectp->BSPTree[ bspnode ].FrontTree > 0 )					\
						RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].FrontTree );	\
				} else {																	\
					if ( bsp_objectp->BSPTree[ bspnode ].FrontTree > 0 )					\
						RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].FrontTree );	\
					if ( bsp_objectp->BSPTree[ bspnode ].BackTree > 0 )						\
						RO_TraverseBSPTree( bsp_objectp->BSPTree[ bspnode ].BackTree );		\
				}																			\
			}																				\
																							\
		} else {                                                                    		\
			AUXDATA_BSP_SCALARPRODUCT_VALUE = abs( (s) );                           		\
			if ( (s) > 0 ) (s) = BSP_ERROR_EPS; else (s) = -BSP_ERROR_EPS;          		\
			goto testplane;                                                         		\
		}                                                                           		\
																							\
	} else if ( (s) > 0

#else

#define CHECKBSPPLANE(s,n,p,v,f)	( (s) > 0 )

#endif


#endif // _RO_OBJMC_H_


