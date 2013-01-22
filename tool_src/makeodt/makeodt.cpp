/*
 * makeodt.cpp
 */


// bsplib headers
#include "BRep.h"
#include "InputData3D.h"
#include "OutputData3D.h"
#include "BspObjectList.h"
#include "BoundingBox.h"
#include "ObjectBSPNode.h"
#include "ObjectBinFormat.h"

#include "getopt.h"

using BspLib::InputData3D;
using BspLib::BspObjectList;
using BspLib::Vector3;
using BspLib::Vertex3;
using BspLib::ObjectBSPTree;


// string constants -----------------------------------------------------------
//
static const char options_invalid[]	= "\nUse \"-h\" for a list of command line options.\n";


// pointer to input data object -----------------------------------------------
//
class Scene {

public:
	Scene() { m_inputscene = NULL; m_objectlist = NULL; m_objbsptree = NULL; }
	~Scene() { delete m_objbsptree; delete m_objectlist; }

	void			AllocObjectList();
	void			DestroyObjectList();

	void			BuildObjectBSPTree();

	void			setInput( InputData3D *inp ) { m_inputscene = inp; }
	InputData3D*	getInput() { return m_inputscene; }
	void			deleteInput() { if ( m_inputscene != NULL ) { delete m_inputscene; m_inputscene = NULL; } }

	BspObjectList*	getObjectList() { return m_objectlist; }

private:
	// pointer to input data object
	BspLib::InputData3D*	m_inputscene;

	// object bsp tree (convex subspaces contain entire objects)
	BspLib::ObjectBSPTree*	m_objbsptree;

	// object list
	BspLib::BspObjectList*	m_objectlist;
};


// allocate new objectlist (an already existing list will be destroyed) -------
//
void Scene::AllocObjectList()
{
	if ( m_objbsptree != NULL ) {
		delete m_objbsptree;
		m_objbsptree = NULL;
	}

	if ( m_objectlist != NULL ) {
		delete m_objectlist;
	}
	m_objectlist = new BspObjectList;
}


// destroy objectlist ---------------------------------------------------------
//
void Scene::DestroyObjectList()
{
	delete m_objbsptree;
	m_objbsptree = NULL;

	delete m_objectlist;
	m_objectlist = NULL;
}


// build a bsp tree of objects from unordered object list ---------------------
//
void Scene::BuildObjectBSPTree()
{
	if ( m_objectlist != NULL ) {
		BspLib::BoundingBox *boxlist = m_objectlist->BuildBoundingBoxList();
		m_objbsptree = new BspLib::ObjectBSPTree;
		m_objbsptree->InitTree( boxlist ? boxlist->PartitionSpace() : NULL );
	}
}


// ----------------------------------------------------------------------------
//
static Scene* world = NULL;


// filename of currently loaded object file -----------------------------------
//
static char current_object_file[ PATH_MAX + 1 ] = "";


// ----------------------------------------------------------------------------
//
static char input_file_name[ PATH_MAX + 1 ] = "";


// ----------------------------------------------------------------------------
//
static int bspgenopt_actions[]		= { 0, 0, 0, 0 };
static int bspgenopt_outpoptions[]	= { 0, 0, 0, 0 };
static int bspgenopt_policy[]		= { 0, 0, 0, 0 };
static int bspgenopt_samplesize		= 0;
static int bspgenopt_sampletype		= 0;

static int bspgeoopt_pregeometry[]	= { 0, 0, 0, 0 };
static int bspgeoopt_postgeometry[]	= { 0, 0, 0, 0 };
static int bspgeoopt_enforceeps[]	= { 0, 0, 0, 0 };
static double bspgeoopt_epsilons[]	= { 0.0, 0.0, 0.0, 0.0 };

static int vrmlopt_gen[]			= { 0, 0, 0, 0, 0, 0 };
static int vrmlopt_lod				= 0;
static int vrmlopt_tessellation		= 8;

static int mesgopt_mesg[]			= { 0, 0, 0, 0, 0 };
static int mesgopt_misc[]			= { 0, 0, 0, 0 };
static int mesgopt_stat[]			= { 0, 0, 0, 0 };
static int mesgopt_err[]			= { 0, 0, 0, 0 };

static int objprocessor_opt[]		= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };


// submit selected bsp compilation options to bsplib --------------------------
//
PRIVATE
void SetBspLibOptions()
{
	using namespace BspLib;

	if ( bspgeoopt_enforceeps[ 3 ] ) {
		EpsAreas::eps_scalarproduct		= bspgeoopt_epsilons[ 3 ];
		EpsAreas::eps_planethickness	= bspgeoopt_epsilons[ 3 ];
//		EpsAreas::eps_vanishdenominator	= bspgeoopt_epsilons[ 3 ];
//		EpsAreas::eps_vanishcomponent	= bspgeoopt_epsilons[ 3 ];
	}
	if ( bspgeoopt_enforceeps[ 2 ] ) {
		EpsAreas::eps_pointonlineseg	= bspgeoopt_epsilons[ 2 ];
	}
	if ( bspgeoopt_enforceeps[ 1 ] ) {
		EpsAreas::eps_vertexmergearea	= bspgeoopt_epsilons[ 1 ];
	}

	static int splittercrit_values[] = {
		Polygon::SPLITTERCRIT_FIRST_POLY,
		Polygon::SPLITTERCRIT_SAMPLE_FIRST_N,
		Polygon::SPLITTERCRIT_SAMPLE_ALL,
		Polygon::SPLITTERCRIT_RANDOM_SAMPLE
	};

	InputData3D::EnableRGBConversion( bspgenopt_actions[ 1 ] );
	InputData3D::EnableMaximumExtent( bspgeoopt_pregeometry[ 3 ], bspgeoopt_epsilons[ 0 ] );
	InputData3D::EnableScaleFactors( bspgenopt_actions[ 2 ] );
	InputData3D::EnableAxesChange( bspgenopt_actions[ 3 ] );
	InputData3D::EnableAllowNGons( !bspgenopt_policy[ 1 ] );

	Polygon::setSplitterSelection( splittercrit_values[ bspgenopt_sampletype ] );
	Polygon::setSampleSize( bspgenopt_samplesize );
	Polygon::setTriangulationFlag( bspgenopt_policy[ 3 ] );

	BspObject::setEliminateDoubletsOnMergeFlag( bspgenopt_policy[ 2 ] );

	int msgflags = Polygon::MESSAGEMASK_DISPLAY_ALL & ~Polygon::MESSAGE_CHECKING_POLYGON_PLANES;
	if ( !mesgopt_mesg[ 0 ] )
		msgflags &= ~(	Polygon::MESSAGE_NEW_SPLITVERTEX |
						Polygon::MESSAGE_REUSING_SPLITVERTEX |
						Polygon::MESSAGE_TRACEVERTEX_INSERTED );
	if ( !mesgopt_mesg[ 1 ] )
		msgflags &= ~Polygon::MESSAGE_SPLITTING_QUADRILATERAL;
	if ( !mesgopt_mesg[ 2 ] )
		msgflags &= ~( Polygon::MESSAGE_STARTVERTEX_IN_SPLITTER_PLANE | Polygon::MESSAGE_VERTEX_IN_SPLITTER_PLANE );
	if ( !mesgopt_mesg[ 3 ] )
		msgflags &= ~Polygon::MESSAGE_INVOCATION;
	if ( !mesgopt_mesg[ 4 ] )
		msgflags &= ~Polygon::MESSAGE_SPLITTING_POLYGON;
	Polygon::setDisplayMessagesFlag( msgflags );

	BRep::setTriangulation( vrmlopt_gen[ 2 ] );
	BRep::setMaterialFlag( vrmlopt_gen[ 4 ] );
	BRep::setMirrorTextureVFlag( vrmlopt_gen[ 5 ] );
	BRep::setTessellation( vrmlopt_tessellation );
}


// set global bsplib options to user specified values -------------------------
//
PRIVATE
void InitBspLibOptions()
{
	bspgeoopt_epsilons[ 3 ]		= BspLib::EpsAreas::eps_planethickness;
	bspgeoopt_epsilons[ 2 ]		= BspLib::EpsAreas::eps_pointonlineseg;
	bspgeoopt_epsilons[ 1 ]		= BspLib::EpsAreas::eps_vertexmergearea;

	bspgeoopt_epsilons[ 0 ]		= 600.0;// maximum extents to force

	bspgeoopt_enforceeps[ 0 ]	= 0;	// use default epsilon areas
	bspgeoopt_enforceeps[ 1 ]	= 0;
	bspgeoopt_enforceeps[ 2 ]	= 0;
	bspgeoopt_enforceeps[ 3 ]	= 0;

	bspgenopt_actions[ 1 ]		= 0;	// don't convert color indexes to rgb triplets
	bspgenopt_actions[ 2 ]		= 0;	// don't apply specified object scale factors
	bspgenopt_actions[ 3 ]		= 0;	// don't apply axes mapping

	bspgenopt_policy[ 0 ]		= 1;	// always normalize normal vectors
	bspgenopt_policy[ 1 ]		= 1;	// allow only triangles and quadrilaterals
	bspgenopt_policy[ 2 ]		= 1;	// eliminate vertex doublets on object merge
	bspgenopt_policy[ 3 ]		= 0;	// don't always triangulate

	vrmlopt_lod					= 0;
	vrmlopt_tessellation		= 4;	// set vrml tessellation resolution per PI/2
	vrmlopt_gen[ 1 ]			= 1;	// tessellate vrml primitives
	vrmlopt_gen[ 2 ]			= 0;	// triangulate vrml objects
	vrmlopt_gen[ 3 ]			= 1;	// build bounding box hierarchy
	vrmlopt_gen[ 4 ]			= 0;	// don't use full material specification
	vrmlopt_gen[ 5 ]			= 1;	// mirror texture v axis

	mesgopt_misc[ 0 ]			= 0;	// don't create messagelog
	mesgopt_misc[ 1 ]			= 0;	// don't create output window

	bspgenopt_samplesize		= 20;	// set n=20 (sample size)
	bspgenopt_sampletype		= 1;	// sample first n polygons

	bspgeoopt_pregeometry[ 0 ]	= 0;	// n/a
	bspgeoopt_pregeometry[ 1 ]	= 0;	// n/a
	bspgeoopt_pregeometry[ 2 ]	= 1;	// check planes of faces
	bspgeoopt_pregeometry[ 3 ]	= 1;	// enforce maximum object extents

	bspgeoopt_postgeometry[ 0 ]	= 0;	// don't insert trace vertices
	bspgeoopt_postgeometry[ 1 ] = 0;	// check for multiple vertices
	bspgeoopt_postgeometry[ 2 ] = 0;	// don't calc node bounding boxes
	bspgeoopt_postgeometry[ 3 ] = 0;	// don't calc explicit separator planes

	mesgopt_mesg[ 0 ]			= 1;	// show message on vertex insertion
	mesgopt_mesg[ 1 ]			= 1;	// show message on triangulation of quadrilateral
	mesgopt_mesg[ 2 ]			= 0;	// don't show message on vertex contained in splitter plane
	mesgopt_mesg[ 3 ]			= 0;	// don't show message on every call of PartitionSpace()
	mesgopt_mesg[ 4 ]			= 1;	// show message on splitting of polygon

	mesgopt_stat[ 0 ]			= 0;	// don't display basic statistics
	mesgopt_stat[ 1 ]			= 0;	// don't display detailed statistics

	bspgenopt_outpoptions[ 0 ]	= 1;	// write normal vectors
	bspgenopt_outpoptions[ 1 ]	= 1;	// write palette info
	bspgenopt_outpoptions[ 2 ]	= 0;	// don't write compiler info
	bspgenopt_outpoptions[ 3 ]	= 0;	// don't write viewer info

	mesgopt_err[ 0 ]			= 1;	// display error message on non-convex polygon

	SetBspLibOptions();					// pass options to bsplib
}





// check if bsp tree is available for current scene ---------------------------
//
int BspTreeAvailable()
{
	BspLib::BspObjectList *objectlist = world->getObjectList();
	BspLib::BspObject *obj = objectlist ? objectlist->getListHead() : NULL;
	return ( obj ? obj->BspTreeAvailable() : 0 );
}


// merge entire object list into single object --------------------------------
//
void MergeListIntoSingleObject()
{
	if ( world->getObjectList() ) {

		if ( !BspTreeAvailable() ) {

			// configure BspLib
			SetBspLibOptions();

			// do actual merge
			world->getObjectList()->CollapseObjectList();
			MSGOUT( "Objectlist merged into single object.\n" );

		} else {

			MSGOUT( "Objects already bsp-compiled: Merging not possible.\n" );
		}

	} else {

		MSGOUT( "No objects to merge.\n" );
	}
}


// ----------------------------------------------------------------------------
//
enum {

	OUTPUT_FORMAT_SINGLE,
	OUTPUT_FORMAT_ODT,
	OUTPUT_FORMAT_OD2,
};



static int output_format = OUTPUT_FORMAT_OD2;
static int bsp_compile_done = FALSE;


static int critical_bsplib_error = 0;


// handle critical error encountered within bsplib ----------------------------
//
PRIVATE
int CriticalBspLibError( int flags )
{
/*
	// close message log if enabled
	if ( messagelog_fp != NULL ) {
		fclose( messagelog_fp );
		messagelog_fp = NULL;
	}
*/
	if ( ( flags & BspLib::SystemIO::CRITERR_ALLOW_RET ) == 0 ) {
		return 0;
	} else {
		critical_bsplib_error = 1;
		return 1;
	}
}


// close an open data file ----------------------------------------------------
//
PRIVATE
void CloseDataFile()
{
	// reset compile done flag
	bsp_compile_done = FALSE;

	// delete old InputData3D object
	world->deleteInput();

	// destroy old object list
	world->DestroyObjectList();

	// no current file name
	*current_object_file = 0;
}


// read data file, parse it, and establish scene graph in memory --------------
//
PRIVATE
int ReadDataFile( char *fullfname, char *fname )
{
	// delete old InputData3D object
	world->deleteInput();

	// allocate new BspObjectList (delete old)
	world->AllocObjectList();

	// configure BspLib
	SetBspLibOptions();

	// create input data
	BspObjectList *objectlist = world->getObjectList();
	world->setInput( new BspLib::InputData3D( *objectlist, fullfname ) );

	// process already parsed input if valid
	if ( !world->getInput()->InputDataValid() ) {
		CloseDataFile();
		return FALSE;
	}

	// calculate plane normals if not already valid (read from input file)
	objectlist->ProcessObjects( BspObjectList::CALC_PLANE_NORMALS );

	// check if bsp tree read from file and update caption accordingly
	int anybsp = objectlist->BspTreeAvailable();
//	Global::UpdateCaption( hwnd, fname, anybsp ?
//		Global::CAPTION_BSPCOMPILED : Global::CAPTION_NOBSPAVAILABLE );

	// check if only flat tree available
	if ( anybsp && !objectlist->BSPTreeAvailable() ) {
		// if only flat bsp tree available build linked tree from it
		objectlist->ProcessObjects( BspObjectList::BUILD_FROM_FLAT );
	}

//	if ( !anybsp ) {
//		world.setUseBSP( FALSE );
//		CheckRenderingMenuItems( GetMenu( hwndMain ) );
//	}

	// set current object file name
	strcpy( current_object_file, fname );

	return TRUE;
}


// ----------------------------------------------------------------------------
//
PRIVATE
int OpenInput()
{
	// reset compile done flag
	bsp_compile_done = FALSE;

	// register callbacks with bsplib
	critical_bsplib_error = 0;
	BspLib::SystemIO::SetCriticalErrorCallback( CriticalBspLibError );

	// read in data file
	int rc = ReadDataFile( input_file_name, input_file_name );

	// unregister callbacks
	BspLib::SystemIO::SetCriticalErrorCallback( NULL );

	return rc;
}


// ----------------------------------------------------------------------------
//
PRIVATE
void SaveOutput()
{
	if ( !world->getObjectList() ) {
		MSGOUT( "No objects to save.\n" );
		return;
	}

	char *outname = new char[ PATH_MAX + 1 ];
	strcpy( outname, current_object_file );
	int n = strlen( outname );
	while ( ( n > 0 ) && ( outname[ n ] != '.' ) )
		--n;
	strcpy( &outname[ n ], ".od2" );

	// configure BspLib
	SetBspLibOptions();

	// check if binary output is desired
	if ( ( output_format == OUTPUT_FORMAT_ODT ) ||
		 ( output_format == OUTPUT_FORMAT_OD2 ) ) {

		if ( BspTreeAvailable() ) {

			// do direct binary output of ODT or OD2 file containing BSP tree
			BspObjectList *objectlist = world->getObjectList();
			BspLib::ObjectBinFormat outobj( *objectlist->getListHead() );

			int format = ( output_format == OUTPUT_FORMAT_OD2 ) ?
				BspLib::ObjectBinFormat::BINFORMAT_OD2 : BspLib::ObjectBinFormat::BINFORMAT_ODT;
			outobj.WriteDataToFile( outname, format );

		} else {

			if ( output_format == OUTPUT_FORMAT_OD2 ) {

				// do direct binary output of OD2 file without BSP tree
				BspObjectList *objectlist = world->getObjectList();
				BspLib::ObjectBinFormat outobj( *objectlist->getListHead() );
				outobj.WriteDataToFile( outname, BspLib::ObjectBinFormat::BINFORMAT_OD2 );
			}
		}

	} else {

		// do output in BspLib's singleoutput formats
		if ( output_format == OUTPUT_FORMAT_SINGLE ) {

			int format;
			if ( BspTreeAvailable() ) {
				format = BspLib::IOData3D::BSP_FORMAT_1_1;
			} else {
				format = BspLib::IOData3D::AOD_FORMAT_1_1;
				// merge all objects in list into single object
				world->getObjectList()->CollapseObjectList();
			}

			// write output file
			if ( world->getInput() != NULL ) {
				world->getInput()->setFileName( outname );
				BspLib::OutputData3D outputdata( *world->getInput(), format );
			}
		}
	}

	delete outname;
}





// ----------------------------------------------------------------------------
//



// ----------------------------------------------------------------------------
//
PRIVATE
int OptSetInputFile( char **filename )
{
	char *name = filename[ 0 ];

	strcpy( input_file_name, name );

	return TRUE;
}

// ----------------------------------------------------------------------------
//
PRIVATE
int OptSetObjectScale( float *scale )
{
	bspgeoopt_epsilons[ 0 ] = *scale;

	return TRUE;
}


// register command line options ----------------------------------------------
//
PRIVATE
void RegisterOptions()
{
	OPT_RegisterStringOption( "i", "input",			OptSetInputFile );
	OPT_RegisterFloatOption(  "s", "scale",			OptSetObjectScale );
}


// ----------------------------------------------------------------------------
//
void msg_callback( const char *msg )
{
	printf( "%s", msg );
//	printf( "\n" );
}


// ----------------------------------------------------------------------------
//
PRIVATE
int InitMakeODT()
{
	BspLib::SystemIO::SetProgramName( "makeodt" );

#if defined ( _WINDOWS ) || defined ( WIN32 )
	BspLib::SystemIO::SetMainWindowHandle( NULL );
#endif

	BspLib::SystemIO::SetInfoMessageCallback( msg_callback );
	BspLib::SystemIO::SetErrorMessageCallback( msg_callback );

	InitBspLibOptions();

	return TRUE;
}


// ----------------------------------------------------------------------------
//
PRIVATE
void ConvertObject()
{
	// create global scene
	world = new Scene;

	// open, convert, save
	if ( OpenInput() ) {
		MergeListIntoSingleObject();
		SaveOutput();
	}

	// delete global scene
	delete world;
	world = NULL;
}


// main -----------------------------------------------------------------------
//
int main( int argc, char **argv )
{
	// init application
	if ( !InitMakeODT() ) {
		return EXIT_FAILURE;
	}

	// register command line options
	RegisterOptions();

	// exec all registered command line options
	if ( !OPT_ExecRegisteredOptions( argc, argv ) ) {
		MSGOUT( options_invalid );
		exit( EXIT_FAILURE );
	}

	// set modified options
	SetBspLibOptions();

	// check for file name
	if ( input_file_name[ 0 ] == 0 ) {
		MSGOUT( "No file name specified.\n" );
		exit( EXIT_FAILURE );
	}

	// do object conversion
	ConvertObject();

	return EXIT_SUCCESS;
}

