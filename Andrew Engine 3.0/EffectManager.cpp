//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "EffectManager.h"


CEffectManager::CEffectManager()
	: BlurEffectAccess()
{

}

CEffectManager::~CEffectManager()
{

}

bool CEffectManager::Init()
{
	return true;
}

bool CEffectManager::InitResources(ID3D11Device* device)
{
	mBasicFX = new CBasicEffect( device, L"Basic.fx" );
	assert( mBasicFX );
	mNormalMapFX = new CNormalMapEffect( device, L"NormalMap.fx" );
	assert( mNormalMapFX) ;
	mFireFX = new CParticleEffect( device, L"Fire.fx", "fire" );
	assert( mFireFX );
	mSparkFX = new CParticleEffect( device, L"Spark.fx", "spark" );
	assert( mSparkFX );
	mShadowFX = new CBuildShadowMapEffect( device, L"BuildShadowMap.fx" );
	assert( mShadowFX );
	mSkyFX = new CSkyEffect( device, L"Sky.fx" );
	assert( mSkyFX );
	mRefractFX = new CRefractionEffect( device, L"Refraction.fx" );
	assert( mRefractFX );
	mWaterFX = new CWaterEffect( device, L"Water.fx" );
	assert( mWaterFX );
	mColorFX = new CColorEffect( device, L"Color.fx" );
	assert( mColorFX );
	mTerrainFX = new CTerrainEffect( device, L"Terrain.fx" );
	assert( mTerrainFX );

	BlurEffectAccess.Init(device);
	//SkinnedEffectAccess.Init(device);
	
	D3DX11_PASS_DESC passDesc;
	mBasicFX->Light1TexTech->GetPassByIndex(0)->GetDesc(&passDesc);
	const D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	HRESULT hr = device->CreateInputLayout(layout,3,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&mPosNormTex);
	if(FAILED(hr))
		return false;


	mNormalMapFX->LightTexTech->GetPassByIndex(0)->GetDesc(&passDesc);
	const D3D11_INPUT_ELEMENT_DESC normalMapLayout[] = 
	{
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
	};
	hr = device->CreateInputLayout(normalMapLayout,4,passDesc.pIAInputSignature,passDesc.IAInputSignatureSize,&mPosNormTexTan);
	if(FAILED(hr))
		return false;


	mFireFX->StreamOutTech->GetPassByIndex(0)->GetDesc(&passDesc);
	const D3D11_INPUT_ELEMENT_DESC particleLayout[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"AGE",      0, DXGI_FORMAT_R32_FLOAT,       0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TYPE",     0, DXGI_FORMAT_R32_UINT,        0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	hr = device->CreateInputLayout( particleLayout, ARRAYSIZE( particleLayout ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mParticleInputLayout );
	if ( FAILED( hr ) )
		return false;

	mSkyFX->SkyTech->GetPassByIndex( 0 )->GetDesc( &passDesc );
	const D3D11_INPUT_ELEMENT_DESC posLayout[1] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	hr = device->CreateInputLayout( posLayout, ARRAYSIZE( posLayout ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mPos );
	if ( FAILED( hr ) )
		return false;

	mColorFX->ColorTechnique->GetPassByIndex( 0 )->GetDesc( &passDesc );
	const D3D11_INPUT_ELEMENT_DESC colorLayout[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	hr = device->CreateInputLayout( colorLayout, ARRAYSIZE( colorLayout ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mPosColor );
	if ( FAILED( hr ) )
		return false;


	mTerrainFX->Light1Tech->GetPassByIndex( 0 )->GetDesc( &passDesc );

	const D3D11_INPUT_ELEMENT_DESC terrainLayout[3] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};


	hr = device->CreateInputLayout( terrainLayout, ARRAYSIZE( terrainLayout ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mTerrainInputLayout );
	if ( FAILED( hr ) ) 
		return false;

	
	mShadowFX->TerrainBuildShadowMapTech->GetPassByIndex( 0 )->GetDesc( &passDesc );
	// Don't need a input element desc as it is the same as in Terrain.fx

	hr = device->CreateInputLayout( terrainLayout, ARRAYSIZE( terrainLayout ), passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mTerrainShadowInputLayout );
	if ( FAILED( hr ) ) 
		return false; 

	return true;
}

void CEffectManager::Shutdown()
{

}

bool CEffectManager::SetBasicFX(const CBasicEffectRenderInfo& info,ID3D11DeviceContext* context)const
{
	Checkh(mBasicFX->SetEyePosW(info.eyePosW));
	Checkh(mBasicFX->SetTexTransform(info.texTransform));
	Checkh(mBasicFX->SetWorld(info.world));
	Checkh(mBasicFX->SetWorldInvTranspose(SMathHelper::InverseTranspose(info.world)));
	Checkh(mBasicFX->SetWorldViewProj(info.world*info.view*info.proj));
	/////////////////////////////////////////////////////////////////
	//WARNING WE ARE NOT SETTING THE WORLD VIEW PROJ TEX////////////
	///////////////////////////////////////////////////////////////
	Checkh(mBasicFX->SetWorldViewProjTex(info.world));
	
	return true;
}

D3DX11_PASS_DESC CEffectManager::GetBasicFXPassDesc()
{ 
	D3DX11_PASS_DESC passDesc;
	mBasicFX->Light1TexTech->GetPassByIndex(0)->GetDesc(&passDesc);
	return passDesc;
}

bool CEffectManager::SetBasicFXDiffuseMap(ID3D11ShaderResourceView* diffuseSRV)const
{
	Checkh(mBasicFX->SetDiffuseMap(diffuseSRV));
	return true;
}

bool CEffectManager::SetBasicFXHasTexture(bool hasTexture)const
{
	if(hasTexture)
	{
		Checkh(mBasicFX->SetHasTexture(1.0f));
	}
	else
	{
		Checkh(mBasicFX->SetHasTexture(0.0f));
	}
	return true;
}

bool CEffectManager::SetBasicFXMaterial(Material mat)const
{
	Checkh(mBasicFX->SetMaterial(mat));
	return true;
}

ID3DX11EffectTechnique* CEffectManager::GetTechnique( RenderTech renderTech, bool usesNormalMap, bool renderShadows, bool reflection, bool fog )const
{
	// Very repetitive... but I didn't care.
	//TODO:
	// Clean this up a bit.
	switch(renderTech)
	{
	case eBASIC:
		if ( renderShadows )
		{
			if ( usesNormalMap )
			{
				if ( fog )
				{
					return mNormalMapFX->LightTexShadowFogTech;
				}
				else
				{
					return mNormalMapFX->LightTexShadowTech;
				}
			}
			else
			{
				if ( reflection )
				{
					if ( fog )
					{
						return mBasicFX->LightTexShadowReflectionFogTech;
					}
					else
					{
						return mBasicFX->LightTexShadowReflection;
					}
				}
				else
				{
					if ( fog )
					{
						return mBasicFX->LightTexShadowFogTech;
					}
					else
					{
						return mBasicFX->LightTexShadowTech;
					}
				}
			}
			
		}
		else
		{
			if ( usesNormalMap )
			{
				return mNormalMapFX->LightTexTech;
			}
			else
			{
				return mBasicFX->Light1TexTech;
			}
		}
		break;

	case eBASIC_NO_DIR_LIGHT:
		if ( usesNormalMap )
			return mNormalMapFX->NoDirLightTexShadowFogTech;
		else
			return mBasicFX->NoDirLightTexShadowFogTech;
		break;
	case eDISPLACEMENT_MAPPING:
		return mNormalMapFX->DisplacementMappingTech;
		break;
	default:
		return mBasicFX->Light1TexTech;
		break;
	}
}

bool CEffectManager::SetAllLighting(const CSceneLighting lightingInfo,CSkinEffect* skinEffect)const
{
	PointLight pointLights[MAX_NUMBER_OF_POINT_LIGHTS];
	for(int i = 0; i < lightingInfo.pointLights.size(); ++i)
	{
		pointLights[i] = lightingInfo.pointLights.at(i);
	}

	if ( lightingInfo.dirLights.size() > MAX_NUMBER_OF_DIR_LIGHTS  )
	{
		DebugWriter::WriteToDebugFile( "The number of dir lights in lighting info is greater then the max number of dir lights!","bool CEffectManager::SetAllLighting(const CSceneLighting lightingInfo)const" );
		return false;
	}

	int numPointLights = lightingInfo.pointLights.size();

	HRESULT hr = mBasicFX->SetNumberOfPointLights(numPointLights);
	Checkh( hr );
	hr = mBasicFX->SetPointLights(pointLights);
	Checkh( hr );
	hr = mBasicFX->SetDirLight( &lightingInfo.dirLights[0] );
	Checkh( hr );

	/*hr = mWaterFX->SetNumberOfPointLights( numPointLights );
	Checkh( hr );
	if ( numPointLights > 0 )
	{
		hr = mWaterFX->SetPointLights( &lightingInfo.pointLights[0] );
		Checkh( hr );
	}*/
	

	hr = mNormalMapFX->SetDirLight( &lightingInfo.dirLights[0] );
	Checkh( hr );
	hr = mNormalMapFX->SetNumberOfPointLights(numPointLights);
	Checkh( hr );
	hr = mNormalMapFX->SetPointLights(pointLights);
	Checkh( hr );

	hr = mRefractFX->SetDirectionalLighting( &lightingInfo.dirLights[0] );
	Checkh( hr );

	

	mTerrainFX->SetDirLights( &lightingInfo.dirLights[0] );


	/*hr = skinEffect->DirectionalLight->SetRawValue( &lightingInfo.dirLights[0], 0, sizeof( DirectionalLight ) );
	Checkh( hr );
	if ( numPointLights > 0 )
	{
		hr = skinEffect->PointLights->SetRawValue( &lightingInfo.pointLights[0], 0, sizeof( PointLight ) * MAX_NUMBER_OF_POINT_LIGHTS );
		Checkh( hr );
	}*/
	
	return true;
}

bool CEffectManager::SetNormalMappingDiffuseMap(ID3D11ShaderResourceView* srv)const
{
	Checkh(mNormalMapFX->SetDiffuseMap(srv));
	return true;
}

bool CEffectManager::SetNormalMappingFX(const CBasicEffectRenderInfo& e,NormalMappingExtraInfo& dpmExtra,ID3D11DeviceContext* context)const
{
	Checkh(mNormalMapFX->SetEyePosW(e.eyePosW));
	Checkh(mNormalMapFX->SetTexTransform(e.texTransform));
	Checkh(mNormalMapFX->SetWorld(e.world));
	Checkh(mNormalMapFX->SetWorldInvTranspose(SMathHelper::InverseTranspose(e.world)));
	Checkh(mNormalMapFX->SetWorldViewProj(e.world * e.view * e.proj));
	Checkh(mNormalMapFX->SetHeightScale(dpmExtra.mHeightScale));
	Checkh(mNormalMapFX->SetMaxTessDistance(dpmExtra.mMaxTessD));
	Checkh(mNormalMapFX->SetMinTessDistance(dpmExtra.mMinTessD));
	Checkh(mNormalMapFX->SetMaxTessFactor(dpmExtra.mMaxTessF));
	Checkh(mNormalMapFX->SetMinTessFactor(dpmExtra.mMinTessF));
	Checkh(mNormalMapFX->SetViewProj(XMMatrixMultiply(e.view,e.proj)));

	return true;
}

bool CEffectManager::SetNormalMappingNormalMap(ID3D11ShaderResourceView* srv)const
{
	Checkh(mNormalMapFX->SetNormalMap(srv));
	return true;
}

bool CEffectManager::SetNormalMappingMaterial(Material mat)const
{
	Checkh(mNormalMapFX->SetMaterial(mat));
	return true;
}

bool CEffectManager::SetNormalMappingHasTexture(bool hasTexture)const
{
	Checkh(mNormalMapFX->SetUsesTexture(hasTexture));
	return true;
}

D3DX11_PASS_DESC CEffectManager::GetNormalMapFXPassDesc()
{
	D3DX11_PASS_DESC passDesc;
	mNormalMapFX->LightTexTech->GetPassByIndex(0)->GetDesc(&passDesc);
	return passDesc;
}

void CEffectManager::DestroyResources()
{
	//SkinnedEffectAccess.Shutdown();
	BlurEffectAccess.Shutdown();
	ReleaseCOM( mTerrainShadowInputLayout );
	ReleaseCOM( mTerrainInputLayout );
	ReleaseCOM( mPosColor );
	ReleaseCOM( mPos );
	ReleaseCOM( mParticleInputLayout );
	ReleaseCOM( mPosNormTexTan );
	ReleaseCOM( mPosNormTex );
	SafeDelete( mTerrainFX );
	SafeDelete( mColorFX );
	SafeDelete( mWaterFX );
	SafeDelete( mRefractFX );
	SafeDelete( mSkyFX );
	SafeDelete( mShadowFX );
	SafeDelete( mSparkFX );
	SafeDelete( mFireFX );
	SafeDelete( mBasicFX );
	SafeDelete( mNormalMapFX );
}




bool CEffectManager::SetDrawFullScreenQuad(ID3D11ShaderResourceView* appliedSRV)
{
	DXUTGetD3D11DeviceContext()->IASetInputLayout(mPosNormTex);
	HRESULT hr;
	XMMATRIX world = XMMatrixIdentity();

	hr = mBasicFX->SetWorld(world);
	Checkh(hr);
	hr = mBasicFX->SetWorldInvTranspose(world);
	Checkh(hr);
	hr = mBasicFX->SetWorldViewProj(world);
	Checkh(hr);
	hr = mBasicFX->SetTexTransform(world);
	Checkh(hr);
	hr = mBasicFX->SetDiffuseMap(appliedSRV);
	Checkh(hr);

	return true;
}


void CEffectManager::SetInputLayout(InputLayoutType ilt)const
{
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	switch( ilt ) 
	{
	case InputLayoutType::Pos:
		context->IASetInputLayout( mPos );
		break;
	case InputLayoutType::PosNormTex:
		context->IASetInputLayout( mPosNormTex );
		break;
	case InputLayoutType::PosColor:
		context->IASetInputLayout( mPosColor );
		break;
	case InputLayoutType::TerrainIL:
		context->IASetInputLayout( mTerrainInputLayout );
		break;
	case InputLayoutType::TerrainShadowIL:
		context->IASetInputLayout( mTerrainShadowInputLayout );
		break;
	case InputLayoutType::ParticleIL:
		context->IASetInputLayout( mParticleInputLayout );
		break;
	case InputLayoutType::PosNormTexTan:
		context->IASetInputLayout( mPosNormTexTan );
		break;
	}
}


void CEffectManager::SetPerFrame(ScenePerFrameInfo& spfi, std::vector<ID3D11ShaderResourceView*>& shadowMaps, CSkinEffect* skinEffect)
{
	// Set all of basic's per frame info.
	mBasicFX->SetShadowMap( shadowMaps );
	mBasicFX->SetCubeMap( spfi.pCubeMap );
	mBasicFX->SetFogColor( spfi.FogColor );
	mBasicFX->SetFogRange( spfi.FogRange );
	mBasicFX->SetFogStart( spfi.FogStart );

	// Set all of normal map's per frame info.
	mNormalMapFX->SetShadowMap( shadowMaps );
	mNormalMapFX->SetFogColor( spfi.FogColor );
	mNormalMapFX->SetFogRange( spfi.FogRange );
	mNormalMapFX->SetFogStart( spfi.FogStart );

	// Set all of skin effect's per frame info.
	skinEffect->SetShadowMap( shadowMaps );
}