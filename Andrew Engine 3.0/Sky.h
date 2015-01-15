#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include <string>

// Some predeclarations so things don't get messy with the includes.
struct CameraInfo;
class CEffectManager;

class CSky
{
public:
	CSky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius);
	~CSky();

	ID3D11ShaderResourceView* CubeMapSRV();

	void Render(const CameraInfo& cam,CEffectManager* effectManager);

private:
	CSky(const CSky& rhs);
	CSky& operator=(const CSky& rhs);

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;

	ID3D11ShaderResourceView* mCubeMapSRV;

	UINT mIndexCount;
};

