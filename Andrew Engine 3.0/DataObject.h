#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DataArchiver.h"
#include "ConstantsList.h"

struct PositionData
{
	PositionData()
	{
		x	= 0;
		y	= 0;
		rx	= 0;
		ry	= 0;
		rz	= 0;
		s	= 0;
		sy	= 0;
		sz	= 0;
	}
	PositionData(float x,float y,float z,float rx,float ry,float rz,float s)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->rx = rx;
		this->ry = ry;
		this->rz = rz;
		this->s = s;
		this->sy = 0;
		this->sz = 0;
	}
	PositionData(float x,float y,float z,float rx,float ry,float rz,float s,float sy,float sz)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->rx = rx;
		this->ry = ry;
		this->rz = rz;
		this->s = s;
		this->sy = sy;
		this->sz = sz;
	}
	float x;
	float y;
	float z;
	float rx;
	float ry;
	float rz;
	float s;
	// Usually not used.
	float sy;
	float sz;
};

class CDataObject
{
public:
	CDataObject(void);
	virtual ~CDataObject(void);

	virtual void Shutdown() = 0;
	virtual void Serilize(CDataArchiver* archiver) = 0;
};

class CGameResource
{
public:
	CGameResource()
	{

	}
	virtual ~CGameResource()
	{

	}

	virtual void DestroyResources() = 0;
	virtual void Shutdown() = 0;
};