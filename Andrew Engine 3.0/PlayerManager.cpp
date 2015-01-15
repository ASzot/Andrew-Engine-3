//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "PlayerManager.h"


CPlayerManager::CPlayerManager(CGameObjMgr* mgr)
	: m_Listener(eInputEventType)
{
	m_ptObjMgr = mgr;
}


CPlayerManager::~CPlayerManager(void)
{
}


bool CPlayerManager::Init(CEventManager* eventManager)
{
	eventManager->RegisterListener(&m_Listener);
	// DON'T EVEN THINK ABOUT DOING ANYTHING WITH THE LISTENER
	// RIGHT NOW BECAUSE THE OBJECT MANAGER IS NOT REGISTERED.

	return true;
}


void CPlayerManager::Shutdown(void)
{
	/*SafeDelete( m_pFlare );*/
	for ( int i = 0; i < m_lPlayers.size(); ++i )
	{
		if ( m_lPlayers.at( i ) )
		{
			delete m_lPlayers.at( i );
			m_lPlayers.at( i ) = 0;
		}
	}
}


bool CPlayerManager::InitResources(ID3D11Device* device)
{
	//CNPC* object = new CNPC();
	//object->SetSpawnPosition( Vec( 5.0f, 3.0f, 8.0f ) );
	//object->RegisterPlayerMgr( this );
	//m_lPlayers.push_back( object );

	//BulletData bd;
	//bd.Mat = PhysMat_Normal;
	//bd.SpecGrav = 4.0f;
	//m_Listener.CreateCharacter( SpawnInfo( PositionData( 15.0f, 3.0f, -10.0f, 0.0f, 0.0f, 0.0f, 0.08f, 0.08f, -0.08f), "data/Models/Anim/soldier.m3d", "Take1", bd ), object );

	/*m_pFlare = new CGun();
	assert( m_pFlare );
	m_pFlare->Initialize();
	m_pFlare->RegisterPlayerManager( this );
	m_pFlare->SetHolding( true );*/

	return true;
}


void CPlayerManager::DestroyResources(void)
{

}


void CPlayerManager::Update(float dt, CGameObjMgr* ptObjMgr)
{


	ProcessMessages();
	for(int i = 0; i < m_lPlayers.size(); ++i)
	{
		m_iPlayerUpdating = i;
		m_lPlayers.at(i)->Update(dt, ptObjMgr);
	}

	//if( GetAsyncKeyState('M') & 0x8000 )
	//	m_pFlare->SetHolding( true );

	//m_pFlare->Update();
}


void CPlayerManager::ProcessMessages(void)
{
	while(m_Listener.HasPendingMessages())
	{
		CHashedString message = m_Listener.GetRecievedMessage();
		// For now do nothing with the messages.
		if ( message == CHashedString("c1DropHolding") )
		{
			/*m_pFlare->SetHolding( false );*/
		}
	}
}


List<CNPC*> CPlayerManager::GetNonInclusivePlayers()
{
	// Just return the list without the updating player.
	List<CNPC*> list = m_lPlayers;
	List<CNPC*>::iterator i = list.begin() + m_iPlayerUpdating;
	list.erase( i );

	return list;
}

int CPlayerManager::GetUpdatingPlayerIndex()
{
	return m_iPlayerUpdating;
}

void CPlayerManager::RegisterMainPlayer( CMainPlayer* pMainPlayer )
{
	m_ptMainPlayer = pMainPlayer;
}

CMainPlayer* CPlayerManager::GetMainPlayer()
{
	return m_ptMainPlayer;
}