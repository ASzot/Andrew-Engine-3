//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "Actor.h"
#include "Terrain.h"


CActor::CActor(void)
	: m_OPC()  // This will init all constraints to infinite values as if there were no constraints.
{
	m_bFirstImpact = true;
}


CActor::~CActor(void)
{
}


void CActor::CheckConstraints(IGamePhysics* phys, IObject* obj)
{
	ObjectId id = obj->GetId();

	// Ensure we are going slower then the max velocity.
	Vec currentVel = phys->VGetVelocity( id );
	Vec origCurrentVel = currentVel;
	float signX = ( ( currentVel.X() < 0 ) ? -1.0f : 1.0f );
	float signY = ( ( currentVel.Y() < 0 ) ? -1.0f : 1.0f );
	float signZ = ( ( currentVel.Z() < 0 ) ? -1.0f : 1.0f );

	if ( currentVel.aX() > m_OPC.m_vMaxVel.aX() )
	{
		currentVel.SetX( m_OPC.m_vMaxVel.X() * signX );
	}
	if ( currentVel.aY() > m_OPC.m_vMaxVel.aY() )
	{
		currentVel.SetY( m_OPC.m_vMaxVel.Y() * signY );
	}
	if ( currentVel.aZ() > m_OPC.m_vMaxVel.aZ() )
	{
		currentVel.SetZ( m_OPC.m_vMaxVel.Z() * signZ );
	}

	if ( origCurrentVel != currentVel )
	{
		phys->VSetVelocity( id, currentVel );
	}
}


void CActor::CheckTerrain(IGamePhysics* phys, IObject* obj)
{
	Vec currentPos = Vec( obj->X(), obj->Y(), obj->Z() );
	ObjectId id = obj->GetId();
	
	float terrainHeight = g_GetTerrainHeightAt( currentPos.X(), currentPos.Z() );
	if ( terrainHeight == FLT_MIN )
	{
		// Terrain isn't existent don't even check.
		return;
	}
	Vec bounds = ( obj->GetBoundriesOfCollisionBox() * 0.5f );
	float actualHeight = currentPos.Y() - bounds.Y();
	if ( terrainHeight >= actualHeight )
	{
		// We are below the terrain, change our position.
 		float newHeight = terrainHeight;
		// Apply a force directly upwards to offset gravity.
		Vec up = Vec( 0, 1, 0 );
		float massOfObject = obj->GetMass() * 4;
		float newtons = 9.8f * massOfObject;
		phys->VApplyForce( up, newtons, id );

		Mat4x4 updatedPos;
		updatedPos.BuildTranslation( currentPos.X(), newHeight, currentPos.Z() );
		phys->VKinematicMove( updatedPos, id );

		Vec currentVel = phys->VGetVelocity( id );

		if ( m_bFirstImpact )
		{
			currentVel.SetY( 0.0f );
			phys->VSetVelocity( id, currentVel  );
		}

		// Simulate the force of friction from the ground.
		const float frictionOfGround = -6.0f;
		Vec forceOfFriction = currentVel * frictionOfGround;
		phys->VApplyForce( forceOfFriction, id );
		
		m_bFirstImpact = false;
	}
	else
	{
		m_bFirstImpact = true;
	}
}


void CActor::SetMaxVelocity( Vec& maxVel )
{
	m_OPC.m_vMaxVel = maxVel;
}

float CActor::GetMaxSpeed()
{
	return m_OPC.m_vMaxVel.Length();
}