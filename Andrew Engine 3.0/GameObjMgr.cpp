//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "GameObjMgr.h"



CGameObjMgr::CGameObjMgr(void)
{
	m_iPlayerIndex = 0;
	m_iCurrentKey = 0;
	g_pGameObjMgr = this;
}


CGameObjMgr::~CGameObjMgr(void)
{
}

void CGameObjMgr::AddObjectPointCloud( IObject* obj, XMFLOAT3* verts, int numVerts )
{
	std::pair<ObjectIndexToObjectMap::iterator,bool> result;
	result = m_ObjectList.insert(IndexObjectKey(m_iCurrentKey,obj));

	obj->SetIndexOfObject(m_iCurrentKey);

	if(!result.second)
	{
		FatalError("Two objects with the same index some how both got into m_ObjectList in CGameManager");
	}

	m_Physics->VAddPointCloud( verts, numVerts, obj, obj->GetSpecGrav(), obj->GetPhysicsMaterial() );
	++m_iCurrentKey;
}

void CGameObjMgr::Shutdown()
{
}


void CGameObjMgr::ResyncPhysics()
{
	// Will assign all IObject matricies to the ObjectMotionState matricies.
	m_Physics->ResyncPhysics( this );
	// Since all the physics objects and the render objects are now calibrated.
	PhysicsSynced( true );
}


IGamePhysics* g_GetGamePhysics()
{
	return g_pGameObjMgr->GetPhysics();
}

CGameObjMgr* g_GetGameObjMgr()
{
	return g_pGameObjMgr;
}

