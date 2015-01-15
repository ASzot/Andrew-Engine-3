//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "ThirdPersonCamera.h"


CThirdPersonCamera::CThirdPersonCamera(void)
{
	m_vCamRight = XMVectorSet(1.0f,0.0f,0.0f,0.0f);
	m_vCamForward = XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	m_vCamUp = XMVectorSet(0.0f,1.0f,0.0f,0.0f);
	m_vDefaultForward = XMVectorSet(0.0f,0.0f,1.0f,0.0f);
	m_fTargetDist = 7.5f;
	mPitch = 0.0f;
	mYaw = 0.0f;
	m_fTester = 0.0f;
}


CThirdPersonCamera::~CThirdPersonCamera(void)
{
}


void CThirdPersonCamera::SetYaw(float yaw)
{
	mYaw = yaw;
}


float CThirdPersonCamera::GetYaw(void)
{
	return mYaw;
}


void CThirdPersonCamera::SetPitch(float pitch)
{
	if(pitch > 0.85f)
		pitch = 0.85f;
	if(pitch < -0.85)
		pitch = -0.85f;
	mPitch = pitch;
}


float CThirdPersonCamera::GetPitch(void)
{
	return mPitch;
}


void CThirdPersonCamera::UpdateCamera(void)
{
	m_fTester += 0.01f;

	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(-mPitch,mYaw,0);
	//XMMATRIX rotationMatrix = XMMatrixRotationY();
	m_vPosition = XMVector3TransformNormal(m_vDefaultForward,rotationMatrix);
	m_vPosition = XMVector3Normalize(m_vPosition);

	m_vPosition = (m_vPosition * m_fTargetDist) + m_vTarget;
	
	m_vCamForward = XMVector3Normalize(m_vTarget - m_vPosition);
	// So the camera sits on only the xz plane.
	m_vCamForward = XMVectorSetY(m_vCamForward,0.0f);
	m_vCamForward = XMVector3Normalize(m_vCamForward);

	m_vCamRight = XMVectorSet(-XMVectorGetZ(m_vCamForward),0.0f,XMVectorGetX(m_vCamForward),0.0f);

	m_vCamUp = XMVector3Normalize(XMVector3Cross(XMVector3Normalize(m_vPosition - m_vTarget),m_vCamRight));

	m_mView = XMMatrixLookAtLH(m_vPosition,m_vTarget,m_vCamUp);
}


void CThirdPersonCamera::SetTarget(XMFLOAT3 tar)
{
	m_vTarget = XMVectorSet(tar.x,tar.y,tar.z,0.0f);
}


XMVECTOR CThirdPersonCamera::GetTarget(void)
{
	return m_vTarget;
}


void CThirdPersonCamera::SetPosition(XMFLOAT4 pos)
{
	m_vPosition = XMLoadFloat4(&pos);
}


XMVECTOR CThirdPersonCamera::GetPosition(void)
{
	return m_vPosition;
}


XMMATRIX CThirdPersonCamera::GetViewMatrix(void)
{
	return m_mView;
}


void CThirdPersonCamera::SetLens(float aspectRatio, float nearPlane, float farPlane)
{
	m_mProj = XMMatrixPerspectiveFovLH(XM_PI / 4.0f,aspectRatio,nearPlane,farPlane);
}


XMMATRIX CThirdPersonCamera::GetProj(void)
{
	return m_mProj;
}


XMVECTOR CThirdPersonCamera::GetForward(void)
{
	return m_vCamForward;
}


XMVECTOR CThirdPersonCamera::GetRight(void)
{
	return m_vCamRight;
}
