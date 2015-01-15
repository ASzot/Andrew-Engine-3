#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "IObject.h"

class CShadowMapper
{
public:
	CShadowMapper(ID3D11Device* device, UINT width, UINT height);
	~CShadowMapper(void);

	ID3D11ShaderResourceView* DepthMapSRV(int index);

	void BindDsvAndSetNullRenderTarget(ID3D11DeviceContext* dc, int index);

	std::vector<ID3D11ShaderResourceView*> GetShadowMaps();

private:
	CShadowMapper(const CShadowMapper& rhs);
	CShadowMapper& operator=(const CShadowMapper& rhs);

	void CreateDSVAndSRV( );

private:
	UINT mWidth;
	UINT mHeight;

	std::vector<ID3D11ShaderResourceView*> mDepthMapSRV;
	std::vector<ID3D11DepthStencilView*> mDepthMapDSV;

	D3D11_VIEWPORT mViewport;
};

