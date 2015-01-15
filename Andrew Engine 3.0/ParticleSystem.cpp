//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "ParticleSystem.h"
#include "Models.h"
#include "EventManager.h"
#include "ParticleManager.h"

ParticleSystem::ParticleSystem()
: mInitVB(0), mDrawVB(0), mStreamOutVB(0), mTexArraySRV(0), mRandomTexSRV(0)
{
	mFirstRun = true;
	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge      = 0.0f;

	m_bPaused = false;

	mEyePosW  = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitPosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	mEmitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);

	mTextureSize = 0.5f;
	mInitVelocity = 4.0f;
	mRandomXMod = 0.5f;
	mRandomZMod = 0.5f;
}

void ParticleSystem::Pause()
{
	m_bPaused = true;
}

void ParticleSystem::Resume()
{
	m_bPaused = false;
}


void ParticleSystem::DestroyResources()
{
	ReleaseCOM(mInitVB);
	ReleaseCOM(mDrawVB);
	ReleaseCOM(mStreamOutVB);
}

ParticleSystem::~ParticleSystem()
{
	
}

void ParticleSystem::Serilize(CDataArchiver* archiver)
{
	archiver->WriteToStream( 'p' );
	archiver->Space();
	archiver->WriteToStream( mEmitPosW.x );
	archiver->Space();
	archiver->WriteToStream( mEmitPosW.y );
	archiver->Space();
	archiver->WriteToStream( mEmitPosW.z );
	archiver->Space();

	archiver->WriteToStream( mEmitDirW.x );
	archiver->Space();
	archiver->WriteToStream( mEmitDirW.y );
	archiver->Space();
	archiver->WriteToStream( mEmitDirW.z );
	archiver->Space();

	archiver->WriteToStream( mMaxParticles );
	archiver->Space();
	archiver->WriteToStream( mTextureSize );
	archiver->Space();
	archiver->WriteToStream( mInitVelocity );
	archiver->Space();
	archiver->WriteToStream( mRandomXMod );
	archiver->Space();
	archiver->WriteToStream( mRandomZMod );
	archiver->Space();
	archiver->WriteToStream( m_fEmitTime );
	archiver->Space();
	archiver->WriteToStream( mParticleAccel );
	archiver->Space();
	archiver->WriteToStream( mFX->code );

	archiver->WriteToStream( '\n' );
}

float ParticleSystem::GetAge()const
{
	return mAge;
}

void ParticleSystem::SetEyePos(const XMFLOAT3& eyePosW)
{
	mEyePosW = eyePosW;
}

void ParticleSystem::SetEmitPos(const XMFLOAT3& emitPosW)
{
	mEmitPosW = emitPosW;
}

void ParticleSystem::SetEmitDir(const XMFLOAT3& emitDirW)
{
	mEmitDirW = emitDirW;
}

void ParticleSystem::SetInitInfo( ParticleSystemInfo psi )
{
	mMaxParticles = psi.MaxParticles;
	mTextureSize = psi.TextureSize;
	mRandomXMod = psi.RandomXMod;
	mRandomZMod = psi.RandomZMod;
	m_fEmitTime = psi.EmitTime;
	mInitVelocity = psi.Vel;
	SetEmitDir( psi.EmitDir );
	SetEmitPos( psi.Pos );
	mParticleAccel = psi.Accel;
}

void ParticleSystem::Init(ID3D11Device* device, CParticleEffect* fx, ID3D11ShaderResourceView* texArraySRV, 
	                      ID3D11ShaderResourceView* randomTexSRV, UINT maxParticles)
{
	mMaxParticles = maxParticles;

	mFX = fx;

	mTexArraySRV  = texArraySRV;
	mRandomTexSRV = randomTexSRV; 

	BuildVB(device);
}

void ParticleSystem::Reset()
{
	mFirstRun = true;
	mAge      = 0.0f;
}

void ParticleSystem::Update(float dt, float gameTime)
{
	mGameTime = gameTime;
	mTimeStep = dt;

	mAge += dt;
}

void ParticleSystem::Draw( ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager )
{
	XMMATRIX VP = cam.View * cam.Proj;

	//
	// Set constants.
	//
	mFX->SetViewProj( VP );
	mFX->SetGameTime( mGameTime );
	mFX->SetTimeStep(mTimeStep );
	mFX->SetEyePosW( mEyePosW );
	mFX->SetEmitPosW( mEmitPosW );
	mFX->SetEmitDirW( mEmitDirW );
	mFX->SetTexArray( mTexArraySRV );
	mFX->SetRandomTex( mRandomTexSRV );
	mFX->SetInitVel( mInitVelocity );
	mFX->SetTextureSize( mTextureSize );
	mFX->SetRandomXMod( mRandomXMod );
	mFX->SetRandomZMod( mRandomZMod );
	mFX->SetEmitTime( m_fEmitTime );
	mFX->SetPaused( m_bPaused );
	mFX->SetParticleAccel( mParticleAccel );

	//
	// Set IA stage.
	//
	effectManager->SetInputLayout( InputLayoutType::ParticleIL );
    dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	UINT stride = sizeof(ParticleVert);
    UINT offset = 0;

	// On the first pass, use the initialization VB.  Otherwise, use
	// the VB that contains the current particle list.
	if( mFirstRun )
		dc->IASetVertexBuffers(0, 1, &mInitVB, &stride, &offset);
	else
		dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	//
	// Draw the current particle list using stream-out only to update them.  
	// The updated vertices are streamed-out to the target VB. 
	//
	dc->SOSetTargets(1, &mStreamOutVB, &offset);

    D3DX11_TECHNIQUE_DESC techDesc;
	mFX->StreamOutTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mFX->StreamOutTech->GetPassByIndex( p )->Apply(0, dc);
        
		if( mFirstRun )
		{
			dc->Draw(1, 0);
			mFirstRun = false;
		}
		else
		{
			dc->DrawAuto();
		}
    }

	// done streaming-out--unbind the vertex buffer
	ID3D11Buffer* bufferArray[1] = {0};
	dc->SOSetTargets(1, bufferArray, &offset);

	// ping-pong the vertex buffers
	std::swap(mDrawVB, mStreamOutVB);

	//
	// Draw the updated particle system we just streamed-out. 
	//
	dc->IASetVertexBuffers(0, 1, &mDrawVB, &stride, &offset);

	mFX->DrawTech->GetDesc( &techDesc );
    for(UINT p = 0; p < techDesc.Passes; ++p)
    {
        mFX->DrawTech->GetPassByIndex( p )->Apply(0, dc);
        
		dc->DrawAuto();
    }
}

void ParticleSystem::BuildVB(ID3D11Device* device)
{
	//
	// Create the buffer to kick-off the particle system.
	//

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(ParticleVert) * 1;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	ParticleVert p;
	ZeroMemory(&p, sizeof(ParticleVert));
	p.Age  = 0.0f;
	p.Type = 0;

 
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &p;

	device->CreateBuffer(&vbd, &vinitData, &mInitVB);
	
	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(ParticleVert) * mMaxParticles;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;

    device->CreateBuffer(&vbd, 0, &mDrawVB);
	device->CreateBuffer(&vbd, 0, &mStreamOutVB);
}

void ParticleSystem::SetDlgInfo( ParticleSystemData psd )
{
	mMaxParticles = psd.MaxParticles;
	mTextureSize = psd.TextureSize;
	mRandomXMod = psd.RandomXMod;
	mRandomZMod = psd.RandomZMod;
	mInitVelocity = psd.InitVelocity;
	m_fEmitTime = psd.EmitTime;
	mParticleAccel = XMFLOAT3( psd.pax, psd.pay, psd.paz );
	SetEmitDir( XMFLOAT3( psd.dx, psd.dy, psd.dz ) );
	SetEmitPos( XMFLOAT3( psd.x, psd.y, psd.z ) );

	CompleteRestart( psd );
}

void ParticleSystem::SetEmitInfo( XMFLOAT3 dir, XMFLOAT3 particleAccel )
{
	SetEmitDir( dir );
	mParticleAccel = particleAccel;
}

ParticleSystemData ParticleSystem::GetDlgInfo()
{
	ParticleSystemData psd;

	psd.dx = mEmitDirW.x;
	psd.dy = mEmitDirW.y;
	psd.dz = mEmitDirW.z;

	psd.x = mEmitPosW.x;
	psd.y = mEmitPosW.y;
	psd.z = mEmitPosW.z;

	psd.pax = mParticleAccel.x;
	psd.pay = mParticleAccel.y;
	psd.paz = mParticleAccel.z;

	psd.InitVelocity = mInitVelocity;
	psd.MaxParticles = mMaxParticles;
	psd.RandomXMod = mRandomXMod;
	psd.RandomZMod = mRandomZMod;
	psd.TextureSize = mTextureSize;
	psd.EmitTime = m_fEmitTime;

	psd.code = mFX->code;

	return psd;
}

void ParticleSystem::CompleteRestart(ParticleSystemData psi)
{
	DestroyResources();
	mFirstRun = true;
	mGameTime = 0.0f;
	mTimeStep = 0.0f;
	mAge      = 0.0f;

	mInitVB			= 0; 
	mDrawVB			= 0; 
	mStreamOutVB	= 0;

	CParticleEffect* effect = g_GetEventManager()->GetParticleManager()->GetParticleEffect( psi.code, 0 );
	CTexture tex = g_GetEventManager()->GetParticleManager()->GetTextureOfEffect( psi.code, 0 );

	Init( DXUTGetD3D11Device(), effect, tex.GetTexture(), mRandomTexSRV, mMaxParticles );
}


void ParticleSystem::MoveLeft( const float speed )
{
	mEmitPosW.x += speed;
}

void ParticleSystem::MoveRight( const float speed )
{
	mEmitPosW.x -= speed;
}

void ParticleSystem::MoveUp( const float speed )
{
	mEmitPosW.y += speed;
}

void ParticleSystem::MoveDown( const float speed )
{
	mEmitPosW.y -= speed;
}

void ParticleSystem::MoveForward( const float speed )
{
	mEmitPosW.z += speed;
}

void ParticleSystem::MoveBack( const float speed )
{
	mEmitPosW.z -= speed;
}