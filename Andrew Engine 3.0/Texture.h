//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "DXUT.h"
#include <string>
#include <vector>

class CTexture
{
public:
	CTexture(void);
	~CTexture(void);

	bool Init( std::wstring filename );
	bool Init( std::string filename );

	void DestroyResources();

	ID3D11ShaderResourceView* GetTexture();

	void CreateRandom1D();
	bool Create2DArray( std::vector<std::wstring>& filenames, DXGI_FORMAT format = DXGI_FORMAT_FROM_FILE, UINT filter = D3DX11_FILTER_NONE, UINT mipFilter = D3DX11_FILTER_LINEAR );

private:
	ID3D11ShaderResourceView* m_pSRV;
};

