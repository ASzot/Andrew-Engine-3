#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Models.h"
#include "GeometryGenerator.h"
#include "RenderSettings.h"
#include "CollisionHelper.h"



// Global typedef because i am lazy
typedef std::vector<XMFLOAT3> tPoints;



//A little object for encapsulating the obj model into a easy managable class 
class CObjModel
{
public:
	CObjModel()
	{ mSubsets = 0; m_fOpacity = 1.0f; }
	~CObjModel()
	{}

	bool Initialize(std::string filename,ID3D11Device* device,std::vector<std::wstring>& textureNames)
	{
		return ModelLoader::LoadObjModel(filename,&mVB,&mIB,mSubsetIndexStart,mSubsetTexture,mMaterial,mSubsets,true,true,mVertexArray,mIndexArray,textureNames,device,mMeshSRV);
	}

	void DestroyResources()
	{
		ReleaseCOM(mVB);
		ReleaseCOM(mIB);
		for(int i = 0; i < mMeshSRV.size();++i)
		{
			ReleaseCOM(mMeshSRV[i]);
		}
	}

	void Shutdown()
	{
		// Nullify all variables so this class can be reset if need be.
		mSubsetIndexStart.clear();
		mSubsetTexture.clear();
		mMeshSRV.clear();
		mMaterial.clear();
		mIndexArray.clear();
		mVertexArray.clear();
		m_fOpacity = 0;
		mSubsets = 0;
	}

	inline D3D11_PRIMITIVE_TOPOLOGY GetTopology(RenderTech renderTech) const
	{
		if(renderTech == eDISPLACEMENT_MAPPING)
			return D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
		return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}

	ID3D11ShaderResourceView* GetSubsetDiffuseMapSRV(int subset)
	{
		return mMeshSRV[mMaterial[mSubsetTexture[subset]].texArrayIndex];
	}

	ID3D11ShaderResourceView* GetSubsetNormalMapSRV( int subset )
	{
		return mMeshSRV[mMaterial[mSubsetTexture[subset]].normMapTexArrayIndex];
	}

	bool GetHasTexture(int subset) const
	{
		return mMaterial[mSubsetTexture[subset]].hasTexture;
	}

	bool GetIsTransparent( int subset )const
	{
		return mMaterial[mSubsetTexture[subset]].transparent;
	}

	float GetTransparency( int subset )const
	{
		return mMaterial[mSubsetTexture[subset]].difColor.w;
	}

	bool GetHasNormalMap( int subset ) const 
	{
		return mMaterial[mSubsetTexture[subset]].hasNormMap;
	}

	int GetIndexDrawAmount(int subset) const
	{
		return mSubsetIndexStart[subset+1] - mSubsetIndexStart[subset];
	}
	int GetIndexStart(int subset) const
	{
		return mSubsetIndexStart[subset];
	}
	inline ID3D11Buffer* GetVertexBuffer()
	{
		return mVB;
	}
	inline ID3D11Buffer* GetIndexBuffer()
	{
		return mIB;
	}
	inline UINT GetStride()
	{
		return sizeof(ObjectVertex);
	}
	int GetNumberOfSubsets()
	{
		return mSubsets;
	}
	Material GetSubsetMaterial(int subset)
	{
		Material mat;
		mat.Diffuse = mMaterial[mSubsetTexture[subset]].difColor;
		//mat.Diffuse.w = m_fOpacity;
		mat.Ambient = XMFLOAT4(0.5f,0.5f,0.5f,0.5f);
		mat.Reflect = XMFLOAT4(0.5f,0.5f,0.5f,1.0f);
		mat.Specular =  mMaterial[mSubsetTexture[subset]].specColor;
		//mat.Specular.w = m_fOpacity;

		return mat;
	}
	
	void SetOpacity(float opacity)
	{
		m_fOpacity = opacity;
	}
	float GetOpacity()
	{
		return m_fOpacity;
	}

	tPoints GetVertices()
	{
		return mVertexArray;
	}

	
private:
	ID3D11Buffer* mVB;
	ID3D11Buffer* mIB;
	std::vector<int> mSubsetIndexStart;
	std::vector<int> mSubsetTexture;
	std::vector<ID3D11ShaderResourceView*> mMeshSRV;
	std::vector<SurfaceMaterial> mMaterial;
	std::vector<DWORD> mIndexArray;
	std::vector<XMFLOAT3> mVertexArray;
	float m_fOpacity;
	int mSubsets;
};


