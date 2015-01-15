//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "Texture.h"
#include <Windows.h>
#include <xnamath.h>
#include "MathHelper.h"


CTexture::CTexture(void)
	: m_pSRV( 0 )
{
}


CTexture::~CTexture(void)
{
}

bool CTexture::Init( std::string filename )
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFileA( DXUTGetD3D11Device(), filename.c_str(), NULL, NULL, &m_pSRV, NULL );
	if ( FAILED( hr ) )
		return false;

	return true;
}

bool CTexture::Init( std::wstring filename )
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile( DXUTGetD3D11Device(), filename.c_str(), NULL, NULL, &m_pSRV, NULL );
	if ( FAILED( hr ) )
		return false;

	return true;
}

void CTexture::DestroyResources()
{
	if ( m_pSRV )
	{
		m_pSRV->Release();
		m_pSRV = 0;
	}
}


ID3D11ShaderResourceView* CTexture::GetTexture()
{
	return m_pSRV;
}

void CTexture::CreateRandom1D()
{
	ID3D11Device* device = DXUTGetD3D11Device();

	XMFLOAT4 randomValues[1024];

	for(int i = 0; i < 1024; ++i)
	{
		randomValues[i].x = SMathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].y = SMathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].z = SMathHelper::RandF(-1.0f, 1.0f);
		randomValues[i].w = SMathHelper::RandF(-1.0f, 1.0f);
	}

    D3D11_SUBRESOURCE_DATA initData;
    initData.pSysMem = randomValues;
	initData.SysMemPitch = 1024*sizeof(XMFLOAT4);
    initData.SysMemSlicePitch = 0;

	//
	// Create the texture.
	//
    D3D11_TEXTURE1D_DESC texDesc;
    texDesc.Width = 1024;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.Usage = D3D11_USAGE_IMMUTABLE;
    texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texDesc.CPUAccessFlags = 0;
    texDesc.MiscFlags = 0;
    texDesc.ArraySize = 1;

	ID3D11Texture1D* randomTex = 0;
    device->CreateTexture1D(&texDesc, &initData, &randomTex);

	//
	// Create the resource view.
	//
    D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texDesc.Format;
    viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    viewDesc.Texture1D.MipLevels = texDesc.MipLevels;
	viewDesc.Texture1D.MostDetailedMip = 0;
	
	ID3D11ShaderResourceView* randomTexSRV = 0;
    device->CreateShaderResourceView(randomTex, &viewDesc, &randomTexSRV);

	if ( randomTex )
	{
		randomTex->Release();
		randomTex = 0;
	}

	m_pSRV = randomTexSRV;
}

bool CTexture::Create2DArray(std::vector<std::wstring>& filenames, DXGI_FORMAT format, UINT filter, UINT mipFilter )
{
	ID3D11Device* device = DXUTGetD3D11Device();
	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();
	HRESULT hr;

	//
	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	//

	UINT size = filenames.size();

	std::vector<ID3D11Texture2D*> srcTex(size);
	for(UINT i = 0; i < size; ++i)
	{
		D3DX11_IMAGE_LOAD_INFO loadInfo;

        loadInfo.Width  = D3DX11_FROM_FILE;
        loadInfo.Height = D3DX11_FROM_FILE;
        loadInfo.Depth  = D3DX11_FROM_FILE;
        loadInfo.FirstMipLevel = 0;
        loadInfo.MipLevels = D3DX11_FROM_FILE;
        loadInfo.Usage = D3D11_USAGE_STAGING;
        loadInfo.BindFlags = 0;
        loadInfo.CpuAccessFlags = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        loadInfo.MiscFlags = 0;
        loadInfo.Format = format;
        loadInfo.Filter = filter;
        loadInfo.MipFilter = mipFilter;
		loadInfo.pSrcInfo  = 0;

        hr = D3DX11CreateTextureFromFile(device, filenames[i].c_str(), 
			&loadInfo, 0, (ID3D11Resource**)&srcTex[i], 0);
		if ( FAILED( hr ) ) 
			return false;
	}

	//
	// Create the texture array.  Each element in the texture 
	// array has the same format/dimensions.
	//

	D3D11_TEXTURE2D_DESC texElementDesc;
	srcTex[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc;
	texArrayDesc.Width              = texElementDesc.Width;
	texArrayDesc.Height             = texElementDesc.Height;
	texArrayDesc.MipLevels          = texElementDesc.MipLevels;
	texArrayDesc.ArraySize          = size;
	texArrayDesc.Format             = texElementDesc.Format;
	texArrayDesc.SampleDesc.Count   = 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage              = D3D11_USAGE_DEFAULT;
	texArrayDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags     = 0;
	texArrayDesc.MiscFlags          = 0;

	ID3D11Texture2D* texArray = 0;
	hr = device->CreateTexture2D( &texArrayDesc, 0, &texArray);
	if ( FAILED( hr ) ) 
		return false;

	//
	// Copy individual texture elements into texture array.
	//

	// for each texture element...
	for(UINT texElement = 0; texElement < size; ++texElement)
	{
		// for each mipmap level...
		for(UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; ++mipLevel)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex2D;
			hr = context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);
			if ( FAILED( hr ) )
				return false;

			context->UpdateSubresource(texArray, 
				D3D11CalcSubresource(mipLevel, texElement, texElementDesc.MipLevels),
				0, mappedTex2D.pData, mappedTex2D.RowPitch, mappedTex2D.DepthPitch);

			context->Unmap(srcTex[texElement], mipLevel);
		}
	}	

	//
	// Create a resource view to the texture array.
	//
	
	D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
	viewDesc.Format = texArrayDesc.Format;
	viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	viewDesc.Texture2DArray.MostDetailedMip = 0;
	viewDesc.Texture2DArray.MipLevels = texArrayDesc.MipLevels;
	viewDesc.Texture2DArray.FirstArraySlice = 0;
	viewDesc.Texture2DArray.ArraySize = size;

	ID3D11ShaderResourceView* texArraySRV = 0;
	hr = device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV);
	if ( FAILED( hr ) ) 
		return false;

	//
	// Cleanup--we only need the resource view.
	//

	if ( texArray )
	{
		texArray->Release();
		texArray = 0;
	}

	for(UINT i = 0; i < size; ++i)
	{
		if ( srcTex[i] )
		{
			srcTex[i]->Release();
			srcTex[i] = 0;
		}
	}

	m_pSRV = texArraySRV;

	return true;
}