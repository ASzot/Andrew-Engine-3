#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "SkinnedMesh.h"
#include "Effects.h"
#include "HashedString.h"



class CAnimatedObject : public IObject
{
public:
	CAnimatedObject();
	virtual ~CAnimatedObject();

private:
	SkinnedModel* mCharacterModel;
	SkinnedModelInstance mCharacterInstance;
	char* m_pcFilename;
	wchar_t* m_pwcTextureFilename;
	char* m_pcClipName;
	float m_fSpeedOfAnimation;
	XMFLOAT4 m_vCurrCharDirection;
	XMFLOAT4 m_vOldCharDirection;
	XMFLOAT4 m_vCharPosition;
	//CAnimObjController* m_pController;
	bool m_bMove;
	Vec m_vDesiredDir;
	float m_fExtraPhysicsHeight;

	// To store all of the shadow transform data.
	std::vector<XMFLOAT4X4> m_ShadowTransforms;

	Collision::BoundingBox mBoundingBox;
public:
	bool Init(PositionData data);
	// For those people who are just too lazy.
	bool Init(PositionData* data);
	bool InitResources(ID3D11Device* device,const char* filename,const wchar_t* textureDir,char* clipName,TextureMgr& textureManager);
	virtual void Shutdown(void);
	void Update(float dt);
	void Render(CameraInfo& cam, CSkinEffect* effect);
	void RenderShadowMap( CLightManager* lightMgr, CameraInfo& cam, CSkinEffect* effect );
	virtual void DestroyResources(void);
	void Serilize(CDataArchiver* dataArchiver);

	void StopMoving()
	{
		m_bMove = false;
	}
	void Animate(Vec& dir, float dt);

	void SetDirection( Vec& dir );

	float* GetExtraPhysicsHeightPtr()
	{
		return &m_fExtraPhysicsHeight;
	}

	virtual Mat4x4 GetWorldMatrix();
	virtual void SetWorldMatrix(Mat4x4& m);

	Collision::BoundingBox* GetBoundingBox()
	{
		return &mBoundingBox;
	}

	WCF::AnimPropDialog::AnimPropData GetAllData()
	{
		WCF::AnimPropDialog::AnimPropData data;
		data.Data.x = mPosX;
		data.Data.y = mPosY;
		data.Data.z = mPosZ;
		data.Data.rx = mRotX;
		data.Data.ry = mRotY;
		data.Data.rz = mRotZ;
		data.Data.s = mScale;
		data.Data.sy = mScaleY;
		data.Data.sz = mScaleZ;
		data.TS = 1.0f;
		data.Speed = m_fSpeedOfAnimation;
		data.APH = m_fExtraPhysicsHeight;
		// It is the managers job to provide the selected object

		return data;
	}

	void SetAllData(WCF::AnimPropDialog::AnimPropData data)
	{
		mPosX = data.Data.x;
		mPosY = data.Data.y;
		mPosZ = data.Data.z;
		mRotX = data.Data.rx;
		mRotY = data.Data.ry;
		mRotZ = data.Data.rz;
		mScale = data.Data.s;
		mScaleY = data.Data.sy;
		mScaleZ = data.Data.sz;
		m_fSpeedOfAnimation = data.Speed;
		m_fExtraPhysicsHeight = data.APH;
	}
	Vec GetDesiredDir()
	{
		return m_vDesiredDir;
	}
	bool* GetMove()
	{
		return &m_bMove;
	}

	Vec GetBoundriesOfCollisionBox()
	{
		UpdateWorldMatrix();
		Collision::BoundingBox* box = GetBoundingBox();
		float w,l,h;
		// Width is x, length is z, and h is y.
		SMathHelper::GetMinAndMaxAsBox(w,h,l,box->MinVertex,box->MaxVertex);

		return Vec(w, h, l);
	}

	virtual void OnLeftClicked(UINT flags);

	void UpdateWorldMatrix(void);
private:
	void CreateBoundingBox(void);
};