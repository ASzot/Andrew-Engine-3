#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "interfaces.h"
#include "GameObjMgr.h"
#include <exception>
#include <list>
#include "BulletDebugDrawer.h"




struct ObjectMotionState : public btMotionState
{
	Mat4x4 m_WorldToPositionTransform;

	ObjectMotionState(Mat4x4 const & startingTransform)
		: m_WorldToPositionTransform(startingTransform)
	{
	}

	// Bullet will call these functions so watch out.
	virtual void getWorldTransform( btTransform& worldTrans)const
	{
		worldTrans = Mat4x4_to_btTransform( m_WorldToPositionTransform );
	}
	virtual void setWorldTransform( const btTransform& worldTrans)
	{
		m_WorldToPositionTransform = btTransform_to_Mat4x4(worldTrans);
	}
};


typedef btHingeConstraint RotatorContraint;
// A struct representing an object in the bullet physics library.
struct BulletActor
{
	btRigidBody* m_pRigidBody;
	float m_desiredDeltaYAngle;
	float m_desiredDeltaYAngleTime;

	explicit BulletActor(btRigidBody* pRigidBody)
	{
		m_pRigidBody = pRigidBody;
		
		m_desiredDeltaYAngle = 0;
		m_desiredDeltaYAngleTime = 0;
	}
};

class CNullPhysicsSystem
	: public IGamePhysics
{
public:
	CNullPhysicsSystem() { }
	virtual ~CNullPhysicsSystem() { }

	virtual bool VInitialize(CGameObjMgr* gameObjMgr){ return true; }
	//virtual bool VInitResources(ID3D11Device* device) { return true; }
	virtual void VSyncVisibleScene(CGameObjMgr* pGameObjMgr) { }
	virtual void VOnUpdate( float deltaSeconds ) { }

	virtual void VAddBox( Vec& dimensions, IObject* obj, float specifiedGrav, enum PhysicsMaterial mat) { }
	virtual void VAddPointCloud( XMFLOAT3* verts, int numVerts, IObject* obj, float specGrav, enum PhysicsMaterial mat ) { } 
	virtual void VRemoveActor(ObjectId id) { }

	// A possible improvement is making it so the methods return false if the
	// object wasn't found. VKinematicMove does this perfectly.
	virtual void VApplyForce(const Vec& dir, float newtons, ObjectId id) { }
	virtual void VApplyForce(const Vec& force, ObjectId id ) { }
	virtual void VApplyTorque(const Vec& dir, float newtons, ObjectId id) { }
	virtual void VApplyImpulse( const Vec& dir, float newtons, ObjectId id ) { }
	virtual bool VKinematicMove(const Mat4x4& mat, ObjectId id) { return true; }
	virtual bool VSetGravityOfObject( const float newGravity, ObjectId id ) { return true; }

	virtual void VRotateY(ObjectId id, float angleRadians, float time) { }
	virtual float VGetOrientationY(ObjectId id) { return 0.0f; }
	virtual void VStopObject(ObjectId id) { }
	virtual void VSetVelocity(ObjectId id, const Vec& vel) { }
	virtual void VSetAngularVelocity(ObjectId id, const Vec& vel) { }
	virtual Vec VGetVelocity(ObjectId id ) { return Vec(0.0f); }
	virtual Vec VGetAngularVelocity(ObjectId id) { return Vec(0.0f); }
	virtual void VAddVelocity(ObjectId id, const Vec& vel, bool trueAdd = false) { }
	virtual void VTranslate(ObjectId id, const Vec& vec) { }
	virtual void VRenderDiagnostics() { } 

	virtual bool VAddConstraint( BulletConstraint* opc ) { return true; }
	virtual int VGetNumberOfConstraints(ObjectId id) { return 0; }
	virtual bool VRemoveConstraints(ObjectId id) { return true; }
	virtual BulletConstraint* VGetConstraint(ObjectId id) { return 0; }
	virtual std::map<ObjectId, BulletConstraint*> VGetAllConstraints() { std::map<ObjectId,BulletConstraint*> returnVal; return returnVal; }

	virtual IObject* VCastRay( Vec& startPos, Vec& endPos, UINT flags ) { return 0; }

	virtual void ResyncPhysics(CGameObjMgr* mgr) {}


	virtual void Shutdown() { }

	virtual Vec GetGravity() { return Vec( 0.0f ); }
	virtual void SetGravity( Vec& v ) {  }
};


struct RayCasterCallback : btBroadphaseRayCallback
{
	virtual bool process( const btBroadphaseProxy* proxy);
};


class CPhysicsSystem :
	public IGamePhysics
{
private:
	btDynamicsWorld*					m_DynamicsWorld;
	// This is for the fast general collision detection.
	btBroadphaseInterface*				m_Broadphase;
	btCollisionDispatcher*				m_Dispatcher;
	btConstraintSolver*					m_Solver;
	btDefaultCollisionConfiguration*	m_CollisionConfiguration;
	CBulletDebugDrawer*					m_BulletDebugDrawer;

	CGameObjMgr*						m_ptGameObjMgr;
	
	typedef std::list<ObjectId> ObjectIdList;
	ObjectIdList m_ObjectIds;

	typedef std::map<ObjectId, BulletActor*> ObjectIdToBulletActorMap;
	ObjectIdToBulletActorMap m_ObjectBodies;

	typedef std::map<btRigidBody const *,ObjectId> RigidBodyToObjetIdMap;
	RigidBodyToObjetIdMap m_RigidBodyToObjectId;

	typedef std::pair<btRigidBody const *, btRigidBody const *> CollisionPair;
	typedef std::set<CollisionPair> CollisionPairs;
	CollisionPairs m_PreviousTickCollisionPairs;

	typedef std::map<ObjectId, BulletConstraint*> ObjectIdToConstraintMap;
	// Cache all of the constraints so they can be retrieved in all of their entirety.
	ObjectIdToConstraintMap m_CachedConstraints;

	// Some funcs to send events relating to the matter of collision pairs.
	void SendCollisionPairAddEvent(btPersistentManifold const * manifold, btRigidBody const * body0, btRigidBody const * body1);
	void SendCollisionPairRemoveEvent(btRigidBody const * body0, btRigidBody const * body1);

	// Some common functionality for adding shapes
	void AddShape(IObject* object, btCollisionShape* shape, btScalar mass, enum PhysicsMaterial mat);

	// For clearing up objects.
	void RemoveCollisionObject(btCollisionObject* const objectToRemove);

	// To get the body associated with the object id.
	btRigidBody* FindActorBody(ObjectId id)const;

	BulletActor* FindBulletActor(ObjectId id)const;

	RayCasterCallback m_RCC;

	static void BulletInternalTickCallback(btDynamicsWorld* world, btScalar timeStep);

	Vec m_vGravity;

	std::vector<Vec> TranslationList;

public:
	ObjectId FindActorId(btRigidBody const *)const;

	//TODO:
	// Organize all of this better.
public:
	CPhysicsSystem(Vec gravity);
	virtual ~CPhysicsSystem();

	virtual bool VInitialize(CGameObjMgr* gameObjMgr);
	//virtual bool VInitResources(ID3D11Device* device);
	virtual void VSyncVisibleScene(CGameObjMgr* pGameObjMgr);
	virtual void VOnUpdate( float deltaSeconds );

	virtual void VAddBox(Vec& dimensions, IObject* obj, float specifiedGrav, enum PhysicsMaterial mat);
	virtual void VAddPointCloud( XMFLOAT3* verts, int numVerts, IObject* obj, float specGrav, enum PhysicsMaterial mat );
	virtual void VRemoveActor(ObjectId id);

	// A possible improvement is making it so the methods return false if the
	// object wasn't found. VKinematicMove does this perfectly.

	virtual void VApplyForce(const Vec& dir, float newtons, ObjectId id);
	virtual void VApplyForce(const Vec& force, ObjectId id );
	virtual void VApplyImpulse( const Vec& dir, float newtons, ObjectId id );
	virtual void VApplyTorque(const Vec& dir, float newtons, ObjectId id);
	virtual bool VKinematicMove(const Mat4x4& mat, ObjectId id);
	virtual bool VSetGravityOfObject( const float newGravity, ObjectId id );

	virtual void VRotateY(ObjectId id, float angleRadians, float time);
	virtual float VGetOrientationY(ObjectId id);
	virtual void VStopObject(ObjectId id);
	virtual void VSetVelocity(ObjectId id, const Vec& vel);
	virtual void VSetAngularVelocity(ObjectId id, const Vec& vel);
	virtual void VAddVelocity(ObjectId id, const Vec& vel, bool trueAdd = false);
	virtual Vec VGetVelocity(ObjectId id );
	virtual Vec VGetAngularVelocity(ObjectId id);
	virtual void VTranslate(ObjectId id, const Vec& vec);

	// Constraint methods.
	
	virtual bool VAddConstraint( BulletConstraint* opc );
	virtual int VGetNumberOfConstraints(ObjectId id);
	virtual bool VRemoveConstraints(ObjectId id);
	virtual BulletConstraint* VGetConstraint(ObjectId id);
	virtual std::map<ObjectId, BulletConstraint*> VGetAllConstraints();

	virtual IObject* VCastRay( Vec& startPos, Vec& endPos, UINT flags );

	// For during debugging to visualize our physics system.
	virtual void VRenderDiagnostics();


	virtual void ResyncPhysics(CGameObjMgr* mgr);

	virtual void Shutdown();

	virtual Vec GetGravity();
	virtual void SetGravity( Vec& v );


};

// A function that will just create the physics and init it.
IGamePhysics* CreateGamePhysics(bool createNullPhysics, float gravity);