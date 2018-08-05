// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "graphics\GeometryPath.h"

namespace graphics
{
	GeometryPath::GeometryPath()
	{
		count = 0;
	}
	Vector2f GeometryPath::LastPoint()
	{
		return Vector2f(data[data.size() - 2], data[data.size() - 1]);
	}
	bool GeometryPath::IsEmpty()
	{
		return data.size() == 0;
	}
	void GeometryPath::Reset()
	{
		data.clear();
		count = 0;
	}
	void GeometryPath::Move(Vector2f controlPoint)
	{
		data.push_back(Reinterpret<float32>(uint32(geometryTypeMove)));
		data.push_back(controlPoint.x);
		data.push_back(controlPoint.y);
		count++;
	}
	void GeometryPath::AddLine(Vector2f controlPoint)
	{
		if (controlPoint == LastPoint()) return;
		data.push_back(Reinterpret<float32>(uint32(geometryTypeLine)));
		data.push_back(controlPoint.x);
		data.push_back(controlPoint.y);
		count++;
	}
	void GeometryPath::AddQuadraticBezier(
		Vector2f controlPoint1,
		Vector2f controlPoint2)
	{
		Vector2f lastPoint = LastPoint();
		Vector<2, float64> v1 = controlPoint1 - lastPoint,
			v2 = controlPoint2 - controlPoint1;
		float64 c = v1[0] * v2[1] - v1[1] * v2[0];
		if (c == 0.0)
		{
			AddLine(controlPoint1);
			AddLine(controlPoint2);
			return;
		}
		data.push_back(Reinterpret<float32>(uint32(geometryTypeQuadraticBezier)));
		data.push_back(controlPoint1.x);
		data.push_back(controlPoint1.y);
		data.push_back(controlPoint2.x);
		data.push_back(controlPoint2.y);
		count++;
	}
	void GeometryPath::AddCubicBezier(
		Vector2f controlPoint1,
		Vector2f controlPoint2,
		Vector2f controlPoint3)
	{
		static constexpr float32 t = 0.5f;
		Vector2f controlPoint0 = Reinterpret<Vector2f>(data[data.size()-2]),
			l = (1.0f / 16.0f)*(controlPoint0 - 3.0f * controlPoint1 + 3.0f * controlPoint2 - controlPoint3),
			m = (1.0f / 16.0f)*(3.0f * controlPoint0 + 9.0f * controlPoint1 - 3.0f * controlPoint2 - controlPoint3),
			n = (1.0f / 16.0f)*(-3.0f * controlPoint0 + 3.0f * controlPoint1 + 3.0f * controlPoint2 + 5.0f * controlPoint3),
			b = Vector2f((1.0f - t)*((1.0f - t)*((1.0f - t)*controlPoint0
				+ (3.0f*t)*controlPoint1) + (3.0f*t*t)*controlPoint2) + (t*t*t)*controlPoint3),
			q1 = l * t + m - b * t / (2.0f * (t - 1.0f)),
			q3 = l * (t + 1.0f) + n - b * (t - 1.0f) / (2.0f * t);
		AddQuadraticBezier(q1, b);
		AddQuadraticBezier(q3, controlPoint3);
	}
	void GeometryPath::AddArcSegment(
		Vector2f controlPoint,
		float32 radiusX,
		float32 radiusY,
		float32 rotation,
		bool isLarge,
		bool isCounterclockwiseSweep)
	{
		if (controlPoint == LastPoint()
			|| radiusX == 0.0f
			|| radiusY == 0.0f)
			return;
		data.push_back(Reinterpret<float32>(uint32(geometryTypeArc)));
		data.push_back(radiusX);
		data.push_back(radiusY);
		data.push_back(rotation);
		data.push_back(0.0f);
		data.push_back(0.0f);
		float32 c1 = Reinterpret<float32>(1);
		uint32 c2 = Reinterpret<uint32>(c1);
		if(isLarge) data.push_back(Reinterpret<float32>(1));
		else data.push_back(Reinterpret<float32>(0));
		if (isCounterclockwiseSweep) data.push_back(Reinterpret<float32>(1));
		else data.push_back(Reinterpret<float32>(0));
		data.push_back(data[data.size() - 1]);
		data.push_back(controlPoint.x);
		data.push_back(controlPoint.y);
		count++;
	}
	void GeometryPath::Append(GeometryPath *path)
	{
		data.insert(data.end(), path->data.begin(), path->data.end());
		count += path->count;
	}
}
