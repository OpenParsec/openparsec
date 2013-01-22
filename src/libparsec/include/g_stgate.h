/*
 * PARSEC HEADER: s_stgate.h
 */

#ifndef _S_STGATE_H_
#define _S_STGATE_H_


// forward declaration
struct genobject_pcluster_s;


// stargate limits ------------------------------------------------------------
//
#define STARGATE_MAX_DEST_IP				255


// stargate custom type structure ---------------------------------------------
//
struct Stargate : CustomObject {

	// properties
	bams_t					rotspeed;
	float 				radius;
	geomv_t 				actdistance;
	int						dormant;
	int						active;
	int						autoactivate;
	int						numpartactive;
	int						actcyllen;
	float					partvel;
	bams_t  				modulspeed;
	float 				modulrad1;
	float 				modulrad2;
	float					modulfade;
	int						acttime;
	char					flare_name   [ MAX_TEXNAME + 1 ];
	char					interior_name[ MAX_TEXNAME + 1 ];

	// internal data
	word					serverid;
	char    				destination_name[ MAX_SERVER_NAME + 1 ];
	//char					destination_ip[ STARGATE_MAX_DEST_IP + 1 ];
	//word					destination_port;
	node_t					destination_node;
	bams_t					modulate_bams;
	int						curactcyllen;
	int						activating;
	int						manually_activated;
	float					actring;
	genobject_pcluster_s*	pcluster;
	Vertex3*				interior_vtxlist;	// vtxs in object space
	Vertex3*				interior_viewvtxs;	// vtxs in view space
	geomv_t*				interior_u;			// u texture coordinates
	geomv_t*				interior_v;			// v texture coordinates
	TextureMap*				interior_texmap;
	int						ping;
	refframe_t				lastpinged;
};


// external functions ---------------------------------------------------------
//
// (none)


#endif // _S_STGATE_H_


