//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Effects.h"

#define CreateEffectMatrix(x) { mFX->GetVariableByName(x)->AsMatrix(); }

#pragma region Effect
IEffect::IEffect(ID3D11Device* device, const LPCWSTR filename)
	: mFX(0)
{
	HRESULT hr;
	ID3D10Blob* compiledShader = 0;
	ID3D10Blob* errorMessage = 0;
	DWORD shaderFlags = 0;
	
#if defined (_DEBUG) || defined (DEBUG)
	shaderFlags |= D3D10_SHADER_DEBUG;
	shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif
	
	hr = D3DX11CompileFromFile(filename,0,0,0,"fx_5_0",shaderFlags,0,0,&compiledShader,&errorMessage,0);
	if(FAILED(hr))
	{
		if(errorMessage != 0)
		{
			MessageBoxA(0,(char*)errorMessage->GetBufferPointer(),"Fatal Error",MB_OK);
			ReleaseCOM(errorMessage);
		}
	}

	hr = D3DX11CreateEffectFromMemory(compiledShader->GetBufferPointer(),compiledShader->GetBufferSize(),
		0,device,&mFX);
	if(FAILED(hr))
	{
		assert(L"We couldn't create the effect!");
	}

	ReleaseCOM(compiledShader);
	ReleaseCOM(errorMessage);
}

IEffect::~IEffect()
{
	ReleaseCOM(mFX);
}

#pragma endregion


#pragma region
CBasicEffect::CBasicEffect(ID3D11Device* device, const LPCWSTR filename) 
	: IEffect(device,filename)
{
	LightTexShadowReflectionFogTech = mFX->GetTechniqueByName( "LightTexShadowReflectionFog" );
	LightTexShadowReflection		= mFX->GetTechniqueByName( "LightTexShadowReflection" );
	LightTexShadowFogTech			= mFX->GetTechniqueByName( "LightTexShadowFog" );
	LightTexShadowTech				= mFX->GetTechniqueByName( "LightTexShadow" );
	Light1TexTech					= mFX->GetTechniqueByName( "Light1Tex" );
	NoLightTex						= mFX->GetTechniqueByName( "NoLightTex" );
	NoDirLightTexShadowFogTech = mFX->GetTechniqueByName( "NoDirLightTexShadowFog" );

	WorldViewProj     = mFX->GetVariableByName( "gWorldViewProj" )->AsMatrix();
	World             = mFX->GetVariableByName( "gWorld" )->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	WorldViewProjTex  = mFX->GetVariableByName("gWorldViewProjTex")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	ShadowTransform	  = mFX->GetVariableByName("gShadowTransform")->AsMatrix();
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();

	FogColor          = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart          = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange          = mFX->GetVariableByName("gFogRange")->AsScalar();

	Mat               = mFX->GetVariableByName("gMaterial");
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	CubeMap           = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
	ShadowMap		  = mFX->GetVariableByName("gShadowMap")->AsShaderResource();
	HasTexture		  = mFX->GetVariableByName("gHasTexture")->AsScalar();
	NumberOfPointLights = mFX->GetVariableByName("gNumberOfPointLights")->AsScalar();

	DirLight		  = mFX->GetVariableByName("gDirLight");
	PointLights		  = mFX->GetVariableByName("gPointLights");

	Flashing		  =	mFX->GetVariableByName("g_bFlashing")->AsScalar();
}

CBasicEffect::~CBasicEffect()
{

}

#pragma endregion


#pragma region NormalMap
CNormalMapEffect::CNormalMapEffect(ID3D11Device* device,const LPCWSTR filename)
	: IEffect(device,filename)
{
	UsesTexture				= mFX->GetVariableByName("gUsesTexture")->AsScalar();
	Flashing				= mFX->GetVariableByName( "g_bFlashing" )->AsScalar();
	PointLights				= mFX->GetVariableByName("gPointLights");
	NumberOfPointLights		= mFX->GetVariableByName("gNumberOfPointLights")->AsScalar();
	DirLight				= mFX->GetVariableByName("gDirLight");
	Mat						= mFX->GetVariableByName("gMaterial");

	DiffuseMap				= mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap				= mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	ShadowMap				= mFX->GetVariableByName("gShadowMap")->AsShaderResource();

	LightTexTech			= mFX->GetTechniqueByName("LightTexTech");
	DisplacementMappingTech = mFX->GetTechniqueByName("DisplacementMappingTech");
	LightTexShadowTech		= mFX->GetTechniqueByName("LightTexShadowTech");
	LightTexShadowFogTech   = mFX->GetTechniqueByName("LightTexShadowFogTech");
	NoDirLightTexShadowFogTech = mFX->GetTechniqueByName( "NoDirLightTexShadowFogTech" );

	WorldViewProj			= mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World					= mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose		= mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform			= mFX->GetVariableByName("gTexTransform")->AsMatrix();
	ViewProj				= mFX->GetVariableByName("gViewProj")->AsMatrix();
	ShadowTransform			= mFX->GetVariableByName("gShadowTransform")->AsMatrix();

	EyePosW					= mFX->GetVariableByName("gEyePosW")->AsVector();

	FogColor				= mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart				= mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange				= mFX->GetVariableByName("gFogRange")->AsScalar();

	HeightScale				= mFX->GetVariableByName("gHeightScale")->AsScalar();
	MaxTessDistance			= mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance			= mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	MinTessFactor			= mFX->GetVariableByName("gMinTessFactor")->AsScalar();
	MaxTessFactor			= mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
}

CNormalMapEffect::~CNormalMapEffect()
{

}

#pragma endregion


#pragma region ParticleEffect
CParticleEffect::CParticleEffect(ID3D11Device* device,LPCWSTR filename, const char* code)
	: IEffect(device,filename)
{
	StreamOutTech   = mFX->GetTechniqueByName("StreamOutTech");
	DrawTech		= mFX->GetTechniqueByName("DrawTech");

	ViewProj		= mFX->GetVariableByName("gViewProj")->AsMatrix();
	GameTime		= mFX->GetVariableByName("gGameTime")->AsScalar();
	TimeStep		= mFX->GetVariableByName("gTimeStep")->AsScalar();
	TextureSize		= mFX->GetVariableByName("gTextureSize")->AsScalar(); 
	InitVelocity	= mFX->GetVariableByName("gInitVelocity")->AsScalar();
	RandomXMod		= mFX->GetVariableByName("gRandomXMod")->AsScalar();
	RandomZMod		= mFX->GetVariableByName("gRandomZMod")->AsScalar();
	EmitTime		= mFX->GetVariableByName("gEmitTime")->AsScalar();
	Paused			= mFX->GetVariableByName("gPaused")->AsScalar();
	EyePosW			= mFX->GetVariableByName("gEyePosW")->AsVector();
	EmitPosW		= mFX->GetVariableByName("gEmitPosW")->AsVector();
	EmitDirW		= mFX->GetVariableByName("gEmitDirW")->AsVector();
	TexArray		= mFX->GetVariableByName("gTexArray")->AsShaderResource();
	RandomTex		= mFX->GetVariableByName("gRandomTex")->AsShaderResource();
	ParticleAccel	= mFX->GetVariableByName("gAccelW")->AsVector();

	this->code = code;
}
CParticleEffect::~CParticleEffect()
{
	
}


#pragma endregion


#pragma region BlurEffect
CBlurEffect::CBlurEffect(ID3D11Device* device,LPCWSTR filename)
	: IEffect(device,filename)
{
	HorzBlurTech = mFX->GetTechniqueByName("HorzBlur");
	VertBlurTech = mFX->GetTechniqueByName("VertBlur");

	Weights     = mFX->GetVariableByName("gWeights")->AsScalar();
	InputMap    = mFX->GetVariableByName("gInput")->AsShaderResource();
	OutputMap   = mFX->GetVariableByName("gOutput")->AsUnorderedAccessView();
}

CBlurEffect::~CBlurEffect()
{

}


#pragma endregion


#pragma region SkinEffect
CSkinEffect::CSkinEffect(ID3D11Device* device,LPCWSTR filename)
	: IEffect(device,filename)
{
	LightTexSkin			= mFX->GetTechniqueByName("LightTexSkinned");
	LightTexShadowSkin		= mFX->GetTechniqueByName( "LightTexShadowSkinned" );
	BuildShadowMap			= mFX->GetTechniqueByName( "BuildShadowMap" );

	NumberOfPointLights		= mFX->GetVariableByName("gNumberOfPointLights")->AsScalar();
	FogStart				= mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange				= mFX->GetVariableByName("gFogRange")->AsScalar();

	FogColor				= mFX->GetVariableByName("gFogColor")->AsVector();
	EyePos					= mFX->GetVariableByName("gEyePosW")->AsVector();

	World					= mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose		= mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	WorldViewProj			= mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	TexTransform			= mFX->GetVariableByName("gTexTransform")->AsMatrix();
	BoneTransforms			= mFX->GetVariableByName("gBoneTransforms")->AsMatrix();

	ShadowTransform			= mFX->GetVariableByName("gShadowTransform")->AsMatrix();

	DiffuseMap				= mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap				= mFX->GetVariableByName("gNormalMap")->AsShaderResource();

	ShadowMap				= mFX->GetVariableByName("gShadowMap")->AsShaderResource();

	PointLights				= mFX->GetVariableByName("gPointLights");
	DirectionalLight		= mFX->GetVariableByName( "g_DirLight" );
	Mat						= mFX->GetVariableByName("gMaterial");
}

CSkinEffect::~CSkinEffect()
{

}

#pragma endregion


#pragma region LineEffect
CLineEffect::CLineEffect(ID3D11Device* device, LPCWSTR filename)
	: IEffect(device,filename)
{
	m_fxTechnique	= mFX->GetTechniqueByName("DefaultDrawLine");

	m_fxView		= mFX->GetVariableByName("gView")->AsMatrix();
	m_fxProj		= mFX->GetVariableByName("gProj")->AsMatrix();
	m_fxWorld		= mFX->GetVariableByName("gWorld")->AsMatrix();

	D3DX11_PASS_DESC passDesc;
	m_fxTechnique->GetPassByIndex(0)->GetDesc(&passDesc);
	const D3D11_INPUT_ELEMENT_DESC lineLayout[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT,0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	HRESULT hr = device->CreateInputLayout( lineLayout, ARRAYSIZE(lineLayout), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_InputLayout );
	Checkhr(hr, "Couldn't create the input layout!\nCLineEffect::CLineEffect(ID3D11Device* device, LPCWSTR filename)");
}

CLineEffect::~CLineEffect()
{

}

#pragma endregion


#pragma region BuildShadowMapEffect
CBuildShadowMapEffect::CBuildShadowMapEffect(ID3D11Device* device, const std::wstring& filename)
	: IEffect( device, filename.c_str() )
{
	BuildShadowMapTech           = mFX->GetTechniqueByName("BuildShadowMapTech");
	BuildShadowMapAlphaClipTech  = mFX->GetTechniqueByName("BuildShadowMapAlphaClipTech");

	TessBuildShadowMapTech           = mFX->GetTechniqueByName("TessBuildShadowMapTech");
	TessBuildShadowMapAlphaClipTech  = mFX->GetTechniqueByName("TessBuildShadowMapAlphaClipTech");

	TerrainBuildShadowMapTech		= mFX->GetTechniqueByName( "TerrainBuildShadowMap" );
	
	ViewProj          = mFX->GetVariableByName("gViewProj")->AsMatrix();
	WorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	World             = mFX->GetVariableByName("gWorld")->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
	TexTransform      = mFX->GetVariableByName("gTexTransform")->AsMatrix();
	
	EyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
	HeightScale       = mFX->GetVariableByName("gHeightScale")->AsScalar();
	MaxTessDistance   = mFX->GetVariableByName("gMaxTessDistance")->AsScalar();
	MinTessDistance   = mFX->GetVariableByName("gMinTessDistance")->AsScalar();
	MinTessFactor     = mFX->GetVariableByName("gMinTessFactor")->AsScalar();
	MaxTessFactor     = mFX->GetVariableByName("gMaxTessFactor")->AsScalar();
	DiffuseMap        = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
	NormalMap         = mFX->GetVariableByName("gNormalMap")->AsShaderResource();
	
}

CBuildShadowMapEffect::~CBuildShadowMapEffect()
{

}

#pragma endregion


#pragma region SkyEffect
CSkyEffect::CSkyEffect(ID3D11Device* device, const std::wstring& filename)
	: IEffect( device, filename.c_str() )
{
	SkyTech       = mFX->GetTechniqueByName("SkyTech");
	WorldViewProj = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	CubeMap       = mFX->GetVariableByName("gCubeMap")->AsShaderResource();
}


CSkyEffect::~CSkyEffect()
{

}
#pragma endregion


#pragma region RefractionEffect
CRefractionEffect::CRefractionEffect( ID3D11Device* device, LPCWSTR filename )
	: IEffect( device, filename )
{
	World		= mFX->GetVariableByName( "g_mWorld" )->AsMatrix();
	Proj		= mFX->GetVariableByName( "g_mProj" )->AsMatrix();
	View		= mFX->GetVariableByName( "g_mView" )->AsMatrix();
	WorldViewProj = mFX->GetVariableByName( "g_mWorldViewProj" )->AsMatrix();
	WorldInvTranspose = mFX->GetVariableByName( "g_mWorldInvTranspose" )->AsMatrix();
		
	ClipPlane	= mFX->GetVariableByName( "g_f4ClipPlane" )->AsVector();
	EyePos		= mFX->GetVariableByName( "g_f3EyePos" )->AsVector();

	DiffuseMap	= mFX->GetVariableByName( "g_txShaderTexture" )->AsShaderResource();

	DirLight	= mFX->GetVariableByName( "g_DirLight" );

	RefractWithBasicLighting = mFX->GetTechniqueByName( "RefractWithBasicLighting" );
}

CRefractionEffect::~CRefractionEffect()
{

}

#pragma endregion


#pragma region WaterEffect
CWaterEffect::CWaterEffect( ID3D11Device* device, LPCWSTR filename )
	: IEffect( device, filename )
{
	WorldViewProj			= mFX->GetVariableByName( "g_mWorldViewProj" )->AsMatrix();
	World					= mFX->GetVariableByName( "g_mWorld" )->AsMatrix();
	Proj					= mFX->GetVariableByName( "g_mProj" )->AsMatrix();
	View					= mFX->GetVariableByName( "g_mView" )->AsMatrix();
	Reflection				= mFX->GetVariableByName( "g_mReflectionMatrix" )->AsMatrix();
	ReflProjWorld			= mFX->GetVariableByName( "g_mReflProjWorld" )->AsMatrix();
	ViewProjWorld			= mFX->GetVariableByName( "g_mViewProjWorld" )->AsMatrix();

	PointLights				= mFX->GetVariableByName("gPointLights");
	NumOfPointLights		= mFX->GetVariableByName( "g_iNumOfPointLights" )->AsScalar();
	EyePos					= mFX->GetVariableByName( "g_f3EyePos" )->AsVector();

	WaterTranslation		= mFX->GetVariableByName( "g_fWaterTranslation" )->AsScalar();
	ReflectRefractScale		= mFX->GetVariableByName( "g_fReflectionRefractScale" )->AsScalar();
	DiffuseAbsorbity        = mFX->GetVariableByName( "g_fDiffuseAbsorbity" )->AsScalar();
	SpecularAbsorbity		= mFX->GetVariableByName( "g_fSpecularAbsorbity" )->AsScalar();

	ReflectionTexture		= mFX->GetVariableByName( "g_txReflectionTexture" )->AsShaderResource();
	RefractionTexture		= mFX->GetVariableByName( "g_txRefractionTexture" )->AsShaderResource();
	NormalMap				= mFX->GetVariableByName( "g_txNormalTexture" )->AsShaderResource();

	WaterRefractTech		= mFX->GetTechniqueByName( "WaterRefractTech" );
	WaterRefractReflectTech = mFX->GetTechniqueByName( "WaterReflectRefractTech" );
}

CWaterEffect::~CWaterEffect()
{

}

#pragma endregion


#pragma region ColorEffect
CColorEffect::CColorEffect( ID3D11Device* device, LPCWSTR filename )
	: IEffect( device, filename )
{
	WorldViewProj			= mFX->GetVariableByName( "g_mWorldViewProj" )->AsMatrix();

	ColorTechnique			= mFX->GetTechniqueByName( "ColorTechnique" );
}

CColorEffect::~CColorEffect()
{

}

#pragma endregion


#pragma region TerrainEffect
CTerrainEffect::CTerrainEffect( ID3D11Device* device, const std::wstring& filename )
	: IEffect( device, filename.c_str() )
{
	Light1Tech    = mFX->GetTechniqueByName("Light1");
	Light2Tech    = mFX->GetTechniqueByName("Light2");
	Light3Tech    = mFX->GetTechniqueByName("Light3");
	Light1FogTech = mFX->GetTechniqueByName("Light1Fog");
	Light2FogTech = mFX->GetTechniqueByName("Light2Fog");
	Light3FogTech = mFX->GetTechniqueByName("Light3Fog");
	LightRefract = mFX->GetTechniqueByName("LightRefract");

	ViewProj           = mFX->GetVariableByName("gViewProj")->AsMatrix();
	WorldViewProj	   = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
	ShadowTransform	   = mFX->GetVariableByName("gShadowTransform")->AsMatrix();
	World			   = mFX->GetVariableByName("gWorld")->AsMatrix();
	EyePosW            = mFX->GetVariableByName("gEyePosW")->AsVector();
	ClipPlane		   = mFX->GetVariableByName("g_f4ClipPlane")->AsVector();
	FogColor           = mFX->GetVariableByName("gFogColor")->AsVector();
	FogStart           = mFX->GetVariableByName("gFogStart")->AsScalar();
	FogRange           = mFX->GetVariableByName("gFogRange")->AsScalar();
	DirLights          = mFX->GetVariableByName("gDirLights");
	Mat                = mFX->GetVariableByName("gMaterial");

	MinDist            = mFX->GetVariableByName("gMinDist")->AsScalar();
	MaxDist            = mFX->GetVariableByName("gMaxDist")->AsScalar();
	MinTess            = mFX->GetVariableByName("gMinTess")->AsScalar();
	MaxTess            = mFX->GetVariableByName("gMaxTess")->AsScalar();
	TexelCellSpaceU    = mFX->GetVariableByName("gTexelCellSpaceU")->AsScalar();
	TexelCellSpaceV    = mFX->GetVariableByName("gTexelCellSpaceV")->AsScalar();
	WorldCellSpace     = mFX->GetVariableByName("gWorldCellSpace")->AsScalar();
	WorldFrustumPlanes = mFX->GetVariableByName("gWorldFrustumPlanes")->AsVector();

	Layer1			   = mFX->GetVariableByName("gLayer1")->AsShaderResource();
	Layer2             = mFX->GetVariableByName("gLayer2")->AsShaderResource();
	Layer3             = mFX->GetVariableByName("gLayer3")->AsShaderResource();
	Layer4             = mFX->GetVariableByName("gLayer4")->AsShaderResource();
	Layer5             = mFX->GetVariableByName("gLayer5")->AsShaderResource();
	BlendMap           = mFX->GetVariableByName("gBlendMap")->AsShaderResource();
	HeightMap          = mFX->GetVariableByName("gHeightMap")->AsShaderResource();
	ShadowMap		   = mFX->GetVariableByName("gShadowMap")->AsShaderResource();
}

CTerrainEffect::~CTerrainEffect()
{

}

#pragma endregion