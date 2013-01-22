/*
* PARSEC HEADER: G_extra.h
*/

#ifndef _G_EXTRA_H_
#define _G_EXTRA_H_

#ifdef PARSEC_CLIENT
	//FIXME: move this into E_GLOBAL.H for the client
	#define TheGameExtraManager (G_ExtraManager::GetExtraManager())
#endif //PARSEC_CLIENT

// class maintaining all EXTRA related stuff ----------------------------------
//
class G_ExtraManager
{
public:

	int 			ExtraProbability;
	int 			MaxExtraArea;
	int 			MinExtraDist;
	int 			ProbHelixCannon;
	int 			ProbLightningDevice;
	int 			ProbPhotonCannon;
	int 			ProbProximityMine;
	int 			ProbMissilePack;
	int 			ProbDumbMissPack;
	int 			ProbHomMissPack;
	int 			ProbSwarmMissPack;
	int 			ProbRepairExtra;
	int 			ProbAfterburner;
	int				ProbHoloDecoy;
	int 			ProbInvisibility;
	int 			ProbInvulnerability;
	int 			ProbEnergyField;
	int 			ProbLaserUpgrade;
	int 			ProbLaserUpgrade1;
	int 			ProbLaserUpgrade2;
	int 			ProbEmpUpgrade1;
	int 			ProbEmpUpgrade2;

protected:

	G_ExtraManager();

	// place extras after ship was shot down
	void _PlaceShipExtras( int num, int objclass, ShipObject *shippo );


	// boost extra collected: energy boost 
	char* _CollectBoostEnergy( ShipObject* cur_ship, Extra1Obj* extra1po );
       
	// boost extra collected: repair damage 
	char* _CollectBoostRepair( ShipObject* cur_ship, Extra1Obj* extra1po );

	// package extra collected: dumb missiles
	char* _CollectPackDumb( ShipObject* cur_ship, Extra2Obj* extra2po );

	// package extra collected: guided missiles
	char* _CollectPackGuide( ShipObject* cur_ship, Extra2Obj* extra2po );

	// device extra collected: swarm missiles
	char* _CollectPackSwarm( ShipObject* cur_ship, Extra2Obj* extra2po );

	// package extra collected: proximity mines
	char* _CollectPackMine( ShipObject* cur_ship, Extra2Obj* extra2po );

	// device extra collected: helix cannon
	char* _CollectDeviceHelix( ShipObject* cur_ship );

	// device extra collected: lightning device
	char* _CollectDeviceLightning( ShipObject* cur_ship );

	// device extra collected: photon cannon
	char* _CollectDevicePhoton( ShipObject* cur_ship );

	// device extra collected: afterburner
	char* _CollectDeviceAfterburner( ShipObject* cur_ship );

	// device extra collected: invisibility
	char* _CollectDeviceInvisibility( ShipObject* cur_ship );

	// device extra collected: invulnerability
	char* _CollectDeviceInvulnerability( ShipObject* cur_ship );

	// device extra collected: decoy
	char* _CollectDeviceDecoy( ShipObject* cur_ship );

	// device extra collected: laser upgrade 1
	char* _CollectDeviceLaserUpgrade1( ShipObject* cur_ship );

	// device extra collected: laser upgrade 2
	char* _CollectDeviceLaserUpgrade2( ShipObject* cur_ship );

	// device extra collected: emp upgrade 1
	char* _CollectDeviceEmpUpgrade1( ShipObject* cur_ship );

	// device extra collected: emp upgrade 2
	char* _CollectDeviceEmpUpgrade2( ShipObject* cur_ship );

	// helper function when collecting devices
	char* _CollectDevice( int nDevice, ShipObject* cur_ship );

	// helper function when collecting specials
	char* _CollectSpecial( int nSpecial, ShipObject* cur_ship );

  // function when colliding with a mine
  char* _CollisionProximityMine( Mine1Obj *minepo );

public:

	// SINGLETON pattern
	static G_ExtraManager* GetExtraManager()
	{
		static G_ExtraManager _TheGameExtraManager;
		return &_TheGameExtraManager;
	}

	// place extras in vicinity of local ship object
	void OBJ_DoExtraPlacement();

	// create extras after ship was shot down
	void OBJ_CreateShipExtras( ShipObject *shippo );

	// fill member variables of extras
	void OBJ_FillExtraMemberVars( ExtraObject* extrapo );

	// kill extra that immediately follows passed node in list
	void OBJ_KillExtra( ExtraObject* precnode, int collected );

	// animate an extra
	void OBJ_AnimateExtra( ExtraObject* extrapo );


	// perform action according to extra type and determine message to show
	char* CollectExtra( ShipObject* cur_ship, ExtraObject* curextra );
       char* _CollectBoostEnergyField( ShipObject* cur_ship, int boost );
};



#endif // _G_EXTRA_H_
