//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#ifndef PARTICLE_SYSTEM
#define PARTICLE_SYSTEM

#include <d3d11.h>
#include <d3dx11.h>
#include <D3DX10math.h>
#include <D3Dcompiler.h>
#include "EffectConstants.h"
#include "EffectManager.h"
#include "Camera.h"

//////////////////////////
// Forward declarations:
struct ParticleSystemInfo;

class ParticleSystem
{

private:
	struct ParticleVert
	{
		XMFLOAT3 InitialPos;
		XMFLOAT3 InitialVel;
		XMFLOAT2 Size;
		float Age;
		unsigned int Type;
	};

public:
	
	ParticleSystem();
	virtual ~ParticleSystem();

	// Time elapsed since the system was reset.
	float GetAge()const;

	void DestroyResources();

	void SetEyePos(const XMFLOAT3& eyePosW);
	void SetEmitPos(const XMFLOAT3& emitPosW);
	void SetEmitDir(const XMFLOAT3& emitDirW);

	void Pause();
	void Resume();

	void SetDlgInfo( ParticleSystemData psd );
	void SetInitInfo( ParticleSystemInfo psi );
	ParticleSystemData GetDlgInfo();

	void Init( ID3D11Device* device, CParticleEffect* fx, 
		ID3D11ShaderResourceView* texArraySRV, 
		ID3D11ShaderResourceView* randomTexSRV, 
		UINT maxParticles );

	void Reset();
	void Update( float dt, float gameTime );
	void Draw( ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager );

	void SetEmitInfo( XMFLOAT3 emitDir, XMFLOAT3 particleAccel ); 

	void Serilize(CDataArchiver* archiver);

	void MoveLeft( const float speed );
	void MoveRight( const float speed );
	void MoveUp( const float speed );
	void MoveDown( const float speed );
	void MoveForward( const float speed );
	void MoveBack( const float speed );


private:
	void BuildVB(ID3D11Device* device);

	ParticleSystem(const ParticleSystem& rhs);
	ParticleSystem& operator=(const ParticleSystem& rhs);

private:
	// Clears all variables and 100% restarts the particle system.
	void CompleteRestart(ParticleSystemData psi);
 
protected:
 
	UINT mMaxParticles;
	bool mFirstRun;

	float mGameTime;
	float mTimeStep;
	float mAge;

	XMFLOAT3 mEyePosW;
	XMFLOAT3 mEmitPosW;
	XMFLOAT3 mEmitDirW;
	XMFLOAT3 mParticleAccel;

	float mTextureSize;
	float mInitVelocity;
	float mRandomXMod;
	float mRandomZMod;
	float m_fEmitTime;

	bool m_bPaused;

	CParticleEffect* mFX;

	ID3D11Buffer* mInitVB;	
	ID3D11Buffer* mDrawVB;
	ID3D11Buffer* mStreamOutVB;
 
	ID3D11ShaderResourceView* mTexArraySRV;
	ID3D11ShaderResourceView* mRandomTexSRV;
};

#endif