cbuffer cbPerFrame
{
	float  g_fTime;   
	float3 g_LightDir;
	float3 g_vEyePt;
	float3 g_vRight;
	float3 g_vUp;
	float3 g_vForward;
	float4x4 g_mWorldViewProjection;   
	float4x4 g_mInvViewProj;
	float4x4 g_mWorld;
};

cbuffer cbglowlights
{
	uint   g_NumGlowLights;
	float4 g_vGlowLightPosIntensity[MAX_GLOWLIGHTS];
	float4 g_vGlowLightColor[MAX_GLOWLIGHTS];
	
	float3  g_vGlowLightAttenuation;
};

//--------------------------------------------------------------------------------------
// Texture samplers
//--------------------------------------------------------------------------------------
Texture2D g_txMeshTexture;          // Color texture for mesh
SamplerState g_samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------

DepthStencilState DepthRead
{
    DepthEnable = TRUE;
    DepthWriteMask = 0;
    DepthFunc = LESS_EQUAL;
};


BlendState ForwardBlending
{
	AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = TRUE;
    BlendEnable[1] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
    SrcBlendAlpha = ZERO;
    DestBlendAlpha = ZERO;
    BlendOpAlpha = ADD;
    RenderTargetWriteMask[0] = 0x0F;
    RenderTargetWriteMask[1] = 0x0F;
};

RasterizerState RSSolid
{
	FillMode = Solid;
};

//--------------------------------------------------------------------------------------
// Vertex shader output structure
//--------------------------------------------------------------------------------------
struct VS_PARTICLEINPUT
{
	float4 Position   : POSITION;
	float2 TextureUV  : TEXCOORD0;
	float  fLife      : LIFE; 
	float  fRot	      : THETA;
	float4 Color	  : COLOR0;
};


struct VS_PARTICLEOUTPUT
{
    float4 Position   : SV_POSITION; // vertex position 
    float3 TextureUVI : TEXCOORD0;   // vertex texture coords
    float3 SinCosThetaLife : TEXCOORD1;
    float4 Color	  : COLOR0;
};


//--------------------------------------------------------------------------------------
// Render particle information into the particle buffer
//--------------------------------------------------------------------------------------
VS_PARTICLEOUTPUT RenderParticlesVS( VS_PARTICLEINPUT input )
{
    VS_PARTICLEOUTPUT Output;
    
    // Standard transform
    Output.Position = mul(input.Position, g_mWorldViewProjection);
    Output.TextureUVI.xy = input.TextureUV; 
    Output.Color = input.Color;
    
    // Get the world position
    float3 WorldPos = mul( input.Position, g_mWorld ).xyz;

	// Loop over the glow lights (from the explosions) and light our particle
	float runningintensity = 0;
	uint count = g_NumGlowLights;
	for( uint i=0; i<count; i++ )
	{
		float3 delta = g_vGlowLightPosIntensity[i].xyz - WorldPos;
		float distSq = dot(delta,delta);
		float3 d = float3(1,sqrt(distSq),distSq);
		
		float fatten = 1.0 / dot( g_vGlowLightAttenuation, d );
		
		float intensity = fatten * g_vGlowLightPosIntensity[i].w * g_vGlowLightColor[i].w;
		runningintensity += intensity;
		Output.Color += intensity * g_vGlowLightColor[i];
	}
	Output.TextureUVI.z = runningintensity;
    
    // Rotate our texture coordinates
    float fRot = -input.fRot;
    Output.SinCosThetaLife.x = sin( fRot );
    Output.SinCosThetaLife.y = cos( fRot );
    Output.SinCosThetaLife.z = input.fLife;
    
    return Output;    
}


//--------------------------------------------------------------------------------------
// Render particle information into the screen
//--------------------------------------------------------------------------------------
float4 RenderParticlesPS( VS_PARTICLEOUTPUT input ) : SV_TARGET
{ 	
	float4 diffuse = g_txMeshTexture.Sample( g_samLinear, input.TextureUVI.xy );
	
	// unbias
	float3 norm = diffuse.xyz * 2 - 1;
	
	// rotate
	float3 rotnorm;
	float fSinTheta = input.SinCosThetaLife.x;
	float fCosTheta = input.SinCosThetaLife.y;
	
	rotnorm.x = fCosTheta * norm.x - fSinTheta * norm.y;
	rotnorm.y = fSinTheta * norm.x + fCosTheta * norm.y;
	rotnorm.z = norm.z;
	
	// rebias
	norm = rotnorm;
	
	// Fade
	float alpha = diffuse.a * (1.0f - input.SinCosThetaLife.z);
	
	// rebias	
	float intensity = input.TextureUVI.z * alpha;
	
	// move normal into world space
    float3 worldnorm;
    worldnorm = -norm.x * g_vRight;
    worldnorm += norm.y * g_vUp;
    worldnorm += -norm.z * g_vForward;
    
    float lighting = max( 0.1, dot( worldnorm, g_LightDir ) );
    
    float3 flashcolor = input.Color.xyz * intensity;
    float3 lightcolor = input.Color.xyz * lighting;
    float3 lerpcolor = lerp( lightcolor, flashcolor, intensity );
    float4 color = float4( lerpcolor, alpha );
	
	return color;
}




//--------------------------------------------------------------------------------------
// Renders scene to render target using D3D10 Techniques
//--------------------------------------------------------------------------------------
technique11 RenderParticles
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_5_0, RenderParticlesVS( ) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_5_0, RenderParticlesPS( ) ) );

		SetBlendState( ForwardBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DepthRead, 0 );
        SetRasterizerState( RSSolid );
    }
}