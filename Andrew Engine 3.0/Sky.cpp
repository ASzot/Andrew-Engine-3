//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "Sky.h"
#include "IObject.h"
#include "GeometryGenerator.h"
#include "EffectManager.h"
#include "RenderSettings.h"


CSky::CSky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius)
{
	D3DX11CreateShaderResourceViewFromFile(device, cubemapFilename.c_str(), 0, 0, &mCubeMapSRV, 0);

	GeometryGenerator::MeshData sphere;
	GeometryGenerator geoGen;
	geoGen.CreateSphere(skySphereRadius, 30, 30, sphere);

	std::vector<XMFLOAT3> vertices(sphere.Vertices.size());

	for(size_t i = 0; i < sphere.Vertices.size(); ++i)
	{
		vertices[i] = sphere.Vertices[i].Position;
	}

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = sizeof(XMFLOAT3) * vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];

    device->CreateBuffer(&vbd, &vinitData, &mVB);
	

	mIndexCount = sphere.Indices.size();

	D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(USHORT) * mIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
	ibd.StructureByteStride = 0;
    ibd.MiscFlags = 0;

	std::vector<USHORT> indices16;
	indices16.assign(sphere.Indices.begin(), sphere.Indices.end());

    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices16[0];

    device->CreateBuffer(&ibd, &iinitData, &mIB);
}

CSky::~CSky()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);
	ReleaseCOM(mCubeMapSRV);
}

ID3D11ShaderResourceView* CSky::CubeMapSRV()
{
	return mCubeMapSRV;
}

void CSky::Render(const CameraInfo& cam, CEffectManager* effectManager)
{
	SRenderSettings::SetNoCull();

	// center sky about eye in world space
	XMFLOAT3 eyePos = cam.Position;
	XMMATRIX T = XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);

	XMMATRIX viewProj = cam.View * cam.Proj;
	XMMATRIX WVP = XMMatrixMultiply( T, viewProj );

	effectManager->mSkyFX->SetWorldViewProj( WVP );
	effectManager->mSkyFX->SetCubeMap( mCubeMapSRV );

	ID3D11DeviceContext* context = DXUTGetD3D11DeviceContext();

	UINT stride = sizeof( XMFLOAT3 );
    UINT offset = 0;

    context->IASetVertexBuffers( 0, 1, &mVB, &stride, &offset );
	context->IASetIndexBuffer( mIB, DXGI_FORMAT_R16_UINT, 0 );

	effectManager->SetInputLayout( InputLayoutType::Pos );
	context->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	
	D3DX11_TECHNIQUE_DESC techDesc;
	effectManager->mSkyFX->SkyTech->GetDesc( &techDesc );

    for ( UINT p = 0; p < techDesc.Passes; ++p )
    {
		ID3DX11EffectPass* pass = effectManager->mSkyFX->SkyTech->GetPassByIndex( p );

		pass->Apply( 0, context );

		context->DrawIndexed( mIndexCount, 0, 0 );
	}

	SRenderSettings::SetDefaultCull();
}