//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include "Player.h"

enum Deceleration { Slow = 3, Normal = 2, Fast = 1 };

class CSteeringBehaviors : public CPlayer
{
public:
	CSteeringBehaviors(void);
	~CSteeringBehaviors(void);

	virtual void ProcessMessage( UINT message ) = 0;
	virtual void UpdatePlayerLogic(Vec& pos) { }

	void Seek( Vec& target );
	void Flee( Vec& target );
	void Arrive( Vec& target, Deceleration deceleration );
	void Pursuit( CActor* evader );
	void Evade( CActor* pursuer );
	void Wander();
	void EvadeObstacles();
	virtual void OnRayHit( UINT flags );
	
protected:
	// Length of the ray is the ray length in the z direction.
	// The return value is the object ( if any ) that was hit with the ray.
	IObject* CastRay(const float lengthOfRay);
	XMMATRIX GetWorldWithoutScale();

protected:
	float m_fWanderRadius;
	float m_fWanderDistance;
	float m_fWanderJitter;
	Vec m_vWanderTarget;
	bool m_bSeeking;
};

