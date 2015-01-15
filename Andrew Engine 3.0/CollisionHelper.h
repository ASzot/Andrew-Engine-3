#pragma once

#include <xnamath.h>
#include <float.h>
#include "MathHelper.h"

///////////////////
// Using the vectors for the SIMD (SingleInstructionMultipleData).



namespace Collision
{
	struct BoundingBox
	{
		XMFLOAT4 MaxVertex;
		XMFLOAT4 MinVertex;
	};

	static BoundingBox CalculateBoundingBox(std::vector<XMFLOAT3> boundingBoxVerts,XMMATRIX& worldSpace)
	{
		BoundingBox bb;
		XMVECTOR boundingBoxMin;
		XMVECTOR boundingBoxMax;

		XMFLOAT3 minVertex = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
		XMFLOAT3 maxVertex = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		//Loop through the 8 vertices describing the bounding box
		for(UINT i = 0; i < boundingBoxVerts.size(); i++)
		{		
			//Transform the bounding boxes vertices to the objects world space
			XMVECTOR Vert = XMVectorSet(boundingBoxVerts[i].x, boundingBoxVerts[i].y, boundingBoxVerts[i].z, 0.0f);
			Vert = XMVector3TransformCoord(Vert, worldSpace);

			//Get the smallest vertex 
			minVertex.x = min(minVertex.x, XMVectorGetX(Vert));	// Find smallest x value in model
			minVertex.y = min(minVertex.y, XMVectorGetY(Vert));	// Find smallest y value in model
			minVertex.z = min(minVertex.z, XMVectorGetZ(Vert));	// Find smallest z value in model

			//Get the largest vertex 
			maxVertex.x = max(maxVertex.x, XMVectorGetX(Vert));	// Find largest x value in model
			maxVertex.y = max(maxVertex.y, XMVectorGetY(Vert));	// Find largest y value in model
			maxVertex.z = max(maxVertex.z, XMVectorGetZ(Vert));	// Find largest z value in model
		}

		//Store Bounding Box's min and max vertices
		boundingBoxMin = XMVectorSet(minVertex.x, minVertex.y, minVertex.z, 0.0f);
		boundingBoxMax = XMVectorSet(maxVertex.x, maxVertex.y, maxVertex.z, 0.0f);

		XMStoreFloat4(&bb.MaxVertex,boundingBoxMax);
		XMStoreFloat4(&bb.MinVertex,boundingBoxMin);

		return bb;
	}

	static bool BoundingBoxCollision(BoundingBox b1,BoundingBox b2)
	{
		XMVECTOR firstObjBoundingBoxMinVertex = XMLoadFloat4(&b1.MinVertex);
		XMVECTOR firstObjBoundingBoxMaxVertex = XMLoadFloat4(&b1.MaxVertex);
		XMVECTOR secondObjBoundingBoxMinVertex = XMLoadFloat4(&b2.MinVertex);
		XMVECTOR secondObjBoundingBoxMaxVertex = XMLoadFloat4(&b2.MaxVertex);

		//Is obj1's max X greater than obj2's min X? If not, obj1 is to the LEFT of obj2
		if (XMVectorGetX(firstObjBoundingBoxMaxVertex) > XMVectorGetX(secondObjBoundingBoxMinVertex))

			//Is obj1's min X less than obj2's max X? If not, obj1 is to the RIGHT of obj2
			if (XMVectorGetX(firstObjBoundingBoxMinVertex) < XMVectorGetX(secondObjBoundingBoxMaxVertex))

				//Is obj1's max Y greater than obj2's min Y? If not, obj1 is UNDER obj2
				if (XMVectorGetY(firstObjBoundingBoxMaxVertex) > XMVectorGetY(secondObjBoundingBoxMinVertex))

					//Is obj1's min Y less than obj2's max Y? If not, obj1 is ABOVE obj2
					if (XMVectorGetY(firstObjBoundingBoxMinVertex) < XMVectorGetY(secondObjBoundingBoxMaxVertex)) 

						//Is obj1's max Z greater than obj2's min Z? If not, obj1 is IN FRONT OF obj2
						if (XMVectorGetZ(firstObjBoundingBoxMaxVertex) > XMVectorGetZ(secondObjBoundingBoxMinVertex))

							//Is obj1's min Z less than obj2's max Z? If not, obj1 is BEHIND obj2
							if (XMVectorGetZ(firstObjBoundingBoxMinVertex) < XMVectorGetZ(secondObjBoundingBoxMaxVertex))

								//If we've made it this far, then the two bounding boxes are colliding
								return true;

		//If the two bounding boxes are not colliding, then return false
		return false;
	}

	static bool PointInside(BoundingBox b1, Vec& p1)
	{
		if ( p1.X() > b1.MinVertex.x && p1.X() < b1.MaxVertex.x )
		{
			if ( p1.Y() > b1.MinVertex.y && p1.Y() < b1.MaxVertex.y )
			{
				if ( p1.Z() > b1.MinVertex.z && p1.Z() < b1.MaxVertex.z )
					return true;
			}
		}
		
		if ( p1.X() > b1.MaxVertex.x && p1.X() < b1.MinVertex.x )
		{
			if ( p1.Y() > b1.MaxVertex.y && p1.Y() < b1.MinVertex.y )
			{
				if ( p1.Z() > b1.MaxVertex.z && p1.Z() < b1.MinVertex.z )
					return true;
			}
		}

		return false;
	}


	static Vec GetDirectionOfCollision(BoundingBox& aBox,BoundingBox& bBox)
	{
		// TODO:
		// Make this thing better.
		if( (aBox.MinVertex.y < bBox.MaxVertex.y) || (aBox.MaxVertex.y < bBox.MaxVertex.y) )
			return Vec(eDown);
		return eUp; 
	}
};

