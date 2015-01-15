
#include "LightHelper.fx"

#define NO_SHADOW_POINT_LIGHT 9696969.123
 
cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	PointLight gPointLights[12];
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;

	bool g_bFlashing;

	int gNumberOfPointLights;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gWorldViewProjTex;
	float4x4 gTexTransform;
	float4x4 gShadowTransform[12];

	Material gMaterial;
	float	 gHasTexture;

}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gShadowMap[12];
Texture2D gDiffuseMap;

TextureCube gCubeMap;

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 4;

	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerState samLinear
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = WRAP;
	AddressV = WRAP;
};

SamplerComparisonState samShadow
{
	Filter=COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	AddressU=BORDER;
	AddressV=BORDER;
	AddressW=BORDER;
	BorderColor=float4(0.0f,0.0f,0.0f,0.0f);

	ComparisonFunc=LESS;
};
 
struct VertexIn
{
	float3 PosL    : POSITION;
	float3 NormalL : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
	float2 Tex     : TEXCOORD;
	float4 ShadowPosH[12] : TEXCOORD1;
};



VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW		= mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW	= mul(vin.NormalL, (float3x3)gWorldInvTranspose);
		
	// Transform to homogeneous clip space.
	vout.PosH		= mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex		= mul( float4( vin.Tex, 0.0f, 1.0f ), gTexTransform ).xy;

	for ( int i = 0; i < 12; ++i )
		vout.ShadowPosH[i]	= mul( float4( vin.PosL, 1.0f ), gShadowTransform[i] );


	return vout;
}
 
float4 PS(VertexOut pin, 
          uniform int gLightCount, 
		  uniform bool gUseTexure, 
		  uniform bool gAlphaClip, 
		  uniform bool gFogEnabled, 
		  uniform bool gReflectionEnabled,
		  uniform bool gShadowMapEnabled,
		  uniform bool gUseDirLight ) : SV_Target
{
	if ( g_bFlashing )
	{
		return float4( 0.0f, 1.0f, 0.0f, 1.0f );	
	}

	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if(gUseTexure && gHasTexture==1.0f)
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}
	 
	//
	// Lighting.
	//

	float4 litColor = texColor;

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);
	

	if ( gUseDirLight > 0 )
	{
		//float4 PA, PD, PS;
		//ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEye, 
		//	PA, PD, PS);

		//ambient += PA;
		//diffuse += shadowFactor * PD;
		//spec    += shadowFactor * PS;
	}

	float shadowFactors[12] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	if ( gShadowMapEnabled )
	{
		for ( int i = 0; i < 12; ++i )
			shadowFactors[i] = CalcTheShadowFactor( samShadow, gShadowMap[i], pin.ShadowPosH[i] );

	}


	// Compute the light contribution from each point light.
	[unroll]
	for(int i = 0; i < gNumberOfPointLights; ++i)
	{

		float4 A, D, S;
		ComputePointLight(gMaterial, gPointLights[i], pin.PosW, pin.NormalW, toEye, A, D, S);

		if ( gPointLights[ i ].LookAt.x == NO_SHADOW_POINT_LIGHT && gPointLights[ i ].LookAt.x == NO_SHADOW_POINT_LIGHT && gPointLights[ i ].LookAt.x == NO_SHADOW_POINT_LIGHT )
		{
			shadowFactors[ i ] = 1.0f;
		}

		ambient += A;
		diffuse += shadowFactors[i] * D;
		spec	+= shadowFactors[i] * S;
	}

	litColor = texColor*(ambient + diffuse) + spec;

	if( gReflectionEnabled )
	{
		float3 incident = -toEye;
		float3 reflectionVector = reflect(incident, pin.NormalW);
		float4 reflectionColor  = gCubeMap.Sample(samAnisotropic, reflectionVector);

		litColor += gMaterial.Reflect*reflectionColor;
	}
 
	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

float4 BasicPS(VertexOut pin, uniform int gLightCount, uniform bool gUseTexure, uniform bool gAlphaClip, uniform bool gFogEnabled) : SV_Target
{
	// Interpolating normal can unnormalize it, so normalize it.
    pin.NormalW = normalize(pin.NormalW);

	// The toEye vector is used in lighting.
	float3 toEye = gEyePosW - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;
	
    // Default to multiplicative identity.
    float4 texColor = float4(1, 1, 1, 1);
    if( gUseTexure )
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samAnisotropic, pin.Tex );

		if(gAlphaClip)
		{
			// Discard pixel if texture alpha < 0.1.  Note that we do this
			// test as soon as possible so that we can potentially exit the shader 
			// early, thereby skipping the rest of the shader code.
			clip(texColor.a - 0.1f);
		}
	}

	return texColor;
	 
	//
	// Lighting.
	//

	float4 litColor = texColor;
	if( gLightCount > 0  )
	{
		// Start with a sum of zero.
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

		// Sum the light contribution from each light source.  
		float4 A, D, S;
		ComputeDirectionalLight(gMaterial, gDirLight, pin.NormalW, toEye, 
			A, D, S);

		ambient += A;
		diffuse += D;
		spec    += S;

		// Modulate with late add.
		litColor = texColor*(ambient + diffuse) + spec;
	}

	//
	// Fogging
	//

	if( gFogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = gMaterial.Diffuse.a * texColor.a;

    return litColor;
}

technique11 NoDirLightTexShadowFog
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( 0, true, false, true, false, true, false ) ) );
	}
}

technique11 LightTexShadowReflectionFog
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( 0, true, false, true, true, true, true ) ) );
	}
}

technique11 LightTexShadowReflection
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( 0, true, false, false, true, true, true ) ) );
	}
}

technique11 LightTexShadowFog
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( 0, true, false, true, false, true, true ) ) );
	}
}

technique11 LightTexShadow
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( 0, true, false, false, false, true, true ) ) );
	}
}

technique11 Light1Tex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, PS(0, true, false, false, false, false, true ) ) );
    }
}

technique11 NoLightTex
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, BasicPS( 0, true, false, false ) ) );
	}
}