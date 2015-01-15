//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "IObject.h"
#include "Physics.h"
#include "HashedString.h"

// This abstraction is so the physics system could potentially be used and physics engines could be swapped out.

class CGameObjMgr;

class IGamePhysics
{
public:
	virtual ~IGamePhysics()
	{}

	//  For the debug drawer.
	virtual bool VInitialize(CGameObjMgr* gameObjMgr) = 0;
	//virtual bool VInitResources(ID3D11Device* device) = 0;
	virtual void VSyncVisibleScene(CGameObjMgr* pGameObjMgr) = 0;
	virtual void VOnUpdate( float deltaSeconds ) = 0;

	virtual void VAddBox(Vec& dimensions, IObject* obj, float specifiedGravity, enum PhysicsMaterial mat) = 0;
	virtual void VAddPointCloud( XMFLOAT3* verts, int numVerts, IObject* obj, float specGrav, enum PhysicsMaterial mat ) = 0;
	virtual void VRemoveActor(ObjectId id) = 0;

	virtual void VApplyForce(const Vec& dir, float newtons, ObjectId id) = 0;
	virtual void VApplyForce(const Vec& force, ObjectId id ) = 0;
	virtual void VApplyImpulse( const Vec& dir, float newtons, ObjectId id ) = 0;
	virtual void VApplyTorque(const Vec& dir, float newtons, ObjectId id) = 0;
	virtual bool VKinematicMove(const Mat4x4& mat, ObjectId id) = 0;
	virtual bool VSetGravityOfObject( const float newGravity, ObjectId id ) = 0;

	virtual void VRotateY(ObjectId id, float angleRadians, float time) = 0;
	virtual float VGetOrientationY(ObjectId id) = 0;
	virtual void VStopObject(ObjectId id) = 0;
	virtual void VSetVelocity(ObjectId id, const Vec& vel) = 0;
	virtual void VSetAngularVelocity(ObjectId id, const Vec& vel) = 0;
	virtual void VAddVelocity(ObjectId id, const Vec& vel, bool trueAdd = false) = 0;
	virtual Vec VGetVelocity(ObjectId id ) = 0;
	virtual Vec VGetAngularVelocity(ObjectId id) = 0;
	virtual void VTranslate(ObjectId id, const Vec& vec) = 0;
	virtual void VRenderDiagnostics() = 0;

	virtual bool VAddConstraint( BulletConstraint* opc ) = 0;
	virtual int VGetNumberOfConstraints(ObjectId id) = 0;
	virtual bool VRemoveConstraints(ObjectId id) = 0;
	virtual BulletConstraint* VGetConstraint(ObjectId id) = 0;
	virtual std::map<ObjectId, BulletConstraint*> VGetAllConstraints() = 0;

	virtual IObject* VCastRay( Vec& startPos, Vec& endPos, UINT flags ) = 0;

	virtual void ResyncPhysics(CGameObjMgr* mgr) = 0;

	virtual void Shutdown() = 0;


	virtual Vec GetGravity() = 0;
	virtual void SetGravity( Vec& v ) = 0;
};