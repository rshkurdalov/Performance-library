// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "math\VectorMath.h"
#include <vector>

namespace algo
{
	class DistanceGeometry
	{
	public:
		struct ConvexGeometry
		{
			std::vector<Vector2f> points;
			float32 initialDistance;
		};
		std::vector<ConvexGeometry> geometryParts;
	};

	void CreateDistanceGeometry(
		Vector2f *vertexData,
		uint32 count,
		DistanceGeometry *geometry,
		Vector2f *initialVertex = nullptr,
		float32 initialDistance = 0.0f);
}
