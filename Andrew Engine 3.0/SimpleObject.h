#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "iobject.h"
#include "GeometryGenerator.h"
#include "EffectManager.h"

class CSimpleObject :
	public IObject
{
public:
	enum ObjectTypes { Box, Cylinder, Sphere };
	
	CSimpleObject(void)
		: IObject()
	{

	}
	~CSimpleObject(void)
	{

	}

	//The texture name is without the location just the name of the file
	bool Initialize(float x,float y, float z, float rx,float ry,float rz, float s,float t,std::string textureFilename,ID3D11Device* device,ObjectTypes type,float width,float height,float depth)
	{
		mX = x; 
		mY = y;
		mZ = z;
		mRX = rx;
		mRY = ry;
		mRZ = rz;
		mS = s;
		mT = t;
		GeometryGenerator::MeshData meshData;
		GeometryGenerator geoGen;
		switch(type)
		{
		case Box:
			geoGen.CreateBox(width,height,depth,meshData);
			break;
		case Cylinder:
			geoGen.CreateCylinder(1.0f, 1.0f, 3.0f, 15, 15,meshData);
			break;
		case Sphere:
			geoGen.CreateSphere(3.0f,20,20,meshData);
			break;
		default:
			geoGen.CreateBox(width,height,depth,meshData);
			break;
		}
		if(!LoadGeometryBuffers(meshData,device))\
			return false;

		mMat.Ambient = XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
		mMat.Diffuse = XMFLOAT4(0.7f,0.7f,0.7f,1.0f);
		mMat.Reflect = XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
		mMat.Specular = XMFLOAT4(0.7f,0.7f,0.7f,1.0f);

		return true;
	}

	void DestroyResources()
	{
		ReleaseCOM(mIB);
		ReleaseCOM(mVB);
	}

	void Shutdown()
	{

	}

	void Serilize(CDataArchiver* dataArchiver)
	{

	}

	void Render(ID3D11DeviceContext* deviceContext,Camera& cam,const CEffectManager* effectManager)
	{
		XMMATRIX world,scaling,texScaling,rotX,rotY,rotZ,translation;
		translation = XMMatrixTranslation(mX,mY,mZ);
		scaling = XMMatrixScaling(mS,mS,mS);
		texScaling = XMMatrixScaling(mT,mT,mT);
		rotX = XMMatrixRotationX(mRX);
		rotY = XMMatrixRotationY(mRY);
		rotZ = XMMatrixRotationZ(mRZ);
		world = scaling * rotX * rotY * rotZ * translation;

		CBasicEffectRenderInfo er;
		er.eyePosW = cam.GetPosition();
		er.fogColor = XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
		er.fogRange = 125.0f;
		er.fogStart = 50.0f;
		er.proj = cam.Proj();
		er.view = cam.View();
		er.texTransform = texScaling;
		er.world = world;
		er.cubeMapSRV = NULL;
		er.shadowSRV = NULL;
		deviceContext->IASetIndexBuffer(mIB,DXGI_FORMAT_R32_UINT,0);
		UINT offset = 0;
		UINT stride = sizeof(GeometryGenerator::Vertex);
		deviceContext->IASetVertexBuffers(0,1,&mVB,&stride,&offset);
		if(!effectManager->SetBasicFX(er,deviceContext))
			FatalError("Couldn't set the basic fx in SimpleObject!");
		effectManager->SetBasicFXHasTexture( false );
		effectManager->SetBasicFXMaterial( mMat );
		effectManager->SetBasicFXDiffuseMap( NULL );
		ID3DX11EffectTechnique* activeTech = effectManager->GetTechnique( eBASIC, false, false, false, false );
		D3DX11_TECHNIQUE_DESC techDesc;
		activeTech->GetDesc(&techDesc);
		for(UINT p = 0; p < techDesc.Passes; ++p)
		{
			activeTech->GetPassByIndex(p)->Apply(0,deviceContext);
			deviceContext->DrawIndexed(mIndexCount,0,0);
		}
	}

private:
	bool LoadGeometryBuffers(GeometryGenerator::MeshData& meshData,ID3D11Device* device)
	{
		D3D11_BUFFER_DESC bd;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.ByteWidth = sizeof(GeometryGenerator::Vertex) * meshData.Vertices.size();
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		bd.Usage = D3D11_USAGE_DEFAULT;
		D3D11_SUBRESOURCE_DATA sd;
		sd.pSysMem = &meshData.Vertices[0];
		Checkh(device->CreateBuffer(&bd,&sd,&mVB));

		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		mIndexCount = meshData.Indices.size(); 
		bd.ByteWidth = sizeof(UINT) * mIndexCount;
		sd.pSysMem = &meshData.Indices[0];
		Checkh(device->CreateBuffer(&bd,&sd,&mIB));

		return true;
	}

private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	Material mMat;
	bool mHasNormalMap;
	float mX;
	float mY;
	float mZ;
	float mRX;
	float mRY;
	float mRZ;
	float mS;
	float mT;
	UINT mIndexCount;
};

