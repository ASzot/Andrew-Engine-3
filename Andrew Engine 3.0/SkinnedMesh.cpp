//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "SkinnedMesh.h"
#include "LoadM3d.h"

SkinnedModel::SkinnedModel(ID3D11Device* device, TextureMgr& texMgr, const std::string& modelFilename, const std::wstring& texturePath)
{
	std::vector<M3dMaterial> mats;
	M3DLoader m3dLoader;
	if(!m3dLoader.LoadM3d(modelFilename, Vertices, Indices, Subsets, mats, SkinnedData))
		FatalError("Couldn't load a animation!");

	ModelMesh.SetVertices(device, &Vertices[0], Vertices.size());
	ModelMesh.SetIndices(device, &Indices[0], Indices.size());
	ModelMesh.SetSubsetTable(Subsets);

	SubsetCount = mats.size();

	for(UINT i = 0; i < SubsetCount; ++i)
	{
		Mat.push_back(mats[i].Mat);

		ID3D11ShaderResourceView* diffuseMapSRV = texMgr.CreateTexture(texturePath + mats[i].DiffuseMapName);
		DiffuseMapSRV.push_back(diffuseMapSRV);

		ID3D11ShaderResourceView* normalMapSRV = texMgr.CreateTexture(texturePath + mats[i].NormalMapName);
		NormalMapSRV.push_back(normalMapSRV);
	}

}

SkinnedModel::~SkinnedModel()
{
}

void SkinnedModelInstance::Update(float dt)
{
	TimePos += dt;
	Model->SkinnedData.GetFinalTransforms(ClipName, TimePos, FinalTransforms);

	// Loop animation
	if(TimePos > Model->SkinnedData.GetClipEndTime(ClipName))
		TimePos = 0.0f;




}

std::vector<XMFLOAT3> SkinnedModelInstance::GetVertices()
{
	std::vector<XMFLOAT3> points;
	points.resize(Model->Vertices.size());
	for( int i = 0; i < points.size(); ++ i)
	{
		points[i] = Model->Vertices[i].Pos;
	}

	return points;
}