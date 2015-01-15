#pragma once 

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "EventManager.h"
#include "ObjectManager.h"

class CPlayerManager;

class CGameObject
{
public:
	CGameObject( ObjectId idOfObject );
	CGameObject();
	virtual ~CGameObject();

	Vec GetPosition();
	Vec GetRotation();
	Vec GetVelocity();
	void SetWorldMatrix( CXMMATRIX M );
	void SetVelocity(Vec velocity);
	void Accelerate(Vec dir, float acceleration);
	float GetMass();
	float GetSpeed();

	void InitObject(ObjectId idOfObject);

	void RegisterPlayerManager( CPlayerManager* pPlayerMgr );

	virtual bool Initialize() = 0;
	virtual void Update() = 0;

protected:
	CStaticObject* m_ptObj;
	CPlayerManager* m_ptPlayerMgr;
};

class CHoldableObject : public CGameObject
{
public:
	CHoldableObject( ObjectId idOfObject );
	CHoldableObject();
	virtual ~CHoldableObject();

	virtual void Update();
	virtual bool Initialize() = 0;


	void SetHolding( bool holding );
	void ThrowObject();

	XMMATRIX GetHoldingTransform();

protected:
	bool m_bHolding;
	Vec m_vHoldingPos;
	float m_xRotModifier;
};


class CFlare : public CHoldableObject
{
public:
	CFlare();
	virtual ~CFlare();

	bool Initialize();

	void Update();

private:
	ParticleSystem* m_pParticleSystem;
	PointLight* m_pPointLight;
};

class CGun : public CHoldableObject
{
public: 
	CGun();
	virtual ~CGun();

	bool Initialize();

	void Update();

private:
};