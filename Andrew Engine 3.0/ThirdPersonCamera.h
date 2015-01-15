#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "IObject.h"

class CThirdPersonCamera
{
public:
	CThirdPersonCamera(void);
	~CThirdPersonCamera(void);


private:
	float mYaw;
	float mPitch;
	XMVECTOR m_vCamRight;
	XMVECTOR m_vCamForward;
	XMVECTOR m_vCamUp;

	XMVECTOR m_vTarget;
	XMVECTOR m_vPosition;
	XMVECTOR m_vDefaultForward;
	float m_fTargetDist;

	XMMATRIX m_mView;
	XMMATRIX m_mProj;
	float m_fTester;
public:
	void SetYaw(float yaw);
	float GetYaw(void);
	void SetPitch(float pitch);
	float GetPitch(void);

	void UpdateCamera(void);

	void SetTarget(XMFLOAT3 tar);
	XMVECTOR GetTarget(void);
	void SetPosition(XMFLOAT4 pos);
	XMVECTOR GetPosition(void);
	XMMATRIX GetViewMatrix(void);

	void SetLens(float aspectRatio, float nearPlane, float farPlane);
	XMMATRIX GetProj(void);
	XMVECTOR GetForward(void);
	XMVECTOR GetRight(void);
};

