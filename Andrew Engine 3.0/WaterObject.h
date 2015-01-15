#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "StaticObject.h"

class CEffectManager;

class CWaterObject : public CStaticObject
{
public:
	CWaterObject();
	CWaterObject( float posX, float posY, float posZ );
	CWaterObject(float posX, float posY, float posZ, float scaleX, float scaleZ, bool useCircle);
	virtual ~CWaterObject(void);

	virtual void Render( ID3D11DeviceContext* context, CameraInfo& cam, const CEffectManager* effectManager, ExtraRenderingData& erd );

	WaterSystemOptions GetDlgInfo();
	void SetDlgInfo( WaterSystemOptions wso );
	
	std::vector<ObjectId> GetTargetedObjects();


	// The objects which will be rendered as refractions.
	std::vector<ObjectId> m_TargetedObjects;

	void SetWaterScale( float waterScale );
	void SetWaterTexTrans( float waterTexTrans );

	float GetWaterScale();
	float GetWaterTexTrans();

	void UpdateWaterTrans();

	bool IsUsingCircle();

	float GetDiffuseAb();
	float GetSpecAb();

private:
	float m_fWaterScale;
	float m_fCurrentWaterTexTrans;
	float m_fWaterTexTrans;
	float m_fDiffuseAbsorbity;
	float m_fSpecularAbsorbity;
	bool m_bUseCircle;

	const char* m_cPlaneModelName;
	const char* m_cCircleModelName;
};

