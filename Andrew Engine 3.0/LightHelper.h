#ifndef LIGHTHELPER_H
#define LIGHTHELPER_H

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <xnamath.h>
#include <vector>

// Note: Make sure structure alignment agrees with HLSL structure padding rules. 
//   Elements are packed into 4D vectors with the restriction that an element
//   cannot straddle a 4D vector boundary.

struct DirectionalLight
{
	DirectionalLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;
	XMFLOAT3 Direction;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct PointLight
{
	PointLight() 
	{ 
		ZeroMemory(this, sizeof(this)); 
		Ambient =	XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		Diffuse =	XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		Att =		XMFLOAT3(0.0f, 0.1f, 0.0f);
		Range =		25.0f;
		Specular =	XMFLOAT4(0.7f, 0.7f, 0.7f, 1.0f);
		Position =  XMFLOAT3(0.0f, 5.0f, 0.0f);
		LookAt = XMFLOAT3( 0.0f, 0.0f, 0.0f );
	}

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	XMFLOAT3 LookAt;
	float Pad1;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (A0, A1, A2, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct SpotLight
{
	SpotLight() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular;

	// Packed into 4D vector: (Position, Range)
	XMFLOAT3 Position;
	float Range;

	// Packed into 4D vector: (Direction, Spot)
	XMFLOAT3 Direction;
	float Spot;

	// Packed into 4D vector: (Att, Pad)
	XMFLOAT3 Att;
	float Pad; // Pad the last float so we can set an array of lights if we wanted.
};

struct Material
{
	Material() { ZeroMemory(this, sizeof(this)); }

	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Specular; // w = SpecPower
	XMFLOAT4 Reflect;
};

struct CSceneLighting
{
	std::vector<PointLight> pointLights;
	std::vector<DirectionalLight> dirLights;
};

#endif // LIGHTHELPER_H