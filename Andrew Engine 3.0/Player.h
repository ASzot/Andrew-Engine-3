#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "GameObjMgr.h"
#include "CollisionHelper.h"
#include "AnimatedObject.h"
#include "Actor.h"

using namespace Collision;

class CNPC;
class CPlayerManager;

class CPlayer : public CActor
{
public:
	CPlayer();
	~CPlayer(void);

	// Includes the most simple movement methods.
	virtual void StopPlayer();
	virtual void ChangeFacingDirection( Vec& dir );
	void SetVelocity(Vec& velocity);
	inline CAnimatedObject* GetCurrentObject();
	virtual Vec GetPosition();
	virtual Vec GetVelocity();
	Vec GetFacing();
	float GetSpeed();

	void RegisterPlayerMgr( CPlayerManager* pPlayerMgr );

	virtual void OnRayHit( UINT flags );

	void SetSpawnPosition( Vec& newSpawn );
	void Respawn();

	virtual void Update(float dt, CGameObjMgr* mgr);
	void OnObjectCreated(int indexOfObject);

	virtual void ProcessMessage( UINT message ) = 0;

	inline ObjectId GetIndexOfObject() { return m_iIndex; }

protected:
	virtual void UpdateLogic(CGameObjMgr* mgr);
	virtual void UpdatePlayerLogic(Vec& pos) { }

protected:
	int m_iIndex;
	friend class CNPC;

	Vec m_vSpawnLocation;

	CPlayerManager* m_ptPlayerMgr;

	float m_fSpeed;

	bool m_bRequestedStop;
};