//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "RenderTextureClass.h"


CRenderTexture::CRenderTexture(void)
{
	m_pRenderTargetView = 0;
	m_pRenderTargetTexture = 0;
	m_pSRV = 0;
}


CRenderTexture::~CRenderTexture(void)
{
}


bool CRenderTexture::Initialize( int textureHeight, int textureWidth )
{
	ID3D11Device* device = DXUTGetD3D11Device();
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;


	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, NULL, &m_pRenderTargetTexture);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(m_pRenderTargetTexture, &renderTargetViewDesc, &m_pRenderTargetView);
	if(FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(m_pRenderTargetTexture, &shaderResourceViewDesc, &m_pSRV);
	if(FAILED(result))
	{
		return false;
	}

	return true;
}
	
void CRenderTexture::DestroyResources()
{
	ReleaseCOM( m_pSRV );
	ReleaseCOM( m_pRenderTargetView );
	ReleaseCOM( m_pRenderTargetTexture );
}

///<summary>Clears the render texture with the appropriate color and dsv.<summary>
void CRenderTexture::ClearRenderTarget( float cr, float rg, float rb, float ra, ID3D11DepthStencilView* dsv )
{
	float color[4] = { cr, rg, rb, ra };

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	
	context->ClearRenderTargetView( m_pRenderTargetView, color );

	context->ClearDepthStencilView( dsv, D3D11_CLEAR_DEPTH, 1.0f, 0 );
}

void CRenderTexture::SetRenderTarget( ID3D11DepthStencilView* dsv )
{
	DXUTGetD3D11DeviceContext()->OMSetRenderTargets( 1, &m_pRenderTargetView, dsv );	
}
	
ID3D11ShaderResourceView* CRenderTexture::GetShaderResourceView()
{
	return m_pSRV;
}

void CRenderTexture::Resize( UINT width, UINT height )
{
	ReleaseCOM( m_pSRV );
	ReleaseCOM( m_pRenderTargetView );
	ReleaseCOM( m_pRenderTargetTexture );
	
	Initialize( height, width );
}