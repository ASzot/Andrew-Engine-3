#pragma once 

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "AnimatedObject.h"
#include "Player.h"
#include "GameProcesses.h"
#include "ProcessManager.h"
#include "EventManager.h"
#include "Actor.h"

class CHoldableObject;

// Just so we can register the main player camera as an actual game entity.
class CFakeObject : public IObject
{
public:
	CFakeObject(float x, float y, float z)
		: IObject()
	{
		mPosX = x;
		mPosY = y;
		mPosZ = z;
		mScale = 1.0f;
		mScaleY = 1.0f;
		mScaleZ = 1.0f;
		m_BulletData.Mat = PhysMat_Normal;
	}
	virtual ~CFakeObject()
	{

	}

	Vec GetPosAsVec()
	{
		return Vec( mPosX, mPosY, mPosZ );
	}

	virtual Mat4x4 GetWorldMatrix()
	{
		XMMATRIX translation, rotX, rotY, rotZ, final;
		translation = XMMatrixTranslation( mPosX, mPosY, mPosZ );
		rotX = XMMatrixRotationX( mRotX );
		rotY = XMMatrixRotationY( mRotY );
		rotZ = XMMatrixRotationZ( mRotZ );
		final = rotX * rotY * rotZ * translation;
		D3DXMATRIX* m = (D3DXMATRIX*)&final;
		return Mat4x4(*m);
	}
	virtual Vec GetBoundriesOfCollisionBox();
	virtual void Shutdown() { }
	virtual void DestroyResources() { }
	virtual void Serilize( CDataArchiver* archiver ) { }

	virtual void SetWorldMatrix(Mat4x4& mat);

	virtual void OnLeftClicked(UINT flags) {  }
};

class CMainPlayer : public CActor
{
public:
	CMainPlayer();
	virtual ~CMainPlayer();

	bool Initialize(Vec& location, CGameObjMgr* mgr, CEventManager* eventMgr);
	bool InitResources( ID3D11Device* device );

	void DestroyResources();
	void Shutdown();

	void Render(CameraInfo& cam);
	void SetCameraInfo( CameraInfo& cam );

	virtual Vec GetPosition();
	virtual Vec GetVelocity();
	virtual Vec GetFacing();
	virtual float GetSpeed();
	
	CameraInfo GetCameraInfo();

	void DropHoldingObject();
	void ThrowHoldingObject();

	bool IsDead();
	void Respawn( IGamePhysics* phys);

	void Strafe( float dir, CameraInfo& cam, IGamePhysics* phys );
	void Walk( float dir, CameraInfo& cam, IGamePhysics* phys );

	void SetHolding( CHoldableObject* holdingObject );

	void Jump( CameraInfo& cam, IGamePhysics* phys );

	virtual void Update(float dt, CGameObjMgr* mgr);

	void Shot();

	virtual void OnRayHit( UINT flags );

	// Gets the state of the character and how left mouse clicks should be handeled.
	UINT GetLeftClickMessage();

	CFakeObject* m_PhysBody;

private:
	CEventSnooper m_EventListener;
	const float m_fWalkForce;
	const float m_fJumpForce;
	Vec m_vSpawnLocation;
	bool m_bIsMoving;
	CameraInfo m_CameraInfo;
	CHoldableObject* m_ptObjHolding;
};