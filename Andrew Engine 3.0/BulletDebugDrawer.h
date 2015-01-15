#pragma once

#include <btBulletDynamicsCommon.h>
#include "EventManager.h"
#include "LineDrawer.h"


class CBulletDebugDrawer :
	public btIDebugDraw
{
private:
	CLineDrawer* m_pLineDrawer;

public:
	virtual void drawLine( const btVector3& from, const btVector3& to, const btVector3& color );
	virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);

	virtual void draw3dText(const btVector3& location,const char* textString);

	virtual void reportErrorWarning(const char* warningString);
	virtual void setDebugMode(int debugMode);
	virtual int	 getDebugMode() const;

	bool Init(Camera* cam);
	bool InitResources( ID3D11Device* device );

	void DestroyResources()
	{
		m_pLineDrawer->DestroyResources();
	}

	void Shutdown()
	{
		SafeDelete( m_pLineDrawer );
	}
};

