//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "ShadowMapper.h"

CShadowMapper::CShadowMapper(ID3D11Device* device, UINT width, UINT height)
: mWidth(width), mHeight(height), mDepthMapSRV(0), mDepthMapDSV(0)
{
	for ( int i = 0; i < MAX_NUMBER_OF_POINT_LIGHTS; ++i )
		CreateDSVAndSRV();
}

void CShadowMapper::CreateDSVAndSRV( )
{
	ID3D11Device* device = DXUTGetD3D11Device();
	ID3D11ShaderResourceView* tempSRV;
	ID3D11DepthStencilView* tempDSV;

	mViewport.TopLeftX = 0.0f;
    mViewport.TopLeftY = 0.0f;
    mViewport.Width    = static_cast<float>(mWidth);
    mViewport.Height   = static_cast<float>(mHeight);
    mViewport.MinDepth = 0.0f;
    mViewport.MaxDepth = 1.0f;

	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
	// the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
    texDesc.Width     = mWidth;
    texDesc.Height    = mHeight;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format    = DXGI_FORMAT_R24G8_TYPELESS;
    texDesc.SampleDesc.Count   = 1;  
    texDesc.SampleDesc.Quality = 0;  
    texDesc.Usage          = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0; 
    texDesc.MiscFlags      = 0;

    ID3D11Texture2D* depthMap = 0;
    device->CreateTexture2D(&texDesc, 0, &depthMap);

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Texture2D.MipSlice = 0;
    device->CreateDepthStencilView(depthMap, &dsvDesc, &tempDSV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    device->CreateShaderResourceView(depthMap, &srvDesc, &tempSRV);

	mDepthMapSRV.push_back( tempSRV );
	mDepthMapDSV.push_back( tempDSV );

    // View saves a reference to the texture so we can release our reference.
    ReleaseCOM(depthMap);
}

CShadowMapper::~CShadowMapper()
{
	for ( int i = 0; i < mDepthMapSRV.size(); ++i )
		ReleaseCOM( mDepthMapSRV.at( i ) );
    for ( int i = 0; i < mDepthMapDSV.size(); ++i )
		ReleaseCOM( mDepthMapDSV.at( i ) );
}

ID3D11ShaderResourceView* CShadowMapper::DepthMapSRV(int index)
{
	return mDepthMapSRV.at( index );

}

void CShadowMapper::BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc, int index)
{
    dc->RSSetViewports(1, &mViewport);

	// Set null render target because we are only going to draw to depth buffer.
	// Setting a null render target will disable color writes.
    ID3D11RenderTargetView* renderTargets[1] = { 0 };
    dc->OMSetRenderTargets( 1, renderTargets, mDepthMapDSV.at( index ) );
    
	dc->ClearDepthStencilView(mDepthMapDSV.at( index ), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

std::vector<ID3D11ShaderResourceView*> CShadowMapper::GetShadowMaps()
{
	return mDepthMapSRV;
}