#include "LightHelper.fx"
 
cbuffer cbPerFrame
{
	PointLight gPointLights[12];
	int gNumberOfPointLights;
	DirectionalLight g_DirLight;
	float3 gEyePosW;

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor; 
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gShadowTransform[12];
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
}; 

cbuffer cbSkinned
{
	float4x4 gBoneTransforms[96];
};

// Textures.
Texture2D gDiffuseMap;
Texture2D gNormalMap;

Texture2D gShadowMap[12];


// Sampler states.
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

struct SkinnedVertexIn
{
	float3 PosL       : POSITION;
	float3 NormalL    : NORMAL;
	float2 Tex        : TEXCOORD;
	float4 TangentL   : TANGENT;
	float3 Weights    : WEIGHTS;
	uint4 BoneIndices : BONEINDICES;
};

struct VertexOut
{
	float4 PosH       : SV_POSITION;
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
	float4 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD0;
	float4 ShadowPosH[12] : TEXCOORD1;
};


VertexOut SkinnedVS(SkinnedVertexIn vin)
{
    VertexOut vout;

	// Init array or else we get strange warnings about SV_POSITION.
	float weights[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

	float3 posL     = float3(0.0f, 0.0f, 0.0f);
	float3 normalL  = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for(int i = 0; i < 4; ++i)
	{
	    // Assume no nonuniform scaling when transforming normals, so 
		// that we do not have to use the inverse-transpose.

	    posL     += weights[i]*mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]).xyz;
		normalL  += weights[i]*mul(vin.NormalL,  (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
		tangentL += weights[i]*mul(vin.TangentL.xyz, (float3x3)gBoneTransforms[vin.BoneIndices[i]]);
	}
 
	// Transform to world space space.
	vout.PosW     = mul(float4(posL, 1.0f), gWorld).xyz;
	vout.NormalW  = mul(normalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = float4(mul(tangentL, (float3x3)gWorld), vin.TangentL.w);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;

	for ( int i = 0; i < 12; ++i )
		vout.ShadowPosH[i] = mul( float4( vin.PosL,1.0f ), gShadowTransform[i] );


	return vout;
}


float4 PS(VertexOut pin, uniform bool shadowMap) : SV_Target
{
	pin.NormalW = normalize(pin.NormalW);

	float3 toEye = gEyePosW - pin.PosW;
	
	float distToEye = length(toEye);

	toEye /= distToEye;

	float4 texColor = float4(1,1,1,1);

	texColor = gDiffuseMap.Sample(samLinear, pin.Tex);

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex);
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);

	// Compute the shadow factor.
	float shadowFactors[12] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	if ( shadowMap )
	{
		for ( int i = 0; i < 12; ++i )
			shadowFactors[i] = CalcTheShadowFactor( samShadow, gShadowMap[i], pin.ShadowPosH[i] );
	}


	float4 litColor = texColor;
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float4 DA, DD, DS;
	ComputeDirectionalLight( gMaterial, g_DirLight, bumpedNormalW, toEye, DA, DD, DS );
	ambient += DA;
	diffuse += DD;
	spec	+= DS;

	[unroll]
	for(int i = 0; i < gNumberOfPointLights; ++i)
	{
		float4 A,D,S;
		ComputePointLight(gMaterial, gPointLights[i],pin.PosW, bumpedNormalW,toEye,A,D,S);

		ambient += A;
		diffuse += shadowFactors[ i ] * D;
		spec	+= shadowFactors[ i ] * S;
	}
		
	litColor = texColor*(ambient + diffuse) + spec;

	litColor.a = gMaterial.Diffuse.a * texColor.a;

	return litColor;
}

technique11 LightTexSkinned
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( false ) ) );
	}
}

technique11 BuildShadowMap
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( NULL );
	}
}

technique11 LightTexShadowSkinned
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, SkinnedVS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( true ) ) );
	}
}
