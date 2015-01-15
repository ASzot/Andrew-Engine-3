#include "DXUT.h"
#include "BulletDebugDrawer.h"

void CBulletDebugDrawer::drawLine( const btVector3& from, const btVector3& to, const btVector3& color )
{
	Vec vFrom(from.x(),from.y(),from.z());
	Vec vTo(to.x(),to.y(),to.z());

	m_pLineDrawer->SetDrawColor( XMFLOAT4( color.x(), color.y(), color.z(), 1.0f) );
	m_pLineDrawer->SetDrawLocation( vFrom, vTo );
	m_pLineDrawer->DrawLine( DXUTGetD3D11DeviceContext() );
}

void CBulletDebugDrawer::drawContactPoint( const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color )
{

}

void CBulletDebugDrawer::reportErrorWarning(const char* warningString)
{

}

void CBulletDebugDrawer::draw3dText(const btVector3& location,const char* textString)
{

}

// This controls what is drawn.
void CBulletDebugDrawer::setDebugMode( int debugMode )
{
	// not implemented this class just draws everything, all the time.
}

int CBulletDebugDrawer::getDebugMode() const
{
	// This is so everything is drawn
	return -1;
}

bool CBulletDebugDrawer::Init(Camera* cam)
{
	m_pLineDrawer = new CLineDrawer( cam ,XMFLOAT4(1.0f,0.0f,0.0f,1.0f) );
	assert(m_pLineDrawer);

	return true;
}

bool CBulletDebugDrawer::InitResources( ID3D11Device* device )
{
	if(!m_pLineDrawer->InitResources( device ))
		return false;

	return true;
}