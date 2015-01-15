//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Scene.h"

CScene::CScene()
	: mListener(eInputEventType), m_MainPlayer()
{

}

CScene::~CScene()
{

}

bool CScene::InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd,Camera* cam,CEventManager* eventManager)
{
	m_bFirstLoad = false;
	Beep( 500, 100 );

	m_pPhysicsMgr->InitResources( device,cam,m_pObjMgr );
	// Basically update the loading dlg.

	if(!mGraphics->InitResources(device,sd))
		return false;

	// This needs to be loaded.
	Vec startPos ( 5.0f, 7.0f, -10.0f );
	if (!m_MainPlayer.Initialize( startPos, m_pObjMgr, eventManager ) )
	{
		FatalError( "Couldn't init the main player." );
		return false;
	}
	if ( !m_MainPlayer.InitResources( device ) ) 
	{
		FatalError( "Couldn't init main players resources!\nbool CScene::InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd,Camera* cam)");
		return false;
	}

	m_PlayerManager->RegisterMainPlayer( &m_MainPlayer );

	m_PlayerManager->InitResources(device);
	
	Beep( 750, 100 );
	
	return true;
}

bool CScene::Init(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow)
{
	m_pObjMgr = new CGameObjMgr();
	assert(m_pObjMgr);

	m_pPhysicsMgr = new CPhysicsManager(instance,mainWindow);
	assert(m_pPhysicsMgr);
	// The physics system is registered here.
	if(!m_pPhysicsMgr->Init(m_pObjMgr, eventManager))
		return false;

	mGraphics = new CGraphics( instance, mainWindow );
	assert(mGraphics);
	if ( !mGraphics->Init( eventManager, instance, mainWindow, m_pObjMgr ) )
		return false;

	// This has to be after the init of graphics.
	m_PlayerManager = new CPlayerManager( m_pObjMgr );
	m_PlayerManager->Init(eventManager);

	eventManager->RegisterListener(&mListener);

	return true;
}

void CScene::Update(GameStates gameState,double time, float elapsedTime,CameraInfo& cam,ID3D11Device* device,ID3D11DeviceContext* context)
{
	m_MainPlayer.SetCameraInfo( cam );
	float moveSpeed = 20.0f * elapsedTime;
	const float characterMoveSpeed = 10.0f * elapsedTime;
	if( GetAsyncKeyState('W') & 0x8000)
	{
		if ( gameState != eRunning )
			cam.Position = WalkCamera(moveSpeed,cam.LookAt,cam.Position);
		else
			m_MainPlayer.Walk(characterMoveSpeed,cam, m_pObjMgr->GetPhysics() );
	}

	if( GetAsyncKeyState('S') & 0x8000)
	{
		if ( gameState != eRunning )
			cam.Position = WalkCamera(-moveSpeed,cam.LookAt,cam.Position);
		else
			m_MainPlayer.Walk(-characterMoveSpeed,cam, m_pObjMgr->GetPhysics() );
	}

	
	if( GetAsyncKeyState('A') & 0x8000 )
	{
		if ( gameState != eRunning )
			cam.Position = StrafeCamera(-moveSpeed,cam.Right,cam.Position);
		else
			m_MainPlayer.Strafe(-characterMoveSpeed,cam, m_pObjMgr->GetPhysics() );
	}

	if( GetAsyncKeyState('D') & 0x8000 )
	{
		if ( gameState != eRunning )
			cam.Position = StrafeCamera(moveSpeed,cam.Right,cam.Position);
		else
			m_MainPlayer.Strafe(characterMoveSpeed,cam, m_pObjMgr->GetPhysics() );
	}




	if ( gameState == eRunning)
	{
		Vec actualCamPos = m_MainPlayer.m_PhysBody->GetPosAsVec();
		cam.Position = actualCamPos.ToFloat3();
	}

	m_MainPlayer.Update( elapsedTime, m_pObjMgr );
	// Keep in mind the order of these updates does matter.
	m_pPhysicsMgr->Update(elapsedTime,time);
	
	CheckEvents( cam );

	// Check to see if the main player is dead.
	if ( m_MainPlayer.IsDead() )
		m_MainPlayer.Respawn( m_pPhysicsMgr->m_pPhysicsSystem );

	if ( gameState == eRunning )
		m_PlayerManager->Update( elapsedTime, m_pObjMgr );
	mGraphics->Update(gameState,time,elapsedTime,cam,device,context);
}

void CScene::Render(ID3D11DeviceContext* context,CameraInfo& cam)
{
	m_MainPlayer.Render(cam );
	m_pPhysicsMgr->Render( );
	mGraphics->Render(context,cam);
}

void CScene::Shutdown()
{
	DestroyObject(m_pObjMgr);
	DestroyObject(mGraphics);
	DestroyObject(m_pPhysicsMgr);
	DestroyObject( m_PlayerManager );
	m_MainPlayer.Shutdown();
}

void CScene::DestroyResources()
{
	ReleaseResources(mGraphics);
	ReleaseResources( m_PlayerManager );
	m_MainPlayer.DestroyResources();
}

void CScene::Serilize( CDataArchiver* dataArchiver )
{
	mGraphics->Serilize( dataArchiver );
	m_pPhysicsMgr->Serilize( dataArchiver );
}

// Loads a new scene completely destroys the old one
bool CScene::LoadScene(const char* filename, ID3D11Device* device)
{
	if ( !m_bFirstLoad )
	{
		m_pPhysicsMgr->Restart();
		PhysicsSynced( true );
	}
	
	bool result = mGraphics->LoadScene( filename, device );

	if ( !m_bFirstLoad )
	{
		ReleaseResources(m_PlayerManager);
		DestroyObject(m_PlayerManager);

		m_MainPlayer.DestroyResources();
		m_MainPlayer.Shutdown();
		
		m_MainPlayer.Initialize(Vec( 5.0f, 7.0f, -10.0f ), g_GetGameObjMgr(), g_GetEventManager());
		m_MainPlayer.InitResources(device);

		m_PlayerManager = new CPlayerManager(g_GetGameObjMgr());
		m_PlayerManager->Init( g_GetEventManager() );
		m_PlayerManager->InitResources(device);
	}

	m_bFirstLoad = false;
	return result;
}

void CScene::OnResize(const DXGI_SURFACE_DESC* sd, ID3D11Device* device)
{
	mGraphics->OnResize(sd,device);
}


XMFLOAT3 CScene::WalkCamera(float d,XMFLOAT3 look,XMFLOAT3 pos)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&look);
	XMVECTOR p = XMLoadFloat3(&pos);
	XMFLOAT3 position;
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, l, p));
	return position;
}

XMFLOAT3 CScene::StrafeCamera(float d,XMFLOAT4 right,XMFLOAT3 pos)
{
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat4(&right);
	XMVECTOR p = XMLoadFloat3(&pos);
	XMFLOAT3 position;
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, r, p));
	return position;
}

void CScene::CheckEvents(CameraInfo& cam)
{
	while(mListener.HasPendingMessages())
	{
		CHashedString message = mListener.GetRecievedMessage();
		if ( message == "c1MainPlayerJump" )
			m_MainPlayer.Jump( cam, m_pObjMgr->GetPhysics() );
		else if ( message == "c1MainPlayerDropObject" )
			m_MainPlayer.DropHoldingObject();
		else if ( message == "c1MainPlayerThrowObject" )
			m_MainPlayer.ThrowHoldingObject();
	}
}


void CScene::OnLeftMouseBtnDown(WPARAM mouseState,int x,int y,Camera& cam)
{
	UINT clickFlags = m_MainPlayer.GetLeftClickMessage();
	m_pPhysicsMgr->CastRay( x, y, cam, clickFlags );
	m_MainPlayer.Shot();
}