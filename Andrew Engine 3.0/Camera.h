#ifndef CCAMERA_H
#define CCAMERA_H

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "IObject.h"

struct CameraInfo
{
	XMMATRIX View;
	XMMATRIX Proj;
	// The reflection is based off the height given.
	XMMATRIX Refl;
	XMFLOAT3 LookAt;
	XMFLOAT3 Position;
	XMFLOAT4 Right;
	XMFLOAT4 Forward;
	float Yaw;
	float Pitch;
};

class Camera
{
public:
	Camera();
	~Camera();

	// Get/Set world camera position.
	XMVECTOR GetPositionXM()const;
	XMFLOAT3 GetPosition()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const XMFLOAT3& v);
	
	// Get camera basis vectors.
	XMVECTOR GetRightXM()const;
	XMFLOAT3 GetRight()const;
	XMVECTOR GetUpXM()const;
	XMFLOAT3 GetUp()const;
	XMVECTOR GetLookXM()const;
	XMFLOAT3 GetLook()const;

	// Get frustum properties.
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;
	
	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf,int screenWidth,int screenHeight);

	// Define camera space via LookAt parameters.
	void LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp);
	void LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up);

	// Get View/Proj matrices.
	XMMATRIX View()const;
	XMMATRIX Proj()const;
	XMMATRIX ViewProj()const;
	XMMATRIX ReflectionView()const;

	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
	void Walk(float d);

	// Rotate the camera.
	void Pitch(float angle);
	void RotateY(float angle);

	// After modifying camera position/orientation, call to rebuild the view matrix.
	void UpdateViewMatrix();
	void UpdateViewThirdPerson();

	inline float GetYaw()    { return m_fTotalYaw; }
	inline float GetPitch() { return m_fTotalPitch; }

	void UpdateRenderReflection( float height );
	static XMMATRIX ConstructReflectionMatrix( float height, XMFLOAT3 pos, XMFLOAT3 lookAt );

private:

	// Camera coordinate system with coordinates relative to world space.
	XMFLOAT3 mPosition;
	XMFLOAT3 mRight;
	XMFLOAT3 mUp;
	XMFLOAT3 mLook;

	// Cache frustum properties.
	float mNearZ;
	float mFarZ;
	float mAspect;
	float mFovY;
	float mNearWindowHeight;
	float mFarWindowHeight;
	bool firstTime;
	float mYaw;
	float mPitch;

	float m_fTotalYaw;
	float m_fTotalPitch;

	// Cache View/Proj matrices.
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	XMFLOAT4X4 m_4x4ReflectionView;

public:
//	bool Init(CEventManager* eventManager);
};

#endif // CAMERA_H