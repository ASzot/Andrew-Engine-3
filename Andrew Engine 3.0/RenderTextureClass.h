//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "DXUT.h"

#ifndef Checkh
#define Checkh(x) { if(FAILED(x)) { return false; } }
#endif

#ifndef ReleaseCOM
#define ReleaseCOM(x) { if(x) { x->Release(); x = 0; } }
#endif

class CRenderTexture
{
public:
	CRenderTexture(void);
	~CRenderTexture(void);
	
	bool Initialize( int textureHeight, int textureWidth );
	
	void DestroyResources();

	void Resize( UINT width, UINT height );

	///<summary>Clears the render texture with the appropriate color and dsv.<summary>
	void ClearRenderTarget( float cr, float rg, float rb, float ra, ID3D11DepthStencilView* dsv = DXUTGetD3D11DepthStencilView() );
	void SetRenderTarget( ID3D11DepthStencilView* dsv = DXUTGetD3D11DepthStencilView() );
	
	ID3D11ShaderResourceView* GetShaderResourceView();

private:
	ID3D11Texture2D* m_pRenderTargetTexture;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11ShaderResourceView* m_pSRV;
};

