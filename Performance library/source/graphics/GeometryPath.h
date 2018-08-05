// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "math\VectorMath.h"
#include <vector>

namespace graphics
{
	class GeometryPath
	{
		friend class Geometry;
		friend class gpu::RenderTarget;
	protected:
		static const uint32 geometryTypeLine = 0;
		static const uint32 geometryTypeQuadraticBezier = 1;
		static const uint32 geometryTypeArc = 2;
		static const uint32 geometryTypeMove = 3;

		std::vector<float32> data;
		uint32 count;
		Vector2f LastPoint();
	public:
		GeometryPath();

		// Path is empty if it does not have any vertex
		bool IsEmpty();

		// Reset path data and return to empty state
		void Reset();

		// Begins or continues geometry path
		void Move(Vector2f controlPoint);

		// Geometry path must not be empty
		void AddLine(Vector2f controlPoint);

		// Geometry path must not be empty
		void AddQuadraticBezier(
			Vector2f controlPoint1,
			Vector2f controlPoint2);

		// Geometry path must not be empty
		void AddCubicBezier(
			Vector2f controlPoint1,
			Vector2f controlPoint2,
			Vector2f controlPoint3);

		// Geometry path must not be empty
		// Rotation must be in range [0, 2*pi]
		// Arc will not added if radiusX or radiusY is zero
		void AddArcSegment(
			Vector2f controlPoint,
			float32 radiusX,
			float32 radiusY,
			float32 rotation,
			bool isLarge,
			bool isCounterclockwiseSweep);

		// Concatenate two paths
		void Append(GeometryPath *path);
	};
}
