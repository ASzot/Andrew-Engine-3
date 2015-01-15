#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "SteeringBehaviors.h"

class CNPC :
	public CSteeringBehaviors
{
public:
	CNPC(void);
	~CNPC(void);


	virtual void ProcessMessage( UINT message );

	void OnBeingShot();

private:
	virtual void UpdatePlayerLogic(Vec& pos);

private:
};

