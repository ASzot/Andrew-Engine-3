//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once


#include <string>
#include <vector>
#include <list>
#include <Windows.h>
#include <xnamath.h>

enum EGraphicsSettings{ eGRAPHICS_SETTINGS_HIGH, eGRAPHICS_SETTINGS_MEDIUM, eGRAPHICS_SETTINGS_LOW };


#define MAX_NUMBER_OF_POINT_LIGHTS 12
#define MAX_NUMBER_OF_DIR_LIGHTS 1

#ifndef FOR_EACH_IN
#define FOR_EACH_IN(object,function) { for(int i =0; i < object.size(); ++i) object.at(i)->function; }
#endif

#define NUMBER_OF_TEXT_BUFFER_INDEXS 5
#define TEXT_BUFFER_SIZE 70
#define MAX_SIZE_OF_ARRAY_FOR_CONVERTING 3

#define HAS_SELECTED_NEW_RENDER_TECH 69

// The number of steps in the loading dialog box.
#define INITIAL_LOADING_STEPS 10

#define GREEN_FLASH_COLOR 15


// Factors for windows.
#define STATIC_OBJECT_DATA_FACTOR 100


// Some typedefs to make life easier.
typedef std::vector<std::wstring> WStringList;
typedef std::vector<std::string> StringList;


// We also have some random structs in here.
struct ObjectPhysicsPropData
{
	float SpecGrav;
	// Just the enum casted.
	int Material;
	float GravityX;
	float GravityY;
	float GravityZ;
};

struct TerrainDlgData
{
	std::string heightmapFilename;
	std::string blendmapFilename;
	float frictionOfTerrain;
};

struct WaterSystemOptions
{
	WaterSystemOptions()
	{
		x = 0;
		y = 0;
		z = 0;
		rx = 0;
		ry = 0;
		rz = 0;
		sx = 0;
		sy = 0;
		sz = 0;
		TexTrans = 0;
		WaterScale = 0;
		DiffuseAbsorbity = 0;
		SpecularAbsorbity = 0;
	}
	std::vector<int> TargetedObjectsForRefraction;
	float x,y,z;
	float sx,sy,sz;
	float rx, ry, rz;
	float TexTrans;
	float WaterScale;
	bool UseCircle;
	float DiffuseAbsorbity;
	float SpecularAbsorbity;
};

struct ParticleSystemData
{
	ParticleSystemData()
	{
		EmitTime=RandomZMod=RandomXMod=InitVelocity=TextureSize=dx=dy=dz=x=y=z=0.0f;
		MaxParticles = 0;
	}
	float x,y,z;
	float dx,dy,dz;
	unsigned int MaxParticles;
	float TextureSize;
	float InitVelocity;
	float RandomXMod;
	float RandomZMod;
	float EmitTime;
	float pax,pay,paz;
	const char* code;
};

struct GraphicsInfo
{
	bool UseSkymap;
	bool Flash;
};

// Predefine class.
class BulletConstraint;

struct KinematicsInfo
{
	bool HasConstraint;
	int SelectedObject;
	float AppliedForce;
	float AppliedTorque;
	XMFLOAT3 LinearVelocity;
	XMFLOAT3 AngularVelocity;
	BulletConstraint* Constraint;
};

enum eManager { eObjectMgr, eLightMgr, eParticleMgr, eWaterMgr };


///<summary> Init info for the terrain object. \n( Super specific right? ) </summary>
struct InitInfo
{
	std::wstring HeightMapFilename;
	std::wstring LayerMapFilename0;
	std::wstring LayerMapFilename1;
	std::wstring LayerMapFilename2;
	std::wstring LayerMapFilename3;
	std::wstring LayerMapFilename4;
	std::wstring BlendMapFilename;
	float HeightScale;
	unsigned int HeightmapWidth;
	unsigned int HeightmapHeight;
	float CellSpacing;
};

struct PureFloatData
{
	PureFloatData()
	{
		x1=x2=x3=x4=0.0f;
		y1=y2=y3=y4=0.0f;
		z1=z2=z3=z4=0.0f;
	}
	float x1,x2,x3,x4;
	float y1,y2,y3,y4;
	float z1,z2,z3,z4;
};



// Have to do with validating that the user hasn't changed some physics params 
// and then forgot to update the physics. 
bool IsPhysicsSynced();

void PhysicsSynced( bool sync );


// File defines
static std::string TEXTURES_FOLDER		= "data/Textures/";
static std::string HEIGHTMAPS_FOLDER	= "data/Textures/Heightmaps";
static std::string SKYMAPS_FOLDER		= "data/Textures/Skymaps";
static std::string COLORMAPS_FOLDER		= "data/Textures/Heightmaps/ColorMaps";
static std::string MODELS_FOLDER		= "data/Models/";


// The ray cast flags.
#define SHOT_BULLET			0
#define CAST_RAY			1




// Random defines.
#define TIME_TEXT_ALIVE		5

#define ERROR_INDEX			-1