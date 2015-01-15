//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Terrain.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include "GameObjMgr.h"
#include "d3dx11effect.h"
#include "EffectManager.h"
#include "WindowCreatorFoundation.h"


CTerrain* g_ptTerrain = 0;

CTerrain::CTerrain() : 
	mQuadPatchVB(0), 
	mQuadPatchIB(0), 
	mBlendMapSRV(0), 
	mHeightMapSRV(0),
	mNumPatchVertices(0),
	mNumPatchQuadFaces(0),
	mNumPatchVertRows(0),
	mNumPatchVertCols(0)
{
	XMStoreFloat4x4(&mWorld, XMMatrixIdentity());

	mMat.Ambient  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Diffuse  = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	mMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 64.0f);
	mMat.Reflect  = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	g_ptTerrain = this;
}

CTerrain::~CTerrain()
{
	Shutdown();
}

float CTerrain::GetWidth()const
{
	// Total terrain width.
	return (mInfo.HeightmapWidth-1)*mInfo.CellSpacing;
}

float CTerrain::GetDepth()const
{
	// Total terrain depth.
	return (mInfo.HeightmapHeight-1)*mInfo.CellSpacing;
}

float CTerrain::GetHeight(float x, float z)const
{
	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f*GetWidth()) /  mInfo.CellSpacing;
	float d = (z - 0.5f*GetDepth()) / -mInfo.CellSpacing;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = mHeightmap[row*mInfo.HeightmapWidth + col];
	float B = mHeightmap[row*mInfo.HeightmapWidth + col + 1];
	float C = mHeightmap[(row+1)*mInfo.HeightmapWidth + col];
	float D = mHeightmap[(row+1)*mInfo.HeightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if( s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s*uy + t*vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f-s)*uy + (1.0f-t)*vy;
	}
}

XMMATRIX CTerrain::GetWorld()const
{
	return XMLoadFloat4x4(&mWorld);
}

void CTerrain::SetWorld(CXMMATRIX M)
{
	XMStoreFloat4x4(&mWorld, M);
}

void CTerrain::Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo)
{
	mInfo = initInfo;

	// Divide heightmap into patches such that each patch has CellsPerPatch.
	mNumPatchVertRows = ((mInfo.HeightmapHeight-1) / CellsPerPatch) + 1;
	mNumPatchVertCols = ((mInfo.HeightmapWidth-1) / CellsPerPatch) + 1;

	mNumPatchVertices  = mNumPatchVertRows*mNumPatchVertCols;
	mNumPatchQuadFaces = (mNumPatchVertRows-1)*(mNumPatchVertCols-1);

	size_t dot = initInfo.HeightMapFilename.find( '.' );
	std::wstring extension( initInfo.HeightMapFilename.begin() + dot, initInfo.HeightMapFilename.end() );

	if ( extension == L".bmp" )
		LoadHeightmapBmp();
	else if ( extension == L".raw" )
		LoadHeightmapRaw();
	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB(device);
	BuildQuadPatchIB(device);
	BuildHeightmapSRV(device);

	std::vector<std::wstring> layerFilenames;
	layerFilenames.push_back(mInfo.LayerMapFilename0);
	layerFilenames.push_back(mInfo.LayerMapFilename1);
	layerFilenames.push_back(mInfo.LayerMapFilename2);
	layerFilenames.push_back(mInfo.LayerMapFilename3);
	layerFilenames.push_back(mInfo.LayerMapFilename4);
	
	LoadTextures( initInfo );

	D3DX11CreateShaderResourceViewFromFile(device, 
		mInfo.BlendMapFilename.c_str(), 0, 0, &mBlendMapSRV, 0);
}

void CTerrain::Draw(ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager, ExtraRenderingData& erd)
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	effectManager->SetInputLayout( InputLayoutType::TerrainIL );

	UINT stride = sizeof( TerrainVertex );
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
	dc->IASetIndexBuffer(mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX viewProj = cam.View * cam.Proj;
	XMMATRIX world  = XMLoadFloat4x4(&mWorld);
	XMMATRIX worldInvTranspose = SMathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world*viewProj;

	XMFLOAT4 worldPlanes[6];
	ExtractFrustumPlanes(worldPlanes, viewProj);

	// Set per frame constants.
	effectManager->mTerrainFX->SetViewProj(viewProj);
	effectManager->mTerrainFX->SetWorld( world );
	effectManager->mTerrainFX->SetWorldViewProj( worldViewProj );
	effectManager->mTerrainFX->SetEyePosW(cam.Position);
	effectManager->mTerrainFX->SetFogColor( XMVectorSet( 0.5f, 0.5f, 0.5f, 1.0f ) );
	effectManager->mTerrainFX->SetFogStart(15.0f);
	effectManager->mTerrainFX->SetFogRange(175.0f);
	effectManager->mTerrainFX->SetMinDist(20.0f);
	effectManager->mTerrainFX->SetMaxDist(500.0f);
	effectManager->mTerrainFX->SetMinTess(0.0f);
	effectManager->mTerrainFX->SetMaxTess(6.0f);
	effectManager->mTerrainFX->SetTexelCellSpaceU(1.0f / mInfo.HeightmapWidth);
	effectManager->mTerrainFX->SetTexelCellSpaceV(1.0f / mInfo.HeightmapHeight);
	effectManager->mTerrainFX->SetWorldCellSpace(mInfo.CellSpacing);
	effectManager->mTerrainFX->SetWorldFrustumPlanes(worldPlanes);
	
	effectManager->mTerrainFX->SetLayer1( mLayer1.GetTexture() );
	effectManager->mTerrainFX->SetLayer2( mLayer2.GetTexture() );
	effectManager->mTerrainFX->SetLayer3( mLayer3.GetTexture() );
	effectManager->mTerrainFX->SetLayer4( mLayer4.GetTexture() );
	effectManager->mTerrainFX->SetLayer5( mLayer5.GetTexture() );
	effectManager->mTerrainFX->SetBlendMap(mBlendMapSRV);
	effectManager->mTerrainFX->SetHeightMap(mHeightMapSRV);

	effectManager->mTerrainFX->SetShadowMap( erd.pShadowMap );
	effectManager->mTerrainFX->SetShadowTransform( erd.ShadowTransform * world );

	effectManager->mTerrainFX->SetMaterial(mMat);

	ID3DX11EffectTechnique* tech = effectManager->mTerrainFX->Light1Tech;
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );

    for(UINT i = 0; i < techDesc.Passes; ++i)
    {
        ID3DX11EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0, dc);

		dc->DrawIndexed(mNumPatchQuadFaces*4, 0, 0);
	}	

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	dc->HSSetShader(0, 0, 0);
	dc->DSSetShader(0, 0, 0);
}

void CTerrain::RenderRefraction( ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager )
{
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	effectManager->SetInputLayout( InputLayoutType::TerrainIL );

	UINT stride = sizeof( TerrainVertex );
    UINT offset = 0;
    dc->IASetVertexBuffers(0, 1, &mQuadPatchVB, &stride, &offset);
	dc->IASetIndexBuffer(mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0);

	XMMATRIX viewProj = cam.View * cam.Proj;
	XMMATRIX world  = XMLoadFloat4x4(&mWorld);
	XMMATRIX worldInvTranspose = SMathHelper::InverseTranspose(world);
	XMMATRIX worldViewProj = world*viewProj;

	XMFLOAT4 worldPlanes[6];
	ExtractFrustumPlanes(worldPlanes, viewProj);

	// Set per frame constants.
	effectManager->mTerrainFX->SetViewProj(viewProj);
	effectManager->mTerrainFX->SetWorld( world );
	effectManager->mTerrainFX->SetWorldViewProj( worldViewProj );
	effectManager->mTerrainFX->SetEyePosW(cam.Position);
	effectManager->mTerrainFX->SetFogColor( XMVectorSet( 0.5f, 0.5f, 0.5f, 1.0f ) );
	effectManager->mTerrainFX->SetFogStart(15.0f * 5);
	effectManager->mTerrainFX->SetFogRange(175.0f * 5);
	effectManager->mTerrainFX->SetMinDist(20.0f);
	effectManager->mTerrainFX->SetMaxDist(500.0f);
	effectManager->mTerrainFX->SetMinTess(0.0f);
	effectManager->mTerrainFX->SetMaxTess(6.0f);
	effectManager->mTerrainFX->SetTexelCellSpaceU(1.0f / mInfo.HeightmapWidth);
	effectManager->mTerrainFX->SetTexelCellSpaceV(1.0f / mInfo.HeightmapHeight);
	effectManager->mTerrainFX->SetWorldCellSpace(mInfo.CellSpacing);
	effectManager->mTerrainFX->SetWorldFrustumPlanes(worldPlanes);
	
	effectManager->mTerrainFX->SetLayer1( mLayer1.GetTexture() );
	effectManager->mTerrainFX->SetLayer2( mLayer2.GetTexture() );
	effectManager->mTerrainFX->SetLayer3( mLayer3.GetTexture() );
	effectManager->mTerrainFX->SetLayer4( mLayer4.GetTexture() );
	effectManager->mTerrainFX->SetLayer5( mLayer5.GetTexture() );
	effectManager->mTerrainFX->SetBlendMap(mBlendMapSRV);
	effectManager->mTerrainFX->SetHeightMap(mHeightMapSRV);

	effectManager->mTerrainFX->SetMaterial(mMat);

	ID3DX11EffectTechnique* tech = effectManager->mTerrainFX->LightRefract;
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );

    for(UINT i = 0; i < techDesc.Passes; ++i)
    {
        ID3DX11EffectPass* pass = tech->GetPassByIndex(i);
		pass->Apply(0, dc);

		dc->DrawIndexed(mNumPatchQuadFaces*4, 0, 0);
	}	

	// FX sets tessellation stages, but it does not disable them.  So do that here
	// to turn off tessellation.
	dc->HSSetShader(0, 0, 0);
	dc->DSSetShader(0, 0, 0);
}

void CTerrain::LoadHeightmapRaw()
{
	// A height for each vertex
	std::vector<unsigned char> in( mInfo.HeightmapWidth * mInfo.HeightmapHeight );

	// Open the file.
	std::ifstream inFile;
	inFile.open(mInfo.HeightMapFilename.c_str(), std::ios_base::binary);

	if(inFile)
	{
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it.
	mHeightmap.resize(mInfo.HeightmapHeight * mInfo.HeightmapWidth, 0);
	for(UINT i = 0; i < mInfo.HeightmapHeight * mInfo.HeightmapWidth; ++i)
	{
		mHeightmap[i] = (in[i] / 255.0f)*mInfo.HeightScale;
	}
}

bool CTerrain::LoadHeightmapBmp()
{
	FILE* filePtr;
	UINT count;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	unsigned char* bitmapImage;
	int imageSize;

	std::string heightMapAsString = WCF::ConversionTools::WStrToStr( mInfo.HeightMapFilename );
	const char* filename = heightMapAsString.c_str();

	int error = fopen_s( &filePtr, filename, "rb" );
	if ( error != 0 )
		return false;

	count = fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, filePtr);
	if ( count != 1 )
		return false;

	count = fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, filePtr);
	if ( count != 1 )
		return false;

	// 3 for the number of channels a bmp image contains.
	imageSize = mInfo.HeightmapHeight * mInfo.HeightmapWidth * 3;

	bitmapImage = new unsigned char[ imageSize ];
	if ( !bitmapImage )
		return false;

	fseek( filePtr, bitmapFileHeader.bfOffBits, SEEK_SET );

	count = fread( bitmapImage, 1, imageSize, filePtr );
	if ( count != imageSize )
		return false;

	error = fclose( filePtr );
	if ( error != 0 )
		return false;

	mHeightmap.resize( mInfo.HeightmapHeight * mInfo.HeightmapWidth, 0 );

	int totalIterations = mInfo.HeightmapHeight * mInfo.HeightmapWidth;
	unsigned char height;

	int k = 0;
	for ( int i = 0; i < totalIterations; ++i )
	{
		height = bitmapImage[ k ];
		mHeightmap[ i ] = ( height / 255.0f ) * mInfo.HeightScale;

		k += 3;
	}

	delete[] bitmapImage;
	bitmapImage = 0;

	return true;
}

void CTerrain::Smooth()
{
	std::vector<float> dest( mHeightmap.size() );

	for(UINT i = 0; i < mInfo.HeightmapHeight; ++i)
	{
		for(UINT j = 0; j < mInfo.HeightmapWidth; ++j)
		{
			dest[i*mInfo.HeightmapWidth+j] = Average(i,j);
		}
	}

	// Replace the old heightmap with the filtered one.
	mHeightmap = dest;
}

bool CTerrain::InBounds(int i, int j)
{
	// True if ij are valid indices; false otherwise.
	return 
		i >= 0 && i < (int)mInfo.HeightmapHeight && 
		j >= 0 && j < (int)mInfo.HeightmapWidth;
}

float CTerrain::Average(int i, int j)
{
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  Note
	// that if a pixel is missing neighbor, we just don't include it
	// in the average--that is, edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
	// and no iterations of the outer for loop occur.
	for(int m = i-1; m <= i+1; ++m)
	{
		for(int n = j-1; n <= j+1; ++n)
		{
			if( InBounds(m,n) )
			{
				avg += mHeightmap[m*mInfo.HeightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void CTerrain::CalcAllPatchBoundsY()
{
	mPatchBoundsY.resize(mNumPatchQuadFaces);

	// For each patch
	for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
	{
		for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void CTerrain::CalcPatchBoundsY(UINT i, UINT j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	UINT x0 = j*CellsPerPatch;
	UINT x1 = (j+1)*CellsPerPatch;

	UINT y0 = i*CellsPerPatch;
	UINT y1 = (i+1)*CellsPerPatch;

	float minY = FLT_MAX;
	float maxY = FLT_MIN;
	for(UINT y = y0; y <= y1; ++y)
	{
		for(UINT x = x0; x <= x1; ++x)
		{
			UINT k = y*mInfo.HeightmapWidth + x;
			minY = SMathHelper::Min(minY, mHeightmap[k]);
			maxY = SMathHelper::Max(maxY, mHeightmap[k]);
		}
	}

	UINT patchID = i*(mNumPatchVertCols-1)+j;
	mPatchBoundsY[patchID] = XMFLOAT2(minY, maxY);
}

void CTerrain::BuildQuadPatchVB(ID3D11Device* device)
{
	std::vector<TerrainVertex> patchVertices(mNumPatchVertRows*mNumPatchVertCols);

	float halfWidth = 0.5f*GetWidth();
	float halfDepth = 0.5f*GetDepth();

	float patchWidth = GetWidth() / (mNumPatchVertCols-1);
	float patchDepth = GetDepth() / (mNumPatchVertRows-1);
	float du = 1.0f / ( mNumPatchVertCols - 1 );
	float dv = 1.0f / ( mNumPatchVertRows - 1 );

	for(UINT i = 0; i < mNumPatchVertRows; ++i)
	{
		float z = halfDepth - i*patchDepth;
		for(UINT j = 0; j < mNumPatchVertCols; ++j)
		{
			float x = -halfWidth + j*patchWidth;

			patchVertices[i*mNumPatchVertCols+j].Pos = XMFLOAT3(x, 0.0f, z );

			// Stretch texture over grid.
			patchVertices[i*mNumPatchVertCols+j].Tex.x = j*du;
			patchVertices[i*mNumPatchVertCols+j].Tex.y = i*dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
	{
		for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
		{
			UINT patchID = i*(mNumPatchVertCols-1)+j;
			patchVertices[i*mNumPatchVertCols+j].BoundsY = mPatchBoundsY[patchID];
		}
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(TerrainVertex) * patchVertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &patchVertices[0];
    device->CreateBuffer(&vbd, &vinitData, &mQuadPatchVB);
}

void CTerrain::BuildQuadPatchIB(ID3D11Device* device)
{
	std::vector<USHORT> indices(mNumPatchQuadFaces*4); // 4 indices per quad face

	// Iterate over each quad and compute indices.
	int k = 0;
	for(UINT i = 0; i < mNumPatchVertRows-1; ++i)
	{
		for(UINT j = 0; j < mNumPatchVertCols-1; ++j)
		{
			// Top row of 2x2 quad patch
			indices[k]   = i*mNumPatchVertCols+j;
			indices[k+1] = i*mNumPatchVertCols+j+1;

			// Bottom row of 2x2 quad patch
			indices[k+2] = (i+1)*mNumPatchVertCols+j;
			indices[k+3] = (i+1)*mNumPatchVertCols+j+1;

			k += 4; // next quad
		}
	}

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(USHORT) * indices.size();
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    device->CreateBuffer(&ibd, &iinitData, &mQuadPatchIB);
}

void CTerrain::BuildHeightmapSRV(ID3D11Device* device)
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = mInfo.HeightmapWidth;
	texDesc.Height = mInfo.HeightmapHeight;
    texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format    = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count   = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// HALF is defined in xnamath.h, for storing 16-bit float.
	std::vector<HALF> hmap(mHeightmap.size());
	std::transform(mHeightmap.begin(), mHeightmap.end(), hmap.begin(), XMConvertFloatToHalf);
	
	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
    data.SysMemPitch = mInfo.HeightmapWidth*sizeof(HALF);
    data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	device->CreateTexture2D(&texDesc, &data, &hmapTex);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	device->CreateShaderResourceView(hmapTex, &srvDesc, &mHeightMapSRV);

	// SRV saves reference.
	ReleaseCOM(hmapTex);
}

void CTerrain::ExtractFrustumPlanes(XMFLOAT4 planes[6], CXMMATRIX M)
{
	//
	// Left
	//
	planes[0].x = M(0,3) + M(0,0);
	planes[0].y = M(1,3) + M(1,0);
	planes[0].z = M(2,3) + M(2,0);
	planes[0].w = M(3,3) + M(3,0);

	//
	// Right
	//
	planes[1].x = M(0,3) - M(0,0);
	planes[1].y = M(1,3) - M(1,0);
	planes[1].z = M(2,3) - M(2,0);
	planes[1].w = M(3,3) - M(3,0);

	//
	// Bottom
	//
	planes[2].x = M(0,3) + M(0,1);
	planes[2].y = M(1,3) + M(1,1);
	planes[2].z = M(2,3) + M(2,1);
	planes[2].w = M(3,3) + M(3,1);

	//
	// Top
	//
	planes[3].x = M(0,3) - M(0,1);
	planes[3].y = M(1,3) - M(1,1);
	planes[3].z = M(2,3) - M(2,1);
	planes[3].w = M(3,3) - M(3,1);

	//
	// Near
	//
	planes[4].x = M(0,2);
	planes[4].y = M(1,2);
	planes[4].z = M(2,2);
	planes[4].w = M(3,2);

	//
	// Far
	//
	planes[5].x = M(0,3) - M(0,2);
	planes[5].y = M(1,3) - M(1,2);
	planes[5].z = M(2,3) - M(2,2);
	planes[5].w = M(3,3) - M(3,2);

	// Normalize the plane equations.
	for(int i = 0; i < 6; ++i)
	{
		XMVECTOR v = XMPlaneNormalize(XMLoadFloat4(&planes[i]));
		XMStoreFloat4(&planes[i], v);
	}
}


ID3D11ShaderResourceView* CTerrain::CreateTexture2DArraySRV(
		ID3D11Device* device, ID3D11DeviceContext* context,
		std::vector<std::wstring>& filenames,
		DXGI_FORMAT format,
		UINT filter, 
		UINT mipFilter)
{
	//
	// Load the texture elements individually from file.  These textures
	// won't be used by the GPU (0 bind flags), they are just used to 
	// load the image data from file.  We use the STAGING usage so the
	// CPU can read the resource.
	//

	UINT size = filenames.size();

	HRESULT hr;

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
	device->CreateTexture2D( &texArrayDesc, 0, &texArray);

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
			context->Map(srcTex[texElement], mipLevel, D3D11_MAP_READ, 0, &mappedTex2D);

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
	device->CreateShaderResourceView(texArray, &viewDesc, &texArraySRV);

	//
	// Cleanup--we only need the resource view.
	//

	ReleaseCOM(texArray);

	for(UINT i = 0; i < size; ++i)
		ReleaseCOM(srcTex[i]);

	return texArraySRV;
}


bool CTerrain::LoadTextures( InitInfo initInfo )
{
	if ( !mLayer1.Init( initInfo.LayerMapFilename0 ) )
		return false;
	if ( !mLayer2.Init( initInfo.LayerMapFilename1 ) )
		return false;
	if ( !mLayer3.Init( initInfo.LayerMapFilename2 ) ) 
		return false;
	if ( !mLayer4.Init( initInfo.LayerMapFilename3 ) )
		return false;
	if ( !mLayer5.Init( initInfo.LayerMapFilename4 ) )
		return false;

	return true;
}



TerrainDlgData CTerrain::GetDlgData()
{
	TerrainDlgData data;
	data.blendmapFilename = WCF::ConversionTools::WStrToStr( mInfo.BlendMapFilename );
	data.heightmapFilename = WCF::ConversionTools::WStrToStr( mInfo.HeightMapFilename );
	data.frictionOfTerrain = 0.7f;

	return data;
}

void CTerrain::SetDlgData( TerrainDlgData data )
{
	std::string blendMapFilename = WCF::ConversionTools::WStrToStr( mInfo.BlendMapFilename );
	std::string heightMapFilename = WCF::ConversionTools::WStrToStr( mInfo.HeightMapFilename );

	// Only restart our terrain if the user has decided to use a different map or something which requires a restart.
	if ( ( data.blendmapFilename != blendMapFilename )
		|| ( data.heightmapFilename != heightMapFilename ) )
	{
		Shutdown();

		InitInfo ii;
		ii.BlendMapFilename = WCF::ConversionTools::StrToWStr( data.blendmapFilename );
		ii.HeightMapFilename = WCF::ConversionTools::StrToWStr( data.heightmapFilename );

		Init( DXUTGetD3D11Device(), DXUTGetD3D11DeviceContext(), ii );
	}
}

void CTerrain::Shutdown()
{
	mLayer1.DestroyResources();
	mLayer2.DestroyResources();
	mLayer3.DestroyResources();
	mLayer4.DestroyResources();
	mLayer5.DestroyResources();

	ReleaseCOM(mQuadPatchVB);
	ReleaseCOM(mQuadPatchIB);
	ReleaseCOM(mBlendMapSRV);
	ReleaseCOM(mHeightMapSRV);
}

void CTerrain::RenderShadowMap( ID3D11DeviceContext* dc, CameraInfo& cam, CEffectManager* effectManager )
{
	dc->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	effectManager->SetInputLayout( InputLayoutType::TerrainIL );

	UINT stride = sizeof( TerrainVertex );
    UINT offset = 0;
    dc->IASetVertexBuffers( 0, 1, &mQuadPatchVB, &stride, &offset );
	dc->IASetIndexBuffer( mQuadPatchIB, DXGI_FORMAT_R16_UINT, 0 );

	XMMATRIX viewProj = cam.View * cam.Proj;
	XMMATRIX world  = XMLoadFloat4x4(&mWorld);

	effectManager->mShadowFX->SetEyePosW( cam.Position );
	effectManager->mShadowFX->SetTexTransform( XMMatrixIdentity() );
	effectManager->mShadowFX->SetViewProj( viewProj );
	effectManager->mShadowFX->SetWorld( world );
	effectManager->mShadowFX->SetWorldInvTranspose( SMathHelper::InverseTranspose( world ) );
	effectManager->mShadowFX->SetWorldViewProj( world * cam.View * cam.Proj );

	ID3DX11EffectTechnique* tech = effectManager->mShadowFX->TerrainBuildShadowMapTech;
    D3DX11_TECHNIQUE_DESC techDesc;
    tech->GetDesc( &techDesc );

    for ( UINT i = 0; i < techDesc.Passes; ++i )
    {
        ID3DX11EffectPass* pass = tech->GetPassByIndex( i );
		pass->Apply( 0, dc );

		dc->DrawIndexed( mNumPatchQuadFaces * 4, 0, 0 );
	}	
}

void CTerrain::Serilize( CDataArchiver* archiver )
{
	archiver->WriteToStream( 't' );
	archiver->Space();
	archiver->WriteToStream( mInfo.BlendMapFilename );
	archiver->Space();
	archiver->WriteToStream( mInfo.CellSpacing );
	archiver->Space();
	archiver->WriteToStream( mInfo.HeightMapFilename );
	archiver->Space();
	archiver->WriteToStream( mInfo.HeightmapHeight );
	archiver->Space();
	archiver->WriteToStream( mInfo.HeightmapWidth );
	archiver->Space();
	archiver->WriteToStream( mInfo.HeightScale );
	archiver->Space();
	archiver->WriteToStream( mInfo.LayerMapFilename0 );
	archiver->Space();
	archiver->WriteToStream( mInfo.LayerMapFilename1 );
	archiver->Space();
	archiver->WriteToStream( mInfo.LayerMapFilename2 );
	archiver->Space();
	archiver->WriteToStream( mInfo.LayerMapFilename3 );
	archiver->Space();
	archiver->WriteToStream( mInfo.LayerMapFilename4 );
	archiver->WriteToStream( '\n' );
}


float g_GetTerrainHeightAt( float x, float z )
{
	if ( g_ptTerrain )
		return g_ptTerrain->GetHeight( x, z );
	else
		return FLT_MIN;
}

CTerrain* g_GetTerrain()
{
	return g_ptTerrain;
}