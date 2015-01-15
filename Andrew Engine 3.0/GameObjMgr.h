#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <map>
#include "Interfaces.h" 


class CGameObjMgr
{
public:
	CGameObjMgr(void);
	~CGameObjMgr(void);

	void AddObject(IObject* obj)
	{
		std::pair<ObjectIndexToObjectMap::iterator,bool> result;
		result = m_ObjectList.insert(IndexObjectKey(m_iCurrentKey,obj));

		obj->SetIndexOfObject(m_iCurrentKey);

		if(!result.second)
		{
			FatalError("Two objects with the same index somehow both got into m_ObjectList in CGameManager");
		}

		m_Physics->VAddBox( obj->GetBoundriesOfCollisionBox(), obj, obj->GetSpecGrav(), obj->GetPhysicsMaterial() );
		++m_iCurrentKey;
	}

	void AddObjectPointCloud( IObject* obj, XMFLOAT3* verts, int numVerts );

	void MovePlayer( ObjectId indexOfPlayer, Vec& vel )
	{
		m_Physics->VAddVelocity(indexOfPlayer,vel );
	}

	
	int GetNumberOfObjects()
	{
		return m_ObjectList.size();
	}

	IObject* GetObjectByIndex(ObjectId id)
	{
		ObjectIndexToObjectMap::iterator result = m_ObjectList.find(id);
		if(result != m_ObjectList.end() )
			return result->second;
		FatalError("Tried to access a invalid object!/nCGameObjMgr::GetObjectByIndex (Invalid ObjectId)");
	}


	void Shutdown();

	void ResyncPhysics();

	void RegisterPhysicsSystem(IGamePhysics* phys)
	{
		m_Physics = phys;
	}

	Vec GetGlobalGravity()
	{
		return m_Physics->GetGravity();
	}

	void  SetGlobalGravity( Vec& vec )
	{
		m_Physics->SetGravity( vec );
	}

	IGamePhysics* GetPhysics()
	{
		return m_Physics;
	}

private:
	ObjectIndexToObjectMap m_ObjectList;
	int m_iPlayerIndex;
	int m_iCurrentKey;
	IGamePhysics* m_Physics;
};



namespace
{
	CGameObjMgr* g_pGameObjMgr;
}

IGamePhysics* g_GetGamePhysics();
CGameObjMgr* g_GetGameObjMgr();