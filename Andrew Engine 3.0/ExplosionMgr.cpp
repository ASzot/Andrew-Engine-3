//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "ExplosionMgr.h"
#include "DebugWriter.h"

#define MAX_NUM_PARTICLES 2000


CExplosionMgr::CExplosionMgr(void)
	:   m_NumStalkParticles( 500 ),
		m_NumGroundExpParticles( 345 ),
		m_NumLandMineParticles( 125 ),
		m_NumMushParticles( 200 )
{
	m_NumActiveMushSystems		= 0;
	m_NumActiveBurstSystems		= 0;
	m_NumActiveMineSystems		= 0;

	m_NumParticles				= 200;
	m_fSpread					= 4.0f;
	m_fStartSize				= 0.0f;
	m_fEndSize					= 10.0f;
	m_fSizeExponent				= 128.0f;

	m_fMushroomCloudLifeSpan	= 10.0f;
	m_fGroundBurstLifeSpan		= 9.0f;
	m_fPopperLifeSpan			= 9.0f;


	m_fMushroomStartSpeed		= 20.0f;
	m_fStalkStartSpeed			= 50.0f;
	m_fGroundBurstStartSpeed	= 100.0f;
	m_fLandMineStartSpeed		= 250.0f;

	m_fEndSpeed					= 4.0f;
	m_fSpeedExponent			= 32.0f;
	m_fFadeExponent				= 4.0f;
	m_fRollAmount				= 0.2f;
	m_fWindFalloff				= 20.0f;
	m_vPosMul					= D3DXVECTOR3( 1,1,1 );
	m_vDirMul					= D3DXVECTOR3( 1,1,1 );
	m_vWindVel					= D3DXVECTOR3( -2.0f,10.0f,0 );
	m_vGravity					= D3DXVECTOR3( 0,-9.8f,0.0f );

	m_vFlashAttenuation			= D3DXVECTOR4( 0,0.0f,3.0f,0 );
	m_vMeshLightAttenuation		= D3DXVECTOR4( 0,0,1.5f,0 );
	m_fFlashLife				= 0.5f;
	m_fFlashIntensity			= 1000.0f;

	m_fGroundPlane				= 0.5f;
	m_fLightRaise				= 1.0f;

	m_fWorldBounds				= 100.0f;

	m_NumParticlesToDraw		= 0;


	m_pParticleTextureSRV			= NULL;

	m_pParticleBuffer				= NULL;

	m_pEffect10						= NULL;
	m_pVertexLayout					= NULL;
	m_pRenderParticles				= NULL;

	m_ptxDiffuse					= NULL;
	m_pLightDir						= NULL;
	m_pmWorldViewProjection			= NULL;
	m_pmWorld						= NULL;
	m_pmInvViewProj					= NULL;
	m_pfTime						= NULL;
	m_pvEyePt						= NULL;
	m_pvRight						= NULL;
	m_pvUp							= NULL;
	m_pvForward						= NULL;

	m_pNumGlowLights				= NULL;
	m_pvGlowLightPosIntensity		= NULL;
	m_pvGlowLightColor				= NULL;
	m_pvGlowLightAttenuation		= NULL;


	m_vFlashColor[0] = D3DXVECTOR4( 1.0f, 0.5f, 0.00f, 0.9f );
	m_vFlashColor[1] = D3DXVECTOR4( 1.0f, 0.3f, 0.05f, 0.9f );
	m_vFlashColor[2] = D3DXVECTOR4( 1.0f, 0.4f, 0.00f, 0.9f );
	m_vFlashColor[3] = D3DXVECTOR4( 0.8f, 0.3f, 0.05f, 0.9f );
}

void CExplosionMgr::Update( CameraInfo& camera, double fTime, float fElapsedTime )
{
	// Update the camera's position based on user input 
	if (fElapsedTime  > 0.1f ) fElapsedTime = 0.1f;

	static bool firstTime = true;

	D3DXVECTOR3 vEye;
	D3DXMATRIX mView;
	vEye = D3DXVECTOR3( camera.LookAt.x, camera.LookAt.y, camera.LookAt.z );
	mView = *(D3DXMATRIX*)&camera.View;
	D3DXVECTOR3 vRight( mView._11, mView._21, mView._31 );
	D3DXVECTOR3 vUp( mView._12, mView._22, mView._32 );
	D3DXVECTOR3 vFoward( mView._13, mView._23, mView._33 );

	D3DXVec3Normalize( &vRight, &vRight );
	D3DXVec3Normalize( &vUp, &vUp );
	D3DXVec3Normalize( &vFoward, &vFoward );

	m_pvRight->SetFloatVector( ( float* )&vRight );
	m_pvUp->SetFloatVector( ( float* )&vUp );
	m_pvForward->SetFloatVector( ( float* )&vFoward );

	UINT NumActiveSystems = 0;
	D3DXVECTOR4 vGlowLightPosIntensity[MAX_PARTICLE_SYSTEMS];
	D3DXVECTOR4 vGlowLightColor[MAX_PARTICLE_SYSTEMS];


	// Advance the system
	for( UINT i = 0; i < ( m_NumActiveBurstSystems + m_NumActiveMineSystems + ( m_NumActiveMushSystems * 2 ) ); i++ )
	{
		m_ParticleSystems[i]->AdvanceSystem( ( float )fTime, fElapsedTime, vRight, vUp, m_vWindVel, m_vGravity );
	}

	PARTICLE_VERTEX* pVerts = NULL;
	D3D11_MAPPED_SUBRESOURCE mappedData;
	DXUTGetD3D11DeviceContext()->Map( m_pParticleBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData );
	pVerts = reinterpret_cast<PARTICLE_VERTEX*>(mappedData.pData);

	CopyParticlesToVertexBuffer( pVerts, vEye, vRight, vUp );

	DXUTGetD3D11DeviceContext()->Unmap( m_pParticleBuffer, 0);

	// Multiplied by two since there is the stalk and the mushroom component of the system.
	for( UINT i = 0; i < m_NumActiveMushSystems * 2; i += 2 )
	{
		float fCurrentTime = m_ParticleSystems[i]->GetCurrentTime();
		float fLifeSpan = m_ParticleSystems[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			// This code is executed when the particle system is about to spawn. ( Not including the first time ).

			RemoveSystem( i, MushroomExplosionType );
			// This is all good since we don't want any counters to decrement.
			RemoveSystem( i, InvalidExplosionType );
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < m_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = m_ParticleSystems[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = m_ParticleSystems[i]->GetFlashColor();

			float fIntensity = m_fFlashIntensity * ( ( m_fFlashLife - fCurrentTime ) / m_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + m_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Ground bursts
	for( UINT i = ( m_NumActiveMushSystems * 2 ); i < m_NumActiveBurstSystems; i++ )
	{
		float fCurrentTime = m_ParticleSystems[i]->GetCurrentTime();
		float fLifeSpan = m_ParticleSystems[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			// This code is executed when the particle system is about to spawn. ( Not including the first time ).

			RemoveSystem( i, GroundBustExplosionType );

			//firstTime = true;
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < m_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = m_ParticleSystems[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = m_ParticleSystems[i]->GetFlashColor();

			float fIntensity = m_fFlashIntensity * ( ( m_fFlashLife - fCurrentTime ) / m_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + m_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}

	// Land mines
	for( UINT i = ( m_NumActiveBurstSystems + ( m_NumActiveMushSystems * 2 ) ); i < ( m_NumActiveBurstSystems + m_NumActiveMineSystems + ( m_NumActiveMushSystems * 2 ) ); i++ )
	{
		float fCurrentTime = m_ParticleSystems[i]->GetCurrentTime();
		float fLifeSpan = m_ParticleSystems[i]->GetLifeSpan();
		if( fCurrentTime > fLifeSpan )
		{
			// This code is executed when the particle system is about to spawn. ( Not including the first time ).

			RemoveSystem( i, MineExplosionType );

			
		}
		else if( fCurrentTime > 0.0f && fCurrentTime < m_fFlashLife && NumActiveSystems < MAX_FLASH_LIGHTS )
		{
			D3DXVECTOR3 vCenter = m_ParticleSystems[i]->GetCenter();
			D3DXVECTOR4 vFlashColor = m_ParticleSystems[i]->GetFlashColor();

			float fIntensity = m_fFlashIntensity * ( ( m_fFlashLife - fCurrentTime ) / m_fFlashLife );
			vGlowLightPosIntensity[NumActiveSystems] = D3DXVECTOR4( vCenter.x, vCenter.y + m_fLightRaise, vCenter.z,
				fIntensity );
			vGlowLightColor[NumActiveSystems] = vFlashColor;
			NumActiveSystems ++;
		}
	}


	// Setup light variables
	m_pNumGlowLights->SetInt( NumActiveSystems );
	m_pvGlowLightPosIntensity->SetFloatVectorArray( ( float* )&vGlowLightPosIntensity, 0, NumActiveSystems );
	m_pvGlowLightColor->SetFloatVectorArray( ( float* )&vGlowLightColor, 0, NumActiveSystems );
	m_pvGlowLightAttenuation->SetFloatVector( ( float* )&m_vFlashAttenuation );
}

CExplosionMgr::~CExplosionMgr(void)
{
}

void CExplosionMgr::CreateShader( ID3D11Device* device )
{
	HRESULT hr;
	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* errorMessage = 0;
	DWORD shaderFlags = 0;

	// Read the D3DX effect file
	WCHAR str[MAX_PATH];
	char strMaxGlowLights[MAX_PATH];
	char strMaxInstances[MAX_PATH];
	sprintf_s( strMaxGlowLights, MAX_PATH, "%d", MAX_FLASH_LIGHTS );
	sprintf_s( strMaxInstances, MAX_PATH, "%d", MAX_INSTANCES );
	D3D10_SHADER_MACRO macros[3] =
	{
		{ "MAX_GLOWLIGHTS", strMaxGlowLights },
		{ "MAX_INSTANCES", strMaxInstances },
		{ NULL, NULL }
	};

#if defined (_DEBUG) || defined (DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	hr = D3DX11CompileFromFile( L"ExplosionParticles.fx", macros, 0, 0, "fx_5_0", shaderFlags, 0, 0, &compiledShader, &errorMessage, 0 );
	if(FAILED(hr))
	{
		if(errorMessage != 0)
		{
			MessageBoxA(0,(char*)errorMessage->GetBufferPointer(),"Fatal Error",MB_OK);
			ReleaseCOM(errorMessage);
		}
	}

	hr = D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(),compiledShader->GetBufferSize(),
		0,device,&m_pEffect10);
	if(FAILED(hr))
	{
		assert(L"We couldn't create the effect!");
	}

	ReleaseCOM(compiledShader);
	ReleaseCOM(errorMessage);
}

HRESULT CExplosionMgr::Initialize( ID3D11Device* device )
{
	HRESULT hr;


	CreateShader( device );

	// Obtain technique objects
	m_pRenderParticles = m_pEffect10->GetTechniqueByName( "RenderParticles" );

	// Obtain variables
	m_ptxDiffuse = m_pEffect10->GetVariableByName( "g_txMeshTexture" )->AsShaderResource();
	m_pLightDir = m_pEffect10->GetVariableByName( "g_LightDir" )->AsVector();
	m_pmWorldViewProjection = m_pEffect10->GetVariableByName( "g_mWorldViewProjection" )->AsMatrix();
	m_pmWorld = m_pEffect10->GetVariableByName( "g_mWorld" )->AsMatrix();
	m_pmInvViewProj = m_pEffect10->GetVariableByName( "g_mInvViewProj" )->AsMatrix();
	m_pfTime = m_pEffect10->GetVariableByName( "g_fTime" )->AsScalar();
	m_pvEyePt = m_pEffect10->GetVariableByName( "g_vEyePt" )->AsVector();
	m_pvRight = m_pEffect10->GetVariableByName( "g_vRight" )->AsVector();
	m_pvUp = m_pEffect10->GetVariableByName( "g_vUp" )->AsVector();
	m_pvForward = m_pEffect10->GetVariableByName( "g_vForward" )->AsVector();

	m_pNumGlowLights = m_pEffect10->GetVariableByName( "g_NumGlowLights" )->AsScalar();
	m_pvGlowLightPosIntensity = m_pEffect10->GetVariableByName( "g_vGlowLightPosIntensity" )->AsVector();
	m_pvGlowLightColor = m_pEffect10->GetVariableByName( "g_vGlowLightColor" )->AsVector();
	m_pvGlowLightAttenuation = m_pEffect10->GetVariableByName( "g_vGlowLightAttenuation" )->AsVector();

	// Create our vertex input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "LIFE",      0, DXGI_FORMAT_R32_FLOAT,       0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "THETA",     0, DXGI_FORMAT_R32_FLOAT,       0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3DX11_PASS_DESC PassDesc;
	V_RETURN( m_pRenderParticles->GetPassByIndex( 0 )->GetDesc( &PassDesc ) );
	V_RETURN( device->CreateInputLayout( layout, 5, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &m_pVertexLayout ) );

	// Particle system
	UINT MaxParticles = MAX_MUSHROOM_CLOUDS * ( m_NumParticles + m_NumStalkParticles ) +
		( MAX_GROUND_BURSTS - MAX_MUSHROOM_CLOUDS ) * m_NumGroundExpParticles +
		( MAX_PARTICLE_SYSTEMS - MAX_GROUND_BURSTS ) * m_NumLandMineParticles;
	V_RETURN( CreateParticleArray( MaxParticles ) );


	srand( timeGetTime() );

	m_NumParticlesToDraw = 0;

	D3D11_BUFFER_DESC BDesc;
	BDesc.ByteWidth = sizeof( PARTICLE_VERTEX ) * 6 * MAX_NUM_PARTICLES;
	BDesc.Usage = D3D11_USAGE_DYNAMIC;
	BDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	BDesc.MiscFlags = 0;
	V_RETURN( device->CreateBuffer( &BDesc, NULL, &m_pParticleBuffer ) );

	V_RETURN( D3DX11CreateShaderResourceViewFromFileA( device, "data/Particles/ExplosionParticle.dds", NULL, NULL, &m_pParticleTextureSRV, NULL ) );


	return S_OK;
}

void CExplosionMgr::RenderParticles( ID3D11DeviceContext* context )
{
	//IA setup
	context->IASetInputLayout( m_pVertexLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = m_pParticleBuffer;
	Strides[0] = sizeof( PARTICLE_VERTEX );
	Offsets[0] = 0;
	context->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	context->IASetIndexBuffer( NULL, DXGI_FORMAT_R16_UINT, 0 );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	m_ptxDiffuse->SetResource( m_pParticleTextureSRV );

	//Render
	D3DX11_TECHNIQUE_DESC techDesc;
	m_pRenderParticles->GetDesc( &techDesc );

	m_NumParticlesToDraw = GetNumActiveParticles();
	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		m_pRenderParticles->GetPassByIndex( p )->Apply( 0, context );
		context->Draw( 6 * m_NumParticlesToDraw, 0 );
	}
}

void CExplosionMgr::Render( ID3D11DeviceContext* context, double fTime, float fElapsedTime, CameraInfo& camera, D3DXVECTOR3 lightDir )
{
	// Ensure that there is actually a system to render.
	if ( m_NumActiveMushSystems == 0 && m_NumActiveMineSystems == 0 && m_NumActiveBurstSystems == 0 )
		return;
	HRESULT hr;

	D3DXVECTOR3 vEyePt;
	D3DXMATRIX mWorldViewProjection;
	D3DXVECTOR4 vLightDir;
	D3DXMATRIX mWorld;
	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mViewProj;
	D3DXMATRIX mInvViewProj;

	// Get the projection & view matrix from the camera class
	D3DXMatrixIdentity( &mWorld );
	vEyePt = D3DXVECTOR3( camera.LookAt.x, camera.LookAt.y, camera.LookAt.z );
	mProj = *(D3DXMATRIX*)&camera.Proj;
	mView = *(D3DXMATRIX*)&camera.View;

	mWorldViewProjection = mView * mProj;
	mViewProj = mView * mProj;
	D3DXMatrixInverse( &mInvViewProj, NULL, &mViewProj );
	D3DXMATRIX mSceneWorld;
	D3DXMatrixScaling( &mSceneWorld, 20, 20, 20 );
	D3DXMATRIX mSceneWVP = mSceneWorld * mViewProj;
	vLightDir = D3DXVECTOR4( lightDir, 1 );

	// Per frame variables
	V( m_pmWorldViewProjection->SetMatrix( ( float* )&mSceneWVP ) );
	V( m_pmWorld->SetMatrix( ( float* )&mSceneWorld ) );
	V( m_pLightDir->SetFloatVector( ( float* )vLightDir ) );
	V( m_pmInvViewProj->SetMatrix( ( float* )&mInvViewProj ) );
	V( m_pfTime->SetFloat( ( float )fTime ) );
	V( m_pvEyePt->SetFloatVector( ( float* )&vEyePt ) );


	V( m_pmWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection ) );
	V( m_pmWorld->SetMatrix( ( float* )&mWorld ) );

	RenderParticles( context );
}

void CExplosionMgr::OnDeviceDestroyed()
{
	SAFE_RELEASE( m_pEffect10 );
	SAFE_RELEASE( m_pVertexLayout );

	for( UINT i = 0; i < m_ParticleSystems.size(); i++ )
	{
		SAFE_DELETE( m_ParticleSystems[i] );
	}
	SAFE_RELEASE( m_pParticleBuffer );
	SAFE_RELEASE( m_pParticleTextureSRV );

	DestroyParticleArray();
}

void CExplosionMgr::AddMushroomExplosion( D3DXVECTOR3 position, int numberOfStalkParticles, int numberOfMushParticles, ExplosionParameters ep )
{
	D3DXVECTOR4 vColor0( 1.0f,1.0f,1.0f,1 );
	D3DXVECTOR4 vColor1( 0.6f,0.6f,0.6f,1 );

	CExplosionParticleSystem* system;
	system = new CMushroomParticleSystem();
	float fStartTime = 0.0f;
	D3DXVECTOR4 vFlashColor = m_vFlashColor[ rand() % MAX_FLASH_COLORS ];

	
	if ( ep.StartTime == NO_PARAMETERS )
	{
		system->SetCenter( position );
		system->SetStartTime( fStartTime );
		system->SetFlashColor( vFlashColor );
		system->CreateParticleSystem( numberOfMushParticles );
		system->SetSystemAttributes( position,
			m_fSpread, m_fMushroomCloudLifeSpan, m_fFadeExponent,
			m_fStartSize, m_fEndSize, m_fSizeExponent,
			m_fMushroomStartSpeed, m_fEndSpeed, m_fSpeedExponent,
			m_fRollAmount, m_fWindFalloff,
			1, 0, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ),
			vColor0, vColor1,
			m_vPosMul, m_vDirMul );
	}
	else
	{
		system->SetCenter( position );
		system->SetStartTime( ep.StartTime );
		system->SetFlashColor( ep.FlashColor );
		system->CreateParticleSystem( numberOfMushParticles );
		system->SetSystemAttributes( position, ep.Spread, ep.LifeSpan,
			ep.FadeExponent, ep.StartSize, ep.EndSize, ep.SizeExponent, ep.StartSpeed,
			ep.EndSpeed, ep.SpeedExponent, ep.RollAmount, ep.WindFalloff, ep.NumStreamers,
			ep.SpeedVariance, ep.Direction, ep.DirectionVariance, ep.Color0, ep.Color1, ep.PosMul, ep.DirMul );
	}
	

	m_ParticleSystems.push_back( system );

	m_NumParticlesToDraw += numberOfMushParticles;

	CExplosionParticleSystem* stalkSystem;
	stalkSystem = new CStalkParticleSystem();

	if ( ep.StartTime == NO_PARAMETERS )
	{
		stalkSystem->SetCenter( position );
		stalkSystem->SetStartTime( fStartTime );
		stalkSystem->SetFlashColor( vFlashColor );
		stalkSystem->CreateParticleSystem( numberOfStalkParticles );
		stalkSystem->SetSystemAttributes( position,
			15.0f, m_fMushroomCloudLifeSpan, m_fFadeExponent * 2.0f,
			m_fStartSize * 0.5f, m_fEndSize * 0.5f, m_fSizeExponent,
			m_fStalkStartSpeed, -1.0f, m_fSpeedExponent,
			m_fRollAmount, m_fWindFalloff,
			1, 0, D3DXVECTOR3( 0, 0, 0 ), D3DXVECTOR3( 0, 0, 0 ),
			vColor0, vColor1,
			D3DXVECTOR3( 1, 0.1f, 1 ), D3DXVECTOR3( 1, 0.1f, 1 ) );
	}
	else
	{
		stalkSystem->SetCenter( position );
		stalkSystem->SetStartTime( ep.StartTime );
		stalkSystem->SetFlashColor( ep.FlashColor );
		stalkSystem->CreateParticleSystem( m_NumParticles );
		stalkSystem->SetSystemAttributes( position, ep.Spread, ep.LifeSpan,
			ep.FadeExponent, ep.StartSize, ep.EndSize, ep.SizeExponent, ep.StartSpeed,
			ep.EndSpeed, ep.SpeedExponent, ep.RollAmount, ep.WindFalloff, ep.NumStreamers,
			ep.SpeedVariance, ep.Direction, ep.DirectionVariance, ep.Color0, ep.Color1, ep.PosMul, ep.DirMul );
	}

	

	m_ParticleSystems.push_back( stalkSystem );

	m_NumParticlesToDraw += numberOfStalkParticles;

	// The stalk and mush components are accounted for later.
	m_NumActiveMushSystems++;
}

void CExplosionMgr::AddGroundBurstExplosion( D3DXVECTOR3 position, int numberOfGroundBurstParticles, ExplosionParameters ep )
{
	D3DXVECTOR4 vColor0( 1.0f,1.0f,1.0f,1 );
	D3DXVECTOR4 vColor1( 0.6f,0.6f,0.6f,1 );

	CExplosionParticleSystem* system;

	system = new CGroundBurstParticleSystem();

	float fStartTime = 0.0f;
	D3DXVECTOR4 vFlashColor = m_vFlashColor[ rand() % MAX_FLASH_COLORS ];

	float fStartSpeed = m_fGroundBurstStartSpeed + RPercent() * 30.0f;

	if ( ep.StartTime == NO_PARAMETERS )
	{
		system->SetCenter( position );
		system->SetStartTime( fStartTime );
		system->SetStartSpeed( fStartSpeed );
		system->SetFlashColor( vFlashColor );
		system->CreateParticleSystem( numberOfGroundBurstParticles );
		system->SetSystemAttributes( position,
			1.0f, m_fGroundBurstLifeSpan, m_fFadeExponent,
			0.5f, 8.0f, 1.0f,
			m_fGroundBurstStartSpeed, m_fEndSpeed, 4.0f,
			m_fRollAmount, 1.0f,
			30, 100.0f, D3DXVECTOR3( 0, 0.5f, 0 ), D3DXVECTOR3( 1.0f, 0.5f,
			1.0f ),
			vColor0, vColor1,
			m_vPosMul, m_vDirMul );
	}
	else
	{
		system->SetCenter( position );
		system->SetStartTime( ep.StartTime );
		system->SetStartSpeed( ep.StartSpeed );
		system->SetFlashColor( ep.FlashColor );
		system->CreateParticleSystem( m_NumParticles );
		system->SetSystemAttributes( position, ep.Spread, ep.LifeSpan,
			ep.FadeExponent, ep.StartSize, ep.EndSize, ep.SizeExponent, ep.StartSpeed,
			ep.EndSpeed, ep.SpeedExponent, ep.RollAmount, ep.WindFalloff, ep.NumStreamers,
			ep.SpeedVariance, ep.Direction, ep.DirectionVariance, ep.Color0, ep.Color1, ep.PosMul, ep.DirMul );
	}
	

	m_ParticleSystems.push_back( system );

	m_NumParticlesToDraw += numberOfGroundBurstParticles;

	m_NumActiveBurstSystems++;
}

void CExplosionMgr::AddMineExplosion( D3DXVECTOR3 position, int numberOfMineParticles, ExplosionParameters ep )
{
	D3DXVECTOR4 vColor0( 1.0f,1.0f,1.0f,1 );
	D3DXVECTOR4 vColor1( 0.6f,0.6f,0.6f,1 );

	CExplosionParticleSystem* system;

	system = new CLandMineParticleSystem();
	float fStartTime = 0.0f;
	D3DXVECTOR4 vFlashColor = m_vFlashColor[ rand() % MAX_FLASH_COLORS ];

	float fStartSpeed = m_fLandMineStartSpeed + RPercent() * 100.0f;

	if ( ep.StartTime == NO_PARAMETERS )
	{
		system->SetCenter( position );
		system->SetStartTime( fStartTime );
		system->SetStartSpeed( fStartSpeed );
		system->SetFlashColor( vFlashColor );
		system->CreateParticleSystem( numberOfMineParticles );
		system->SetSystemAttributes( position,
			1.5f, m_fPopperLifeSpan, m_fFadeExponent,
			1.0f, 6.0f, 1.0f,
			m_fLandMineStartSpeed, m_fEndSpeed, 2.0f,
			m_fRollAmount, 4.0f,
			0, 70.0f, D3DXVECTOR3( 0, 0.8f, 0 ), D3DXVECTOR3( 0.3f, 0.2f,
			0.3f ),
			vColor0, vColor1,
			m_vPosMul, m_vDirMul );

	}
	else
	{
		system->SetCenter( position );
		system->SetStartTime( ep.StartTime );
		system->SetStartSpeed( ep.StartSpeed );
		system->SetFlashColor( ep.FlashColor );
		system->CreateParticleSystem( numberOfMineParticles );
		system->SetSystemAttributes( position, ep.Spread, ep.LifeSpan,
			ep.FadeExponent, ep.StartSize, ep.EndSize, ep.SizeExponent, ep.StartSpeed,
			ep.EndSpeed, ep.SpeedExponent, ep.RollAmount, ep.WindFalloff, ep.NumStreamers,
			ep.SpeedVariance, ep.Direction, ep.DirectionVariance, ep.Color0, ep.Color1, ep.PosMul, ep.DirMul );
	}

	m_ParticleSystems.push_back( system );

	m_NumParticlesToDraw += numberOfMineParticles;

	m_NumActiveMineSystems++;
}

void CExplosionMgr::RemoveSystem( int index, ExplosionType type )
{
	SafeDelete( m_ParticleSystems.at(index) );
	std::vector<CExplosionParticleSystem*>::iterator beginErase;
	beginErase = m_ParticleSystems.begin() + index;
	m_ParticleSystems.erase( beginErase, beginErase + 1 );

	// Don't add any error checking for this as we an error type is valid input for type if we don't want any decrementing.
	if ( type == GroundBustExplosionType )
	{
		m_NumActiveBurstSystems--;
		SetNumActiveParticles( GetNumActiveParticles() - m_NumGroundExpParticles );
		m_NumParticlesToDraw -= m_NumGroundExpParticles;
	}
	else if ( type == MineExplosionType )
	{
		m_NumActiveMineSystems--;
		SetNumActiveParticles( GetNumActiveParticles() - m_NumLandMineParticles );
		m_NumParticlesToDraw -= m_NumLandMineParticles;
	}
	else if ( type == MushroomExplosionType )
	{
		m_NumActiveMushSystems--;
		// Subtract for both the stalk and the mush.
		SetNumActiveParticles( GetNumActiveParticles() - m_NumMushParticles - m_NumStalkParticles );
		m_NumParticlesToDraw -= m_NumStalkParticles;
		m_NumParticlesToDraw -= m_NumMushParticles;

	}
}

void CExplosionMgr::AddExplosion( ExplosionType type, Vec position )
{
	ExplosionParameters ep;
	ep.StartTime = NO_PARAMETERS;

	AddExplosion( type, position, ep );
}

void CExplosionMgr::AddExplosion( ExplosionType type, Vec position, ExplosionParameters ep )
{
	switch( type )
	{
	case GroundBustExplosionType:
		AddGroundBurstExplosion( D3DXVECTOR3( position.X(), position.Y(), position.Z() ), m_NumGroundExpParticles, ep );
		break;
	case MineExplosionType:
		AddMineExplosion( D3DXVECTOR3( position.X(), position.Y(), position.Z() ), m_NumLandMineParticles, ep );
		break;
	case MushroomExplosionType:
		AddMushroomExplosion( D3DXVECTOR3( position.X(), position.Y(), position.Z() ), m_NumStalkParticles, m_NumMushParticles, ep );
		break;
	case InvalidExplosionType:
	default:
		DebugWriter::WriteToDebugFile(L"Invalid explosion type!");
	}
}


