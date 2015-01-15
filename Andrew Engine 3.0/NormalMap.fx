#include "LightHelper.fx"


cbuffer cbPerFrame
{
	DirectionalLight gDirLight;
	PointLight gPointLights[12];
	float3 gEyePosW;

	float4x4 gShadowTransform[12];

	float  gFogStart;
	float  gFogRange;
	float4 gFogColor;
	int gNumberOfPointLights;
	float gHeightScale;
	float gMaxTessDistance;
	float gMinTessDistance;
	float gMinTessFactor;
	float gMaxTessFactor;
};

cbuffer cbPerObject
{
	float4x4 gWorld;
	float4x4 gViewProj;
	float4x4 gWorldInvTranspose;
	float4x4 gWorldViewProj;
	float4x4 gTexTransform;
	Material gMaterial;
	int gUsesTexture;
	bool g_bFlashing;
}; 

// Nonnumeric values cannot be added to a cbuffer.
Texture2D gDiffuseMap;
Texture2D gNormalMap;
Texture2D gShadowMap[12];


// States
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
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
	float3 TangentL : TANGENT;
};

struct VertexOutTess
{
	float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
	float3 TangentW   : TANGENT;
	float2 Tex        : TEXCOORD;
	float  TessFactor : TESS;
	float3 PosL		  : POSITION_LOCAL;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
	float4 ShadowPosH[12] : TEXCOORD1;
};






VertexOutTess VSTess(VertexIn vin)
{
	VertexOutTess vout;
	
	// Transform to world space space.
	vout.PosW     = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW  = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);

	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul(float4(vin.Tex, 0.0f, 1.0f), gTexTransform).xy;
	
	float d = distance(vout.PosW, gEyePosW);

	// Normalized tessellation factor. 
	// The tessellation is 
	//   0 if d >= gMinTessDistance and
	//   1 if d <= gMaxTessDistance.  
	float tess = saturate( (gMinTessDistance - d) / (gMinTessDistance - gMaxTessDistance) );
	
	// Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
	vout.TessFactor = gMinTessFactor + tess*(gMaxTessFactor-gMinTessFactor);

	vout.PosL = vin.PosL;

	return vout;
}


struct PatchTess
{
	float EdgeTess[3] : SV_TessFactor;
	float InsideTess  : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<VertexOutTess,3> patch, 
                  uint patchID : SV_PrimitiveID)
{
	PatchTess pt;
	
	// Average tess factors along edges, and pick an edge tess factor for 
	// the interior tessellation.  It is important to do the tess factor
	// calculation based on the edge properties so that edges shared by 
	// more than one triangle will have the same tessellation factor.  
	// Otherwise, gaps can appear.
	pt.EdgeTess[0] = 0.5f*(patch[1].TessFactor + patch[2].TessFactor);
	pt.EdgeTess[1] = 0.5f*(patch[2].TessFactor + patch[0].TessFactor);
	pt.EdgeTess[2] = 0.5f*(patch[0].TessFactor + patch[1].TessFactor);
	pt.InsideTess  = pt.EdgeTess[0];
	
	return pt;
}

struct HullOut
{
	float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
	float3 PosL		: POSITION_LOCAL;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<VertexOutTess,3> p, 
           uint i : SV_OutputControlPointID,
           uint patchId : SV_PrimitiveID)
{
	HullOut hout;
	
	// Pass through shader.
	hout.PosW     = p[i].PosW;
	hout.NormalW  = p[i].NormalW;
	hout.TangentW = p[i].TangentW;
	hout.Tex      = p[i].Tex;
	hout.PosL	  = p[i].PosL;
	
	return hout;
}


[domain("tri")]
VertexOut DS(PatchTess patchTess, 
             float3 bary : SV_DomainLocation, 
             const OutputPatch<HullOut,3> tri)
{
	VertexOut dout;
	
	// Interpolate patch attributes to generated vertices.
	dout.PosW     = bary.x*tri[0].PosW     + bary.y*tri[1].PosW     + bary.z*tri[2].PosW;
	dout.NormalW  = bary.x*tri[0].NormalW  + bary.y*tri[1].NormalW  + bary.z*tri[2].NormalW;
	dout.TangentW = bary.x*tri[0].TangentW + bary.y*tri[1].TangentW + bary.z*tri[2].TangentW;
	dout.Tex      = bary.x*tri[0].Tex      + bary.y*tri[1].Tex      + bary.z*tri[2].Tex;
	
	// Interpolating normal can unnormalize it, so normalize it.
	dout.NormalW = normalize(dout.NormalW);
	
	//
	// Displacement mapping.
	//
	
	// Choose the mipmap level based on distance to the eye; specifically, choose
	// the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
	const float MipInterval = 20.0f;
	float mipLevel = clamp( (distance(dout.PosW, gEyePosW) - MipInterval) / MipInterval, 0.0f, 6.0f);
	
	// Sample height map (stored in alpha channel).
	float h = gNormalMap.SampleLevel(samLinear, dout.Tex, mipLevel).a;
	
	// Offset vertex along normal.
	dout.PosW += (gHeightScale*(h-1.0))*dout.NormalW;
	
	// Project to homogeneous clip space.
	dout.PosH = mul(float4(dout.PosW, 1.0f), gViewProj);

	for ( int i = 0; i < 12; ++i )
		dout.ShadowPosH[i] = mul( float4( tri[0].PosL, 1.0f ), gShadowTransform[i] );	
	
	return dout;
}




VertexOut VS(VertexIn vin)
{
	VertexOut vout;
	
	// Transform to world space space.
	vout.PosW     = mul(float4(vin.PosL, 1.0f), gWorld).xyz;
	vout.NormalW  = mul(vin.NormalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, (float3x3)gWorld);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f), gWorldViewProj);
	
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = mul( float4( vin.Tex, 0.0f, 1.0f ), gTexTransform ).xy;

	for ( int i = 0; i < 12; ++i )
		vout.ShadowPosH[i] = mul( float4( vin.PosL,1.0f ), gShadowTransform[i] );	

	return vout;
}
 
float4 PS(VertexOut pin, uniform bool shadowMap, uniform bool fogEnabled, uniform bool useDirLight) : SV_Target
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
    if( gUsesTexture == 1 )
	{
		// Sample texture.
		texColor = gDiffuseMap.Sample( samLinear, pin.Tex );
	}

	//
	// Normal mapping
	//

	float3 normalMapSample = gNormalMap.Sample(samLinear, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);
	Material mat;
	mat.Ambient  = float4(0.8f, 0.8f, 0.8f, 1.0f);
	mat.Diffuse  = float4(1.0f, 1.0f, 1.0f, 1.0f);
	mat.Specular = float4(0.4f, 0.4f, 0.4f, 16.0f);
	mat.Reflect  = float4(0.0f, 0.0f, 0.0f, 1.0f);


	//
	// Lighting.
	//

	float4 litColor = texColor;

	// Start with a sum of zero. 
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec    = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float shadowFactors[12] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
	if ( shadowMap )
	{
		for ( int i = 0; i < 12; ++i )
			shadowFactors[i] = CalcTheShadowFactor( samShadow, gShadowMap[i], pin.ShadowPosH[i]) * 2.0f;
	}
	
	if ( useDirLight ) 
	{
		//float4 A,D,S;
		//ComputeDirectionalLight( mat, gDirLight, bumpedNormalW, toEye, A, D, S );
			
		//ambient += A;
		//diffuse += shadowFactor * D;
		//spec	+= shadowFactor * S;
	}
	

	// Sum the light contribution from each light source.  
	[unroll]
	for(int i = 0; i < gNumberOfPointLights; ++i)
	{
		float4 A, D, S;
		ComputePointLight(mat, gPointLights[i], pin.PosW, bumpedNormalW, toEye, 
			A, D, S);

		ambient += A;
		diffuse += D * shadowFactors[i];
		spec    += S * shadowFactors[i];
	}

	litColor = texColor*(ambient + diffuse) + spec;


	if( fogEnabled )
	{
		float fogLerp = saturate( (distToEye - gFogStart) / gFogRange ); 

		// Blend the fog color and the lit color.
		litColor = lerp(litColor, gFogColor, fogLerp);
	}

	if ( g_bFlashing )
	{
		return float4( 0.0f, 1.0f, 0.0f, 1.0f );
	}

	// Common to take alpha from diffuse material and texture.
	litColor.a = mat.Diffuse.a * texColor.a;


    return litColor;
}

technique11 LightTexShadowFogTech
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_5_0, PS( true, true, true ) ) );
	}
}

technique11 LightTexTech
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_5_0, PS( false, false, true ) ) );
	}
}

technique11 LightTexShadowTech
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_5_0, PS( true, false, true ) ) );
	}
}

technique11 NoDirLightTexShadowFogTech
{
	pass P0
	{
		SetVertexShader( CompileShader(vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader(ps_5_0, PS( true, true, false ) ) );
	}
}

technique11 DisplacementMappingTech
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_5_0, VSTess() ) );
		SetHullShader( CompileShader(hs_5_0, HS() ) );
		SetDomainShader( CompileShader(ds_5_0, DS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS( true, false, true ) ) );
	}
}