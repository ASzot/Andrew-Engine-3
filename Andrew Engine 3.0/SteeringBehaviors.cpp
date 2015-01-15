//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "SteeringBehaviors.h"


CSteeringBehaviors::CSteeringBehaviors(void)
	: CPlayer()
{
	m_fWanderRadius = 5.0f;
	m_fWanderDistance = 60.0f;
	m_fWanderJitter = 0.01f;
	
	m_vWanderTarget.Normalize();
}


CSteeringBehaviors::~CSteeringBehaviors(void)
{
}


void CSteeringBehaviors::Seek( Vec& target )
{
	Vec pos = GetPosition();
	Vec dir = target - pos;
	ChangeFacingDirection( dir );
	SetVelocity( dir );
}

void CSteeringBehaviors::Arrive( Vec& target, Deceleration deceleration )
{
	Vec toTarget = target - GetPosition();

	float distance = toTarget.Length();

	if ( distance > 0 )
	{
		// Used to fine tweak the deceleration.
		const float decelTweaker = 0.4f;

		// Calculate the speed.
		m_fSpeed = distance / ( ( float )deceleration * decelTweaker );

		m_fSpeed = min( m_fSpeed, GetMaxSpeed() );

		ChangeFacingDirection( toTarget );
		SetVelocity( toTarget );
	}
	else
		SetVelocity( Vec( 0.0f, 0.0f, 0.0f ) );
}

void CSteeringBehaviors::Flee( Vec& target )
{
	// Basically just the opposite of seek.
	Vec pos = GetPosition();
	Vec dir = pos - target;
	ChangeFacingDirection( dir );
	SetVelocity( dir );
}

void CSteeringBehaviors::Pursuit( CActor* evader )
{
	Vec toEvader = evader->GetPosition() - GetPosition();

	float relativeHeading = GetFacing().Dot( evader->GetFacing() );
	
	// acos( 0.95 ) = 18.
	if ( ( toEvader.Dot( GetFacing() ) > 0.0f ) && ( relativeHeading < -0.95f ) ) 
	{
		// Already facing the target.
		Seek( evader->GetPosition() );
	}

	// Not ahead so predict where the evader will be.
	const float lookAheadTimeModifier = 1.0f;
	float lookAheadTime = toEvader.Length() / ( m_fSpeed + evader->GetSpeed() );

	lookAheadTime *= lookAheadTimeModifier;

	return Seek( evader->GetPosition() + evader->GetVelocity() * lookAheadTime );
}

void CSteeringBehaviors::Evade( CActor* pursuer )
{
	// Not necessary to included a check for facing direction this time.
	Vec toPursuer = pursuer->GetPosition() - GetPosition();

	float lookAheadTime = toPursuer.Length() / ( m_fSpeed + pursuer->GetSpeed() );

	return Flee( pursuer->GetPosition() + pursuer->GetVelocity() * lookAheadTime );
}

void CSteeringBehaviors::Wander()
{
	if ( !m_bSeeking )
	{
		m_vWanderTarget = m_vWanderTarget + Vec( SMathHelper::RandF( -1, 1 )* m_fWanderJitter, 0.0f,
			SMathHelper::RandF( -1, 1 ) * m_fWanderJitter );
	
		m_vWanderTarget.Normalize();
	
		m_vWanderTarget = m_vWanderTarget * m_fWanderRadius;
		m_vWanderTarget.AddZ(10.0f);
	
		//Vec targetLocal = m_vWanderTarget + Vec( 0, 0, m_fWanderDistance );
		// Transform the coordinate into world space.
	
		m_bSeeking = true;
	}
	else
	{
		Vec dir = m_vWanderTarget.Transform( GetWorldWithoutScale() ) - GetPosition();
		// Check if we reached our destination.
		if ( dir.Length() < 0.1f )
			m_bSeeking = false;

		Vec targetWorld = m_vWanderTarget.Transform( GetWorldWithoutScale() );
		m_fSpeed = 5.0f;
		Seek(targetWorld);
	}
}

void CSteeringBehaviors::EvadeObstacles()
{
	const float minLength = 5.0f;
	const float brakingWeight = 0.2f;
	const float rayLength = 5.0f;
	IObject* obj = CastRay( rayLength );
	if ( obj )
	{
		// There was an intersection at some point within the range of our rayLength.

		Vec steeringForce;
		// This should be proportional to our speed and our closeness to the object.
		Vec toObject = Vec( obj->X(), obj->Y(), obj->Z() ) - GetPosition();
		float distanceFromObject = ( toObject ).Length();

		// Negative x - means the object is to the of right of us and a positive x means it is to the left.
		float sign = ( toObject.X() < 0.0f ) ? -1.0f : 1.0f;

		steeringForce.SetX(m_fSpeed * ( 1 / distanceFromObject ) * -sign);

		// Transform to world space.
		/*steeringForce = steeringForce.Transform( GetWorldWithoutScale() );
		steeringForce.SetY(0.0f);*/

		g_GetGamePhysics()->VAddVelocity( m_iIndex, steeringForce );
		ChangeFacingDirection(steeringForce);
	}

}

void CSteeringBehaviors::OnRayHit( UINT flags )
{
	CPlayer::OnRayHit( flags );
}

IObject* CSteeringBehaviors::CastRay( const float lengthOfRay )
{
	IGamePhysics* phys = g_GetGamePhysics();
	
	XMMATRIX final = GetWorldWithoutScale();

	Vec endPos = Vec( 0.0f, 0.0f, lengthOfRay );
	Vec finalEndPos = endPos.Transform(final);

	// The primary object.
	IObject* obj1 = phys->VCastRay(GetPosition(), finalEndPos, CAST_RAY );

	endPos = Vec( 1.0f, 0.0f, lengthOfRay );
	finalEndPos = endPos.Transform(final);
	Vec pos = GetPosition();
	pos.AddX(1.0f);
	IObject* obj2 = phys->VCastRay( pos, finalEndPos, CAST_RAY );

	endPos = Vec( -1.0f, 0.0f, lengthOfRay );
	finalEndPos = endPos.Transform(final);
	pos = GetPosition();
	pos.AddX(-1.0f);
	IObject* obj3 = phys->VCastRay( pos, finalEndPos, CAST_RAY );

	if ( obj1 )
		return obj1;
	else if ( obj2 )
		return obj2;
	else if ( obj3 )
		return obj3;
	else
		return 0;
}

XMMATRIX CSteeringBehaviors::GetWorldWithoutScale()
{
	CGameObjMgr* mgr = g_GetGameObjMgr();
	CAnimatedObject* obj = static_cast<CAnimatedObject*>( mgr->GetObjectByIndex( m_iIndex ) );


	XMMATRIX final, rotx, roty, rotz, trans;
	rotx = XMMatrixRotationX( obj->RX() * ( 180 / XM_PI ) );
	roty = XMMatrixRotationY( obj->RY() * ( 180 / XM_PI ) );
	rotz = XMMatrixRotationZ( obj->RZ() * ( 180 / XM_PI ) );
	trans = XMMatrixTranslationFromVector( GetPosition().ToVector() );
	final = rotx * roty * rotz * trans;

	return final;
}
