// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "graphics/Geometry.h"
#include "gpu/VulkanAPI.h"
#include "gpu/GpuDevice.h"
#include <algorithm>

namespace graphics
{
	void Geometry::ConvertArc(
		Vector2f p0,
		Vector2f p1,
		float32 &rx,
		float32 &ry,
		float32 rotation,
		bool isLarge,
		bool isCounterclockwiseSweep,
		float32 &cx,
		float32 &cy,
		float32 &startAngle,
		float32 &endAngle)
	{
		rotation = m2PI - rotation;
		isLarge = !isLarge;
		float32 dx = (p0.x - p1.x) / 2.0f;
		float32 dy = (p0.y - p1.y) / 2.0f;
		Vector2f transformedPoint(
			cos(rotation)*dx + sin(rotation)*dy,
			-sin(rotation)*dx + cos(rotation)*dy);
		float32 radiiCheck = transformedPoint.x* transformedPoint.x / (rx*rx)
			+ transformedPoint.y*transformedPoint.y / (ry*ry);
		if (radiiCheck > 1.0f)
		{
			rx = sqrt(radiiCheck)*rx;
			ry = sqrt(radiiCheck)*ry;
		}

		float32 cSquareNumerator = rx * rx*ry*ry
			- rx * rx*transformedPoint.y*transformedPoint.y
			- ry * ry*transformedPoint.x*transformedPoint.x;
		float32 cSquareRootDenom = rx * rx*transformedPoint.y*transformedPoint.y
			+ ry * ry*transformedPoint.x*transformedPoint.x;
		float32 cRadicand = cSquareNumerator / cSquareRootDenom;
		if (cRadicand < 0.0f) cRadicand = 0.0f;
		float32 cCoef = sqrt(cRadicand);
		if (isLarge == isCounterclockwiseSweep) cCoef = -cCoef;
		Vector2f transformedCenter(
			cCoef * ((rx*transformedPoint.y) / ry),
			cCoef * (-(ry*transformedPoint.x) / rx));

		cx = cos(rotation)*transformedCenter.x
			- sin(rotation)*transformedCenter.y + ((p0.x + p1.x) / 2.0f);
		cy = sin(rotation)*transformedCenter.x
			+ cos(rotation)*transformedCenter.y + ((p0.y + p1.y) / 2.0f);

		Vector2f startVector(
			(transformedPoint.x - transformedCenter.x) / rx,
			(transformedPoint.y - transformedCenter.y) / ry);
		startAngle = VectorAngleBetweenVectors(Vector2f(1.0f, 0.0f), startVector);
		if (startVector.y > 0.0f)
			startAngle = m2PI - startAngle;
		Vector2f endVector(
			(-transformedPoint.x - transformedCenter.x) / rx,
			(-transformedPoint.y - transformedCenter.y) / ry);
		endAngle = VectorAngleBetweenVectors(Vector2f(1.0f, 0.0f), endVector);
		if (endVector.y > 0.0f)
			endAngle = m2PI - endAngle;
		if (!isCounterclockwiseSweep) Swap(startAngle, endAngle);
	}
	Vector2f Geometry::Bezier2(Vector2f p0, Vector2f p1, Vector2f p2, float32 t)
	{
		return (1.0f - t)*((1.0f - t)*p0 + (2.0f*t)*p1) + (t*t)*p2;
	}
	void Geometry::TransformPoint(float32 *point)
	{
		float32 p0 = point[0];
		point[0] = point[0] * transform[0][0]
			+ point[1] * transform[1][0]
			+ transform[2][0];
		point[1] = point[1] * transform[1][1]
			+ p0 * transform[0][1]
			+ transform[2][1];
	}
	void Geometry::AdjustVertical(float32 &y)
	{
		const float32 dy = 0.125f;
		uint32 idx = (uint32)((y - (float32)scanlineStart) * 8.0f);
		if (y - ((float32)scanlineStart + idx * dy) < 1e-3f) y += 1e-3f;
	}
	void Geometry::TransformGeometry(float32 *data, uint32 count)
	{
		Vector2f prevUntransformed;
		msize geometryPathOffset = 1;
		prevUntransformed.x = data[geometryPathOffset];
		prevUntransformed.y = data[geometryPathOffset + 1];
		TransformPoint(data + geometryPathOffset);
		AdjustVertical(data[geometryPathOffset + 1]);
		xMin = Min(xMin, data[geometryPathOffset]);
		xMax = Max(xMax, data[geometryPathOffset]);
		yMin = Min(yMin, data[geometryPathOffset + 1]);
		yMax = Max(yMax, data[geometryPathOffset + 1]);
		geometryPathOffset += 2;
		for (uint32 i = 1; i < count; i++)
		{
			if (Reinterpret<uint32>(data[geometryPathOffset]) == GeometryPath::geometryTypeLine)
			{
				prevUntransformed.x = data[geometryPathOffset + 1];
				prevUntransformed.y = data[geometryPathOffset + 2];
				TransformPoint(data + geometryPathOffset + 1);
				AdjustVertical(data[geometryPathOffset + 2]);
				xMin = Min(xMin, data[geometryPathOffset + 1]);
				xMax = Max(xMax, data[geometryPathOffset + 1]);
				yMin = Min(yMin, data[geometryPathOffset + 2]);
				yMax = Max(yMax, data[geometryPathOffset + 2]);
				geometryPathOffset += 3;
			}
			else if (Reinterpret<uint32>(data[geometryPathOffset]) == GeometryPath::geometryTypeQuadraticBezier)
			{
				prevUntransformed.x = data[geometryPathOffset + 3];
				prevUntransformed.y = data[geometryPathOffset + 4];
				TransformPoint(data + geometryPathOffset + 1);
				AdjustVertical(data[geometryPathOffset + 2]);
				TransformPoint(data + geometryPathOffset + 3);
				AdjustVertical(data[geometryPathOffset + 4]);
				xMin = Min(xMin, data[geometryPathOffset + 1], data[geometryPathOffset + 3]);
				xMax = Max(xMax, data[geometryPathOffset + 1], data[geometryPathOffset + 3]);
				yMin = Min(yMin, data[geometryPathOffset + 2], data[geometryPathOffset + 4]);
				yMax = Max(yMax, data[geometryPathOffset + 2], data[geometryPathOffset + 4]);
				geometryPathOffset += 5;
			}
			else if (Reinterpret<uint32>(data[geometryPathOffset]) == GeometryPath::geometryTypeArc)
			{
				Vector2f dpUntransformed(
					data[geometryPathOffset + 9] - prevUntransformed.x,
					data[geometryPathOffset + 10] - prevUntransformed.y);
				prevUntransformed.x = data[geometryPathOffset + 9];
				prevUntransformed.y = data[geometryPathOffset + 10];
				TransformPoint(data + geometryPathOffset + 9);
				Vector2f p0(data[geometryPathOffset - 2], data[geometryPathOffset - 1]);
				float dAngle = VectorAngleBetweenVectors(
					dpUntransformed,
					Vector2f(data[geometryPathOffset + 9], data[geometryPathOffset + 10]) - p0);
				if (!VectorCCWTestRH(
					Vector2f(0.0f, 0.0f),
					dpUntransformed,
					Vector2f(data[geometryPathOffset + 9],
						data[geometryPathOffset + 10]) - p0))
					dAngle = m2PI - dAngle;
				data[geometryPathOffset + 3] += dAngle;
				if (data[geometryPathOffset + 3] >= m2PI) data[geometryPathOffset + 3] -= m2PI;
				bool isLarge = true, isCounterclockwiseSweep = true;
				if (Reinterpret<uint32>(data[geometryPathOffset + 6]) == 0) isLarge = false;
				if (Reinterpret<uint32>(data[geometryPathOffset + 7]) == 0) isCounterclockwiseSweep = false;
				AdjustVertical(data[geometryPathOffset + 10]);
				ConvertArc(
					p0,
					Vector2f(data[geometryPathOffset + 9], data[geometryPathOffset + 10]),
					data[geometryPathOffset + 1],
					data[geometryPathOffset + 2],
					data[geometryPathOffset + 3],
					isLarge,
					isCounterclockwiseSweep,
					data[geometryPathOffset + 4],
					data[geometryPathOffset + 5],
					data[geometryPathOffset + 6],
					data[geometryPathOffset + 7]);
				xMin = Min(xMin, data[geometryPathOffset + 4]
					- Max(data[geometryPathOffset + 1], data[geometryPathOffset + 2]));
				xMax = Max(xMax, data[geometryPathOffset + 4]
					+ Max(data[geometryPathOffset + 1], data[geometryPathOffset + 2]));
				yMin = Min(yMin, data[geometryPathOffset + 5]
					- Max(data[geometryPathOffset + 1], data[geometryPathOffset + 2]));
				yMax = Max(yMax, data[geometryPathOffset + 5]
					+ Max(data[geometryPathOffset + 1], data[geometryPathOffset + 2]));
				geometryPathOffset += 11;
			}
			else
			{
				prevUntransformed.x = data[geometryPathOffset + 1];
				prevUntransformed.y = data[geometryPathOffset + 2];
				TransformPoint(data + geometryPathOffset + 1);
				AdjustVertical(data[geometryPathOffset + 1]);
				xMin = Min(xMin, data[geometryPathOffset + 1]);
				xMax = Max(xMax, data[geometryPathOffset + 1]);
				yMin = Min(yMin, data[geometryPathOffset + 2]);
				yMax = Max(yMax, data[geometryPathOffset + 2]);
				geometryPathOffset += 3;
			}
		}
	}
	void Geometry::AdvanceLine(
		Vector2f p0,
		Vector2f p1,
		std::vector<std::vector<float32>> &scanline)
	{
		const float32 dy = 0.125f;
		float32 xi1, scan, dpr = (p1.x - p0.x) / (p1.y - p0.y);
		uint32 idx = (uint32)((Min(p0.y, p1.y) - (float32)scanlineStart) * 8.0f),
			idxMax = (uint32)((Max(p0.y, p1.y) - (float32)scanlineStart) * 8.0f);
		scan = (float32)scanlineStart + idx * dy + dy;
		while (idx < idxMax)
		{
			xi1 = p0.x + (scan - p0.y)*dpr;
			scanline[idx].push_back(xi1);
			scanlineWidth = Max(scanlineWidth, scanline[idx].size());
			idx++;
			scan += dy;
		}
	}
	void Geometry::AdvanceBezier(
		Vector2f p0,
		Vector2f p1,
		Vector2f p2,
		std::vector<std::vector<float32>> &scanline)
	{
		const float32 dy = 0.125f;
		float32 yMin = Min(p0.y, p1.y, p2.y),
			yMax = Max(p0.y, p1.y, p2.y);
		uint32 idx = (uint32)((yMin - (float32)scanlineStart) * 8.0f),
			idxMax = (uint32)((yMax - (float32)scanlineStart) * 8.0f);
		float32 scan = (float32)scanlineStart + idx * dy + dy;
		float32 a = p0.y - 2.0f*p1.y + p2.y;
		if (abs(a) < 0.001f) a = copysign(0.001f, a);
		float32 ar = 0.5f / a,
			b = 2.0f*p1.y - 2.0f*p0.y,
			c, d, xi1, xi2;
		while (idx < idxMax)
		{
			c = p0.y - scan;
			d = b * b - 4.0f*a*c;
			if (d >= 0.0)
			{
				d = sqrt(d);
				xi1 = (d - b)*ar;
				xi2 = (-d - b)*ar;
				if (xi1 >= 0.0f && xi1 <= 1.0f)
				{
					scanline[idx].push_back(Bezier2(p0, p1, p2, xi1).x);
					scanlineWidth = Max(scanlineWidth, scanline[idx].size());
				}
				if (xi2 >= 0.0f && xi2 <= 1.0f)
				{
					scanline[idx].push_back(Bezier2(p0, p1, p2, xi2).x);
					scanlineWidth = Max(scanlineWidth, scanline[idx].size());
				}
			}
			idx++;
			scan += dy;
		}
	}
	void Geometry::AdvanceArc(
		Vector2f p0,
		Vector2f p1,
		float32 cx,
		float32 cy,
		float32 rx,
		float32 ry,
		float32 rotation,
		std::vector<std::vector<float32>>& scanline)
	{
		const float32 dy = 0.125f;
		float32 yMin = cy - Max(rx, ry),
			yMax = cy + Max(rx, ry);
		uint32 idx = (uint32)((yMin - (float32)scanlineStart) * 8.0f),
			idxMax = (uint32)((yMax - (float32)scanlineStart) * 8.0f);
		float32 scan = (float32)scanlineStart + idx * dy + dy - cy;
		float32 sinTheta = -sin(rotation),
			cosTheta = cos(rotation),
			a = cosTheta * cosTheta / (rx*rx) + sinTheta * sinTheta / (ry*ry),
			bk = 2.0f*cosTheta*sinTheta*(1.0f / (rx*rx) - 1.0f / (ry*ry)),
			ck = (sinTheta*sinTheta / (rx*rx) + cosTheta * cosTheta / (ry*ry)),
			ar = 0.5f / a,
			b, c, d, xi1, xi2;
		while (idx <= idxMax)
		{
			b = bk * scan;
			c = ck * scan*scan - 1.0f;
			d = b * b - 4.0f*a*c;
			if (d >= 0.0f)
			{
				d = sqrt(d);
				xi1 = (d - b)*ar + cx;
				xi2 = (-d - b)*ar + cx;
				if (VectorCCWTestRH(p0, Vector2f(xi1, scan + cy), p1))
				{
					scanline[idx].push_back(xi1);
					scanlineWidth = Max(scanlineWidth, scanline[idx].size());
				}
				if (VectorCCWTestRH(p0, Vector2f(xi2, scan + cy), p1))
				{
					scanline[idx].push_back(xi2);
					scanlineWidth = Max(scanlineWidth, scanline[idx].size());
				}
			}
			idx++;
			scan += dy;
		}
	}
	void Geometry::Reset()
	{
		if (ready)
		{
			GpuDevice *device;
			VkQueryGpuDevice(&device);
			device->DeallocateMemory(scanlineOffset);
			ready = false;
		}
	}
	bool Geometry::Prepare()
	{
		if (fillPath.count < 2) return false;
		std::vector<float32> pathData = fillPath.data;
		xMin = FLT_MAX;
		xMax = -FLT_MAX;
		yMin = FLT_MAX;
		yMax = -FLT_MAX;
		TransformGeometry(pathData.data(), fillPath.count);
		xMin = floor(xMin);
		xMax = ceil(xMax);
		yMin = floor(yMin);
		yMax = ceil(yMax);

		std::vector<std::vector<float32>> scanline(8 * (msize)(yMax - yMin));
		Vector2f p0, p1, p2, ps;
		p0.x = pathData[1];
		p0.y = pathData[2];
		ps = p0;
		scanlineStart = (int32)yMin;
		scanlineHeight = 8 * (int32)(yMax - yMin);
		scanlineWidth = 0;
		uint32 pathOffset = 3;
		for (uint32 i = 1; i < fillPath.count; i++)
		{
			if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeLine)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				if (p0.y != p1.y)
					AdvanceLine(p0, p1, scanline);
				p0 = p1;
				pathOffset += 3;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeQuadraticBezier)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				p2.x = pathData[pathOffset + 3];
				p2.y = pathData[pathOffset + 4];
				AdvanceBezier(p0, p1, p2, scanline);
				p0 = p2;
				pathOffset += 5;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeMove)
			{
				if (p0.y != ps.y)
					AdvanceLine(p0, ps, scanline);
				p0.x = pathData[pathOffset + 1];
				p0.y = pathData[pathOffset + 2];
				ps = p0;
				pathOffset += 3;
			}
			else
			{
				Vector2f p0t = p0, p1t(pathData[pathOffset + 9], pathData[pathOffset + 10]);
				p0 = p1t;
				if (Reinterpret<uint32>(pathData[pathOffset + 8]) == 0)
					Swap(p0t, p1t);
				AdvanceArc(
					p0t,
					p1t,
					pathData[pathOffset + 4],
					pathData[pathOffset + 5],
					pathData[pathOffset + 1],
					pathData[pathOffset + 2],
					pathData[pathOffset + 3],
					scanline);
				pathOffset += 11;
			}
		}
		AdvanceLine(p0, ps, scanline);

		if (scanlineWidth == 0 || scanlineHeight == 0)
			return false;
		GpuDevice *device;
		VkQueryGpuDevice(&device);
		scanlineOffset = device->AllocateMemory(
			scanlineWidth*scanlineHeight * sizeof(float32));
		void *mapped;
		device->MapMemory(
			scanlineOffset,
			scanlineWidth*scanlineHeight * sizeof(float32),
			&mapped);
		for (msize i = 0; i < scanline.size(); i++)
		{
			std::sort(scanline[i].begin(), scanline[i].end(), std::greater<>());
			while (scanline[i].size() < scanlineWidth)
				scanline[i].insert(scanline[i].begin(), FLT_MAX);
			memcpy(
				(float32 *)mapped + scanlineWidth * i,
				scanline[i].data(),
				scanlineWidth * sizeof(float32));
		}
		device->UnmapMemory();

		ready = true;
		return true;
	}
	Geometry::Geometry()
	{
		MatrixSetIdentity(&transform);
		ready = false;
	}
	Geometry::~Geometry()
	{
		Reset();
	}
	void Geometry::SetTransform(Matrix3x2f &transform)
	{
		Reset();
		this->transform = transform;
	}
	Matrix3x2f Geometry::GetTransform()
	{
		return transform;
	}
	void Geometry::DrawLine(
		Vector2f a,
		Vector2f b,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(a);
		path.AddLine(b);
		DrawGeometry(&path, false, lineWidth);
	}
	void Geometry::DrawTriangle(
		Vector2f a,
		Vector2f b,
		Vector2f c,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(a);
		path.AddLine(b);
		path.AddLine(c);
		DrawGeometry(&path, true, lineWidth);
	}
	void Geometry::FillTriangle(
		Vector2f a,
		Vector2f b,
		Vector2f c)
	{
		Reset();
		GeometryPath path;
		path.Move(a);
		path.AddLine(b);
		path.AddLine(c);
		FillGeometry(&path);
	}
	void Geometry::DrawRectangle(
		Rectf& rect,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(rect.right, rect.top));
		path.AddLine(Vector2f(rect.left, rect.top));
		path.AddLine(Vector2f(rect.left, rect.bottom));
		path.AddLine(Vector2f(rect.right, rect.bottom));
		DrawGeometry(&path, true, lineWidth);
	}
	void Geometry::FillRectangle(Rectf& rect)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(rect.right, rect.top));
		path.AddLine(Vector2f(rect.left, rect.top));
		path.AddLine(Vector2f(rect.left, rect.bottom));
		path.AddLine(Vector2f(rect.right, rect.bottom));
		FillGeometry(&path);
	}
	void Geometry::DrawRoundedRectangle(
		Rectf &rect,
		float32 rx,
		float32 ry,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(rect.right, rect.top + ry));
		path.AddArcSegment(Vector2f(rect.right - rx, rect.top), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left + rx, rect.top));
		path.AddArcSegment(Vector2f(rect.left, rect.top + ry), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left, rect.bottom - ry));
		path.AddArcSegment(Vector2f(rect.left + rx, rect.bottom), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.right - rx, rect.bottom));
		path.AddArcSegment(Vector2f(rect.right, rect.bottom - ry), rx, ry, 0, false, true);
		DrawGeometry(&path, true, lineWidth);
	}
	void Geometry::FillRoundedRectangle(
		Rectf &rect,
		float32 rx,
		float32 ry)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(rect.right, rect.top + ry));
		path.AddArcSegment(Vector2f(rect.right - rx, rect.top), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left + rx, rect.top));
		path.AddArcSegment(Vector2f(rect.left, rect.top + ry), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left, rect.bottom - ry));
		path.AddArcSegment(Vector2f(rect.left + rx, rect.bottom), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.right - rx, rect.bottom));
		path.AddArcSegment(Vector2f(rect.right, rect.bottom - ry), rx, ry, 0, false, true);
		FillGeometry(&path);
	}
	void Geometry::DrawEllipse(
		Vector2f center,
		float32 rx,
		float32 ry,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(center.x + rx, center.y));
		path.AddArcSegment(Vector2f(center.x - rx, center.y), rx, ry, 0, true, true);
		path.AddArcSegment(Vector2f(center.x + rx, center.y), rx, ry, 0, true, true);
		DrawGeometry(&path, true, lineWidth);
	}
	void Geometry::FillEllipse(
		Vector2f center,
		float32 rx,
		float32 ry)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(center.x + rx, center.y));
		path.AddArcSegment(Vector2f(center.x - rx, center.y), rx, ry, 0, true, true);
		path.AddArcSegment(Vector2f(center.x + rx, center.y), rx, ry, 0, true, true);
		FillGeometry(&path);
	}
	void Geometry::DrawGeometry(
		GeometryPath *path,
		bool isClosed,
		float32 lineWidth)
	{
		/*Reset();
		lineWidth /= 2.0f;
		GeometryPath stroke;
		std::vector<Vector2f> points;
		std::vector<float32> &pathData = path->data;
		Vector2f p0, p1, p2, ps;
		p0.x = pathData[1];
		p0.y = pathData[2];
		ps = p0;
		scanlineStart = (int32)yMin;
		scanlineHeight = 8 * (int32)(yMax - yMin);
		scanlineWidth = 0;
		uint32 pathOffset = 3;
		for (uint32 i = 1; i < fillPath.count; i++)
		{
			if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeLine)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				if (p0.y != p1.y)
					AdvanceLine(p0, p1, scanline);
				p0 = p1;
				pathOffset += 3;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeQuadraticBezier)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				p2.x = pathData[pathOffset + 3];
				p2.y = pathData[pathOffset + 4];
				AdvanceBezier(p0, p1, p2, scanline);
				p0 = p2;
				pathOffset += 5;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeMove)
			{
				if (p0.y != ps.y)
					AdvanceLine(p0, ps, scanline);
				p0.x = pathData[pathOffset + 1];
				p0.y = pathData[pathOffset + 2];
				ps = p0;
				pathOffset += 3;
			}
			else
			{
				Vector2f p0t = p0, p1t(pathData[pathOffset + 9], pathData[pathOffset + 10]);
				p0 = p1t;
				if (Reinterpret<uint32>(pathData[pathOffset + 8]) == 0)
					Swap(p0t, p1t);
				AdvanceArc(
					p0t,
					p1t,
					pathData[pathOffset + 4],
					pathData[pathOffset + 5],
					pathData[pathOffset + 1],
					pathData[pathOffset + 2],
					pathData[pathOffset + 3],
					scanline);
				pathOffset += 11;
			}
		}
		AdvanceLine(p0, ps, scanline);
		this->fillPath.Append(&stroke);*/
	}
	void Geometry::FillGeometry(GeometryPath *path)
	{
		Reset();
		this->fillPath.Append(path);
	}
}
