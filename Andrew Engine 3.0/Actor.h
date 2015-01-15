//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "GameObjMgr.h"

class CActor
{
public:
	CActor(void);
	virtual ~CActor(void);
	virtual void Update(float dt, CGameObjMgr* mgr) = 0;

	void SetMaxVelocity( Vec& maxVel );
	float GetMaxSpeed();

	virtual Vec GetVelocity() = 0;
	virtual Vec GetPosition() = 0;
	virtual Vec GetFacing() = 0;
	virtual float GetSpeed() = 0;

	virtual void OnRayHit( UINT flags ) = 0;

protected:
	void CheckTerrain(IGamePhysics* phys, IObject* obj);
	void CheckConstraints(IGamePhysics* phys, IObject* obj);

public:
	ObjectPhysConstraint m_OPC;

private:
	bool m_bFirstImpact;

};

