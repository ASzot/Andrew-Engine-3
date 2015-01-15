//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "TerrainMgr.h"
#include "WindowCreatorFoundation.h"


CTerrainMgr::CTerrainMgr(HINSTANCE hInstance, HWND hwnd)
	: CManager( hInstance, hwnd, eTerrainEventType, new WCF::TerrainDialog() ) 
{
	m_pTerrain = 0;
}


CTerrainMgr::~CTerrainMgr(void)
{
}


bool CTerrainMgr::Init(CEventManager* eventManager,CHashedString messageForWindowCreation)
{
	return CManager::Init( eventManager, messageForWindowCreation );
}

bool CTerrainMgr::InitializeResources(ID3D11Device* device)
{
	CManager::InitializeResources( device );

	//InitInfo tii;
	//tii.HeightMapFilename	= L"data/Textures/Heightmaps/heightmap05.bmp";
	//tii.LayerMapFilename0	= L"data/Textures/Grounds/Grass.dds";
	//tii.LayerMapFilename1	= L"data/Textures/Grounds/RockFragments.dds";
	//tii.LayerMapFilename2	= L"data/Textures/Grounds/dirt004.dds";
	//tii.LayerMapFilename3	= L"data/Textures/Grounds/snow.dds";
	//tii.LayerMapFilename4	= L"data/Textures/stone.dds";
	//tii.BlendMapFilename	= L"data/Textures/Heightmaps/TextureLayers/blend3.png";
	//tii.HeightScale			= 50.0f;
	//tii.HeightmapWidth		= 2048;
	//tii.HeightmapHeight		= 2048;
	//tii.CellSpacing			= 0.5f;

	return true;
}

void CTerrainMgr::Shutdown(void)
{
	CManager::Shutdown();
}

void CTerrainMgr::DestroyResources(void)
{
	CManager::DestroyResources();
	SafeDelete( m_pTerrain );
}

void CTerrainMgr::Update(float dt)
{
	CManager::Update( dt );
}

void CTerrainMgr::Serilize(CDataArchiver* archiver)
{
	if ( m_pTerrain )
		m_pTerrain->Serilize( archiver );
}

BOOL CTerrainMgr::CreateDlgWindow()
{
	if ( !m_pTerrain )
		return TRUE;
	WCF::TerrainDialog* window = (WCF::TerrainDialog*)mWindow;
	TerrainDlgData tdd = m_pTerrain->GetDlgData();

	return window->RunWindow( mhInstance, mhWnd, tdd );
}

void CTerrainMgr::OnDialogQuit()
{
	if ( !m_pTerrain )
		return;
	WCF::TerrainDialog* window = ( WCF::TerrainDialog* )mWindow;
	TerrainDlgData data = window->GetWindowData();

	m_pTerrain->SetDlgData( data );
}

void CTerrainMgr::Render( CEffectManager* effectManager, CameraInfo& cam, ExtraRenderingData& erd )
{
	if ( !m_pTerrain )
		return;
	m_pTerrain->Draw( DXUTGetD3D11DeviceContext(), cam, effectManager, erd );
}

void CTerrainMgr::RenderToShadowMap( CEffectManager* effectManager, CameraInfo& cam )
{
	if ( m_pTerrain )
		m_pTerrain->RenderShadowMap( DXUTGetD3D11DeviceContext(), cam ,effectManager );
}

bool CTerrainMgr::LoadScene(InitInfo ii)
{
	if ( ii.HeightMapFilename.empty() )
		return true;
	m_pTerrain = new CTerrain();
	assert( m_pTerrain );

	m_pTerrain->Init( DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext(), ii );

	return true;
}

void CTerrainMgr::DestroyGameAssets()
{
	SafeDelete(m_pTerrain);
}
