//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "ParticleManager.h"
#include "RenderSettings.h"
#include "System.h"


CParticleManager::CParticleManager(HINSTANCE hInstance,HWND hwnd) 
	: CManager(hInstance,hwnd,eParticleSystem,new WCF::ParticlePropDialog())
{
	m_iSelectedSystem = -1;
	m_ptEffectManager = 0;
}


CParticleManager::~CParticleManager(void)
{
}


bool CParticleManager::Init(CEventManager* eventManager, CHashedString messageToWatch)
{
	if(!CManager::Init(eventManager,messageToWatch))
		return false;

	eventManager->RegisterParticleMgr( this );

	return true;
}

bool CParticleManager::InitializeResources(ID3D11Device* device, CEffectManager* effectManager)
{
	m_RandomTex.CreateRandom1D();
	std::vector<std::wstring> flares;
	flares.push_back( L"data/Textures/flare0.dds" );
	m_FireTex.Create2DArray( flares );

	std::vector<std::wstring> sparks;
	sparks.push_back( L"data/Particles/Flare.png" );
	m_SparkTex.Create2DArray( sparks );

	m_ptEffectManager = effectManager;

	m_ExplosionMgr.Initialize(device);

	return true;
}


void CParticleManager::Shutdown(void)
{
	CManager::Shutdown();
	for(int i = 0; i < mParticleSystemList.size(); ++i)
	{
		delete mParticleSystemList.at(i);
		mParticleSystemList.at(i) = 0;
	}
}


void CParticleManager::DestroyResources(void)
{
	//ReleaseCOM(m_pDepthStencilSRV);
	//ReleaseCOM(m_pDepthStencilView);
	//ReleaseCOM(m_pDepthStencilTexture);
	CManager::DestroyResources();
	m_ExplosionMgr.OnDeviceDestroyed();
	for(int i = 0; i < mParticleSystemList.size(); ++i)
	{
		mParticleSystemList.at(i)->DestroyResources();
	}

	m_SparkTex.DestroyResources();
	m_FireTex.DestroyResources();
	m_RandomTex.DestroyResources();
}


void CParticleManager::Update(double time, float elapsedTime,CameraInfo& cam,ID3D11Device* device,ID3D11DeviceContext* context)
{
	CManager::Update(elapsedTime);
	for ( ParticleSysList::iterator i = mParticleSystemList.begin(); i != mParticleSystemList.end(); ++i )
	{
		(*i)->Update( elapsedTime, time );
	}

	// Send the selected system as text for system to render.
	std::wstring textToRenderStr = WCF::ConversionTools::IntToWStr( m_iSelectedSystem );
	g_GetEventManager()->GetSystem()->SendTextForRendering( textToRenderStr );
	m_ExplosionMgr.Update( cam, time, elapsedTime );
}

void CParticleManager::ProcessMessage(CHashedString message)
{
	const float moveSpeed = 0.05f;

	ParticleSystem* pSys = 0;
	if ( m_iSelectedSystem > -1 )
		pSys = mParticleSystemList.at( m_iSelectedSystem );

	if ( mEventListener.GetSelectedMgr() == eManager::eParticleMgr && m_iSelectedSystem > -1 )
	{
		if ( m_iSelectedSystem > -1 ) 
		{
#pragma region ObjectMovement
			if ( message == "c1EditorMoveForward" )
			{
				pSys->MoveForward( moveSpeed );
			}
			else if ( message == "c1EditorMoveBackward" )
			{
				pSys->MoveBack( moveSpeed );
			}
			else if ( message == "c1EditorMoveLeft" )
			{
				pSys->MoveLeft( moveSpeed );
			}
			else if ( message == "c1EditorMoveRight" )
			{
				pSys->MoveRight( moveSpeed );
			}
			else if ( message == "c1EditorMoveUp" )
			{
				pSys->MoveUp( moveSpeed );
			}
			else if ( message == "c1EditorMoveDown" )
			{
				pSys->MoveDown( moveSpeed );
			}
#pragma endregion
		}
	}

	if ( message == "c5IncSelectedSystem" )
	{
		int totalSystems = mParticleSystemList.size();
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
	else if ( message == "c5DecSelectedSystem" )
	{
		int totalSystems = mParticleSystemList.size();
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


}


BOOL CParticleManager::CreateDlgWindow()
{
	WCF::ParticlePropDialog* window = (WCF::ParticlePropDialog*)mWindow;
	ParticleSystemData data;
	if ( m_iSelectedSystem > -1 )
		data = mParticleSystemList.at(m_iSelectedSystem)->GetDlgInfo();

	return window->RunWindow(mhInstance,mhWnd,data);
}

void CParticleManager::Serilize(CDataArchiver* dataArchiver)
{
	for ( ParticleSysList::iterator i = mParticleSystemList.begin(); i != mParticleSystemList.end(); ++i )
	{
		(*i)->Serilize( dataArchiver );
	}
}


bool CParticleManager::CreateParticleSystem( XMFLOAT3 pos, XMFLOAT3 emitDir, float texSize, float vel, float randX, float randZ, UINT maxParticles )
{ 
	ID3D11Device* device = DXUTGetD3D11Device();
	ParticleSystem* pSys = new ParticleSystem();
	ParticleSystemInfo psi;
	psi.EmitDir = emitDir;
	psi.MaxParticles = maxParticles;
	psi.Pos = pos;
	psi.RandomXMod = randX;
	psi.RandomZMod = randZ;
	psi.Accel = XMFLOAT3( 0.0f, 4.0f, 0.0f );
	psi.Vel = vel;
	psi.EmitTime = 0.005f;
	psi.TextureSize = texSize;
	pSys->SetInitInfo( psi );
	pSys->Init( device, m_ptEffectManager->mSparkFX, m_SparkTex.GetTexture(), m_RandomTex.GetTexture(), 500 );

	mParticleSystemList.push_back( pSys );

	return true;
}


void CParticleManager::OnDialogQuit()
{
	WCF::ParticlePropDialog* dialog = (WCF::ParticlePropDialog*)mWindow;
	ParticleSystemData data = dialog->GetWindowData();

	if ( m_iSelectedSystem > -1 )
		mParticleSystemList.at( m_iSelectedSystem )->SetDlgInfo( data );
}

void CParticleManager::Render(ID3D11DeviceContext* context,CameraInfo& cam,CEffectManager* effectManager)
{
	for ( ParticleSysList::iterator i = mParticleSystemList.begin(); i != mParticleSystemList.end(); ++i )
	{
		SRenderSettings::ResetAllStates();
		( *i )->SetEyePos( cam.Position );
		( *i )->Draw( context, cam, effectManager );
		SRenderSettings::ResetAllStates();
	}
	m_ExplosionMgr.Render( context, DXUTGetTime(), DXUTGetElapsedTime(), cam, D3DXVECTOR3( 1, 1, 0 ) );
	SRenderSettings::ResetAllStates();
}


void CParticleManager::OnResized(const DXGI_SURFACE_DESC* sd,ID3D11Device* device)
{
}

bool CParticleManager::LoadScene(ID3D11Device* device, std::vector<ParticleSystemInfo> psi, CEffectManager* effectManager)
{
	if ( mParticleSystemList.size() > 0 )
	{
		DestroyResources();
		Shutdown();
	}
	for ( std::vector<ParticleSystemInfo>::iterator i = psi.begin(); i != psi.end(); ++i )
	{
		ParticleSystemInfo curPsi = ( *i );

		ParticleSystem* pSys = new ParticleSystem();
		assert( pSys );

		pSys->SetInitInfo( curPsi );
		pSys->Init( device, GetParticleEffect( curPsi.code, effectManager ), GetTextureOfEffect( curPsi.code, effectManager ).GetTexture(), m_RandomTex.GetTexture(), curPsi.MaxParticles );
		mParticleSystemList.push_back( pSys );
	}

	return true;
}

void CParticleManager::ListenToMessages(char* message)
{
	if ( message == "AddSystem" )
		CreateDefaultParticleSystem();
	else if ( message == "DeleteSystem" )
		RemoveParticleSystem( m_iSelectedSystem );
}

CParticleEffect* CParticleManager::GetParticleEffect( std::string code, CEffectManager* effectManager )
{
	if ( effectManager == 0 )
	{
		if ( m_ptEffectManager )
			effectManager = m_ptEffectManager;
		else
			DebugWriter::WriteToDebugFile( "The pointer to the effect manager was attempted to be accessed but doesn't exist!", "CTexture CParticleManager::GetTextureOfEffect( std::string code, CEffectManager* effectManager )" );
	}
	if ( code == effectManager->mFireFX->code )
		return effectManager->mFireFX;
	else if ( code == effectManager->mSparkFX->code )
		return effectManager->mSparkFX;

	return 0;
}


ObjectId CParticleManager::AddParticleSystem( ParticleSystem* particleSystem)
{
	if ( particleSystem )
	{
		mParticleSystemList.push_back( particleSystem );
	}

	return ( mParticleSystemList.size() - 1 );
}

void CParticleManager::RemoveParticleSystem( ObjectId id )
{
	ParticleSysList::iterator i = mParticleSystemList.begin() + id;
	(*i)->DestroyResources();
	delete (*i);
	(*i) = 0;
	mParticleSystemList.erase( i );
	m_iSelectedSystem = -1;
}

CTexture CParticleManager::GetTextureOfEffect( std::string code, CEffectManager* effectManager )
{
	if ( effectManager == 0 )
	{
		if ( m_ptEffectManager )
			effectManager = m_ptEffectManager;
		else
			DebugWriter::WriteToDebugFile( "The pointer to the effect manager was attempted to be accessed but doesn't exist!", "CTexture CParticleManager::GetTextureOfEffect( std::string code, CEffectManager* effectManager )" );
	}
	if ( code == effectManager->mFireFX->code )
		return m_FireTex;
	else if ( code == effectManager->mSparkFX->code )
		return m_SparkTex;
	else
		return m_FireTex;
}

void CParticleManager::CreateDefaultParticleSystem()
{
	CreateParticleSystem( XMFLOAT3( 0.0f, 2.0f, 3.0f ), XMFLOAT3( 0.0f, 1.0f, 0.0f ), 3.0f, 4.0f, 0.5f, 0.5f, 500 );
}

CTexture CParticleManager::GetRandomTexture()
{
	return m_RandomTex;
}

CExplosionMgr* CParticleManager::GetExplosionMgr()
{
	return &m_ExplosionMgr;
}

void CParticleManager::DestroyGameAssets()
{
	for(int i = 0; i < mParticleSystemList.size(); ++i)
	{
		mParticleSystemList.at(i)->DestroyResources();
	}

	mParticleSystemList.clear();
	mParticleSystemList.resize(0);
}
