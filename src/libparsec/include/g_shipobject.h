

class G_ShipObject : public ShipObject
{
protected:
public:
	
	int BoostEnergy( int nEnergy )
	{
		if ( CurEnergy == MaxEnergy ) {
			return FALSE;
		} else {
			CurEnergy += nEnergy;
			if ( CurEnergy > MaxEnergy ) {
				CurEnergy = MaxEnergy;
			}
			return TRUE;
		}
	}

	int BoostRepair( int nRepair )
	{
		if ( CurDamage == 0 ) {
			return FALSE;
		} else {
			CurDamage -= nRepair;
			if ( CurDamage < 0 ) {
				CurDamage = 0;
			}
			return TRUE;
		}
	}

	int BoostMissiles( int nNumMissiles )
	{
		if ( NumMissls == MaxNumMissls ) {
			return FALSE;
		} else {
			NumMissls += nNumMissiles;
			if ( NumMissls > MaxNumMissls ) {
				NumMissls = MaxNumMissls;
			}
			return TRUE;
		}
	}

	int BoostHomMissiles( int nNumMissiles )
	{
		if ( NumHomMissls == MaxNumHomMissls ) {
			return FALSE;
		} else {
			NumHomMissls += nNumMissiles;
			if ( NumHomMissls > MaxNumHomMissls ) {
				NumHomMissls = MaxNumHomMissls;
			}
			return TRUE;
		}
	}

	int BoostPartMissiles( int nNumMissiles )
	{
		if ( NumPartMissls == MaxNumPartMissls ) {
			return FALSE;
		} else {
			NumPartMissls += nNumMissiles;
			if ( NumPartMissls > MaxNumPartMissls ) {
				NumPartMissls = MaxNumPartMissls;
			}
			return TRUE;
		}
	}

	int BoostMines( int nMines )
	{
		if ( NumMines == MaxNumMines ) {
			return FALSE;
		} else {
			NumMines += nMines;
			if ( NumMines > MaxNumMines ) {
				NumMines = MaxNumMines;
			}
			return TRUE;
		}
	}

	int CollectDevice( int mask )
	{
		if ( ( Weapons & mask ) == 0 ) {
			Weapons |= mask;
			return TRUE;
		} else {
			return FALSE;
		}
	}

	// enable afterburner function ------------------------------------------------
	//
	void EnableAfterBurner()
	{
		// set active flag 
		Specials |= SPMASK_AFTERBURNER;
		afterburner_energy = AFTERBURNER_ENERGY;
	}

	// enable invisibility function -----------------------------------------------
	//
	void EnableInvisibility()
	{
		// set active flag 
		Specials |= SPMASK_INVISIBILITY;

		//TODO:
		// well, nobody got around to implementing this :)
	}


	// enable invulnerability function --------------------------------------------
	//
	void EnableInvulnerability()
	{
#ifdef PARSEC_CLIENT
		// create particle cluster visualizing invulnerability shield
		if ( SFX_EnableInvulnerabilityShield( (ShipObject*)this ) ) {

			// set active flag 
			Specials |= SPMASK_INVULNERABILITY;

			// set shield strength
			MegaShieldAbsorption = MegaShieldStrength;
		}
#elif defined ( PARSEC_SERVER )

		// set active flag 
		Specials |= SPMASK_INVULNERABILITY;

		// set shield strength
		MegaShieldAbsorption = TheGame->MegaShieldStrength;
#endif
	}

	// enable decoy device --------------------------------------------------------
	//
	void EnableDecoy()
	{
		// set active flag 
		Specials |= SPMASK_DECOY;
	}


	// enable laser upgrade 1  ----------------------------------------------------
	//
	void EnableLaserUpgrade1()
	{
		// set active flag 
		Specials |= SPMASK_LASER_UPGRADE_1;
	}


	// enable laser upgrade 2  ----------------------------------------------------
	//
	void EnableLaserUpgrade2()
	{
		// set active flag 
		Specials |= SPMASK_LASER_UPGRADE_2;
	}


	// enable emp upgrade 1  ------------------------------------------------------
	//
	void EnableEmpUpgrade1()
	{
		// set active flag 
		Specials |= SPMASK_EMP_UPGRADE_1;
	}


	// enable emp upgrade 2  ------------------------------------------------------
	//
	void EnableEmpUpgrade2()
	{
		// set active flag 
		Specials |= SPMASK_EMP_UPGRADE_2;
	}


	int CollectSpecial( int mask )
	{
		switch( mask ) {
			case SPMASK_AFTERBURNER:
				EnableAfterBurner();
				break;
			case SPMASK_INVISIBILITY:
				EnableInvisibility();
				break;
			case SPMASK_INVULNERABILITY:
				EnableInvulnerability();
				break;
			case SPMASK_DECOY:
				EnableDecoy();
				break;
			default:
				//NOTE: all other specials must be handled by directly calling the Enable* function
				ASSERT( FALSE );
				break;
		}

		if ( ( Specials & mask ) == 0 ) {
			return TRUE;
		} else {
			return FALSE;
		}
	}
};

