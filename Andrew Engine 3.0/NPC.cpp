//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "NPC.h"
#include "PlayerManager.h"


CNPC::CNPC(void)
{

}


CNPC::~CNPC(void)
{
}


void CNPC::UpdatePlayerLogic(Vec& pos)
{
	List<CNPC*> players = m_ptPlayerMgr->GetNonInclusivePlayers();

	/*Pursuit(m_ptPlayerMgr->GetMainPlayer());
	EvadeObstacles();*/

	//if( GetAsyncKeyState('C') & 0x8000 )
	//{
	//	Respawn();
	//}
}



void CNPC::ProcessMessage( UINT message )
{
	switch( message )
	{
	case SHOT_BULLET:
		OnBeingShot();
		break;
	}
}

void CNPC::OnBeingShot()
{

}