#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "GameObjMgr.h"
#include "EventManager.h"
#include <list>
#include "MainPlayer.h"
#include "NPC.h"
#include "GameObjects.h"



class CPlayerManager
{
public:
	CPlayerManager(CGameObjMgr* mgr);
	~CPlayerManager(void);


private:
	CGameObjMgr* m_ptObjMgr;
	CEventSnooper m_Listener;
	List<CNPC*> m_lPlayers;
	int m_iPlayerUpdating;
	CMainPlayer* m_ptMainPlayer;
	CHoldableObject* m_pFlare;

public:
	bool Init(CEventManager* eventManager);
	void Shutdown(void);
	bool InitResources(ID3D11Device* device);
	void DestroyResources(void);
	void Update(float dt, CGameObjMgr* ptObjMgr);

	CMainPlayer* GetMainPlayer();
	void RegisterMainPlayer( CMainPlayer* pMainPlayer );

	List<CNPC*> GetNonInclusivePlayers();
	int GetUpdatingPlayerIndex();
private:
	void ProcessMessages(void);
};

