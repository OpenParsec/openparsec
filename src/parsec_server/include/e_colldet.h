/*
* PARSEC HEADER: G_CollDet.h
*/
#include "g_emp.h"
#ifndef _G_COLLDET_H_
#define _G_COLLDET_H_

// class handling the collision detection -------------------------------------
//
class G_CollDet
{
protected:

	ShipObject*	cur_ship;
	Vertex3		obj_pos;
	Vertex3		test_pos;
	Vertex3		prev_pos;
	geomv_t		bd_sphere;
	geomv_t		bd_sphere2;
	int			test_shield;

protected:

	// collision detection helper function
	int _CheckShipProjectileDisjoint();

  
	// ship collided with laser
	void _CollisionResponse_LaserShip( LaserObject *curlaser );

	// check whether any ship is hit by laser beam
	void _CheckShipLaserCollision();

	// check if ship collected extra
	void _CheckShipExtraCollision();

	// check if ship hit by emp
	void _CheckShipEmpCollision();

	// ship collided with EMP
	void _CollisionResponse_EmpShip( Emp *curemp );

    //Ship collided with missile
    void _CheckShipMissileCollision();
    //response definitions
    void _CollisionResponse_MissileShip( MissileObject *curmissile );
    void _CollisionResponse_MineShip( Mine1Obj *curmine );
    void OBJ_ShipHelixDamage( ShipObject *shippo, int owner );
    void OBJ_ShipLightningDamage( ShipObject *shippo, int owner );
    void OBJ_ShipPhotonDamage( ShipObject *shippo, int owner );
    
	G_CollDet()	{}
	~G_CollDet()	{}

public:
	// SINGLETON pattern
	static G_CollDet* GetGameCollDet()
	{
		static G_CollDet _TheGameCollDet;
		return &_TheGameCollDet;
	}

	// perform collision detection tests
	void OBJ_CheckCollisions();
    void LinearParticleCollision( linear_pcluster_s *cluster, int pid );
    void CheckEnergyField( pcluster_s *cluster );
    void OBJ_ShipSwarmDamage( ShipObject *shippo, int owner );
    int CheckLightningParticleShipCollision( Vertex3& particlepos, int owner );
    //Particle Collisions
    int PRT_ParticleInBoundingSphere( ShipObject *shippo, Vertex3& point );

};

#endif // _G_COLLDET_H_

