//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "IObject.h"
#include "ExplosionParticleEffect.h"
#include "Effects.h"


#define MAX_FLASH_COLORS 4


#define MAX_MUSHROOM_CLOUDS 8
#define MAX_GROUND_BURSTS 15
#define MAX_PARTICLE_SYSTEMS 23
#define MAX_FLASH_LIGHTS 8
#define MAX_INSTANCES 200

#define NO_PARAMETERS -69

enum ExplosionType { MushroomExplosionType, GroundBustExplosionType, MineExplosionType, InvalidExplosionType };

struct ExplosionParameters
{
	ExplosionParameters()
	{
		StartTime = NO_PARAMETERS;
	}

	float StartTime;
	float StartSpeed;
	D3DXVECTOR4 FlashColor;
	float Spread;
	float LifeSpan;
	float FadeExponent;
	float StartSize;
	float EndSize;
	float SizeExponent;
	float EndSpeed;
	float SpeedExponent;
	float RollAmount;
	float WindFalloff;
	UINT NumStreamers;
	float SpeedVariance;
	D3DXVECTOR3 Direction;
	D3DXVECTOR3 DirectionVariance;
	D3DXVECTOR4 Color0;
	D3DXVECTOR4 Color1;
	D3DXVECTOR3 PosMul;
	D3DXVECTOR3 DirMul;
};

class CExplosionMgr
{
public:
	CExplosionMgr(void);
	~CExplosionMgr(void);

	void Update( CameraInfo& camera, double fTime, float fElapsedTime );
	HRESULT Initialize( ID3D11Device* device );

	void Render( ID3D11DeviceContext* context, double fTime, float fElapsedTime, CameraInfo& camera, D3DXVECTOR3 lightDir );

	void OnDeviceDestroyed();

	void AddExplosion( ExplosionType type, Vec position );
	void AddExplosion( ExplosionType type, Vec position, ExplosionParameters ep );

private:
	void RenderParticles( ID3D11DeviceContext* context );
	void CreateShader(ID3D11Device* device);

	void AddMushroomExplosion( D3DXVECTOR3 position, int numberOfStalkParticles, int numberOfMushParticles, ExplosionParameters ep );
	void AddGroundBurstExplosion( D3DXVECTOR3 position, int numberOfGroundBurstParticles, ExplosionParameters ep );
	void AddMineExplosion( D3DXVECTOR3 position, int numberOfMineParticles, ExplosionParameters ep );

	void RemoveSystem( int index, ExplosionType type );

private:
	UINT								m_NumActiveMushSystems;
	UINT								m_NumActiveBurstSystems;
	UINT								m_NumActiveMineSystems;

	const UINT							m_NumStalkParticles;
	const UINT							m_NumGroundExpParticles;
	const UINT							m_NumLandMineParticles;
	const UINT							m_NumMushParticles;

	UINT                                m_NumParticles;
	float                               m_fSpread;
	float                               m_fStartSize;
	float                               m_fEndSize;
	float                               m_fSizeExponent;

	float                               m_fMushroomCloudLifeSpan;
	float                               m_fGroundBurstLifeSpan;
	float                               m_fPopperLifeSpan;


	float                               m_fMushroomStartSpeed;
	float                               m_fStalkStartSpeed;
	float                               m_fGroundBurstStartSpeed;
	float                               m_fLandMineStartSpeed;

	float                               m_fEndSpeed;
	float                               m_fSpeedExponent;
	float                               m_fFadeExponent;
	float                               m_fRollAmount;
	float                               m_fWindFalloff;
	D3DXVECTOR3                         m_vPosMul;
	D3DXVECTOR3                         m_vDirMul;
	D3DXVECTOR3                         m_vWindVel;
	D3DXVECTOR3                         m_vGravity;

	D3DXVECTOR4                         m_vFlashAttenuation;
	D3DXVECTOR4                         m_vMeshLightAttenuation;
	float                               m_fFlashLife;
	float                               m_fFlashIntensity;

	float                               m_fGroundPlane;
	float                               m_fLightRaise;

	float                               m_fWorldBounds;

	UINT                                m_NumParticlesToDraw;

	// Effect stuff.
	ID3D11ShaderResourceView*           m_pParticleTextureSRV;

	ID3D11InputLayout*                  m_pVertexLayout;

	std::vector<CExplosionParticleSystem*>          m_ParticleSystems;

	ID3D11Buffer*                       m_pParticleBuffer;

	ID3DX11Effect*                       m_pEffect10;
	ID3DX11EffectTechnique*              m_pRenderParticles;

	ID3DX11EffectShaderResourceVariable* m_ptxDiffuse;
	ID3DX11EffectVectorVariable*         m_pLightDir;
	ID3DX11EffectMatrixVariable*         m_pmWorldViewProjection;
	ID3DX11EffectMatrixVariable*         m_pmWorld;
	ID3DX11EffectMatrixVariable*         m_pmInvViewProj;
	ID3DX11EffectScalarVariable*         m_pfTime;
	ID3DX11EffectVectorVariable*         m_pvEyePt;
	ID3DX11EffectVectorVariable*         m_pvRight;
	ID3DX11EffectVectorVariable*         m_pvUp;
	ID3DX11EffectVectorVariable*         m_pvForward;

	ID3DX11EffectScalarVariable*         m_pNumGlowLights;
	ID3DX11EffectVectorVariable*         m_pvGlowLightPosIntensity;
	ID3DX11EffectVectorVariable*         m_pvGlowLightColor;
	ID3DX11EffectVectorVariable*         m_pvGlowLightAttenuation;

	D3DXVECTOR4							m_vFlashColor[MAX_FLASH_COLORS];
};

