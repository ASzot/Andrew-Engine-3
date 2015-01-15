#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "EffectManager.h"
#include "ObjModel.h"
#include "GeometryGenerator.h"
#include "CollisionHelper.h"
#include "ConstantsList.h"

//////////////////////////////
// Forward declarations:
class CLightManager;

struct StaticObjectData
{
	StaticObjectData()
	{
		x=y=z=rx=ry=rz=s=tx=ty=tz=0;
	}
	StaticObjectData(float x,float y,float z,float rx,float ry,float rz,float s,float sy,float sz,float tx,float ty,float tz,float ttx,float tty,float ttz,int opacity)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->rx = rx;
		this->ry = ry;
		this->rz = rz;
		this->s = s;
		this->tx = tx;
		this->ty = ty;
		this->tz = tz;
		this->ttx = ttx;
		this->tty = tty;
		this->ttz = ttz;
		this->sy = sy;
		this->sz = sz;

		this->opacity = opacity;
	}
	float x,y,z,rx,ry,rz,s,sy,sz,tx,ty,tz,ttx,tty,ttz;
	int opacity;
};

class CStaticObject : public IObject
{

protected:
	RenderTech mRenderTech;
	
	float mTexScaleX;
	float mTexScaleY;
	float mTexScaleZ;
	float mTexTransX;
	float mTexTransY;
	float mTexTransZ;
	CObjModel mMesh;
	std::string mModelName;
	Collision::BoundingBox mBoundingBox;
	XMFLOAT4X4 m_mWorld;
	RenderTechData m_RenderTechData;
	int m_iFlashing;
	int m_iFlashingCount;

	std::vector<XMFLOAT4X4> m_ShadowTransforms;

public:
	CStaticObject();
	//BTW the rotation is as you would expect x component is for x-rotation and so on...
	CStaticObject(D3DXVECTOR3 pos, D3DXVECTOR3 rotation,XMFLOAT3 scale,XMFLOAT3 texScale,XMFLOAT3 texTrans);
	virtual ~CStaticObject();

	inline float SY() { return mScaleY; }
	inline float SZ() { return mScaleZ; }
	inline void SetSY( float sy ) { mScaleY = sy; }
	inline void SetSZ( float sz ) { mScaleZ = sz; }

	bool Initialize(ID3D11Device* device,const std::string filename, RenderTech renderTech, BulletData data);
	bool Initialize(ID3D11Device* device,const std::string filename, RenderTech renderTech, BulletData data, RenderTechData rtd);

	virtual void Render( ID3D11DeviceContext* context, CameraInfo& cam, const CEffectManager* effectManager, ExtraRenderingData& erd );
	void RenderRefraction( ID3D11DeviceContext* context, CameraInfo& cam, CEffectManager* effectManager );
	void RenderShadowMap( ID3D11DeviceContext* context, CameraInfo& cam, const CEffectManager* effectManager, CLightManager* pLightMgr );

	virtual void Shutdown();

	virtual void Serilize(CDataArchiver* archiver);

	virtual void DestroyResources();

	tPoints GetVertices();

	void Translate(float x, float y, float z);
	void Translate(XMVECTOR dir);
	void MoveX(float distance);
	void MoveY(float distance);
	void MoveZ(float distance);
	void RotateX(float direction);
	void RotateY(float direction);
	void RotateZ(float direction);
	void Scale(float change);
	void TexScale(float change);
	void SetPos(float x,float y,float z);
	void SetPositionData(StaticObjectData d);
	void SetRenderTech(RenderTech renderTech);
	void SetOpacity(float opacity);

	virtual Mat4x4 GetWorldMatrix();
	virtual void SetWorldMatrix(Mat4x4& m);
	void SetWorldMatrixXM(CXMMATRIX m);

	RenderTech GetRenderTech();
	inline float TS() { return mTexScaleX; }

	StaticObjectData GetPositionData();

	void SetRenderData(RenderTechData rtd);
	void SetFlashing( int flashing );
	RenderTechData GetRenderData();

	Collision::BoundingBox* GetBoundingBox();
	void CreateBoundingBox();

	Vec GetBoundriesOfCollisionBox();

	virtual void SetAllCoordinates(PositionData data);

	// Mainly used in creating the window for the object manager.
	// Hence the material being stored as a string not a enum.
	ObjectPhysicsPropData GetPhysicsProperties();

	virtual void OnLeftClicked(UINT flags);

protected:
	void UpdateWorld(void);
};