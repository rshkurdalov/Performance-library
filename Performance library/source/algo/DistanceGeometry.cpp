// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "algo\DistanceGeometry.h"

namespace algo
{
	void CreateDistanceGeometry(
		Vector2f *vertexData,
		uint32 count,
		DistanceGeometry *geometry,
		Vector2f *initialVertex,
		float32 initialDistance)
	{
		constexpr float32 epsilon = 1e-3f;
		uint32 v, vf, vb, vPrev, vClosest, vs, geometryIndex;
		float32 minDist = FLT_MAX, currentDist, turnAngle1, turnAngle2, turnAngle3;
		Vector2f vInitial, vbStart, vIntersected, startVertex, va1, va2;

		if (count < 3) return;
		geometry->geometryParts.push_back(DistanceGeometry::ConvexGeometry());
		geometryIndex = geometry->geometryParts.size() - 1;
		geometry->geometryParts[geometryIndex].initialDistance = initialDistance;
		if (geometryIndex == 0)
		{
			Vector2f *vertices = new Vector2f[count];
			uint32 newCount = 0;
			if (!VectorNearEqual(vertexData[0], vertexData[count - 1], epsilon))
			{
				vertices[0] = vertexData[0];
				newCount++;
			}
			for (uint32 i = 1; i < count; i++)
			{
				if (!VectorNearEqual(vertexData[i], vertexData[i - 1], epsilon))
				{
					vertices[newCount] = vertexData[i];
					newCount++;
				}
			}
			vertexData = vertices;
			count = newCount;
		}

		if (initialVertex != nullptr)
		{
			startVertex = *initialVertex;
			for (v = 0; v < count; v++)
			{
				vIntersected = VectorClosestPointOnLine(
					vertexData[v],
					vertexData[(v == count - 1 ? 0 : v + 1)],
					startVertex);
				currentDist = VectorLength2(startVertex - vIntersected);
				if (currentDist < minDist)
				{
					vInitial = vIntersected;
					vs = v;
					minDist = currentDist;
				}
			}
		}
		else
		{
			vs = 0;
			startVertex = vertexData[0];
			vInitial = startVertex;
		}

		geometry->geometryParts[geometryIndex].points.push_back(startVertex);
		vbStart = vInitial;
		v = vs;
		while (true)
		{
			vb = v;
			v++;
			if (v == count) v = 0;
			vf = v + 1;
			if (vf == count) vf = 0;
			while (v != vs
				&& VectorCCWTestRH(startVertex, vertexData[v], vertexData[vf]))
			{
				v = vf;
				vf++;
				if (vf == count) vf = 0;
			}
			if (v == vs)
			{
				geometry->geometryParts[geometryIndex].points.push_back(vbStart);
				vb++;
				if (vb == count) vb = 0;
				while (vb != vs)
				{
					geometry->geometryParts[geometryIndex].points.push_back(vertexData[vb]);
					vb++;
					if (vb == count) vb = 0;
				}
				geometry->geometryParts[geometryIndex].points.push_back(vertexData[vs]);
				geometry->geometryParts[geometryIndex].points.push_back(vInitial);
				break;
			}

			vClosest = v;
			vf = v;
			if (VectorNearEqual(startVertex, vbStart, epsilon))
			{
				va1 = vbStart;
				va2 = vertexData[(vb == count - 1 ? 0 : vb + 1)];
			}
			else
			{
				va1 = startVertex;
				va2 = vbStart;
			}
			turnAngle2 = VectorAngleBetweenVectors(
				va2 - va1,
				vertexData[v] - va1);
			if (!VectorCCWTestRH(va1, va2, vertexData[v]))
				turnAngle2 = m2PI - turnAngle2;
			turnAngle3 = turnAngle2;
			while (vf != vs)
			{
				vPrev = vf;
				vf++;
				if (vf == count) vf = 0;

				if (VectorNearEqual(vertexData[vPrev], vertexData[v], epsilon))
				{
					turnAngle1 = VectorAngleBetweenVectors(
						vertexData[v] - vertexData[(v == 0 ? count - 1 : v - 1)],
						vertexData[vf] - vertexData[v]);
					if (!VectorCCWTestRH(vertexData[(v == 0 ? count - 1 : v - 1)], vertexData[v], vertexData[vf]))
						Negate(turnAngle1);
				}
				else if (VectorNearEqual(vertexData[vf], vertexData[v], epsilon))
					turnAngle1 = 0.0f;
				else
				{
					if (VectorCCWTestRH(vertexData[v], vertexData[vPrev], vertexData[vf]))
						turnAngle1 += VectorAngleBetweenVectors(
							vertexData[vPrev] - vertexData[v],
							vertexData[vf] - vertexData[v]);
					else turnAngle1 -= VectorAngleBetweenVectors(
						vertexData[vPrev] - vertexData[v],
						vertexData[vf] - vertexData[v]);
				}

				if (VectorNearEqual(vertexData[vPrev], va1, epsilon))
				{
					turnAngle2 = VectorAngleBetweenVectors(
						va2 - va1,
						vertexData[vf] - va1);
					if (!VectorCCWTestRH(va1, va2, vertexData[vf]))
						Negate(turnAngle2);
				}
				else if (VectorNearEqual(vertexData[vf], va1, epsilon))
					turnAngle2 = 0.0f;
				else
				{
					if (VectorCCWTestRH(va1, vertexData[vPrev], vertexData[vf]))
						turnAngle2 += VectorAngleBetweenVectors(
							vertexData[vPrev] - va1,
							vertexData[vf] - va1);
					else turnAngle2 -= VectorAngleBetweenVectors(
						vertexData[vPrev] - va1,
						vertexData[vf] - va1);
				}

				if (turnAngle1 >= -epsilon
					&& turnAngle2 <= turnAngle3
					&& (vf != vb && vf != vs
						|| !VectorNearEqual(startVertex, vertexData[vf], epsilon)
						&& !VectorNearEqual(vbStart, vertexData[vf], epsilon)))
				{
					vClosest = vf;
					turnAngle3 = turnAngle2;
				}
			}
			if (vClosest == v)
			{
				vf = v + 1;
				if (vf == count) vf = 0;
			}
			else vf = vb;
			while (true)
			{
				vPrev = vf;
				vf++;
				if (vf == count) vf = 0;
				if (vClosest != vPrev
					&& vClosest != vf
					&& !VectorNearEqual(startVertex, vertexData[vPrev], epsilon)
					&& !VectorNearEqual(startVertex, vertexData[vf], epsilon)
					&& VectorLinesIntersect(
						startVertex,
						vertexData[vClosest],
						vertexData[vPrev],
						vertexData[vf],
						&vIntersected)
					&& VectorDot(vertexData[vClosest] - startVertex, vIntersected - startVertex) >= 0
					&& vIntersected.x - Min(vertexData[vPrev].x, vertexData[vf].x) >= -epsilon
					&& vIntersected.x - Max(vertexData[vPrev].x, vertexData[vf].x) <= epsilon
					&& vIntersected.y - Min(vertexData[vPrev].y, vertexData[vf].y) >= -epsilon
					&& vIntersected.y - Max(vertexData[vPrev].y, vertexData[vf].y) <= epsilon) break;
			}
			uint32 vFanEnd, vTriStart, vTriEnd;
			bool vIntersectedIsFirst;
			if (vClosest == v)
			{
				vFanEnd = v;
				vTriStart = v;
				vTriEnd = vPrev;
				vIntersectedIsFirst = false;
			}
			else
			{
				vFanEnd = vPrev;
				vTriStart = vf;
				vTriEnd = vClosest;
				vIntersectedIsFirst = true;
			}
			geometry->geometryParts[geometryIndex].points.push_back(vbStart);
			vb++;
			if (vb == count) vb = 0;
			while (vb != (vFanEnd == count - 1 ? 0 : vFanEnd + 1))
			{
				geometry->geometryParts[geometryIndex].points.push_back(vertexData[vb]);
				vb++;
				if (vb == count) vb = 0;
			}
			if (vIntersectedIsFirst)
				geometry->geometryParts[geometryIndex].points.push_back(vIntersected);

			std::vector<Vector2f> geometryPart;
			if (vIntersectedIsFirst)
			{
				geometryPart.push_back(vertexData[vTriEnd]);
				geometryPart.push_back(vIntersected);
			}
			while (vTriStart != vTriEnd)
			{
				geometryPart.push_back(vertexData[vTriStart]);
				vTriStart++;
				if (vTriStart == count) vTriStart = 0;
			}
			if (!vIntersectedIsFirst)
			{
				geometryPart.push_back(vertexData[vTriEnd]);
				geometryPart.push_back(vIntersected);
			}
			CreateDistanceGeometry(
				geometryPart.data(),
				geometryPart.size(),
				geometry,
				nullptr,
				initialDistance + VectorLength(startVertex - geometryPart[0]));
			if (!vIntersectedIsFirst)
			{
				vbStart = vIntersected;
				v = vPrev;
			}
			else
			{
				vbStart = vertexData[vClosest];
				v = vClosest;
			}
			if (v == vs)
			{
				geometry->geometryParts[geometryIndex].points.push_back(vertexData[vs]);
				geometry->geometryParts[geometryIndex].points.push_back(vInitial);
				break;
			}
		}

		if (geometryIndex == 0)
			delete[] vertexData;
	}
}
