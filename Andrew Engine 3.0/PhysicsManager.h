//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "CollisionHelper.h"
#include "Timer.h"
#include <algorithm>
#include <set>
#include <btBulletCollisionCommon.h>
#include "PhysicsSystem.h"
#include "Manager.h"


  //////////////////////////////////////////////////////
 ///This controls whether null physics is used or not./
//////////////////////////////////////////////////////
const bool g_bUseNullPhysics = false;


class CPhysicsManager : public CManager
{
public:
	CPhysicsManager( HINSTANCE hInstance, HWND hwnd );
	~CPhysicsManager(void);
	void Update(float dt,double tt);
	void Render( );
	virtual void Serilize( CDataArchiver* archiver );

	void Restart();

	void ListenToMessages(char* message);
	void ProcessMessage(CHashedString message);

protected:
	BOOL CreateDlgWindow();
	void OnDialogQuit();

private:
	// In the near future this will be for sending the events 
	// notifying the objects about collisions.
	CGameObjMgr* m_ptGameObjMgr;

public:
	IGamePhysics*  m_pPhysicsSystem;

public:
	bool Init(CGameObjMgr* mgr, CEventManager* eventMgr);
	bool InitResources( ID3D11Device* device, Camera* cam,CGameObjMgr* gameObjMgr );
	void Shutdown(void);
	void AddPhysicsObject( IObject* obj, rVec dim, float specGrav, enum PhysicsMaterial mat );

	void CastRay( int x, int y, Camera& cam, UINT flags, bool straightForward = false );
};

