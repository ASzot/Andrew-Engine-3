//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "WaterManager.h"
#include "WaterObject.h"
#include "RenderEffects.h"
#include "WaterObject.h"
#include "Terrain.h"


CWaterManager::CWaterManager(HINSTANCE hInstance, HWND hWnd)
	: CManager( hInstance, hWnd, gEEventTypes::eEffectEventType, new WCF::WaterSystemOptionDialog )
{
	m_iSelectedSystem = -1;
	m_cPlaneModelName = "WaterPlane.obj";
	m_cCircleModelName = "CircularPlane.obj";
}


CWaterManager::~CWaterManager(void)
{
}


BOOL CWaterManager::CreateDlgWindow(void)
{
	// For now do nothing.
	WCF::WaterSystemOptionDialog* window = ( WCF::WaterSystemOptionDialog* )mWindow;

	WaterSystemOptions wso = WaterSystemOptions();
	if ( m_iSelectedSystem != -1 )
		wso = m_WaterSystemList.at( m_iSelectedSystem )->GetDlgInfo();


	return window->RunWindow( mhInstance, mhWnd, wso );
}


void CWaterManager::OnDialogQuit(void)
{
	WCF::WaterSystemOptionDialog* window = ( WCF::WaterSystemOptionDialog* )mWindow;

	if ( m_iSelectedSystem != -1 )
		m_WaterSystemList.at( m_iSelectedSystem )->SetDlgInfo( window->GetWindowData() );
}


void CWaterManager::Serilize(CDataArchiver* archiver)
{
	for ( WaterObjectList::iterator i = m_WaterSystemList.begin(); i != m_WaterSystemList.end(); ++i )
	{
		CWaterObject* water = ( *i );
		archiver->WriteToStream( 'w' );
		archiver->Space();
		archiver->WriteToStream( water->X() );
		archiver->Space();
		archiver->WriteToStream( water->Y() );
		archiver->Space();
		archiver->WriteToStream( water->Z() );
		archiver->Space();
		archiver->WriteToStream( water->RX() );
		archiver->Space();
		archiver->WriteToStream( water->RY() );
		archiver->Space();
		archiver->WriteToStream( water->RZ() );
		archiver->Space();
		archiver->WriteToStream( water->S() );
		archiver->Space();
		archiver->WriteToStream( water->SZ() );
		archiver->Space();
		archiver->WriteToStream( water->IsUsingCircle() );
		archiver->Space();
		archiver->WriteToStream( water->GetWaterScale() );
		archiver->Space();
		archiver->WriteToStream( water->GetWaterTexTrans() );
		archiver->Space();
		archiver->WriteToStream( water->m_TargetedObjects.at( 0 ) );
		archiver->Space();
		archiver->WriteToStream( water->GetDiffuseAb() );
		archiver->Space();
		archiver->WriteToStream( water->GetSpecAb() );
		archiver->WriteToStream( '\n' );
	}
	
}

bool CWaterManager::LoadScene( std::vector< WaterSystemOptions > wsoList )
{
	BulletData bd;
	bd.Mat = PhysMat_Normal;
	bd.SpecGrav = 0.0f;

	for( std::vector< WaterSystemOptions >::iterator i = wsoList.begin(); i != wsoList.end(); ++i )
	{
		WaterSystemOptions wso = ( *i );
		CWaterObject* waterObj = new CWaterObject( wso.x, wso.y, wso.z, wso.sx, wso.sz, wso.UseCircle );
		assert( waterObj );
		const char* modelName = wso.UseCircle ? m_cCircleModelName : m_cPlaneModelName;

		if ( !waterObj->Initialize( DXUTGetD3D11Device(), modelName, eBASIC, bd ) )
			return false;
		waterObj->SetDlgInfo( wso );

		m_WaterSystemList.push_back( waterObj );
	}

	return true;
}


bool CWaterManager::Init(CEventManager* eventManager)
{
	CManager::Init( eventManager, CHashedString( "c6CreateWaterMgrDlg" ) );


	return true;
}


void CWaterManager::Shutdown(void)
{
	CManager::Shutdown();
	
	for ( WaterObjectList::iterator i = m_WaterSystemList.begin(); i != m_WaterSystemList.end(); ++i )
	{
		DestroyObject( ( *i ) );
	}

	SafeDelete( m_pWaterNormalMap );
}


void CWaterManager::DestroyResources(void)
{
	CManager::DestroyResources();

	DestroyGameAssets();

	ReleaseResources( m_pWaterNormalMap );
}


void CWaterManager::Update(float dt)
{
	CManager::Update( dt );

	mEventListener.SetSelectedWaterSystem( m_iSelectedSystem );
}


bool CWaterManager::InitializeResources(ID3D11Device* device)
{
	CManager::InitializeResources( device );

	m_pWaterNormalMap = new CTexture();
	if ( !m_pWaterNormalMap->Init( L"data/Textures/waterNormalMap.dds" ) ) 
		return false;


	return true;
}


void CWaterManager::RenderWaterObjects( CEffectManager* effectManager, CameraInfo& info, ID3D11ShaderResourceView** ppRefractionReflectionTexture, RenderEffects* pRenderEffects )
{
	// Set the per frame effect variables.

	HRESULT hr;
	hr = effectManager->mWaterFX->SetProj( XMMatrixTranspose( info.Proj ) );
	Checkhr( hr, "Couldn't set WaterFX Proj" );
	hr = effectManager->mWaterFX->SetView( XMMatrixTranspose( info.View ) );
	Checkhr( hr, "Couldn't set WaterFX View" );
	hr = effectManager->mWaterFX->SetReflectionTexture( ppRefractionReflectionTexture[1] );
	Checkhr( hr, "Couldn't set WaterFX ReflTexture" );
	
	hr = effectManager->mWaterFX->SetNormalMap( m_pWaterNormalMap->GetTexture() );
	Checkhr( hr, "Couldn't set WaterFX NormalMap" );

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	CGameObjMgr* mgr = g_GetGameObjMgr();
	
	// Render the water objects.
	SRenderSettings::SetNoCull();
	SRenderSettings::SetDefaultDSS();
	SRenderSettings::SetDefaultBS();

	for ( WaterObjectList::iterator i = m_WaterSystemList.begin(); i != m_WaterSystemList.end(); ++i )
	{
		CWaterObject* water = ( *i );

		XMMATRIX reflectionMat = Camera::ConstructReflectionMatrix( water->Y(), info.Position, info.LookAt );
		hr = effectManager->mWaterFX->SetReflection( XMMatrixTranspose( reflectionMat ) );
		Checkhr( hr, "Couldn't set WaterFX Refl" );

		// Begin with rendering all of the objects designated by this object.
		pRenderEffects->RenderRefractionToTexture( effectManager, info, water->Y() );
		ObjectIdList targetObjects = water->GetTargetedObjects();
		for ( ObjectIdList::iterator i = targetObjects.begin(); i != targetObjects.end(); ++i )
		{
			int index = (*i);
			if ( index == -2 )
			{
				// This means we are requesting the terrain for refraction, so render the terrain.
				g_GetTerrain()->RenderRefraction( context, info, effectManager );
				continue;
			}
			IObject* obj = mgr->GetObjectByIndex( index );
			// This could be dangerous as we may begin to want to refract other objects as well
			// but it shall serve as a temporary.
			//TODO:
			// Change risky cast.
			CStaticObject* staticObj = ( CStaticObject* )obj;
			staticObj->RenderRefraction( context, info, effectManager );
		}
		pRenderEffects->ResetBackBufferRenderTarget();

		hr = effectManager->mWaterFX->SetRefractionTexture( pRenderEffects->GetRefractionTexture() );
		Checkhr( hr, "Couldn't set WaterFX RefractTexture" );

		ExtraRenderingData erd;
		water->UpdateWaterTrans();
		water->Render( context, info, effectManager, erd );
	}

	

	SRenderSettings::SetDefaultCull();
}

bool CWaterManager::AddWaterSystem()
{
	return AddWaterSystem( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.001f, 0.1f );
}

bool CWaterManager::AddWaterSystem( float x, float y, float z, float rx, float ry, float rz, float s, float texTrans, float waterScale )
{
	BulletData bd;
	bd.Mat = PhysMat_Normal;
	bd.SpecGrav = 0.0f;

	CWaterObject* waterObj = new CWaterObject();
	assert( waterObj );
	if ( !waterObj->Initialize( DXUTGetD3D11Device(), m_cPlaneModelName, eBASIC, bd ) )
		return false;
	waterObj->SetRX( rx );
	waterObj->SetRY( ry );
	waterObj->SetRZ( rz );
	waterObj->SetX( x );
	waterObj->SetY( y );
	waterObj->SetZ( z );
	waterObj->SetScale( s );
	waterObj->SetSY( 1.0f );
	waterObj->SetSZ( s );
	waterObj->SetWaterScale( waterScale );
	waterObj->SetWaterScale( waterScale );
	waterObj->SetWaterTexTrans( texTrans );

	m_WaterSystemList.push_back( waterObj );

	m_iSelectedSystem = m_WaterSystemList.size() - 1;

	return true;
}

void CWaterManager::DeleteSelectedWaterSystem()
{
	if ( m_iSelectedSystem != -1 )
		DeleteWaterSystem( m_iSelectedSystem );
}

void CWaterManager::DeleteWaterSystem( int index )
{
	CWaterObject* objToDelete = m_WaterSystemList.at( index );
	ReleaseResources( objToDelete );
	DestroyObject( objToDelete );
	WaterObjectList::iterator start = m_WaterSystemList.begin() + index;
	WaterObjectList::iterator end = m_WaterSystemList.begin() + index + 1;
	m_WaterSystemList.erase( start, end );
	m_iSelectedSystem = -1;
	mWindow->mIsWindow = false;
}

void CWaterManager::ListenToMessages(char* message)
{
	if ( message == "AddSelectedSystem" )
	{
		IncSelectedSystem();
	}
	else if ( message == "SubSelectedSystem" )
	{
		DecSelectedSystem();
	}
	else if ( message == "AddSystem" )
	{
		AddWaterSystem();
	}
	else if ( message == "SubSystem" )
	{
		DeleteSelectedWaterSystem();
	}
}

void CWaterManager::ProcessMessage(CHashedString message)
{
	if ( message == "c6IncSelectedWaterSystem" )
	{
		IncSelectedSystem();
		mEventListener.RemoveAllOfMessage( message );
	}
	else if ( message == "c6DecSelectedWaterSystem" )
	{
		DecSelectedSystem();
		mEventListener.RemoveAllOfMessage( message );
	}
}

void CWaterManager::IncSelectedSystem()
{
	int totalSystems = m_WaterSystemList.size();
	if ( totalSystems == 0 )
	{
		m_iSelectedSystem = -1;
		return;
	}
	if ( ( m_iSelectedSystem + 1 ) > ( totalSystems - 1 ) )
	{
		m_iSelectedSystem = 0;
		return;
	}
	
	++m_iSelectedSystem;
}

void CWaterManager::DecSelectedSystem()
{
	int totalSystems = m_WaterSystemList.size();
	if ( totalSystems == 0 )
	{
		m_iSelectedSystem = -1;
		return;
	}
	if ( ( m_iSelectedSystem - 1 ) < 0 )
	{
		m_iSelectedSystem = ( totalSystems - 1 );
		return;
	}
	
	--m_iSelectedSystem;
}

void CWaterManager::DestroyGameAssets()
{
	for ( WaterObjectList::iterator i = m_WaterSystemList.begin(); i != m_WaterSystemList.end(); ++i )
	{
		ReleaseResources( ( *i ) );
	}

	m_WaterSystemList.clear();
	m_WaterSystemList.resize(0);
}
