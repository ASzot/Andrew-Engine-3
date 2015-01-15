#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "SDKmesh.h"
#include "SDKmisc.h"
#include <string>
#include "DXUTcamera.h"
#include "LightHelper.h"
#include "Camera.h"
#include <vector>
#include "DataObject.h"
#include "MathHelper.h"
#include <map>
#include "Physics.h"
#include "DebugWriter.h"
#include "Texture.h"

using namespace std;

#define DestroyObject(x) { if(x) { x->Shutdown(); delete x; x=0; } }

#ifndef ReleaseCOM
#define ReleaseCOM(x) { if(x) { x->Release(); x = 0; } }
#endif

#ifndef ReleaseResources
#define ReleaseResources(x) { if(x) { x->DestroyResources(); } }
#endif

#ifndef SafeDelete
#define SafeDelete(x) { if(x) { delete x; x=0; } }
#endif

#ifndef FatalError
#define FatalError(x) { MessageBoxA(NULL,x,"Fatal Error" ,MB_OK | MB_ICONERROR);  }
#endif

#ifndef DisplayError
#define DisplayError(x) { MessageBoxA(NULL,x,"Error",MB_OK); }
#endif

#ifndef Checkhr
#define Checkhr(x,y) { if(FAILED(x)) { MessageBoxA(NULL,y,"Fatal Error" ,MB_OK | MB_ICONERROR);} }
#endif

#ifndef Checkh
#define Checkh(x) { if(FAILED(x)) { return false; } }
#endif

class CActor;

struct BulletData
{
	float SpecGrav;
	PhysicsMaterial Mat;
};

// Used to describe how an object should be rendered.
struct RenderTechData
{
	bool Reflections;
	bool Refractable;
};


// Just a little extension to make things easier feel free to add.
template<typename T>
class List : public std::vector<T>
{
public:
	T* PtrEndElement()
	{
		return &vector<T>::at(vector<T>::size() - 1);
	}
	T EndElement()
	{
		return vector<T>::at(vector<T>::size() - 1);
	}
	T BeginElement()
	{
		return vector<T>::at(0);
	}
	T* PtrBeginElement()
	{
		return vector<T>::at(0);
	}
};

// -1 is typically used for the failure value.
typedef int ObjectId;



enum RenderTech { eBASIC, eBASIC_NO_DIR_LIGHT, eSSAO, eSHADOW, eCUBE_MAP_REFLECT, eDISPLACEMENT_MAPPING };

class IObject : public CDataObject
{
public:
	IObject()
	{
		mPosX = mPosY = mPosZ = mRotX = mRotY = mRotZ = mScale = 0;
		m_iIndexOfObject = -1;
		m_BulletData.SpecGrav = 4.0f;
		m_BulletData.Mat = PhysicsMaterial::PhysMat_Bouncy;
		m_pGameActor = 0;
		m_f4AxisRot = XMFLOAT4( 0.0f, 0.0f, 0.0f, 0.0f );
		m_bIsSerilizible = true;
	}
	virtual ~IObject()
	{

	}

	virtual void Shutdown() = 0;

	// This CAN'T INCLUDE SCALE.
	virtual Mat4x4 GetWorldMatrix() = 0;
	virtual void SetWorldMatrix(Mat4x4& mat)
	{
		Vec newPos = mat.GetPosition();
		mPosX = newPos.X();
		mPosY = newPos.Y();
		mPosZ = newPos.Z();
	}

	virtual void DestroyResources() = 0;

	inline void SetX(float x) { mPosX = x; }
	inline void SetY(float y) { mPosY = y; }
	inline void SetZ(float z) { mPosZ = z; } 
	inline void SetRX(float rx) { mRotX = rx; }
	inline void SetRY(float ry) { mRotY = ry; }
	inline void SetRZ(float rz) { mRotZ = rz; }
	inline void SetAxisR( XMFLOAT4 axisRot ) { m_f4AxisRot; } 
	inline void SetScale(float scale) { mScale = scale; }
	inline void SetScaleY(float scale) { mScaleY = scale; }
	inline void SetScaleZ(float scale) { mScaleZ = scale; }
	inline void SetScaleXYZ(float scale) { mScale = mScaleY = mScaleZ = scale; }
	virtual void SetAllCoordinates(PositionData data)
	{
		mPosX = data.x;
		mPosY = data.y;
		mPosZ = data.z;
		mRotX = data.rx;
		mRotY = data.ry;
		mRotZ = data.rz;
		mScale = data.s;
	}

	float GetSpecGrav() { return m_BulletData.SpecGrav; }
	PhysicsMaterial GetPhysicsMaterial() { return m_BulletData.Mat; }

	void SetAllPhysicsData( BulletData d ) 
	{ 
		//if ( d.Mat != m_BulletData.Mat )
		//	CSystem::m_bWasEditMadeToObj = true;
		m_BulletData = d; 
	}

	inline float X()	{ return mPosX; }
	inline float Y()	{ return mPosY; }
	inline float Z()	{ return mPosZ; }
	inline float RX()	{ return mRotX; }
	inline float RY()	{ return mRotY; }
	inline float RZ()	{ return mRotZ; }
	inline float S()	{ return mScale;}
	inline float SY()	{ return mScaleY; }
	inline float SZ()	{ return mScaleZ; }

	inline float* pX()	{ return &mPosX; }
	inline float* pY()	{ return &mPosY; }
	inline float* pZ()	{ return &mPosZ; }
	inline float* pRX()	{ return &mRotX; }
	inline float* pRY() { return &mRotY; }
	inline float* pRZ()	{ return &mRotZ; }

	virtual void SetPosition(Vec& pos)
	{
		mPosX = pos.X();
		mPosY = pos.Y();
		mPosZ = pos.Z();
	}

	bool operator > ( IObject& otherObj ) const
	{
		return this->m_iIndexOfObject > otherObj.m_iIndexOfObject;
	}

	bool operator < ( IObject& otherObj ) const
	{
		return this->m_iIndexOfObject < otherObj.m_iIndexOfObject;
	}

	void SetIndexOfObject(int newIndex)
	{
		m_iIndexOfObject = newIndex;
	}
	ObjectId GetId()
	{
		if(m_iIndexOfObject == -1)
		{
			FatalError("The index of IObject was attempted to be accesses when it was -1!\n( int IObject::GetIndexOfObject() );");
		}
		return m_iIndexOfObject;
	}

	virtual Vec GetBoundriesOfCollisionBox() = 0;

	virtual void OnLeftClicked(UINT flags) = 0;

	float GetMass()
	{
		Vec bounds = GetBoundriesOfCollisionBox();
		bounds = bounds * 0.5f;
		float volume = bounds.X() * bounds.Y() * bounds.Z();
		return ( volume * GetSpecGrav() );
	}

	void RegisterGameActor( CActor* pGameActor )
	{
		m_pGameActor = pGameActor;
	}

	void SetIsSerilizible(bool b)
	{
		m_bIsSerilizible = b;
	}

	bool GetIsSerilizible()
	{
		return m_bIsSerilizible;
	}

protected:
	float mPosX;
	float mPosY;
	float mPosZ;
	float mRotX;
	float mRotY;
	float mRotZ;
	XMFLOAT4 m_f4AxisRot;
	
	float mScale;
	float mScaleY;
	float mScaleZ;

	bool m_bIsSerilizible;

	BulletData m_BulletData;

	// If the rendering object has a game object as well.
	CActor* m_pGameActor;

	ObjectId m_iIndexOfObject;
};



// Just to make life easier.
typedef std::vector<IObject*> ObjectList;
typedef std::vector<ObjectId> ObjectIdList;
typedef std::map<ObjectId, IObject*> ObjectIndexToObjectMap;
typedef std::pair<ObjectId, IObject*> IndexObjectKey;