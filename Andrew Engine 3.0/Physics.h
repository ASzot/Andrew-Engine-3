#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include "MathHelper.h"
//#include "DataArchiver.h"

// Predefined classes.
//class CDataArchiver;


// Kind of Like an indexer for the g_PhysicsMaterials.
enum PhysicsMaterial
{
	PhysMat_Playdough,
	PhysMat_Normal,
	PhysMat_Bouncy,
	PhysMat_Slippery,
};

// This is kind of like the gravity modifier.
enum PhysicsDensity
{
	//TODO:
	// Add more materials and create the 
	// actual values.

	// Metals and Stone.
	PhysDens_Silicon,
};

struct PhysMaterialData
{
	// By the way restitution is practically bounciness,
	// i.e. a bouncy ball would have .95 restitution while
	// play-dough would have 0 since it doesn't bounce.
	float restitution;
	float friction;
};

namespace
{
	PhysMaterialData g_PhysicsMaterials[] = 
	{
		// restitution	   // Friction
		{ 0.05f,			0.9f }, // Play dough
		{ 0.0f,				0.6f }, // 'Normal'
		{ 0.95f,			0.5f }, // Bouncy
		{ 0.25f,			0.0f }, // Slippery
	};
}


struct ObjectPhysConstraint
{
	ObjectPhysConstraint()
	{
		m_vMaxVel = Vec( FLT_MAX );
	}
	Vec m_vMaxVel;
};


enum BulletConstraintType { HingeType, PointType, SliderType, GenericType };
struct BulletConstraint
{
	BulletConstraintType m_Type;
	int m_TargetedObj;

	// Only writes the data not the standard formatters.
	virtual void Serilize(CDataArchiver* archiver) = 0;
};

struct HingeBulletConstraint : public BulletConstraint
{
	Vec m_Axis;
	Vec m_AxisPos;
	Vec m_Limits;

	virtual void Serilize(CDataArchiver* archiver)
	{
		archiver->WriteToStream( m_Axis.ToFloat3() );
		archiver->Space();
		archiver->WriteToStream( m_AxisPos.ToFloat3() );
		archiver->Space();
		archiver->WriteToStream( m_Limits.ToFloat3() );
	}
};

  //////////////////////////////////////////////////////////////////////////////////////////////
 // Some conversion funcs to convert from the annoying primitive bullet types to our types.////
//////////////////////////////////////////////////////////////////////////////////////////////


// Watch out i am not so sure that this is going to work.
static btTransform Mat4x4_to_btTransform(Mat4x4 const & mat)
{
	btMatrix3x3 bulletRotation;
	btVector3 bulletPosition;

	// Copy rotation.
	for( int row = 0; row < 3; ++row)
		for(int column = 0; column < 3; ++column)
			bulletRotation[row][column] = mat.m[column][row];

	
	for ( int column = 0; column < 3; ++column)
		bulletPosition[column] = mat.m[3][column];
	
	return btTransform( bulletRotation, bulletPosition );
}

static Mat4x4 btTransform_to_Mat4x4( btTransform const & mat )
{
	Mat4x4 returnValue;
	returnValue = returnValue.Identity();

	btMatrix3x3 const & bulletRotation = mat.getBasis();
	btVector3 const & bulletPosition = mat.getOrigin();

	for(int row = 0; row < 3; ++row)
		for(int column = 0; column < 3; ++ column)
			returnValue.m[row][column] = bulletRotation[column][row];
								// There is reasoning behind this column row reverse madness and it has to do
								// with the matrices being column major or row major.

	for(int column = 0; column < 3; ++column)
		returnValue.m[3][column] = bulletPosition[column];

	return returnValue;
}

static Vec btVector_to_Vec(btVector3 const & btVec)
{
	return Vec(btVec.x(), btVec.y(), btVec.z() );
}

static btVector3 Vec_to_btVector3(Vec const & vec)
{
	return btVector3(vec.X(), vec.Y(), vec.Z() );
}