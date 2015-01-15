//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "ShadowPointLight.h"
#include "EffectManager.h"


CShadowPointLight::CShadowPointLight()
	: m_fMag( 5.0f )
{
}

CShadowPointLight::~CShadowPointLight()
{

}

PointLight CShadowPointLight::GetPointLight( int index )
{
	return m_PointLights.at( index );
}

void CShadowPointLight::SetPosition( Vec& pos )
{
	UpdateLookAt();

	m_vPosition = pos;
	for ( PLightList::iterator i = m_PointLights.begin(); i != m_PointLights.end(); ++i )
	{
		( *i ).Position = pos.ToFloat3();
		// Translate the lookAt to the new world space.
		( *i ).LookAt.x += pos.X();
		( *i ).LookAt.y += pos.Y();
		( *i ).LookAt.z += pos.Z();
	}
}

void CShadowPointLight::SetInfo( XMFLOAT4 diff, XMFLOAT3 att, XMFLOAT4 spec, XMFLOAT4 ambient, float range )
{
	for ( PLightList::iterator i = m_PointLights.begin(); i != m_PointLights.end(); ++i )
	{
		( *i ).Diffuse = diff;
		( *i ).Att = att;
		( *i ).Range = range;
		( *i ).Specular = spec;
		( *i ).Ambient = ambient;
	}
}

void CShadowPointLight::UpdateLookAt()
{
	// Add a slight misprecision to the up and down lights because a straight down or up light doesn't work.
	m_PointLights.at( 0 ).LookAt = XMFLOAT3( 0.000001f, m_fMag, 0.0f );

	m_PointLights.at( 1 ).LookAt = XMFLOAT3( m_fMag, 0.0f, 0.0f );

	m_PointLights.at( 2 ).LookAt = XMFLOAT3( 0.000001f, -m_fMag, 0.0f );

	m_PointLights.at( 3 ).LookAt = XMFLOAT3( -m_fMag, 0.0f, 0.0f );
	
	m_PointLights.at( 4 ).LookAt = XMFLOAT3( 0.0f, 0.0f, m_fMag );

	m_PointLights.at( 5 ).LookAt = XMFLOAT3( 0.0f, 0.0f, -m_fMag );
}

std::vector<PointLight> CShadowPointLight::GetPointLights()
{
	return m_PointLights;
}

void CShadowPointLight::Init( PointLight pointLight )
{
	for ( int i = 0; i < 6; ++i )
		m_PointLights.push_back( pointLight );

	SetPosition( Vec( pointLight.Position ) );
}

void CShadowPointLight::Move( Vec mov )
{
	Vec pos = Vec( m_PointLights.at( 0 ).Position ) + mov;

	SetPosition( pos );
}

void CShadowPointLight::AddRange( float range )
{
	for ( PLightList::iterator i = m_PointLights.begin(); i != m_PointLights.end(); ++i )
	{
		( *i ).Range += range;
	}
}
