
#include "LightHelper.fx"

// Globals.
cbuffer MatrixBuffer
{
	matrix g_mWorld;
	matrix g_mView;
	matrix g_mProj;
	matrix g_mWorldViewProj;
	matrix g_mWorldInvTranspose;
};

cbuffer PerFrame
{
	// Not exactly the fanciest lighting setup, but it will get the job done.
//TODO(ADV):
// Support the refraction of light.
	DirectionalLight g_DirLight;
	float3 g_f3EyePos;
};

Texture2D g_txShaderTexture;

SamplerState g_ssSampleType
{
	Filter			= ANISOTROPIC;
	MaxAnisotropy	= 4;

	AddressU		= WRAP;
	AddressV		= WRAP;
};

cbuffer ClipBuffer
{
	float4 m_f4ClipPlane;
};

struct VertexIn
{
	float3 Position : POSITION;
	float3 Normal	: NORMAL;
	float2 Tex		: TEXCOORD;
};

struct VertexOut
{
	float4 Position : SV_POSITION;
	float3 PosW		: POSITION;
	float2 Tex		: TEXCOORD0;
	float3 Normal	: NORMAL;
	float Clip		: SV_ClipDistance0;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;
	vout.Position.w = 1.0f;

	vout.Position = mul( float4( vin.Position, 1.0f ), g_mWorldViewProj );

	vout.PosW = mul( float4( vin.Position, 1.0f ), g_mWorld ).xyz;

	// Re-project the texture coordinates.
	vout.Tex = vin.Tex;

	// Normalize the vector.
	vout.Normal = mul( vin.Normal, (float3x3)g_mWorld );

	vout.Clip = dot ( mul ( float4( vin.Position, 1.0f ), g_mWorld ), m_f4ClipPlane );

	return vout;
}


float4 PS( VertexOut pin ) : SV_Target
{
	pin.Normal = normalize( pin.Normal );

	float4 textureColor;
	float lightIntensity;
	float4 color;
	

	// Sample the texture pixel at this location.
	textureColor = g_txShaderTexture.Sample(g_ssSampleType, pin.Tex);
	
	// Set the default output color to the ambient light value for all pixels.
    color = g_DirLight.Ambient;

	// Invert the light direction for calculations.
	float3 lightDir = -g_DirLight.Direction;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(pin.Normal, lightDir));

	if(lightIntensity > 0.0f)
    {
        // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
        color += (g_DirLight.Diffuse * lightIntensity);
    }

	// Saturate the final light color.
	color = saturate(color);

	// Multiply the texture pixel and the input color to get the final result.
	color = color * textureColor;
	
	return color;
}

technique11 RefractWithBasicLighting
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
	}
}