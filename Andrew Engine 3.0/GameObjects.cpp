//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "GameObjects.h"
#include "ParticleManager.h"
#include "LightManager.h"
#include "PlayerManager.h"
#include "MathHelper.h"


//////////////////////////////////////////
// CGameObject:
///////////////////////////////////////

CGameObject::CGameObject( ObjectId idOfObject )
{
	m_ptObj = g_GetEventManager()->GetObjectManager()->GetObjectByIndex( idOfObject );
}

CGameObject::CGameObject()
{

}

CGameObject::~CGameObject()
{

}

Vec CGameObject::GetPosition()
{
	return Vec( m_ptObj->X(), m_ptObj->Y(), m_ptObj->Z() );
}

Vec CGameObject::GetRotation()
{
	return Vec( m_ptObj->RX(), m_ptObj->RY(), m_ptObj->RZ() );
}

Vec CGameObject::GetVelocity()
{
	return g_GetGameObjMgr()->GetPhysics()->VGetVelocity( m_ptObj->GetId() );
}

void CGameObject::SetVelocity( Vec velocity )
{
	g_GetGameObjMgr()->GetPhysics()->VSetVelocity( m_ptObj->GetId(), velocity );
}

void CGameObject::Accelerate(Vec dir, float acceleration)
{
	dir.Normalize();
	g_GetGameObjMgr()->GetPhysics()->VApplyForce( dir, acceleration, m_ptObj->GetId() );
}

void CGameObject::SetWorldMatrix( CXMMATRIX M ) 
{
	Mat4x4 mat4x4 = *((D3DXMATRIX*)&M);

	g_GetGameObjMgr()->GetPhysics()->VKinematicMove( mat4x4, m_ptObj->GetId() );
	m_ptObj->SetWorldMatrixXM( M );
}

float CGameObject::GetMass()
{
	return m_ptObj->GetMass();
}

void CGameObject::RegisterPlayerManager( CPlayerManager* pPlayerMgr )
{
	m_ptPlayerMgr = pPlayerMgr;
}

void CGameObject::InitObject( ObjectId idOfObject )
{
	m_ptObj = g_GetEventManager()->GetObjectManager()->GetObjectByIndex( idOfObject );
}

//////////////////////////////////////////
// CHoldableObject:
/////////////////////////////////////////
CHoldableObject::CHoldableObject( ObjectId idOfObject )
	: CGameObject( idOfObject )
{
	m_vHoldingPos = Vec( 1.0f, -1.0f, 2.0f );
	m_bHolding = false;
}

CHoldableObject::CHoldableObject()
	: CGameObject()
{
	m_vHoldingPos = Vec( 1.0f, -1.0f, 2.0f );
	m_xRotModifier = 0.0f;
}

CHoldableObject::~CHoldableObject()
{

}

void CHoldableObject::SetHolding( bool holding )
{
	m_bHolding = holding;
	if ( holding )
		m_ptPlayerMgr->GetMainPlayer()->SetHolding( this );
	else
	{
		CameraInfo ci = m_ptPlayerMgr->GetMainPlayer()->GetCameraInfo();
		XMMATRIX holdingTransform = GetHoldingTransform();
		float yRot = ci.Yaw;
		float xRot = ci.Pitch;


		Vec pos = Vec( 1.0f, -1.0f, 2.0f );
		XMMATRIX rotation = XMMatrixRotationX( 90.0f ) * XMMatrixRotationY( yRot );
		pos = pos.Transform( rotation );
		pos = pos.Transform( holdingTransform );

		XMMATRIX mat = rotation * XMMatrixTranslation( pos.X(), pos.Y(), pos.Z() );
		SetWorldMatrix( mat );
	}
}

void CHoldableObject::ThrowObject()
{
	m_bHolding = false;

	CameraInfo ci = m_ptPlayerMgr->GetMainPlayer()->GetCameraInfo();
	XMMATRIX holdingTransform = GetHoldingTransform();
	float yRot = ci.Yaw;


	Vec pos = Vec( 1.0f, -1.0f, 2.0f );
	// Check xRot's domain.
	float xRot;

	XMMATRIX rotation = XMMatrixRotationX( XM_PIDIV2 ) * XMMatrixRotationY( yRot );
	pos = pos.Transform( rotation );
	pos = pos.Transform( holdingTransform );

	XMMATRIX mat = rotation * XMMatrixTranslation( pos.X(), pos.Y(), pos.Z() );
	SetWorldMatrix( mat );

	Vec accelDir = Vec( 0.0f, 0.5f, 1.0f );
	accelDir = accelDir.Transform( rotation );
	accelDir.Normalize();
	Accelerate( accelDir, 135.0f );
}

XMMATRIX CHoldableObject::GetHoldingTransform()
{
	Vec pos = m_ptPlayerMgr->GetMainPlayer()->GetPosition();
	XMMATRIX transform = XMMatrixTranslation( pos.X(), pos.Y(), pos.Z() );


	return transform;
}

void CHoldableObject::Update()
{
	if ( !m_bHolding )
		return;
	// Transform the static object to the correct holding position.
	CameraInfo ci = m_ptPlayerMgr->GetMainPlayer()->GetCameraInfo();
	XMMATRIX holdingTransform = GetHoldingTransform();
	float yRot = ci.Yaw;
	float xRot = ci.Pitch;


	Vec pos = m_vHoldingPos;
	XMMATRIX rotation = XMMatrixRotationX( xRot ) * XMMatrixRotationY( yRot );
	pos = pos.Transform( rotation );
	pos = pos.Transform( holdingTransform );
	
	
	xRot = xRot * ( 180 / XM_PI );
	xRot += m_xRotModifier;
	yRot = yRot * ( 180 / XM_PI );
	Vec rot = Vec( xRot, yRot, 0.0f );
	m_ptObj->SetX( pos.X() );
	m_ptObj->SetY( pos.Y() );
	m_ptObj->SetZ( pos.Z() );
	m_ptObj->SetRX( rot.X() );
	m_ptObj->SetRY( rot.Y() );
	m_ptObj->SetRZ( rot.Z() );
}


/////////////////////////////////////////
// CFlare:
///////////////////////////////////////

CFlare::CFlare()
	: CHoldableObject( )
{
	m_pPointLight = 0;
	m_pParticleSystem = 0;
	m_xRotModifier = 60.0f;
}

CFlare::~CFlare()
{

}

bool CFlare::Initialize()
{
	CStaticObject* obj = new CStaticObject();
	obj->SetScaleXYZ(0.5f);
	BulletData d;
	d.Mat = PhysMat_Normal;
	d.SpecGrav = 4.0f;
	if ( !obj->Initialize(DXUTGetD3D11Device(), "Flare.obj", eBASIC, d ) )
		return false;

	obj->SetScaleXYZ(0.5f);
	obj->SetIsSerilizible( false );

	ObjectId idOfObj = g_GetEventManager()->GetObjectManager()->InsertObject(obj);

	CGameObject::InitObject(idOfObj);


	CParticleManager* pParticleMgr = g_GetEventManager()->GetParticleManager();

	m_pParticleSystem = new ParticleSystem();
	assert( m_pParticleSystem );
	
	// The particle mgr will default to the effect manager it already points to.
	CParticleEffect* effect = pParticleMgr->GetParticleEffect( "spark", 0 );
	CTexture randomTex = pParticleMgr->GetRandomTexture();
	CTexture texArray = pParticleMgr->GetTextureOfEffect( "spark", 0 );
	m_pParticleSystem->Init( DXUTGetD3D11Device(), effect, texArray.GetTexture(), randomTex.GetTexture(), 500 );

	ParticleSystemInfo psi;
	psi.Accel = XMFLOAT3( 5.0f, 5.0f, 5.0f );
	psi.EmitDir = XMFLOAT3( 1.0f, 0.0f, 0.0f );
	psi.EmitTime = 0.005f;
	psi.MaxParticles = 500;
	psi.Pos = GetPosition().ToFloat3();
	psi.RandomXMod = 0.1f;
	psi.RandomZMod = 0.1f;
	psi.TextureSize = 0.9f;
	psi.Vel = 4.0f;

	m_pParticleSystem->SetInitInfo( psi );

	pParticleMgr->AddParticleSystem( m_pParticleSystem );

	// Create the point light.
	PointLight tempLight;
	tempLight.Ambient = XMFLOAT4( 0.2f, 0.0f, 0.0f, 1.0f );
	tempLight.Diffuse = XMFLOAT4( 0.5f, 0.0f, 0.0f, 1.0f );
	tempLight.LookAt = XMFLOAT3( 0.0f, 1.0f, 0.0f );
	tempLight.Position = XMFLOAT3( 0.0f, 0.9f, 0.0f );
	tempLight.Range = 25.0f;
	tempLight.Specular = XMFLOAT4( 0.5f, 0.0f, 0.0f, 1.0f );
	tempLight.Att = XMFLOAT3( 0.0f, 0.1f, 0.0f );

	m_pPointLight = g_GetEventManager()->GetLightManager()->CreatePointLight( tempLight );

	return true;
}

void CFlare::Update()
{
	CHoldableObject::Update();

	// Make sure that the light, particle system, and the object are all in the same position.

	XMMATRIX worldMatrix = m_ptObj->GetWorldMatrix().ToXMMatrix();
	
	// Offset the particle effect so it is at the end of the flare.
	Vec positionRelative = Vec( 0.0f, 0.9f, 0.0f );

	Vec positionWorld = positionRelative.Transform( worldMatrix );

	m_pParticleSystem->SetEmitPos( positionWorld.ToFloat3() );

	positionWorld.SetY( 0.5f );
	m_pPointLight->Position = positionWorld.ToFloat3();


	// Make the emit dir and xz acceleration in world space which only includes rotation as we don't want the magnitudes of the vectors being messed up.
	Vec rot = GetRotation();
	XMMATRIX rotationMatrix = XMMatrixRotationX( XMConvertToRadians( rot.X() ) ) * XMMatrixRotationY( XMConvertToRadians ( rot.Y() ) ) * XMMatrixRotationZ( XMConvertToRadians ( rot.Z() ) );
	Vec dirRelative = Vec( 0.0f, 1.0f, 0.0f );
	Vec accelRelative = Vec( 0.0f, 10.0f, 0.0f );
	Vec dirWorld = dirRelative.Transform( rotationMatrix );
	Vec accelWorld = accelRelative.Transform( rotationMatrix );
	// Add in particle lift, which is the same regardless.
	accelWorld.AddY( 3.0f );

	m_pParticleSystem->SetEmitInfo( dirWorld.ToFloat3(), accelWorld.ToFloat3() );

	// Set the lights look at.
	Vec lookAtRelative = Vec( 0.0f, 10.0f, 0.0f );
	Vec lookAtWorld = lookAtRelative.Transform( worldMatrix );

	m_pPointLight->LookAt = lookAtWorld.ToFloat3();
}


/////////////////////////////
// CGun:
///////////////////////////

bool CGun::Initialize()
{
	CStaticObject* obj = new CStaticObject();

	BulletData d;
	d.Mat = PhysMat_Normal;
	d.SpecGrav = 4.0f;
	if ( !obj->Initialize( DXUTGetD3D11Device(), "Pistol.obj", eBASIC, d ) )
		return false;

	obj->SetScaleXYZ( 0.5f );
	obj->SetIsSerilizible( false );

	ObjectId idOfObj = g_GetEventManager()->GetObjectManager()->InsertObject(obj);

	CGameObject::InitObject(idOfObj);

	m_vHoldingPos = Vec( 1.0f, -1.0f, 3.0f );

	return true;
}

void CGun::Update()
{
	CHoldableObject::Update();


}

CGun::CGun()
	: CHoldableObject()
{

}

CGun::~CGun()
{

}
