#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include <d3dx11.h>
#include "d3dx11effect.h"
#include "IObject.h"
#include "LightHelper.h"
#include <xnamath.h>
#include "EffectConstants.h"

class IEffect
{
public:
	IEffect(ID3D11Device* device, const LPCWSTR filename);
	virtual ~IEffect();

private:
	IEffect(const IEffect& rhs);
	IEffect& operator=(const IEffect& rhs);

protected:
	ID3DX11Effect* mFX;
};

class CBasicEffect : public IEffect
{
public:
	CBasicEffect(ID3D11Device* device, const LPCWSTR filename);
	~CBasicEffect();

	HRESULT SetWorldViewProj(CXMMATRIX M)					{ return WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetWorld(CXMMATRIX M)							{ return World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetWorldInvTranspose(CXMMATRIX M)				{ return WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetTexTransform(CXMMATRIX M)					{ return TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetShadowTransforms(std::vector<XMFLOAT4X4> matrixList)
	{
		XMMATRIX matricies[12];
		for ( int i = 0; i < matrixList.size(); ++i )
		{
			matricies[i] = XMLoadFloat4x4( &matrixList.at( i ) );
		}
		ShadowTransform->SetMatrixArray( reinterpret_cast< const float* >(&matricies), 0, MAX_NUMBER_OF_POINT_LIGHTS );

		return S_OK;
	}
	HRESULT SetEyePosW(const XMFLOAT3& v)					{ return EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	HRESULT SetFogColor(const XMFLOAT4 v)					{ return FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	HRESULT SetFogStart(float f)							{ return FogStart->SetFloat(f); }
	HRESULT SetFogRange(float f)							{ return FogRange->SetFloat(f); }
	HRESULT SetMaterial(const Material& mat)				{ return Mat->SetRawValue(&mat, 0, sizeof(Material)); }
	HRESULT SetDiffuseMap(ID3D11ShaderResourceView* tex)	{ return DiffuseMap->SetResource(tex); }
	HRESULT SetCubeMap(ID3D11ShaderResourceView* tex)		{ return CubeMap->SetResource(tex); }
	HRESULT SetShadowMap(std::vector<ID3D11ShaderResourceView*> texList)
	{
		ShadowMap->SetResourceArray( &texList[0], 0, MAX_NUMBER_OF_POINT_LIGHTS );

		return S_OK;
	}
	HRESULT SetHasTexture(float h)							{ return HasTexture->SetFloat(h); }
	HRESULT SetWorldViewProjTex(CXMMATRIX M)				{ return WorldViewProjTex->SetMatrix(reinterpret_cast<const float*>(&M)); }

	HRESULT SetFlashing( bool flashing )					{ return Flashing->SetBool( flashing ); }

	HRESULT SetDirLight(const DirectionalLight* light)		{ return DirLight->SetRawValue(light, 0, sizeof(DirectionalLight)); }
	HRESULT SetPointLights(const PointLight* lights)		{ return  PointLights->SetRawValue(lights, 0, MAX_NUMBER_OF_POINT_LIGHTS * sizeof(PointLight));}

	HRESULT SetNumberOfPointLights(const int num)			{ return NumberOfPointLights->SetInt(num); }
	
	ID3DX11EffectTechnique* Light1TexTech;
	ID3DX11EffectTechnique* NoLightTex;
	ID3DX11EffectTechnique* LightTexShadowTech;
	ID3DX11EffectTechnique* LightTexShadowFogTech;
	ID3DX11EffectTechnique* LightTexShadowReflection;
	ID3DX11EffectTechnique* LightTexShadowReflectionFogTech;
	ID3DX11EffectTechnique* NoDirLightTexShadowFogTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* WorldViewProjTex;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectScalarVariable* NumberOfPointLights;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectScalarVariable* HasTexture;
	ID3DX11EffectScalarVariable* Flashing;

	ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* PointLights;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
	ID3DX11EffectShaderResourceVariable* CubeMap;
};


#pragma region NormalMapEffect
class CNormalMapEffect : public IEffect
{
public:
	CNormalMapEffect(ID3D11Device* device,const LPCWSTR filename);
	~CNormalMapEffect();

	HRESULT SetDirLight(const DirectionalLight* light)	{ return DirLight->SetRawValue(light, 0, sizeof(DirectionalLight)); }

	HRESULT SetMaterial(const Material& mat)				{ return Mat->SetRawValue(&mat, 0, sizeof(Material)); }

	HRESULT SetDiffuseMap(ID3D11ShaderResourceView* tex)	{ return DiffuseMap->SetResource(tex); }
	HRESULT SetNormalMap(ID3D11ShaderResourceView* tex)		{ return NormalMap->SetResource(tex); }
	HRESULT SetShadowMap(std::vector<ID3D11ShaderResourceView*> texList)
	{
		ShadowMap->SetResourceArray( &texList[0], 0, MAX_NUMBER_OF_POINT_LIGHTS );

		return S_OK;
	}

	HRESULT SetPointLights(const PointLight* lights)		{ return PointLights->SetRawValue(lights,0, MAX_NUMBER_OF_POINT_LIGHTS*sizeof(PointLight));}
	HRESULT SetNumberOfPointLights(const int num)			{ return NumberOfPointLights->SetInt(num); }

	HRESULT SetWorldViewProj(CXMMATRIX M)					{ return WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetWorld(CXMMATRIX M)							{ return World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetWorldInvTranspose(CXMMATRIX M)				{ return WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetTexTransform(CXMMATRIX M)					{ return TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetViewProj(CXMMATRIX M)						{ return ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	HRESULT SetShadowTransforms(std::vector<XMFLOAT4X4> matrixList)
	{
		XMMATRIX matricies[12];
		for ( int i = 0; i < matrixList.size(); ++i )
		{
			matricies[i] = XMLoadFloat4x4( &matrixList.at( i ) );
		}
		ShadowTransform->SetMatrixArray( reinterpret_cast< const float* >(&matricies), 0, MAX_NUMBER_OF_POINT_LIGHTS );

		return S_OK;
	}

	HRESULT SetEyePosW(const XMFLOAT3& v)					{ return EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }

	HRESULT SetFogColor(const XMFLOAT4 v)					{ return FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
	HRESULT SetFogStart(float f)							{ return FogStart->SetFloat(f); }
	HRESULT SetFogRange(float f)							{ return FogRange->SetFloat(f); }

	HRESULT SetHeightScale(float f)							{ return HeightScale->SetFloat(f); }
	HRESULT SetMaxTessDistance(float f)						{ return MaxTessDistance->SetFloat(f); }
	HRESULT SetMinTessDistance(float f)						{ return MinTessDistance->SetFloat(f); }
	HRESULT SetMinTessFactor(float f)						{ return MinTessFactor->SetFloat(f); }
	HRESULT SetMaxTessFactor(float f)						{ return MaxTessFactor->SetFloat(f); }
	HRESULT SetUsesTexture(bool texture)				   
	{
		if(texture)
			return UsesTexture->SetInt(1);
		else
			return UsesTexture->SetInt(0);
	}
	HRESULT SetFlashing( bool flashing )					{ return Flashing->SetBool( flashing ); }

	ID3DX11EffectVariable* PointLights;
	ID3DX11EffectScalarVariable* NumberOfPointLights;
	ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* Mat;

	ID3DX11EffectScalarVariable* UsesTexture;
	ID3DX11EffectScalarVariable* Flashing;

	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
	

	ID3DX11EffectTechnique* LightTexTech;
	ID3DX11EffectTechnique* DisplacementMappingTech;
	ID3DX11EffectTechnique* LightTexShadowTech;
	ID3DX11EffectTechnique* LightTexShadowFogTech;
	ID3DX11EffectTechnique* NoDirLightTexShadowFogTech;


	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;

	ID3DX11EffectScalarVariable* HeightScale;
	ID3DX11EffectScalarVariable* MaxTessDistance;
	ID3DX11EffectScalarVariable* MinTessDistance;
	ID3DX11EffectScalarVariable* MinTessFactor;
	ID3DX11EffectScalarVariable* MaxTessFactor;
};

#pragma endregion

#pragma region ParticleEffect
class CParticleEffect : public IEffect
{
public:
	CParticleEffect(ID3D11Device* device,LPCWSTR filename,const char* code);
	~CParticleEffect();

	void SetViewProj(CXMMATRIX M)                       { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

	void SetGameTime(float f)                           { GameTime->SetFloat(f); }
	void SetTimeStep(float f)                           { TimeStep->SetFloat(f); }
	void SetTextureSize(const float f)					{ TextureSize->SetFloat( f ); }
	void SetInitVel(const float f)						{ InitVelocity->SetFloat( f ); }
	void SetRandomXMod(const float f)					{ RandomXMod->SetFloat( f ); }
	void SetRandomZMod(const float f)					{ RandomZMod->SetFloat( f ); }
	void SetEmitTime( const float f )					{ EmitTime->SetFloat( f ); }
	void SetPaused( const bool b )						{ Paused->SetBool( b ); }

	void SetParticleAccel( const XMFLOAT3& v )			{ ParticleAccel->SetRawValue( &v, 0, sizeof( XMFLOAT3 ) ); }

	void SetEyePosW(const XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEmitPosW(const XMFLOAT3& v)                 { EmitPosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetEmitDirW(const XMFLOAT3& v)                 { EmitDirW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	

	void SetTexArray(ID3D11ShaderResourceView* tex)     { TexArray->SetResource(tex); }
	void SetRandomTex(ID3D11ShaderResourceView* tex)    { RandomTex->SetResource(tex); }
	
	ID3DX11EffectTechnique* StreamOutTech;
	ID3DX11EffectTechnique* DrawTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectScalarVariable* GameTime;
	ID3DX11EffectScalarVariable* TextureSize;
	ID3DX11EffectScalarVariable* InitVelocity;
	ID3DX11EffectScalarVariable* RandomXMod;
	ID3DX11EffectScalarVariable* RandomZMod;
	ID3DX11EffectScalarVariable* EmitTime;
	ID3DX11EffectScalarVariable* Paused;
	ID3DX11EffectScalarVariable* TimeStep;
	ID3DX11EffectVectorVariable* ParticleAccel;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* EmitPosW;
	ID3DX11EffectVectorVariable* EmitDirW;
	ID3DX11EffectShaderResourceVariable* TexArray;
	ID3DX11EffectShaderResourceVariable* RandomTex;

	const char* code;
};

#pragma endregion

#pragma region BlurEffect
class CBlurEffect : public IEffect
{
public:
	CBlurEffect(ID3D11Device* device,LPCWSTR filename);
	~CBlurEffect();

	ID3DX11EffectTechnique* HorzBlurTech;
	ID3DX11EffectTechnique* VertBlurTech;

	ID3DX11EffectScalarVariable* Weights;
	ID3DX11EffectShaderResourceVariable* InputMap;
	ID3DX11EffectUnorderedAccessViewVariable* OutputMap;
};

#pragma endregion

#pragma region SkinEffect
class CSkinEffect : public IEffect
{
public:
	CSkinEffect(ID3D11Device* device,LPCWSTR filename);
	~CSkinEffect();

	HRESULT SetShadowTransforms(std::vector<XMFLOAT4X4> matrixList)
	{
		XMMATRIX matricies[12];
		for ( int i = 0; i < matrixList.size(); ++i )
		{
			matricies[i] = XMLoadFloat4x4( &matrixList.at( i ) );
		}
		ShadowTransform->SetMatrixArray( reinterpret_cast< const float* >(&matricies), 0, MAX_NUMBER_OF_POINT_LIGHTS );

		return S_OK;
	}

	HRESULT SetShadowMap(std::vector<ID3D11ShaderResourceView*> texList)
	{
		ShadowMap->SetResourceArray( &texList[0], 0, MAX_NUMBER_OF_POINT_LIGHTS );

		return S_OK;
	}

	ID3DX11EffectTechnique* LightTexSkin;
	ID3DX11EffectTechnique* LightTexShadowSkin;
	ID3DX11EffectTechnique* BuildShadowMap;

	ID3DX11EffectScalarVariable*	NumberOfPointLights;
	ID3DX11EffectScalarVariable*	FogStart;
	ID3DX11EffectScalarVariable*	FogRange;

	ID3DX11EffectVectorVariable*	FogColor;
	ID3DX11EffectVectorVariable*	EyePos;
	
	ID3DX11EffectMatrixVariable*	World;
	ID3DX11EffectMatrixVariable*	WorldInvTranspose;
	ID3DX11EffectMatrixVariable*	WorldViewProj;
	ID3DX11EffectMatrixVariable*	TexTransform;
	ID3DX11EffectMatrixVariable*	BoneTransforms;

	ID3DX11EffectMatrixVariable* ShadowTransform;

	ID3DX11EffectShaderResourceVariable*	DiffuseMap;
	ID3DX11EffectShaderResourceVariable*	NormalMap;

	ID3DX11EffectShaderResourceVariable* ShadowMap;

	ID3DX11EffectVariable*			PointLights;
	ID3DX11EffectVariable*			DirectionalLight;
	ID3DX11EffectVariable*			Mat;
};

#pragma endregion

#pragma region LineEffect
class CLineEffect
	: public IEffect
{
private:
	ID3DX11EffectTechnique* m_fxTechnique;
	ID3DX11EffectMatrixVariable* m_fxView;
	ID3DX11EffectMatrixVariable* m_fxProj;
	ID3DX11EffectMatrixVariable* m_fxWorld;

	ID3D11InputLayout* m_InputLayout;

public:
	CLineEffect(ID3D11Device* device, LPCWSTR filename);
	~CLineEffect();

	void SetView( CXMMATRIX m ) 
	{
		HRESULT hr = m_fxView->SetMatrix(reinterpret_cast<const float*>(&m));
		Checkhr(hr,"Couldn't set the view!\n void CLineEffect::SetView(XMMATRIX m)");
	}

	void SetProj( CXMMATRIX m)
	{
		HRESULT hr = m_fxProj->SetMatrix(reinterpret_cast<const float*>(&m));
		Checkhr(hr, "Couldn't set the proj!\n void CLineEffect::SetProj(XMMATRIX m)");
	}

	void SetWorld( CXMMATRIX m )
	{
		HRESULT hr = m_fxWorld->SetMatrix(reinterpret_cast<const float*>(&m));
	}

	ID3D11InputLayout* GetInputLayout()
	{
		return m_InputLayout;
	}

	void DestoryResources()
	{
		ReleaseCOM( m_InputLayout );
	}

	ID3DX11EffectTechnique* GetTech()
	{
		return m_fxTechnique;
	}
};

#pragma endregion

#pragma region BuildShadowMapEffect
class CBuildShadowMapEffect : public IEffect
{
public:
	CBuildShadowMapEffect(ID3D11Device* device, const std::wstring& filename);
	~CBuildShadowMapEffect();

	void SetViewProj(CXMMATRIX M)                       { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProj(CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorld(CXMMATRIX M)                          { World->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldInvTranspose(CXMMATRIX M)              { WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetTexTransform(CXMMATRIX M)                   { TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetEyePosW(const XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	
	void SetHeightScale(float f)                        { HeightScale->SetFloat(f); }
	void SetMaxTessDistance(float f)                    { MaxTessDistance->SetFloat(f); }
	void SetMinTessDistance(float f)                    { MinTessDistance->SetFloat(f); }
	void SetMinTessFactor(float f)                      { MinTessFactor->SetFloat(f); }
	void SetMaxTessFactor(float f)                      { MaxTessFactor->SetFloat(f); }

	void SetDiffuseMap(ID3D11ShaderResourceView* tex)   { DiffuseMap->SetResource(tex); }
	void SetNormalMap(ID3D11ShaderResourceView* tex)    { NormalMap->SetResource(tex); }

	ID3DX11EffectTechnique* BuildShadowMapTech;
	ID3DX11EffectTechnique* BuildShadowMapAlphaClipTech;
	ID3DX11EffectTechnique* TessBuildShadowMapTech;
	ID3DX11EffectTechnique* TessBuildShadowMapAlphaClipTech;
	ID3DX11EffectTechnique* TerrainBuildShadowMapTech;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectScalarVariable* HeightScale;
	ID3DX11EffectScalarVariable* MaxTessDistance;
	ID3DX11EffectScalarVariable* MinTessDistance;
	ID3DX11EffectScalarVariable* MinTessFactor;
	ID3DX11EffectScalarVariable* MaxTessFactor;
 
	ID3DX11EffectShaderResourceVariable* DiffuseMap;
	ID3DX11EffectShaderResourceVariable* NormalMap;
};

#pragma endregion

#pragma region SkyEffect
class CSkyEffect : IEffect
{
public:
	CSkyEffect(ID3D11Device* device, const std::wstring& filename);
	~CSkyEffect();

	void SetWorldViewProj(CXMMATRIX M)                  { WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetCubeMap(ID3D11ShaderResourceView* cubemap)  { CubeMap->SetResource(cubemap); }

	ID3DX11EffectTechnique* SkyTech;

	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectShaderResourceVariable* CubeMap;
};
#pragma endregion

#pragma region RefractionEffect
class CRefractionEffect : IEffect
{
public:
	CRefractionEffect( ID3D11Device* device, LPCWSTR filename );
	~CRefractionEffect();

	HRESULT SetWorld( CXMMATRIX M )	{ return World->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetView( CXMMATRIX M )	{ return View->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetProj( CXMMATRIX M )	{ return Proj->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetWorldViewProj( CXMMATRIX M ) { return WorldViewProj->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetWorldInvTranspose( CXMMATRIX M ) { return WorldInvTranspose->SetMatrix( reinterpret_cast<const float*>(&M) ); }

	HRESULT SetClipPlane( XMFLOAT4& F ) { return ClipPlane->SetFloatVector( reinterpret_cast<const float*>(&F) ); }
	HRESULT SetEyePos( XMFLOAT3& F ) { return EyePos->SetFloatVector( reinterpret_cast<const float*>(&F) ); }

	HRESULT SetDiffuseMap( ID3D11ShaderResourceView* srv ) { return DiffuseMap->SetResource( srv ); }

	HRESULT SetDirectionalLighting( const DirectionalLight* dirLight ) { return DirLight->SetRawValue( dirLight, 0, sizeof(DirectionalLight) ); }
	HRESULT SetPointLights( const PointLight* pointLight )		{ return PointLight->SetRawValue( pointLight, 0, sizeof( PointLight ) * MAX_NUMBER_OF_POINT_LIGHTS ); }

	HRESULT SetMaterial( const Material& mat )	{ return Mat->SetRawValue( &mat, 0, sizeof( Material ) ); }

	HRESULT SetNumberOfPointLights( const int numOfPointLights ) { return NumOfPointLights->SetInt( numOfPointLights ); }

	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* Proj;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;

	ID3DX11EffectVariable* DirLight;
	ID3DX11EffectVariable* PointLight;
	ID3DX11EffectScalarVariable* NumOfPointLights;

	ID3DX11EffectVariable* Mat;

	ID3DX11EffectVectorVariable* ClipPlane;
	ID3DX11EffectVectorVariable* EyePos;

	ID3DX11EffectShaderResourceVariable* DiffuseMap; 

	ID3DX11EffectTechnique* RefractWithBasicLighting;
};

#pragma endregion 

#pragma region WaterEffect
class CWaterEffect : IEffect
{
public:
	CWaterEffect( ID3D11Device* device, LPCWSTR filename );
	~CWaterEffect();

	HRESULT SetWorld( CXMMATRIX M )	{ return World->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetView( CXMMATRIX M )	{ return View->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetProj( CXMMATRIX M )	{ return Proj->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetWorldViewProj( CXMMATRIX M ) { return WorldViewProj->SetMatrix( reinterpret_cast<const float*>( &M ) ); }
	HRESULT SetReflection( CXMMATRIX M ) { return Reflection->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetReflProjWorld( CXMMATRIX M ) { return ReflProjWorld->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	HRESULT SetViewProjWorld( CXMMATRIX M ) { return ViewProjWorld->SetMatrix( reinterpret_cast<const float*>(&M) ); }

	HRESULT SetPointLights(const PointLight* lights)
	{
		if ( lights )
		{
			UINT size = MAX_NUMBER_OF_POINT_LIGHTS * sizeof( PointLight );
			if ( size > 0 )
			{
				if ( PointLights->IsValid() == TRUE )
					return  PointLights->SetRawValue( lights, 0, size );
				return S_OK;
			}
			return S_OK;
		}
		return S_OK;
	}
	HRESULT SetNumberOfPointLights( const int numOfPointLights ) { return NumOfPointLights->SetInt( numOfPointLights ); }
	HRESULT SetEyePos( XMFLOAT3& F ) { return EyePos->SetFloatVector( reinterpret_cast<const float*>(&F) ); }

	HRESULT SetWaterTranslation( float f ) { return WaterTranslation->SetFloat( f ); }
	HRESULT SetReflectRefractScale( float f ) { return ReflectRefractScale->SetFloat( f ); }
	HRESULT SetDiffuseAbsorbity( float f ) { return DiffuseAbsorbity->SetFloat( f ); }
	HRESULT SetSpecularAbsorbity( float f ) { return SpecularAbsorbity->SetFloat( f ); }

	HRESULT SetReflectionTexture( ID3D11ShaderResourceView* srv ) { return ReflectionTexture->SetResource( srv ); }
	HRESULT SetRefractionTexture( ID3D11ShaderResourceView* srv ) { return RefractionTexture->SetResource( srv ); }
	HRESULT SetNormalMap( ID3D11ShaderResourceView* srv ) { return NormalMap->SetResource( srv ); }

	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* Proj;
	ID3DX11EffectMatrixVariable* View;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* Reflection;
	ID3DX11EffectMatrixVariable* ReflProjWorld;
	ID3DX11EffectMatrixVariable* ViewProjWorld;

	ID3DX11EffectVariable* PointLights;
	ID3DX11EffectScalarVariable* NumOfPointLights;
	ID3DX11EffectVectorVariable* EyePos;

	ID3DX11EffectScalarVariable* WaterTranslation;
	ID3DX11EffectScalarVariable* ReflectRefractScale;
	ID3DX11EffectScalarVariable* DiffuseAbsorbity;
	ID3DX11EffectScalarVariable* SpecularAbsorbity;

	ID3DX11EffectShaderResourceVariable* ReflectionTexture;
	ID3DX11EffectShaderResourceVariable* RefractionTexture;
	ID3DX11EffectShaderResourceVariable* NormalMap;

	ID3DX11EffectTechnique* WaterRefractTech;
	ID3DX11EffectTechnique* WaterRefractReflectTech;
};

#pragma endregion


//SUPPORTS:
	// Color. No shading or anything else.
// In my opinion the worst effect of them all but useful for testing.
#pragma region ColorEffect
class CColorEffect : public IEffect
{
public:
	CColorEffect( ID3D11Device* device, LPCWSTR filename );
	~CColorEffect();

	HRESULT SetWorldViewProj( CXMMATRIX M )			{ return WorldViewProj->SetMatrix( reinterpret_cast<const float*>(&M) ); }

	ID3DX11EffectMatrixVariable* WorldViewProj;

	ID3DX11EffectTechnique* ColorTechnique;
};

#pragma endregion

//SUPPORTS:
	// No PointLights.
// Used for rendering terrain.
#pragma region TerrainEffect
class CTerrainEffect : public IEffect
{
public:
	CTerrainEffect(ID3D11Device* device, const std::wstring& filename);
	~CTerrainEffect();

	void SetViewProj(CXMMATRIX M)                       { ViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
	void SetWorldViewProj( CXMMATRIX M )				{ WorldViewProj->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	void SetWorld( CXMMATRIX M )						{ World->SetMatrix( reinterpret_cast<const float*>(&M) ); }
	void SetShadowTransform( CXMMATRIX M )				{ ShadowTransform->SetMatrix( reinterpret_cast< const float* >( &M ) ); }
	void SetEyePosW(const XMFLOAT3& v)                  { EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
	void SetClipPlane( const XMFLOAT4& f )				{ ClipPlane->SetFloatVector( reinterpret_cast< const float* >( &f ) ); }
	void SetFogColor(const FXMVECTOR v)                 { FogColor->SetFloatVector( reinterpret_cast< const float* >( &v ) ); }
	void SetFogStart(float f)                           { FogStart->SetFloat(f); }
	void SetFogRange(float f)                           { FogRange->SetFloat(f); }
	void SetDirLights(const DirectionalLight* lights)   { DirLights->SetRawValue(lights, 0, sizeof(DirectionalLight)); }
	void SetMaterial(const Material& mat)               { Mat->SetRawValue(&mat, 0, sizeof(Material)); }

	void SetMinDist(float f)                            { MinDist->SetFloat(f); }
	void SetMaxDist(float f)                            { MaxDist->SetFloat(f); }
	void SetMinTess(float f)                            { MinTess->SetFloat(f); }
	void SetMaxTess(float f)                            { MaxTess->SetFloat(f); }
	void SetTexelCellSpaceU(float f)                    { TexelCellSpaceU->SetFloat(f); }
	void SetTexelCellSpaceV(float f)                    { TexelCellSpaceV->SetFloat(f); }
	void SetWorldCellSpace(float f)                     { WorldCellSpace->SetFloat(f); }
	void SetWorldFrustumPlanes(XMFLOAT4 planes[6])      { WorldFrustumPlanes->SetFloatVectorArray(reinterpret_cast<float*>(planes), 0, 6); }

	void SetLayer1(ID3D11ShaderResourceView* tex)		{ Layer1->SetResource(tex); }
	void SetLayer2(ID3D11ShaderResourceView* tex)		{ Layer2->SetResource(tex); }
	void SetLayer3(ID3D11ShaderResourceView* tex)		{ Layer3->SetResource(tex); }
	void SetLayer4(ID3D11ShaderResourceView* tex)		{ Layer4->SetResource(tex); }
	void SetLayer5(ID3D11ShaderResourceView* tex)		{ Layer5->SetResource(tex); }
	void SetBlendMap(ID3D11ShaderResourceView* tex)     { BlendMap->SetResource(tex); }
	void SetHeightMap(ID3D11ShaderResourceView* tex)    { HeightMap->SetResource(tex); }
	void SetShadowMap( ID3D11ShaderResourceView* tex )	{ ShadowMap->SetResource( tex ); }
	

	ID3DX11EffectTechnique* Light1Tech;
	ID3DX11EffectTechnique* Light2Tech;
	ID3DX11EffectTechnique* Light3Tech;
	ID3DX11EffectTechnique* Light1FogTech;
	ID3DX11EffectTechnique* Light2FogTech;
	ID3DX11EffectTechnique* Light3FogTech;
	ID3DX11EffectTechnique* LightRefract;

	ID3DX11EffectMatrixVariable* ViewProj;
	ID3DX11EffectMatrixVariable* WorldViewProj;
	ID3DX11EffectMatrixVariable* World;
	ID3DX11EffectMatrixVariable* WorldInvTranspose;
	ID3DX11EffectMatrixVariable* ShadowTransform;
	ID3DX11EffectMatrixVariable* TexTransform;
	ID3DX11EffectVectorVariable* EyePosW;
	ID3DX11EffectVectorVariable* ClipPlane;
	ID3DX11EffectVectorVariable* FogColor;
	ID3DX11EffectScalarVariable* FogStart;
	ID3DX11EffectScalarVariable* FogRange;
	ID3DX11EffectVariable* DirLights;
	ID3DX11EffectVariable* Mat;
	ID3DX11EffectScalarVariable* MinDist;
	ID3DX11EffectScalarVariable* MaxDist;
	ID3DX11EffectScalarVariable* MinTess;
	ID3DX11EffectScalarVariable* MaxTess;
	ID3DX11EffectScalarVariable* TexelCellSpaceU;
	ID3DX11EffectScalarVariable* TexelCellSpaceV;
	ID3DX11EffectScalarVariable* WorldCellSpace;
	ID3DX11EffectVectorVariable* WorldFrustumPlanes;

	ID3DX11EffectShaderResourceVariable* Layer1;
	ID3DX11EffectShaderResourceVariable* Layer2;
	ID3DX11EffectShaderResourceVariable* Layer3;
	ID3DX11EffectShaderResourceVariable* Layer4;
	ID3DX11EffectShaderResourceVariable* Layer5;
	ID3DX11EffectShaderResourceVariable* BlendMap;
	ID3DX11EffectShaderResourceVariable* HeightMap;
	ID3DX11EffectShaderResourceVariable* ShadowMap;
};

#pragma endregion