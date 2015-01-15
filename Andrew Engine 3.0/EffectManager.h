#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Effects.h"
#include "MathHelper.h"
#include "IObject.h"

enum InputLayoutType { Pos, PosNormTex, PosColor, PosNormTexTan, TerrainIL, TerrainShadowIL, ParticleIL };

struct CBasicEffectRenderInfo
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX proj;
	XMMATRIX texTransform;
	XMMATRIX shadowTransform;
	XMFLOAT3 eyePosW;
	XMFLOAT4 fogColor;
	float fogStart;
	float fogRange;

	ID3D11ShaderResourceView* shadowSRV;
	ID3D11ShaderResourceView* cubeMapSRV;
};

struct NormalMappingExtraInfo
{
	NormalMappingExtraInfo(float heightScale,float maxTessD,float minTessD,float minTessF,float maxTessF)
	{
		mHeightScale = heightScale;
		mMaxTessD = maxTessD;
		mMinTessD = minTessD;
		mMaxTessF = maxTessF;
		mMinTessF = minTessF;
	}
	float mHeightScale;
	float mMaxTessD;
	float mMinTessD;
	float mMaxTessF;
	float mMinTessF;
};

struct CParticleInfo
{
	D3DXMATRIX mWorld;
	D3DXMATRIX mWorldView;
	D3DXMATRIX mWorldViewProj;
	D3DXMATRIX mInvView;
	D3DXMATRIX mInvProj;
	D3DXVECTOR3 ViewDir;
	D3DXVECTOR3 EyePos;
	float ScreenSize[2];
	float FadeDistance;
	D3DXVECTOR4 OctaveOffsets[4];
	// This is 1.0f / ( farPlane - nearPlane);
	float SizeZScale;
};

struct ScenePerFrameInfo
{
	ID3D11ShaderResourceView* pCubeMap;
	float FogStart;
	float FogRange;
	XMFLOAT4 FogColor;
};


struct BlurEffectAccessor
{
	BlurEffectAccessor()
	{

	}
	~BlurEffectAccessor()
	{

	}
	bool Init(ID3D11Device* device)
	{
		mBlurFX = new CBlurEffect(device,L"Blur.fx");
		assert(mBlurFX);

		return true;
	}
	bool SetWeights(float weights[9])
	{
		HRESULT hr = mBlurFX->Weights->SetFloatArray(weights,0,9);
		Checkhr(hr,"Couldn't set the weights float array");
		return true;
	}
	bool SetInputMap(ID3D11ShaderResourceView* tex)
	{
		HRESULT hr = mBlurFX->InputMap->SetResource(tex);
		Checkhr(hr,"Couldn't set the input texture!");
		return true;
	}
	bool SetOutputMap(ID3D11UnorderedAccessView* tex)
	{
		HRESULT hr = mBlurFX->OutputMap->SetUnorderedAccessView(tex);
		Checkhr(hr,"Couldn't set the output texure!");
		return true;
	}
	D3DX11_PASS_DESC GetBlurFXPasssDesc()
	{
		D3DX11_PASS_DESC passDesc;
		mBlurFX->HorzBlurTech->GetPassByIndex(0)->GetDesc(&passDesc);
		return passDesc;
	}
	ID3DX11EffectTechnique* GetTechnique(bool horzBluring)
	{
		if(horzBluring)
			return mBlurFX->HorzBlurTech;
		return mBlurFX->VertBlurTech;
	}
	void Shutdown()
	{
		SafeDelete(mBlurFX);
	}

private:
	CBlurEffect* mBlurFX;
};


class CEffectManager
{
public:
	CEffectManager();
	~CEffectManager();

	bool Init();

	bool InitResources(ID3D11Device* device);

	void Shutdown();
	void DestroyResources();

	void SetPerFrame( ScenePerFrameInfo& spfi, std::vector<ID3D11ShaderResourceView*>& shadowMaps, CSkinEffect* skinEffect );

	bool SetBasicFX(const CBasicEffectRenderInfo& info,ID3D11DeviceContext* context)const;
	bool SetBasicFXDiffuseMap(ID3D11ShaderResourceView* diffuseSRV)const;
	bool SetBasicFXHasTexture(bool hasTexture)const;
	bool SetBasicFXMaterial(Material mat)const;

	bool SetAllLighting(const CSceneLighting lightingInfo,CSkinEffect* skinEffect)const;

	bool SetNormalMappingFX(const CBasicEffectRenderInfo& info,NormalMappingExtraInfo& dpmExtra,ID3D11DeviceContext* context)const;
	bool SetNormalMappingDiffuseMap(ID3D11ShaderResourceView* diffuseSRV)const;
	bool SetNormalMappingNormalMap(ID3D11ShaderResourceView* nMapSRV)const;
	bool SetNormalMappingMaterial(Material mat)const;
	bool SetNormalMappingHasTexture(bool hasTexture)const;

	//TODO:
	// Cache the current state to prevent redundant state changing calls.
	void SetInputLayout(InputLayoutType ilt)const;

	D3DX11_PASS_DESC GetBasicFXPassDesc();
	D3DX11_PASS_DESC GetNormalMapFXPassDesc();
	D3DX11_PASS_DESC GetParticleFXPassDesc();

	ID3DX11EffectTechnique* GetTechnique(RenderTech renderTech,bool usesNormalMap,bool renderShadows,bool reflection,bool fog)const;
	ID3DX11EffectTechnique* GetParticleTechnqiue(EffectConstants::ParticleRenderTech renderTech);
	ID3DX11EffectTechnique*	GetScreenQuadTechnique() { return mBasicFX->NoLightTex; }

	BlurEffectAccessor BlurEffectAccess;

	CNormalMapEffect* mNormalMapFX;
	CBuildShadowMapEffect* mShadowFX;
	CBasicEffect* mBasicFX;
	CSkyEffect* mSkyFX;
	CRefractionEffect* mRefractFX;
	CWaterEffect* mWaterFX;
	CColorEffect* mColorFX;
	CTerrainEffect* mTerrainFX;

	CParticleEffect* mFireFX;
	CParticleEffect* mSparkFX;

private:
	ID3D11InputLayout* mPosColor;

	ID3D11InputLayout* mPos;
	ID3D11InputLayout* mPosNorm;
	ID3D11InputLayout* mPosNormTex;
	ID3D11InputLayout* mPosNormTexTan;
	ID3D11InputLayout* mParticleInputLayout;
	ID3D11InputLayout* mTerrainInputLayout;
	ID3D11InputLayout* mTerrainShadowInputLayout;
	

public:
	bool SetDrawFullScreenQuad(ID3D11ShaderResourceView* appliedSRV);
};