//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once
#include "dataobject.h"
#include "IObject.h"
#include "Texture.h"
#include "EffectConstants.h"

class CEffectManager;
class CGameObjMgr;

class CTerrain
{
public:
	struct TerrainVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 Tex;
		XMFLOAT2 BoundsY;
	};

	

public:
	CTerrain();
	~CTerrain();

	float GetWidth()const;
	float GetDepth()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);

	void Shutdown();

	void Serilize( CDataArchiver* archiver );

	void Draw(ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager, ExtraRenderingData& erd);
	void RenderRefraction( ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager );
	void RenderShadowMap( ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager );

	TerrainDlgData GetDlgData();
	void SetDlgData( TerrainDlgData data );

private:
	bool LoadHeightmapBmp();
	void LoadHeightmapRaw();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightmapSRV(ID3D11Device* device);
	void ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M);
	ID3D11ShaderResourceView* CreateTexture2DArraySRV(
		ID3D11Device* device, ID3D11DeviceContext* context,
		std::vector<std::wstring>& filenames,
		DXGI_FORMAT format = DXGI_FORMAT_FROM_FILE,
		UINT filter = D3DX11_FILTER_NONE, 
		UINT mipFilter = D3DX11_FILTER_LINEAR);
	bool LoadTextures( InitInfo initInfo );

private:

	// Divide heightmap into patches such that each patch has CellsPerPatch cells
	// and CellsPerPatch+1 vertices.  Use 64 so that if we tessellate all the way 
	// to 64, we use all the data from the heightmap.  
	static const int CellsPerPatch = 64;

	ID3D11Buffer* mQuadPatchVB;
	ID3D11Buffer* mQuadPatchIB;

	CTexture mLayer1;
	CTexture mLayer2;
	CTexture mLayer3;
	CTexture mLayer4;
	CTexture mLayer5;

	ID3D11ShaderResourceView* mBlendMapSRV;
	ID3D11ShaderResourceView* mHeightMapSRV;

	InitInfo mInfo;

	UINT mNumPatchVertices;
	UINT mNumPatchQuadFaces;

	UINT mNumPatchVertRows;
	UINT mNumPatchVertCols;

	XMFLOAT4X4 mWorld;

	Material mMat;

	std::vector<XMFLOAT2> mPatchBoundsY;
	std::vector<float> mHeightmap;
};

float g_GetTerrainHeightAt( float x, float z );
CTerrain* g_GetTerrain();