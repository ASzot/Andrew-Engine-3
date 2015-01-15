//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "ConstantsList.h"


namespace 
{
	bool g_bPhysSynced;
};



bool IsPhysicsSynced()
{
	return g_bPhysSynced;
}

void PhysicsSynced( bool sync )
{
	g_bPhysSynced = sync;
}
