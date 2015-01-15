//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "PhysicsSystem.h"
#include <algorithm>
#include <iterator>
#include "GameObjMgr.h"
#include "ObjectManager.h"
#include <BulletCollision\NarrowPhaseCollision\btRaycastCallback.h>


CPhysicsSystem::CPhysicsSystem(Vec gravity)
{
	m_vGravity = gravity;
}


CPhysicsSystem::~CPhysicsSystem(void)
{
	int x = 0;
}


bool CPhysicsSystem::VInitialize(CGameObjMgr* gameObjMgr)
{
	m_ptGameObjMgr = gameObjMgr;
	m_CollisionConfiguration = (new btDefaultCollisionConfiguration() );
	
	m_Dispatcher = (new btCollisionDispatcher(m_CollisionConfiguration));

	m_Broadphase = (new btDbvtBroadphase() );

	// Manages constraints which apply forces in physics simulations,
	// this setting puts in place no restraints.
	m_Solver = (new btSequentialImpulseConstraintSolver() );

	m_DynamicsWorld = (new btDiscreteDynamicsWorld(	m_Dispatcher,
		m_Broadphase,
		m_Solver,
		m_CollisionConfiguration ) );
	
	m_DynamicsWorld->setInternalTickCallback( BulletInternalTickCallback );
	m_DynamicsWorld->setWorldUserInfo( this );

	btVector3 const tempGrav = Vec_to_btVector3( m_vGravity );

	m_DynamicsWorld->setGravity( tempGrav );

	return true;
}


void CPhysicsSystem::VOnUpdate(float const deltaSeconds)
{
	// This 4 is the max number of sub sets. 
	m_DynamicsWorld->stepSimulation( deltaSeconds, 4 );
}

void CPhysicsSystem::VSyncVisibleScene(CGameObjMgr* pGameObjMgr)
{
	// Keep the physics and the graphics in perfect harmony.

	// Check for changes in the objects game state.
	for( ObjectIdToBulletActorMap::const_iterator it = m_ObjectBodies.begin(); 
		it != m_ObjectBodies.end(); ++it)
	{
		ObjectId const id = it->first;

		ObjectMotionState* const objectMotionState = static_cast<ObjectMotionState*>(it->second->m_pRigidBody->getMotionState());
		assert(objectMotionState);

		IObject* gameObj = pGameObjMgr->GetObjectByIndex(id);
		if( gameObj ) 
		{
			if( gameObj->GetWorldMatrix() != objectMotionState->m_WorldToPositionTransform )
			{
				// Bullet has moved the objects physics object so update the graphical representation as well.
				gameObj->SetWorldMatrix( objectMotionState->m_WorldToPositionTransform );
			}
		}
	}
}

void CPhysicsSystem::AddShape(IObject* const object, btCollisionShape* const shape,btScalar const mass, enum PhysicsMaterial const mat)
{
	Vec scaling = Vec( object->S(),object->SY(),object->SZ() );
	shape->setLocalScaling( btVector3( scaling.aX(), scaling.aY(), scaling.aZ() ) );
	ObjectId id = object->GetId();
	
	if ( !(m_ObjectBodies.find(id) == m_ObjectBodies.end() ) )
		FatalError("Object with more then one physics body?\nReally?");

	btVector3 localInertia(0.0f,0.0f,0.0f);
	if(mass > 0.0f)
		shape->calculateLocalInertia(mass,localInertia);

	ObjectMotionState* const motionState = new ObjectMotionState(object->GetWorldMatrix());


	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,shape,localInertia);

	rbInfo.m_restitution	= g_PhysicsMaterials[mat].restitution;
	rbInfo.m_friction		= g_PhysicsMaterials[mat].friction;

	btRigidBody* const body = new btRigidBody(rbInfo);
	body->setActivationState( DISABLE_DEACTIVATION );

	m_DynamicsWorld->addRigidBody(body);

	BulletActor* pBulletActor = new BulletActor(body);

	// Add it to the collection so it can be checked in VSyncVisibleScene
	m_ObjectBodies[id] = pBulletActor;
	m_RigidBodyToObjectId[body] = id;
	m_ObjectIds.push_back( id );

	pBulletActor->m_desiredDeltaYAngle = object->RY() * ( XM_PI / 180 );
	pBulletActor->m_desiredDeltaYAngleTime = 0.1f;
}


void CPhysicsSystem::RemoveCollisionObject( btCollisionObject* const remove )
{
	// This won't remove the object from our lists so we need to do that as well.
	m_DynamicsWorld->removeCollisionObject(remove);
	for( CollisionPairs::iterator it = m_PreviousTickCollisionPairs.begin(); it != m_PreviousTickCollisionPairs.end(); )
	{
		CollisionPairs::iterator next = it;
		++next;
		
		if( it->first == remove || it->second == remove )
		{
			SendCollisionPairRemoveEvent(it->first,it->second);
			m_PreviousTickCollisionPairs.erase( it );
		}
		it = next;
	}

	// This if statement is in the case that our object to remove in reality isn't a 
	// btRigidBody but actually a derivative of it.
	// For all intensive purposes this won't effect us because we only use btRigidBodies.
	if( btRigidBody * const body = btRigidBody::upcast(remove) )
	{
		// Delete all components of the object.
		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body->getUserPointer();
		delete body->getUserPointer();

		for ( int ii = body->getNumConstraintRefs() - 1; ii >= 0; --ii )
		{
			btTypedConstraint * const constraint = body->getConstraintRef( ii );
			m_DynamicsWorld->removeConstraint( constraint );
			delete constraint;
		}
	}
	delete remove;
}

btRigidBody * CPhysicsSystem::FindActorBody( ObjectId const id ) const
{
	BulletActor* pBulletActor = FindBulletActor( id );
	if(pBulletActor)
		return pBulletActor->m_pRigidBody;
	return NULL;
}

ObjectId CPhysicsSystem::FindActorId(btRigidBody const * const body) const
{
	RigidBodyToObjetIdMap::const_iterator found = m_RigidBodyToObjectId.find( body );
	if( found != m_RigidBodyToObjectId.end() )
	{
		return found->second;
	}

	return -1;
}

BulletActor* CPhysicsSystem::FindBulletActor(ObjectId const id) const
{
	ObjectIdToBulletActorMap::const_iterator found = m_ObjectBodies.find( id  );
	if( found != m_ObjectBodies.end() )
		return found->second;
	return NULL;
}

// For some reason the dim of the animated object was 0 everywhere, look into this. 
void CPhysicsSystem::VAddBox( Vec& dim, IObject* obj, float specGrav, enum PhysicsMaterial mat )
{
	assert(obj);
	

	dim.MultX( 0.5f );
	dim.MultY( 0.5f );
	dim.MultZ( 0.5f );
	btBoxShape * const boxShape = new btBoxShape(Vec_to_btVector3( dim ) );

	float const volume = dim.X() * dim.Y() * dim.Z();
	btScalar const mass =  (volume * specGrav );

	AddShape( obj, boxShape, mass, mat );
}

void CPhysicsSystem::VAddPointCloud( XMFLOAT3* verts, int numVerts, IObject* obj, float specGrav, enum PhysicsMaterial mat )
{
	assert( obj );
	
	btConvexHullShape* const shape = new btConvexHullShape();

	for ( int ii = 0; ii < numVerts; ++ii )
		shape->addPoint( btVector3( verts[ii].x, verts[ii].y, verts[ii].z ) );

	// Approximate the mass of the object.
	btVector3 aabbMin( 0,0,0 ), aabbMax( 0,0,0 );
	shape->getAabb( btTransform::getIdentity(), aabbMin, aabbMax );

	btVector3 const aabbExtents = aabbMax - aabbMin;
	float const volume = aabbExtents.x() * aabbExtents.y() * aabbExtents.z();
	btScalar const mass = volume * specGrav;
	
	AddShape( obj, shape, mass, mat );
}

void CPhysicsSystem::VRemoveActor(ObjectId id)
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		RemoveCollisionObject( body );

		// Clear the object from our lists (maps)
		ObjectIdToBulletActorMap::iterator it = m_ObjectBodies.find( id );
		if ( it != m_ObjectBodies.end() ) 
		{
			BulletActor* pDead = it->second;
			delete pDead;
			m_ObjectBodies.erase( it );
		}
		m_RigidBodyToObjectId.erase( body );
	}
}


void CPhysicsSystem::VApplyForce( const Vec& dir, float newtons, ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		btVector3 const force( dir.X() * newtons, dir.Y() * newtons, dir.Z() * newtons );
		body->applyCentralForce( force );
	}
}

void CPhysicsSystem::VApplyImpulse( const Vec& dir, float newtons, ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		Vec normDir =  dir;
		normDir.Normalize();
		btVector3 const impulse( normDir.X() * newtons, normDir.Y() * newtons, normDir.Z() * newtons );
		body->applyCentralImpulse( impulse );
	}
}

void CPhysicsSystem::VApplyTorque( const Vec& dir, float magnitude, ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		btVector3 torq( dir.X() * magnitude, dir.Y() * magnitude, dir.Z() * magnitude );
		body->applyTorque( torq );
	}
}

bool CPhysicsSystem::VKinematicMove( const Mat4x4& mat, ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) ) 
	{
		body->setWorldTransform( Mat4x4_to_btTransform( mat ) );
		return true;
	}
	return false;
}

void CPhysicsSystem::VRotateY( ObjectId id, float angleRadians, float time ) 
{
	if ( BulletActor * const actor = FindBulletActor( id ) )
	{
		actor->m_desiredDeltaYAngle = angleRadians;
		actor->m_desiredDeltaYAngleTime = time;
	}
}

float CPhysicsSystem::VGetOrientationY( ObjectId id )
{
	BulletActor* pBulletActor = FindBulletActor( id );
	assert( pBulletActor );

	const btTransform& actorTransform = pBulletActor->m_pRigidBody->getCenterOfMassTransform();
	btMatrix3x3 actorRotationMat( actorTransform.getBasis() );

	btVector3 startingVec(0,0,1);
	btVector3 endingVec = actorRotationMat * startingVec;

	endingVec.setY( 0 );

	float const endingVecLength = endingVec.length();
	if( endingVecLength < 0.001 )
	{
		return 0;
	}
	else
	{
		btVector3 cross = startingVec.cross(endingVec);
		float sign = cross.getY() > 0 ? 1.0f : -1.0f;
		return ( acosf( startingVec.dot( endingVec ) / endingVecLength ) * sign );
	}

	// We failed...
	return FLT_MAX;
}

void CPhysicsSystem::VStopObject( ObjectId objectId )
{
	BulletActor* pBulletActor = FindBulletActor( objectId );
	assert( pBulletActor );

	pBulletActor->m_pRigidBody->setLinearVelocity( btVector3(0,0,0) );
}

void CPhysicsSystem::VSetVelocity( ObjectId id, const Vec& vel )
{
	BulletActor* pBulletActor = FindBulletActor( id );
	assert(pBulletActor);

	btVector3 btVel = Vec_to_btVector3( vel );
	pBulletActor->m_pRigidBody->setLinearVelocity( btVel );
}

void CPhysicsSystem::VTranslate( ObjectId id, const Vec& vec )
{
	BulletActor* pBulletActor = FindBulletActor( id );
	assert( pBulletActor );
	btVector3 to = Vec_to_btVector3( vec );
	btVector3 at = pBulletActor->m_pRigidBody->getWorldTransform().getOrigin();
	btVector3 move = to - at;

	pBulletActor->m_pRigidBody->translate( move );
}


// This is called right after bullet physics internal update function
void CPhysicsSystem::BulletInternalTickCallback( btDynamicsWorld* world, btScalar timeStep )
{
	assert( world );

	assert( world->getWorldUserInfo() );
	CPhysicsSystem* const bulletPhysics = static_cast<CPhysicsSystem*>( world->getWorldUserInfo() );

	CollisionPairs currentTickCollisionPairs;


	// Take a look at all of our existing contacts.
	btDispatcher * const dispatcher = world->getDispatcher();
	for ( int manifoldIdx = 0; manifoldIdx < dispatcher->getNumManifolds(); ++manifoldIdx )
	{
		// The manifold is the set of data corresponding to a contact point between the
		// two physics objects. 
		btPersistentManifold const * const manifold = dispatcher->getManifoldByIndexInternal( manifoldIdx );
		assert( manifold );

		btRigidBody const * const body0 = static_cast<btRigidBody const *>( manifold->getBody0() );
		btRigidBody const * const body1 = static_cast<btRigidBody const *>( manifold->getBody1() );

		// This has to be done so the two are in a "predicable" order.
		bool const swapped = body0 > body1;

		btRigidBody const * const sortedBodyA = swapped ? body1 : body0;
		btRigidBody const * const sortedBodyB = swapped ? body0 : body1;

		CollisionPair const thisPair = std::make_pair( sortedBodyA, sortedBodyB );

		currentTickCollisionPairs.insert( thisPair );

		if ( bulletPhysics->m_PreviousTickCollisionPairs.find( thisPair ) == bulletPhysics->m_PreviousTickCollisionPairs.end() )
		{
			// This was a new object that wasn't registered previously, send an event.
			bulletPhysics->SendCollisionPairAddEvent( manifold, body0, body1 );
		}
	}
	
	CollisionPairs removedCollisionPairs;

	// We can use the STL set difference function to find collision pairs that existed during the previous tick but not this one.
	std::set_difference(bulletPhysics->m_PreviousTickCollisionPairs.begin(), bulletPhysics->m_PreviousTickCollisionPairs.end(),
		currentTickCollisionPairs.begin(), currentTickCollisionPairs.end(), std::inserter( removedCollisionPairs,removedCollisionPairs.end() ) );
	
	for( CollisionPairs::const_iterator it = removedCollisionPairs.begin(), end = removedCollisionPairs.end(); it != end; ++it )
	{
		btRigidBody const * const body0 = it->first;
		btRigidBody const * const body1 = it->second;

		bulletPhysics->SendCollisionPairRemoveEvent( body0, body1 );
	}

	bulletPhysics->m_PreviousTickCollisionPairs = currentTickCollisionPairs;

	// Turn the actors manually.
	for ( ObjectIdToBulletActorMap::const_iterator it = bulletPhysics->m_ObjectBodies.begin(); it != bulletPhysics->m_ObjectBodies.end(); ++it )
	{
		if ( it->second->m_desiredDeltaYAngleTime > 0 )
		{
			float a = (float)timeStep;
			float b = it->second->m_desiredDeltaYAngleTime;
			
			// Take the min...
			float const amountOfTime = ( a < b ) ? a : b;
			float const deltaAngle = (amountOfTime / it->second->m_desiredDeltaYAngleTime) * it->second->m_desiredDeltaYAngle;

			btTransform angleTransform;
			angleTransform.setIdentity();
			angleTransform.getBasis().setEulerYPR( 0, deltaAngle, 0 ); //Rotate around the Y - axis of the object.

			btRigidBody * const body = it->second->m_pRigidBody;

			body->setCenterOfMassTransform( body->getCenterOfMassTransform() * angleTransform );

			it->second->m_desiredDeltaYAngle -= deltaAngle;
			it->second->m_desiredDeltaYAngleTime -= amountOfTime;
		}
	}
}


void CPhysicsSystem::SendCollisionPairAddEvent( btPersistentManifold const * manifold, btRigidBody const * const body0, btRigidBody const * const body1 )
{
	if ( body0->getUserPointer() || body1->getUserPointer() )
	{
		// Only the triggers will have non null user pointers.
		// And we don't have triggers so this code block is entirely useless.
	}
	else
	{
		ObjectId const id0 = FindActorId(body0);
		ObjectId const id1 = FindActorId(body1);

		// This is a new collision, so...
		VecList collisionPoints;
		Vec sumNormalForce(0, 0, 0);
		Vec sumFrictionForce(0, 0, 0);
		
		for ( int pointIdx = 0; pointIdx < manifold->getNumContacts(); ++pointIdx )
		{
			btManifoldPoint const & point = manifold->getContactPoint( pointIdx );
			
			collisionPoints.push_back( btVector_to_Vec( point.getPositionWorldOnB() ) );

			sumNormalForce = sumNormalForce + btVector_to_Vec( point.m_combinedRestitution * point.m_normalWorldOnB );
			sumFrictionForce = sumFrictionForce + btVector_to_Vec( point.m_combinedFriction * point.m_lateralFrictionDir1 );
		}
		/*m_ptGameObjMgr->GetObjectByIndex( id0 )->OnCollision( sumNormalForce );
		m_ptGameObjMgr->GetObjectByIndex( id1 )->OnCollision( sumNormalForce );*/
	}
}

void CPhysicsSystem::SendCollisionPairRemoveEvent( btRigidBody const * const body0, btRigidBody const * const body1 )
{
	if( body0->getUserPointer() || body1->getUserPointer() )
	{
		// Only the triggers will have non null user pointers.
		// And we don't have triggers so this code block is entirely useless.
	}

	else
	{
		ObjectId const id0 = FindActorId(body0);
		ObjectId const id1 = FindActorId(body1);

		/*
		if( !id0 || !id1 )
		{
			FatalError("Attempted to remove a collision pair with an invalid object!");
			return;
		}
		*/
		// It would be nice to send a event here. 
	}
}

void CPhysicsSystem::VRenderDiagnostics() 
{
	m_DynamicsWorld->debugDrawWorld();
}

IGamePhysics* CreateGamePhysics(bool nullPhys, float grav)
{
	// Null physics can be created for testing purposes.
	IGamePhysics* phys;
	if( !nullPhys )
		phys = new CPhysicsSystem( Vec( 0.0f, grav, 0.0f ) );
	else
		phys = new CNullPhysicsSystem();
	return phys;
}


void CPhysicsSystem::Shutdown()
{
	// Delete all of the physics bodies we have allocated in memory.
	for ( ObjectIdList::iterator i = m_ObjectIds.begin(); i != m_ObjectIds.end(); ++i )
	{
		VRemoveActor( *i );
	}

	// Destroy in reverse order.
	SafeDelete( m_DynamicsWorld );
	SafeDelete( m_Solver );
	SafeDelete( m_Broadphase );
	SafeDelete( m_Dispatcher );
	SafeDelete(m_CollisionConfiguration);
}


void CPhysicsSystem::ResyncPhysics(CGameObjMgr* mgr)
{
	int numberOfSteps = m_ObjectBodies.size();
	WCF::LoadingBox loadingDlg;
	if ( loadingDlg.RunWindow( DXUTGetHINSTANCE(), DXUTGetHWND(), numberOfSteps ) == FALSE )
		FatalError("Couldn't create the loading box!\nvoid CPhysicsSystem::ResyncPhysics(CGameObjMgr* mgr)");


	for( ObjectIdToBulletActorMap::const_iterator it = m_ObjectBodies.begin(); 
		it != m_ObjectBodies.end(); ++it)
	{
		const ObjectId id = it->first;
		ObjectMotionState* const objectMotionState = static_cast<ObjectMotionState*>(it->second->m_pRigidBody->getMotionState());
		assert( objectMotionState );

		// Now update all of the object motion states based on the IObjects.
		IObject* obj = mgr->GetObjectByIndex( id );
		assert( obj );

		btTransform trans = Mat4x4_to_btTransform( obj->GetWorldMatrix() );
		objectMotionState->setWorldTransform( trans );
		it->second->m_pRigidBody->setWorldTransform( trans );
		PhysicsMaterial mat = obj->GetPhysicsMaterial();
		it->second->m_pRigidBody->setFriction( g_PhysicsMaterials[ mat ].friction );
		it->second->m_pRigidBody->setRestitution( g_PhysicsMaterials[mat].restitution );
		it->second->m_desiredDeltaYAngle = obj->RY() * (XM_PI / 180);
		it->second->m_desiredDeltaYAngleTime = 0.1f;

		loadingDlg.StepDialog();
	}

	loadingDlg.QuitWindow();
}


Vec CPhysicsSystem::GetGravity()
{
	return m_vGravity;
}

void CPhysicsSystem::SetGravity(Vec& v)
{
	if ( m_vGravity != v )
		PhysicsSynced( false );
	m_vGravity = v;
	m_DynamicsWorld->setGravity( Vec_to_btVector3( m_vGravity ) );
}

void CPhysicsSystem::VAddVelocity(ObjectId id, const Vec& vel, bool trueAdd)
{
	BulletActor* pBulletActor = FindBulletActor( id );
	assert(pBulletActor);

	if ( !trueAdd )
	{
		btVector3 btVel = Vec_to_btVector3( vel );
		btVector3 currentVelocity = pBulletActor->m_pRigidBody->getLinearVelocity();
		btVel.setY( currentVelocity.y() );

		pBulletActor->m_pRigidBody->setLinearVelocity( btVel );
	}
	else
	{
		btVector3 btVel = Vec_to_btVector3( vel );
		btVector3 currentVelocity = pBulletActor->m_pRigidBody->getLinearVelocity();
		currentVelocity = currentVelocity + btVel;

		pBulletActor->m_pRigidBody->setLinearVelocity( currentVelocity );
	}
}

IObject* CPhysicsSystem::VCastRay( Vec& startPos, Vec& endPos, UINT flags )
{
	//endPos.Normalize();
	btVector3 from = startPos.TobtVector3();
	btVector3 to = endPos.TobtVector3();

	// Gets the first hit.
	btCollisionWorld::ClosestRayResultCallback	closestResults(from,to);
	closestResults.m_flags |= btTriangleRaycastCallback::kF_None;
	m_DynamicsWorld->rayTest( from, to, closestResults );

	if ( closestResults.hasHit() )
	{
		btRigidBody* bodyOfCollision = (btRigidBody*)closestResults.m_collisionObject;
		if ( bodyOfCollision )
		{
			ObjectId id = FindActorId( bodyOfCollision );
			IObject* objOfCollision = g_GetGameObjMgr()->GetObjectByIndex( id );

			if ( flags != CAST_RAY)
				objOfCollision->OnLeftClicked( flags );

			return objOfCollision;
		}
	}

	return 0;
}

bool RayCasterCallback::process( const btBroadphaseProxy* proxy ) 
{
	btRigidBody* bodyOfCollision = (btRigidBody*)proxy->m_clientObject;
	if ( bodyOfCollision )
	{
		DebugWriter::WriteToDebugFile( "Used deprecated function!", "bool RayCasterCallback::process( const btBroadphaseProxy* proxy )" );

		return true;
	}
	return false;
}

bool CPhysicsSystem::VSetGravityOfObject( const float newGravity, ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) ) 
	{
		// Since gravity will always be in the negative y direction.
		btVector3 grav = btVector3( 0.0f, newGravity, 0.0f );
		body->setGravity( grav );
		return true;
	}
	return false;
}

Vec CPhysicsSystem::VGetVelocity(ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		btVector3 vel = body->getLinearVelocity();

		return Vec( vel.x(), vel.y(), vel.z() );
	}

	return Vec( 0.0f, 0.0f, 0.0f );
}

void CPhysicsSystem::VApplyForce(const Vec& dir, ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		body->applyCentralForce( dir.TobtVector3() );
	}
}

bool CPhysicsSystem::VAddConstraint( BulletConstraint* opc )
{	
	if ( btRigidBody * const body = FindActorBody( opc->m_TargetedObj ) )
	{
		// Cache the result.
		m_CachedConstraints[opc->m_TargetedObj] = opc;

		BulletConstraintType bcType = opc->m_Type;
		if ( bcType == BulletConstraintType::HingeType )
		{
			HingeBulletConstraint* bcHinge = ( HingeBulletConstraint* )opc;
			assert( bcHinge );
			btHingeConstraint* constraint = new btHingeConstraint( *body, bcHinge->m_AxisPos.TobtVector3(), bcHinge->m_Axis.TobtVector3(), false );
			assert( constraint );
			m_DynamicsWorld->addConstraint( constraint );
			body->addConstraintRef( constraint );
		}

		return true;
	}

	return false;
}

int CPhysicsSystem::VGetNumberOfConstraints(ObjectId id)
{
	if ( m_CachedConstraints.find(id) != m_CachedConstraints.end() )
		return 1;
	return 0;
}

bool CPhysicsSystem::VRemoveConstraints( ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		// Remove all constraints.
		int numberOfConstraints = body->getNumConstraintRefs();
		for ( int i = 0; i < numberOfConstraints; ++i )
		{
			btTypedConstraint* constraint = body->getConstraintRef( i );
			body->removeConstraintRef( constraint );
			m_DynamicsWorld->removeConstraint( constraint );
			SafeDelete( constraint );
		}

		if ( m_CachedConstraints.find( id ) != m_CachedConstraints.end() )
		{
			BulletConstraint* bulletConstraint = m_CachedConstraints.find( id )->second;
			SafeDelete( bulletConstraint );
			m_CachedConstraints.erase( m_CachedConstraints.find( id ) );
		}

		return true;
	}
	return false;
}

Vec CPhysicsSystem::VGetAngularVelocity( ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		btVector3 btAnglularVelocity = body->getAngularVelocity();

		return Vec( btAnglularVelocity );
	}
	return Vec(FLT_MIN);
}

void CPhysicsSystem::VSetAngularVelocity( ObjectId id, const Vec& vel )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		body->setAngularVelocity(vel.TobtVector3());
	}
}

BulletConstraint* CPhysicsSystem::VGetConstraint( ObjectId id )
{
	if ( btRigidBody * const body = FindActorBody( id ) )
	{
		if ( VGetNumberOfConstraints( id ) > 0 )
		{
			BulletConstraint* finalConstraint = m_CachedConstraints.find(id)->second;
			return finalConstraint;
		}
		else
			return 0;
	}

	return 0;
}

std::map<ObjectId, BulletConstraint*> CPhysicsSystem::VGetAllConstraints()
{
	return m_CachedConstraints;
}
