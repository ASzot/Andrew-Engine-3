#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "MathHelper.h"
#include "LightHelper.h"

class CEffectManager;

class CShadowPointLight
{
public:
	CShadowPointLight();
	~CShadowPointLight();

	void Init( PointLight pointLight );

	PointLight GetPointLight( int index );
	std::vector<PointLight> GetPointLights();

	void SetPosition( Vec& pos );
	void SetInfo( XMFLOAT4 diff, XMFLOAT3 att, XMFLOAT4 spec, XMFLOAT4 ambient, float range );

	void Move( Vec mov );

	void AddRange( float range );

private:
	void UpdateLookAt();

private:
	Vec m_vPosition;
	typedef std::vector<PointLight> PLightList;
	PLightList m_PointLights;
	float m_fMag;
};