//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "MainPlayer.h"
#include "SoundManager.h"
#include "SparkProcess.h"
#include "Graphics.h"
#include "GameObjects.h"


Vec CFakeObject::GetBoundriesOfCollisionBox()
{
	// This bounding box is that of the user controlled character.
	return Vec( 2.5f, 4.0f, 2.5f );
}

void CFakeObject::SetWorldMatrix( Mat4x4& mat )
{
	Vec pos = mat.GetPosition();
	mPosY = pos.Y() + 1.2f;
	mPosX = pos.X();
	mPosZ = pos.Z();
}

// We only care about creating the weapon.
CMainPlayer::CMainPlayer()
	: m_EventListener( eNothingEventType ),
	m_fWalkForce( 90.0f ),
	m_fJumpForce( 10.0f )
{
	m_ptObjHolding = 0;
	m_bIsMoving = false;
}

CMainPlayer::~CMainPlayer()
{
}

bool CMainPlayer::Initialize(Vec& location, CGameObjMgr* mgr, CEventManager* eventMgr)
{
	m_PhysBody = new CFakeObject( location.X(), location.Y(), location.Z() );

	mgr->AddObject( m_PhysBody );
	
	eventMgr->RegisterListener( &m_EventListener );

	m_vSpawnLocation = location;

	Vec maxVel = Vec( 10.0f, FLT_MAX, 10.0f );
	SetMaxVelocity( maxVel );

	return true;
}

void CMainPlayer::SetHolding( CHoldableObject* holdingObject )
{
	if ( m_ptObjHolding && ( m_ptObjHolding != holdingObject ) )
		m_ptObjHolding->SetHolding( false );
	m_ptObjHolding = holdingObject;
}

void CMainPlayer::Render(CameraInfo& cam)
{

}

void CMainPlayer::SetCameraInfo( CameraInfo& cam )
{
	m_CameraInfo.LookAt		= cam.LookAt;
	m_CameraInfo.Forward	= cam.Forward;
	m_CameraInfo.Pitch		= cam.Pitch;
	m_CameraInfo.Position	= cam.Position;
	//m_CameraInfo.Proj		= cam.Proj;
	//m_CameraInfo.Refl		= cam.Refl;
	m_CameraInfo.Right		= cam.Right;
	//m_CameraInfo.View		= cam.View;
	m_CameraInfo.Yaw		= cam.Yaw;
}

CameraInfo CMainPlayer::GetCameraInfo()
{
	return m_CameraInfo;
}

void CMainPlayer::Strafe( float dir, CameraInfo& cam, IGamePhysics* phys  )
{
	XMVECTOR s = XMVectorReplicate( dir );
	XMVECTOR r = XMLoadFloat4( &cam.Right );
	XMVectorSetZ( r, 1.0f );
	XMVECTOR p = m_PhysBody->GetPosAsVec().ToVector();
	Vec final = XMVectorToVec( XMVectorMultiply( s, r) );
	final.SetY( 0 );

	float mass =m_PhysBody->GetMass();
	float newtons = mass* m_fWalkForce;
	phys->VApplyForce( final, newtons, m_PhysBody->GetId() );

	m_bIsMoving = true;
}

void CMainPlayer::Walk( float dir, CameraInfo& cam, IGamePhysics* phys  )
{
	XMVECTOR s = XMVectorReplicate( dir );
	XMVECTOR l = XMLoadFloat3( &cam.LookAt );
	XMVectorSetZ( l, 1.0f );
	XMVECTOR p = m_PhysBody->GetPosAsVec().ToVector();
	Vec final = XMVectorToVec( XMVectorMultiply( s, l ) );
	final.SetY( 0 );

	float mass =m_PhysBody->GetMass();
	float newtons = mass * m_fWalkForce;
	phys->VApplyForce( final, newtons, m_PhysBody->GetId() );

	m_bIsMoving = true;
	
}

void CMainPlayer::Shutdown()
{
	m_ptObjHolding = 0;
}

void CMainPlayer::Jump( CameraInfo& cam, IGamePhysics* phys )
{
	Vec vl = phys->VGetVelocity( m_PhysBody->GetId() );
	if ( abs ( vl.Y() ) < 0.05f )
	{
		float mass =m_PhysBody->GetMass();
		phys->VApplyImpulse( Vec( 0.0f, 10.0f, 0.0f ), mass * m_fJumpForce, m_PhysBody->GetId() );
	}
}

void CMainPlayer::Update( float dt, CGameObjMgr* mgr )
{
	IGamePhysics* phys = mgr->GetPhysics();

	CheckTerrain( phys, m_PhysBody );
	
	CheckConstraints( phys, m_PhysBody );

	float mass =m_PhysBody->GetMass();
	phys->VApplyForce( Vec( 0.0f, 1.0f, 0.0f ), mass,  m_PhysBody->GetId() );

	if ( !m_bIsMoving )
	{
		// Apply a force to slow the player down.
		Vec vel = phys->VGetVelocity(m_PhysBody->GetId());
		vel = vel * -1.0f;
		vel.SetY(0.0f);
		const float slowForce = 50.0f;
		phys->VApplyForce(vel, slowForce, m_PhysBody->GetId());
	}

	m_bIsMoving = false;
}

void CMainPlayer::Shot()
{
	Vec pos = Vec( 5.0f, 2.0f, -5.0f );
	g_fGetProcessManager()->Attach( new CSoundFXProcess( 100, g_fGetSoundResource( "GunShot" )->GetSoundInstance() ) );
	CProcess* process1 = new CExplosionFXProcess( XMFLOAT3( 20.0f, 0.0f, 20.0f ) );
	//CWaitProcess* waitProcess = new CWaitProcess(500);
	//CWaitProcess* waitProcess2 = waitProcess->Clone();
	//CWaitProcess* waitProcess3 = waitProcess->Clone();
	CProcess* process2 = new CExplosionFXProcess( XMFLOAT3( 10.0f, 0.0f, 20.0f ) );
	CProcess* process3 = new CExplosionFXProcess( XMFLOAT3( 0.0f, 0.0f, 20.0f ) );
	CProcess* process4 = new CExplosionFXProcess( XMFLOAT3( -10.0f, 0.0f, 20.0f ) );
	/*process1->SetNext( waitProcess );
	waitProcess->SetNext( process2 );
	process2->SetNext( waitProcess2 );
	waitProcess2->SetNext( process3 );
	process3->SetNext( waitProcess3 );
	waitProcess3->SetNext( process4 );*/
	process1->SetNext( process2 );
	process2->SetNext( process3 );
	process3->SetNext( process4 );

	g_fGetProcessManager()->Attach( process1 );
}

void CMainPlayer::DestroyResources()
{
  
}


bool CMainPlayer::InitResources(ID3D11Device* device)
{
	ObjectPhysConstraint opc;
	opc.m_vMaxVel = Vec( 0.1f, 0.0f, 0.1f );

	//g_GetGamePhysics()->VAddConstraint( opc, m_PhysBody->GetId() );

	return true;
}

bool CMainPlayer::IsDead()
{
	// Test if the player has fallen off the edge. 
	Vec pos = m_PhysBody->GetPosAsVec();

	if ( pos.Y() < -50.0f )
		return true;


	return false;
}

void CMainPlayer::Respawn( IGamePhysics* phys )
{
	phys->VTranslate( m_PhysBody->GetId(), m_vSpawnLocation );
}

UINT CMainPlayer::GetLeftClickMessage()
{
	return SHOT_BULLET;
}

void CMainPlayer::OnRayHit( UINT flags )
{

}

Vec CMainPlayer::GetPosition()
{
	return Vec( m_PhysBody->X(), m_PhysBody->Y(), m_PhysBody->Z() );
}

Vec CMainPlayer::GetVelocity()
{
	return g_GetGameObjMgr()->GetPhysics()->VGetVelocity( m_PhysBody->GetId() );
}

Vec CMainPlayer::GetFacing()
{
	return Vec( 0.0f, 0.0f, 0.0f );
}

float CMainPlayer::GetSpeed()
{
	Vec vel = g_GetGameObjMgr()->GetPhysics()->VGetVelocity( m_PhysBody->GetId() );
	return vel.Length();
}

void CMainPlayer::DropHoldingObject()
{
	if ( m_ptObjHolding )
	{
		m_ptObjHolding->SetHolding( false );
		m_ptObjHolding = 0;
	}
	
}

void CMainPlayer::ThrowHoldingObject()
{
	if ( m_ptObjHolding )
	{
		m_ptObjHolding->ThrowObject();
		m_ptObjHolding = 0;
	}
}