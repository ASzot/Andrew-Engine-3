#include "LightHelper.fx"

cbuffer MatrixBuffer
{
	matrix g_mWorld;
	matrix g_mView;
	matrix g_mProj;
	matrix g_mWorldViewProj;
	matrix g_mReflProjWorld;
	matrix g_mViewProjWorld;

	matrix g_mReflectionMatrix;
};

cbuffer LightBuffer
{
	float3 g_f3EyePos;
	float g_fDiffuseAbsorbity;
	float g_fSpecularAbsorbity;
	
	int g_iNumOfPointLights;
};

cbuffer Rand
{
	PointLight gPointLights[36];
};

cbuffer WaterBuffer
{
	float g_fWaterTranslation;
	float g_fReflectionRefractScale;
	float2 Padding;
};

Texture2D g_txReflectionTexture;
Texture2D g_txRefractionTexture;
Texture2D g_txNormalTexture;

SamplerState g_ssSampleType
{
	Filter		= MIN_MAG_MIP_LINEAR;
	AddressU	= WRAP;
	AddressV	= WRAP;
};

struct VertexIn
{
	float3 Position    : POSITION;
	float3 Normal : NORMAL;
	float2 Tex     : TEXCOORD;
};

struct VertexOut
{
	float4 Position : SV_POSITION;
	float3 PosW : POSITION;
	float2 Tex : TEXCOORD0;
	float4 ReflectionPosition : TEXCOORD1;
	float4 RefractionPosition : TEXCOORD2;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	//vin.Position.w = 1.0f;

	vout.Position = mul( float4( vin.Position, 1.0f ), g_mWorldViewProj );

	// Just reproject the texture coordinate.
	vout.Tex = vin.Tex;

	// Create the reflection matricies.

	vout.ReflectionPosition = mul( float4( vin.Position, 1.0f ), g_mReflProjWorld );

	vout.RefractionPosition = mul( float4( vin.Position, 1.0f ), g_mViewProjWorld );

	vout.PosW = mul( float4( vin.Position, 1.0f ), g_mWorld );

	return vout;
};

float4 WaterPixelShader( VertexOut pin, uniform bool reflectTex ) : SV_Target
{
	float2 reflectTexCoord;
	float2 refractTexCoord;
	float4 normalMap;
	float3 normal;
	float4 reflectionColor;
	float4 refractionColor;
	float4 color;

	pin.Tex.y += g_fWaterTranslation;

	refractTexCoord.x = pin.RefractionPosition.x / pin.RefractionPosition.w / 2.0f + 0.5f;
	refractTexCoord.y = -pin.RefractionPosition.y / pin.RefractionPosition.w / 2.0f + 0.5f;

	normalMap = g_txNormalTexture.Sample( g_ssSampleType, pin.Tex );
	
	normal = (normalMap.xyz * 2.0f ) - 1.0f;

	Material mat;
	mat.Ambient  = float4(0.8f, 0.8f, 0.8f, 1.0f);
	mat.Diffuse  = float4(1.0f, 1.0f, 1.0f, 1.0f);
	mat.Specular = float4(0.4f, 0.4f, 0.4f, 16.0f);
	mat.Reflect  = float4(0.0f, 0.0f, 0.0f, 1.0f);

	/////////////
	// Lighting:

	float4 ambient	= float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 diffuse	= float4( 0.0f, 0.0f, 0.0f, 0.0f );
	float4 spec		= float4( 0.0f, 0.0f, 0.0f, 0.0f );

	float3 toEye = g_f3EyePos - pin.PosW;
	float distToEye = length( toEye );
	toEye /= distToEye;

	[unroll]
	for ( int i = 0; i < g_iNumOfPointLights; ++i )
	{
		float4 A, D, S;
		ComputePointLight( mat, gPointLights[i], pin.PosW, normal, toEye, 
			A, D, S );

		ambient += A;
		diffuse += D;
		spec    += S;
	}


	// Re-position texture based on normal map.
	refractTexCoord = refractTexCoord + (normal.xy * g_fReflectionRefractScale);
	
	refractionColor = g_txRefractionTexture.Sample( g_ssSampleType, refractTexCoord );

	if ( reflectTex )
	{
		reflectTexCoord.x = pin.ReflectionPosition.x / pin.ReflectionPosition.w / 2.0f + 0.5f;
		reflectTexCoord.y = -pin.ReflectionPosition.y / pin.ReflectionPosition.w / 2.0f + 0.5f;

		reflectTexCoord = reflectTexCoord + (normal.xy * g_fReflectionRefractScale);
		reflectionColor = g_txReflectionTexture.Sample( g_ssSampleType, reflectTexCoord );

		color = lerp( reflectionColor, refractionColor, 0.6f );
	}
	else
	{
		color = refractionColor;
	}

	float4 litColor = color + ( ( diffuse / g_fDiffuseAbsorbity ) + ( spec / g_fSpecularAbsorbity ) );

	return litColor;
}

technique11 WaterReflectRefractTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, WaterPixelShader( true ) ) );
	}
}

technique11 WaterRefractTech
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, WaterPixelShader( false ) ) );
	}
}