#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <Windows.h>
#include <xnamath.h>
#include <D3DX10math.h>
#include <vector>
#include <btBulletCollisionCommon.h>

namespace CT
{
	static std::string WStrToStr( std::wstring in )
	{
		std::string r;
		r.resize( in.size() );
		for ( int i = 0; i < in.size(); ++i )
		{
			r.at(i) = in.at(i);
		}

		return r;
	}
};

class SMathHelper
{
public:
	static XMMATRIX InverseTranspose(CXMMATRIX M)
	{
		// Inverse-transpose is just applied to normals.  So zero out 
		// translation row so that it doesn't get into our inverse-transpose
		// calculation--we don't want the inverse-transpose of the translation.
		XMMATRIX A = M;
		A.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		XMVECTOR det = XMMatrixDeterminant(A);
		return XMMatrixTranspose(XMMatrixInverse(&det, A));
	}

	static float RandF()
	{
		return (float)(rand()) / (float)RAND_MAX;
	}

	// Returns random float in [a, b).
	static float RandF(float a, float b)
	{
		return a + RandF()*(b-a);
	}

	template<typename T>
	static T Min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template<typename T>
	static T Max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}
	 
	template<typename T>
	static T Lerp(const T& a, const T& b, float t)
	{
		return a + (b-a)*t;
	}

	template<typename T>
	static T Clamp(const T& x, const T& low, const T& high)
	{
		return x < low ? low : (x > high ? high : x); 
	}

	static float Round(float input)
	{
		int temp = (int)input;
		return (float)temp;
	}

	static float AngleFromXY(float x, float y)
	{
		float theta = 0.0f;
 
		// Quadrant I or IV
		if(x >= 0.0f) 
		{
			// If x = 0, then atanf(y/x) = +pi/2 if y > 0
			//                atanf(y/x) = -pi/2 if y < 0
			theta = atanf(y / x); // in [-pi/2, +pi/2]

			if(theta < 0.0f)
				theta += 2.0f*3.1415926535f; // in [0, 2*pi).
		}

		// Quadrant II or III
		else      
			theta = atanf(y/x) + 3.1415926535f; // in [0, 2*pi).

		return theta;
	}
	
	static void GetMinAndMaxAsBox( float& w, float& h, float& l, XMFLOAT4 min, XMFLOAT4 max )
	{
		w = abs( max.x - min.x );
		l = abs( max.z - min.z );
		h = abs( max.y - min.y );
	}
};

// Just a little struct to wrap up the annoying XMVECTOR class
enum Directions { eLeft,eForward,eRight,eBackward,eUp,eDown };
struct Vec
{
public:
	Vec()
	{
		vData = XMFLOAT3(0.0f,0.0f,0.0f);
	}
	Vec(float x,float y,float z)
	{
		vData = XMFLOAT3(x,y,z);
	}
	Vec(float u)
	{
		vData = XMFLOAT3(u,u,u);
	}
	Vec(XMFLOAT3 f)
	{
		vData = f;
	}
	Vec(btVector3& v)
	{
		vData.x = v.x();
		vData.y = v.y();
		vData.z = v.z();
	}

	Vec(XMVECTOR& v)
	{
		vData.x = XMVectorGetX(v);
		vData.y = XMVectorGetY(v);
		vData.z = XMVectorGetZ(v);
	}

	Vec(XMFLOAT4 f)
	{
		vData.x = f.x;
		vData.y = f.y;
		vData.z = f.z;
	}

	Vec(Directions d)
	{
		switch(d)
		{
		case eLeft:
			Vec(1.0f,0.0f,0.0f);
			break;
		case eRight:
			Vec(-1.0f,0.0f,0.0f);
			break;
		case eForward:
			Vec(0.0f,0.0f,1.0f);
			break;
		case eBackward:
			Vec(0.0f,0.0f,-1.0f);
			break;
		case eDown:
			Vec(0.0f,-1.0f,0.0f);
			break;
		case eUp:
			Vec(0.0f,1.0f,0.0f);
			break;
		}
	}

	void AddX(float ax)
	{
		vData.x = X() + ax;
	}
	void AddY(float ay)
	{
		vData.y = Y() + ay;
	}
	void AddZ(float az)
	{
		vData.z = Z() + az;
	}

	Vec operator+(const Vec& value)const
	{
		return Vec( (this->X() + value.X()), (this->Y() + value.Y()), (this->Z() + value.Z()) );
	}

	Vec operator-(const Vec& value)const
	{
		return Vec( (this->X() - value.X()), (this->Y() - value.Y()), (this->Z() - value.Z()) );
	}
	Vec operator*(const Vec& value)const
	{
		return Vec( (this->X() * value.X()), (this->Y() * value.Y()), (this->Z() * value.Z()) );
	}
	Vec operator/(const Vec& value)const
	{
		return Vec( (this->X() / value.X()), (this->Y() / value.Y()), (this->Z() / value.Z()) );
	}
	Vec operator*(const float value)const
	{
		return Vec( (this->X() * value), (this->Y() * value), (this->Z() * value) );
	}
	bool operator==(const Vec& value)const
	{
		if( (this->X() == value.X()) 
			&& (this->Y() == value.Y()) 
			&& (this->Z() == value.Z()) )
			return true;
		return false;
	}
	bool operator!=(const Vec& value)const
	{
		if(this->X() == value.X())
			if(this->Y() == value.Y())
				if(this->Z() == value.Z())
					return false;
		return true;
	}

	XMFLOAT3 ToFloat3()
	{
		return vData;
	}
	XMFLOAT4 ToFloat4()
	{
		return XMFLOAT4(X(),Y(),Z(),0.0f);
	}
	XMVECTOR ToVector()
	{
		return XMVectorSet(X(),Y(),Z(),0.0f);
	}

	btVector3 TobtVector3() const
	{
		btVector3 out;
		out.setX( vData.x );
		out.setY( vData.y );
		out.setZ( vData.z );

		return out;
	}

	//Multiplies each component of the vector by the scalar.
	void MultX(float mx)
	{
		vData.x = X() * mx;
	}
	void MultY(float my)
	{
		vData.y = Y() * my;
	}
	void MultZ(float mz)
	{
		vData.z = Z() * mz;
	}


	void Normalize()
	{
		XMVECTOR v = XMLoadFloat3( &vData );
		v = XMVector3Normalize( v );
		XMStoreFloat3( &vData, v );
	}

	float Dot( Vec& v1 )
	{
		return ( ( this->X() * v1.X() ) + ( this->Z() * v1.Z() ) );
	}

	Vec Round()
	{
		vData.x = SMathHelper::Round( vData.x );
		vData.y = SMathHelper::Round( vData.y );
		vData.z = SMathHelper::Round( vData.z );

		return *this;
	}

	//For a custom function you wrote.
	void ForEach(float (*func)(float))
	{
		SetX( func( X() ) );
		SetY( func( Y() ) );
		SetZ( func( Z() ) );
	}

	float Length()
	{
		return sqrt( (vData.x * vData.x) + (vData.y * vData.y) + (vData.z * vData.z) );
	}

	Vec Transform( XMMATRIX& mat )
	{
		XMVECTOR vec = XMLoadFloat3( &vData );
		XMVECTOR final = XMVector3TransformCoord( vec, mat );

		return Vec( final );
	}
	
	inline void SetX(float x) { vData.x = x; }
	inline void SetY(float y) { vData.y = y; }
	inline void SetZ(float z) { vData.z = z; }

	inline float X()const { return vData.x; }
	inline float Y()const { return vData.y; }
	inline float Z()const { return vData.z; }

	inline float aX() const { return abs( vData.x ); }
	inline float aY() const { return abs( vData.y ); }
	inline float aZ() const { return abs( vData.z ); }

private:
	XMFLOAT3 vData;
};


static Vec XMVectorToVec(XMVECTOR in)
{
	return Vec( XMVectorGetX( in ), XMVectorGetY( in ), XMVectorGetZ( in ) );
}


typedef Vec& rVec;
typedef std::vector<Vec> VecList;




struct Mat4x4 : public D3DXMATRIX
{
	inline void SetPosition(Vec const &pos);

	void CreateFromOrientation(float x,float y,float z,float rx,float ry,float rz,float sx,float sy,float sz)
	{
		D3DXMATRIX pos,rotX,rotY,rotZ,scale;
		D3DXMatrixTranslation(&pos,x,y,z);
		D3DXMatrixRotationX(&rotX,rx);
		D3DXMatrixRotationY(&rotY,ry);
		D3DXMatrixRotationZ(&rotZ,rz);
		D3DXMatrixScaling(&scale,sx,sy,sz);
		*this = scale * rotX * rotY * rotZ * scale;
	}
	
	inline Vec GetPosition() const;
	inline Mat4x4 Inverse()const;
	
	Mat4x4(D3DXMATRIX& m)
		: D3DXMATRIX(m)
	{
		
	}
	Mat4x4() : D3DXMATRIX() {  }

	XMMATRIX ToXMMatrix()
	{
		XMMATRIX m;
		for(int i = 0; i < 4; ++i)
		{
			for(int j = 0; j < 4; ++j)
			{
				m.m[i][j] = this->m[i][j];
			}
		}
		return m;
	}

	Mat4x4 Identity()
	{
		D3DXMATRIX ident;
		D3DXMatrixIdentity(&ident);
		return Mat4x4(ident);
	}

	inline void BuildTranslation(const Vec& pos);
	inline void BuildTranslation(const float x,const float y,const float z);
	inline void BuildRotationX(const float radians) { D3DXMatrixRotationX(this,radians); }
	inline void BuildRotationY(const float radians) { D3DXMatrixRotationY(this,radians); }
	inline void BuildRotationZ(const float radians) { D3DXMatrixRotationZ(this,radians); }
	inline void BuildYawPitchRoll(const float yawRadians, const float pitchRadians, const float rollRadians);

};

inline void Mat4x4::SetPosition(Vec const& pos)
{
	m[0][0] = pos.X();
	m[3][1] = pos.Y();
	m[3][2] = pos.Z();
	m[3][3] = 1.0f;
}

inline Vec Mat4x4::GetPosition()const
{
	return Vec(m[3][0],m[3][1],m[3][2]);
}

inline Mat4x4 Mat4x4::Inverse()const
{
	Mat4x4 out;
	D3DXMatrixInverse(&out,0,this);
	return out;
}

inline void Mat4x4::BuildTranslation(const Vec& pos)
{
	*this = Identity();
	m[3][0] = pos.X();
	m[3][1] = pos.Y();
	m[3][2] = pos.Z();
}

inline void Mat4x4::BuildTranslation(const float x,const float y,const float z)
{
	*this = Identity();
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

inline Mat4x4 operator * (const Mat4x4& a, const Mat4x4& b)
{
	Mat4x4 out;
	D3DXMatrixMultiply(&out,&a,&b);
	return out;
}






