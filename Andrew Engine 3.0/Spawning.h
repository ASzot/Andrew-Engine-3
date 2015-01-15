#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Player.h"
#include "Physics.h"

struct SpawnInfo
{
	SpawnInfo(PositionData data, char* filename, char* animClip, BulletData bd, const wchar_t* texDir = L"data/Textures/" )
	{
		this->data = data;
		this->filename = filename;
		this->texDir = texDir;
		this->animClip = animClip;
		this->bulletData = bd;
	}
	PositionData data;
	const char* filename;
	const wchar_t* texDir;
	char* animClip;
	BulletData bulletData;
};