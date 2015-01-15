cbuffer PerFrame
{
	matrix gProj;
	matrix gView;
	matrix gWorld;
};

DepthStencilState IgnoreDepth
{
	DepthEnable = false;
};

BlendState LineBlend
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = TRUE;
	SrcBlend = SRC_ALPHA;
	DestBlend = INV_SRC_ALPHA;
	BlendOp = ADD;
	SrcBlendAlpha = ONE;
	DestBlendAlpha = ZERO;
	BlendOpAlpha = ADD;
	RenderTargetWriteMask[0] = 0x0F;

};

struct VertexIn
{
	float3 Pos : POSITION;
	float4 Color : COLOR0;
};

struct VertexOut
{
	float4 Pos : SV_POSITION;
	float4 Color : COLOR0;
};

VertexOut VS( VertexIn vin )
{
	VertexOut vout;

	//vout.Pos = mul( vin.Pos, gWorld );
	//vout.Pos = mul( vin.Pos, gView  );
	//vout.Pos = mul( vin.Pos, gProj  );
	vout.Pos = float4(vin.Pos, 1.0f);
	vout.Color = vin.Color;
	return vout;
}

float4 PS( VertexOut pin ) : SV_TARGET
{
	return pin.Color;
}

technique11 DefaultDrawLine
{
	pass P0
	{
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PS() ) );
		SetDepthStencilState( IgnoreDepth, 0 );
		SetBlendState( LineBlend, float4(0.0f,0.0f,0.0f,0.0f), 0xffffffff);
	}
}