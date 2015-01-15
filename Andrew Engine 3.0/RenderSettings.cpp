//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "RenderSettings.h"


ID3D11DepthStencilState*	SRenderSettings::mDefaultDSS			= 0;
ID3D11BlendState*			SRenderSettings::mDefaultBS				= 0;
ID3D11BlendState*			SRenderSettings::mTransparencyBS		= 0;
ID3D11RasterizerState*		SRenderSettings::m_pNoCullRS			= 0;
RasterizerState				SRenderSettings::m_RasterizerState		= RasterizerState::DefaultRS;
BlendState					SRenderSettings::m_BlendState			= BlendState::DefaultBS;
DepthStencilState			SRenderSettings::m_DepthStencilState	= DepthStencilState::DefaultDSS;

bool SRenderSettings::InitAllSettings(ID3D11Device* device)
{
	HRESULT hr;
	D3D11_DEPTH_STENCIL_DESC ds;
	ds.DepthEnable						= TRUE;
	ds.DepthWriteMask					= D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc						= D3D11_COMPARISON_LESS;
	ds.StencilEnable					= FALSE;
	ds.StencilReadMask					= D3D11_DEFAULT_STENCIL_READ_MASK;
	ds.StencilWriteMask					= D3D11_DEFAULT_STENCIL_WRITE_MASK;
	ds.FrontFace.StencilFunc			= D3D11_COMPARISON_ALWAYS;
	ds.BackFace.StencilFunc				= D3D11_COMPARISON_ALWAYS;
	ds.FrontFace.StencilDepthFailOp		= D3D11_STENCIL_OP_KEEP;
	ds.BackFace.StencilDepthFailOp		= D3D11_STENCIL_OP_KEEP;
	ds.FrontFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	ds.BackFace.StencilPassOp			= D3D11_STENCIL_OP_KEEP;
	ds.FrontFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	ds.BackFace.StencilFailOp			= D3D11_STENCIL_OP_KEEP;
	
	hr = device->CreateDepthStencilState(&ds,&mDefaultDSS);
	Checkh(hr);

	D3D11_BLEND_DESC bd;
	bd.AlphaToCoverageEnable					= FALSE;
	bd.IndependentBlendEnable					= FALSE;
	bd.RenderTarget[0].BlendEnable				= FALSE;
	bd.RenderTarget[0].SrcBlend					= D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlend				= D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOp					= D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha			= D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha				= D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(&bd,&mDefaultBS);
	Checkh(hr);

	D3D11_BLEND_DESC transparentDesc = {0};
	transparentDesc.AlphaToCoverageEnable = false;
	transparentDesc.IndependentBlendEnable = false;

	transparentDesc.RenderTarget[0].BlendEnable = true;
	transparentDesc.RenderTarget[0].SrcBlend       = D3D11_BLEND_SRC_ALPHA;
	transparentDesc.RenderTarget[0].DestBlend      = D3D11_BLEND_INV_SRC_ALPHA;
	transparentDesc.RenderTarget[0].BlendOp        = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].SrcBlendAlpha  = D3D11_BLEND_ONE;
	transparentDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	transparentDesc.RenderTarget[0].BlendOpAlpha   = D3D11_BLEND_OP_ADD;
	transparentDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(&transparentDesc,&mTransparencyBS);
	Checkh(hr);

	D3D11_RASTERIZER_DESC noCullDesc;
	ZeroMemory(&noCullDesc,sizeof(noCullDesc));
	noCullDesc.FillMode = D3D11_FILL_SOLID;
	noCullDesc.CullMode = D3D11_CULL_NONE;
	noCullDesc.FrontCounterClockwise = false;
	noCullDesc.DepthClipEnable = true;

	hr = device->CreateRasterizerState( &noCullDesc, &m_pNoCullRS );
	

	return true;
}

void SRenderSettings::DestroyAllSettings()
{
	ReleaseCOM( m_pNoCullRS );
	ReleaseCOM( mTransparencyBS );
	ReleaseCOM( mDefaultBS );
	ReleaseCOM( mDefaultDSS );
}
