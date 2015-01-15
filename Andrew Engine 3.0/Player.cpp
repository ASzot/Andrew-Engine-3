//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "Player.h"


CPlayer::CPlayer()
{
	m_iIndex = -1;
	m_bRequestedStop = true;
	m_fSpeed = 2.0f;
	SetMaxVelocity( Vec( 2.0f, 2.0f, 2.0f ) );
	// Default spawn location.
	m_vSpawnLocation = Vec( 0.0f, 0.0f, 0.0f );
	m_ptPlayerMgr = 0;
}


CPlayer::~CPlayer(void)
{
}


void CPlayer::Update(float dt, CGameObjMgr* mgr)
{
	if(m_iIndex != -1)
	{
		// This means we have the index and are ready for action. 
		UpdateLogic(mgr);
	}
}

void CPlayer::SetSpawnPosition( Vec& newSpawn )
{
	m_vSpawnLocation = newSpawn;
}

void CPlayer::Respawn()
{
	if ( m_iIndex != -1 )
		g_GetGameObjMgr()->GetPhysics()->VTranslate( m_iIndex, m_vSpawnLocation );
	else
		DebugWriter::WriteToDebugFile( "Attempted a respawn when the render object wasn't registered!", "void CPlayer::Respawn()" );
}

void CPlayer::OnObjectCreated(int indexOfObject)
{
	m_iIndex = indexOfObject;
	g_GetGameObjMgr()->GetObjectByIndex( indexOfObject )->RegisterGameActor( this );
}

void CPlayer::UpdateLogic(CGameObjMgr* mgr)
{
	CAnimatedObject* obj = static_cast<CAnimatedObject*>( mgr->GetObjectByIndex( m_iIndex ) );
	assert(obj);
	// Do everything as you wish with obj.
	IGamePhysics* gamePhys = mgr->GetPhysics();

	if ( m_bRequestedStop )
	{
		gamePhys->VStopObject( m_iIndex );
		m_bRequestedStop = false;
	}

	UpdatePlayerLogic( Vec( obj->X(), obj->Y(), obj->Z() ) );

	//CheckConstraints( gamePhys, obj );
}


void CPlayer::StopPlayer()
{
	m_bRequestedStop = true;
}

void CPlayer::OnRayHit( UINT flags )
{
	ProcessMessage( flags );
}

void CPlayer::ChangeFacingDirection( Vec& dir )
{
	// Ensure that the direction is just a direction.
	dir.Normalize();
	CGameObjMgr* mgr = g_GetGameObjMgr();
	CAnimatedObject* obj = static_cast<CAnimatedObject*>( mgr->GetObjectByIndex( m_iIndex ) );
	obj->SetDirection( dir );
}

void CPlayer::SetVelocity(Vec& velocity)
{
	CGameObjMgr* mgr = g_GetGameObjMgr();
	IGamePhysics* phys = mgr->GetPhysics();
	// Make sure that the velocity is at first only a direction.
	velocity.Normalize();
	velocity = velocity * m_fSpeed;
	phys->VApplyImpulse(Vec(0.0f,1.0f,0.0f), 100.0f, m_iIndex );
	phys->VAddVelocity( m_iIndex, velocity );
}

CAnimatedObject* CPlayer::GetCurrentObject()
{
	CGameObjMgr* mgr = g_GetGameObjMgr();
	return static_cast<CAnimatedObject*>( mgr->GetObjectByIndex( m_iIndex ) ); 
}

Vec CPlayer::GetPosition()
{
	CAnimatedObject* obj = GetCurrentObject();
	return Vec( obj->X(), obj->Y(), obj->Z() );
}

void CPlayer::RegisterPlayerMgr( CPlayerManager* pPlayerMgr )
{
	m_ptPlayerMgr = pPlayerMgr;
}

Vec CPlayer::GetVelocity()
{
	return g_GetGameObjMgr()->GetPhysics()->VGetVelocity( m_iIndex );
}

Vec CPlayer::GetFacing()
{
	return GetCurrentObject()->GetDesiredDir();
}

float CPlayer::GetSpeed()
{
	return m_fSpeed;
}