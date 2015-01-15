#include "Graphics.h"
#include "WaterManager.h"

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================


// Since we aren't subscribing for the events. But we still want the functions.
CGraphics::CGraphics(HINSTANCE hInstance, HWND hwnd) 
	: CManager( hInstance, hwnd, gEEventTypes::eEffectEventType, new WCF::GraphicsPropDialog() )
{
	m_pShadowMapper = 0;
	m_PerFrameInfo.FogColor = XMFLOAT4( 0.5, 0.5f, 0.5f, 1.0f );
	m_PerFrameInfo.FogRange = 125.0f;
	m_PerFrameInfo.FogStart = 10.0f;
	m_pWaterMgr = 0;
	m_bUseSkymap = false;
	m_bFlashSel = false;
	m_bFirstLoad = true;
}

CGraphics::~CGraphics()
{

}

bool CGraphics::InitResources(ID3D11Device* device,const DXGI_SURFACE_DESC* sd)
{
	CManager::InitializeResources( device );

	mEventListener.ChangeGameState( eLoading );

	if(!mObjectManager->InitResources(device))
		return false;
	
	if(!mEffectManager->InitResources(device))
		return false;

	if(!mParticleManager->InitializeResources( device, mEffectManager ))
		return false;


	SRenderSettings::InitAllSettings(device);
	

	if ( !m_pRenderEffects->InitResources( sd ) )
		return false;

	if ( !m_pAnimObjMgr->InitializeResources( device ) )
		return false;

	const UINT shadowMapSize = 4096.0f; 
	m_pShadowMapper = new CShadowMapper( device, shadowMapSize, shadowMapSize );
	assert( m_pShadowMapper );

	m_pSky = new CSky( device, L"data/Textures/Skymaps/grasscube1024.dds", 5000.0f );
	assert( m_pSky );

	if ( !m_pWaterMgr->InitializeResources( device ) )
		return false;

	if (!m_pTerrainMgr->InitializeResources( device ) )
		return false;

	LoadScene("data/Levels/Default.txt",device);

	return true;
}

void CGraphics::Render(ID3D11DeviceContext* context,CameraInfo& cam)
{
	//THIS NEEDS TO BE BEFORE THE RENDERING OF THE SCENE
	mLightManager->Render( context, mEffectManager, cam, m_pAnimObjMgr->GetEffect() );

	RenderShadowMap( context );
	

	m_pRenderEffects->Prerender( this, mObjectManager, m_pWaterMgr, mEffectManager, cam );
	RenderScene( context, cam, RenderPass::RenderNormalPass );
}

void CGraphics::RenderScene( ID3D11DeviceContext* context, CameraInfo& cam, RenderPass rp  )
{
	// Set all the per frame stuff.
	m_PerFrameInfo.pCubeMap		= m_pSky->CubeMapSRV();
	mEffectManager->SetPerFrame( m_PerFrameInfo, m_pShadowMapper->GetShadowMaps(), m_pAnimObjMgr->GetEffect() );

	ExtraRenderingData erd;
	erd.pShadowMap = NULL;
	erd.ShadowTransform = mLightManager->GetShadowTransform();

	if ( rp == RenderPass::RenderNormalPass )
	{
		mObjectManager->Render (context, cam, mEffectManager, erd );
		if ( mEventListener.GetGameState() == eBuildingLevel )
			mLightManager->RenderLightSymbols( context, mEffectManager, cam );
	}
	else if ( rp == RenderPass::RenderReflectionPass )
	{
		mObjectManager->RenderReflectableObjects( cam, mEffectManager, erd, 1.0f );
	}

	ID3D11ShaderResourceView* nullSRV[ 1 ] = { NULL };
	for ( int i = 0; i < 12; ++i )
		context->PSSetShaderResources( i, 1, nullSRV );
	

	m_pTerrainMgr->Render( mEffectManager, cam, erd );

	m_pAnimObjMgr->Render( cam );
	ID3D11ShaderResourceView* anotherNullSRV[ 1 ] = { NULL };
	for ( int i = 0; i < 12; ++i )
		context->PSSetShaderResources( i, 1, nullSRV );

	if ( m_bUseSkymap )
		m_pSky->Render( cam, mEffectManager );

	mParticleManager->Render(context,cam,mEffectManager);
}

void CGraphics::RenderShadowMap(ID3D11DeviceContext* context)
{
	// Set the camera info based on the lights.
	CameraInfo cam;

	
	// Render the point shadow map for each point light.
	int numberOfPointLights = mLightManager->GetNumberOfPointLights();
	for ( int i = 0; i < numberOfPointLights; ++i )
	{
		m_pShadowMapper->BindDsvAndSetNullRenderTarget( context, i );
		mLightManager->GeneratePointLightView( i );
		cam.Proj = mLightManager->GetLightProj();
		cam.View = mLightManager->GetLightView();
		mObjectManager->RenderShadowMap( context, cam, mEffectManager, mLightManager );
		m_pAnimObjMgr->RenderShadowMap( mLightManager, cam );
	}
	
	//m_pTerrainMgr->RenderToShadowMap( mEffectManager, cam );

	ID3D11RenderTargetView* renderTargets[ 1] = { DXUTGetD3D11RenderTargetView() };
	context->OMSetRenderTargets( 1, renderTargets, DXUTGetD3D11DepthStencilView() );
	D3D11_VIEWPORT vp;
	vp.Width	= (FLOAT)DXUTGetWindowWidth();
	vp.Height	= (FLOAT)DXUTGetWindowHeight();
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.MinDepth = 0.0f;
	context->RSSetViewports( 1, &vp );
	
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	context->ClearRenderTargetView( DXUTGetD3D11RenderTargetView(), clearColor );
	context->ClearDepthStencilView( DXUTGetD3D11DepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
}

void CGraphics::Shutdown()
{
	CManager::Shutdown();

	DestroyObject(m_pTerrainMgr );
	DestroyObject( m_pWaterMgr );
	DestroyObject( m_pAnimObjMgr );
	DestroyObject( m_pRenderEffects );
	DestroyObject( mLightManager );
	DestroyObject( mEffectManager );
	DestroyObject( mObjectManager );
	DestroyObject( mParticleManager );
}

void CGraphics::DestroyResources()
{
	CManager::DestroyResources();


	ReleaseResources( m_pTerrainMgr );
	ReleaseResources( m_pWaterMgr );

	//Because this is all directX resources
	SafeDelete( m_pSky );
	SafeDelete( m_pShadowMapper );


	ReleaseResources(m_pAnimObjMgr);
	ReleaseResources(m_pRenderEffects);
	SRenderSettings::DestroyAllSettings();
	ReleaseResources(mLightManager);
	ReleaseResources(mEffectManager);
	ReleaseResources(mObjectManager);
	ReleaseResources(mParticleManager);
}

bool CGraphics::Init(CEventManager* eventManager,HINSTANCE instance,HWND mainWindow,CGameObjMgr* ctrlMgr)
{
	CManager::Init( eventManager, CHashedString( "c6CreateGraphicsDlg" ) );

	mObjectManager = new CObjectManager(eventManager,instance,ctrlMgr);
	assert(mObjectManager);

	mObjectManager->Initialize(eventManager,mainWindow);

	mEffectManager = new CEffectManager;
	assert(mEffectManager);
	if(!mEffectManager->Init())
		return false;

	mLightManager = new CLightManager( eGRAPHICS_SETTINGS_HIGH );
	if(!mLightManager)
		return false;
	if(!mLightManager->Initialize(instance,mainWindow,eventManager))
		return false;

	mParticleManager = new CParticleManager(instance,mainWindow);
	assert(mParticleManager);
	if(!mParticleManager->Init(eventManager,CHashedString("c5CreateWindow")))
		return false;

	m_pRenderEffects = new RenderEffects( instance, mainWindow );
	assert( m_pRenderEffects );
	if ( !m_pRenderEffects->Init( eventManager ) )
		return false;

	m_pAnimObjMgr = new CAnimObjectMgr();
	assert(m_pAnimObjMgr);
	if(!m_pAnimObjMgr->Init(eventManager,CHashedString("c7CreateWindow"),ctrlMgr))
		return false;

	m_pWaterMgr = new CWaterManager( instance, mainWindow );
	assert( m_pWaterMgr );
	if ( !m_pWaterMgr->Init( eventManager ) )
		return false;

	m_pTerrainMgr = new CTerrainMgr( instance, mainWindow );
	assert( m_pTerrainMgr );
	if ( !m_pTerrainMgr->Init( eventManager, CHashedString( "c8CreateTerrainWindow" ) ) )
		return false;
	
	return true;
}

bool CGraphics::LoadScene(const char* filename,ID3D11Device* device)
{
	mEventListener.ChangeGameState( eLoading );
	SceneInfo sceneInfo = ModelLoader::LoadScene(filename);

	if ( !m_bFirstLoad )
		Unload();

	SetDlgInfo( sceneInfo.GInfo );

	if ( !m_pWaterMgr->LoadScene( sceneInfo.WaterSystemInfo ) )
		return false;

	if( !mLightManager->LoadScene(sceneInfo.LightingInfo) )
		return false;

	if ( !mObjectManager->LoadScene( device, sceneInfo.GeometryInfo ) )
		return false;

	if ( !mParticleManager->LoadScene( device, sceneInfo.PSystemInfo, mEffectManager ) )
		return false;

	if ( !m_pTerrainMgr->LoadScene( sceneInfo.TerrainLoadInfo ) )
		return false;

	mEventListener.ChangeGameState( eRunning );

	m_bFirstLoad = false;
}

void CGraphics::Update(GameStates gameState,double time, float elapsedTime,CameraInfo& cam,ID3D11Device* device,ID3D11DeviceContext* context)
{
	CManager::Update( elapsedTime );


	mParticleManager->Update( time, elapsedTime, cam, device, context );
	m_pTerrainMgr->Update( time );
	m_pWaterMgr->Update( time );
	m_pAnimObjMgr->Update(time,elapsedTime);
	mLightManager->Update(gameState, elapsedTime);
	mObjectManager->Update( gameState, m_bFlashSel );
}

void CGraphics::Serilize(CDataArchiver* dataArchiver)
{
	dataArchiver->WriteToStream( 'g' );
	dataArchiver->Space();
	dataArchiver->WriteToStream( m_bUseSkymap );
	dataArchiver->WriteToStream( '\n' );

	m_pTerrainMgr->Serilize( dataArchiver );
	m_pWaterMgr->Serilize( dataArchiver );
	m_pAnimObjMgr->Serilize( dataArchiver );
	mLightManager->Serilize( dataArchiver );
	mObjectManager->Serilize( dataArchiver );
	mParticleManager->Serilize( dataArchiver );
}

void CGraphics::OnResize(const DXGI_SURFACE_DESC* sd, ID3D11Device* device)
{
	mParticleManager->OnResized(sd,device);
	m_pRenderEffects->OnResize(sd->Width,sd->Height);
}


BOOL CGraphics::CreateDlgWindow()
{
	WCF::GraphicsPropDialog* window = ( WCF::GraphicsPropDialog* )mWindow;

	GraphicsInfo gi;
	gi.UseSkymap = m_bUseSkymap;
	gi.Flash = m_bFlashSel;

	return window->RunWindow( DXUTGetHINSTANCE(), DXUTGetHWND(), gi );
}

void CGraphics::OnDialogQuit()
{
	WCF::GraphicsPropDialog* window = ( WCF::GraphicsPropDialog* )mWindow;

	GraphicsInfo gi = window->GetWindowData();

	SetDlgInfo( gi );
}

void CGraphics::SetDlgInfo( GraphicsInfo info )
{
	m_bUseSkymap = info.UseSkymap;
	m_bFlashSel = info.Flash;
}

void CGraphics::Unload()
{
	m_pWaterMgr->DestroyGameAssets();
	mLightManager->DestroyGameAssets();
	mObjectManager->DestroyGameAssets();
	mParticleManager->DestroyGameAssets();
	m_pTerrainMgr->DestroyGameAssets();
}
