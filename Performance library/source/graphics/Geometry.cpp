// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "graphics/Geometry.h"
#include "gpu/GpuDevice.h"
#include <algorithm>

namespace graphics
{
	void Geometry::ConvertArc(
		Vector2f p0,
		Vector2f p1,
		float32 &rx,
		float32 &ry,
		float32 theta,
		bool isLarge,
		bool isCounterclockwiseSweep,
		float32 &cx,
		float32 &cy,
		float32 &startAngle,
		float32 &endAngle)
	{
		theta = m2PI - theta;
		isLarge = !isLarge;
		float32 dx = (p0.x - p1.x) / 2.0f;
		float32 dy = (p0.y - p1.y) / 2.0f;
		Vector2f transformedPoint(
			cos(theta)*dx + sin(theta)*dy,
			-sin(theta)*dx + cos(theta)*dy);
		float32 radiiCheck = transformedPoint.x* transformedPoint.x / (rx*rx)
			+ transformedPoint.y*transformedPoint.y / (ry*ry);
		if (radiiCheck > 1.0f)
		{
			rx = sqrt(radiiCheck)*rx;
			ry = sqrt(radiiCheck)*ry;
		}

		float64 cSquareNumerator = rx * rx*ry*ry
			- rx * rx*transformedPoint.y*transformedPoint.y
			- ry * ry*transformedPoint.x*transformedPoint.x;
		float64 cSquareRootDenom = rx * rx*transformedPoint.y*transformedPoint.y
			+ ry * ry*transformedPoint.x*transformedPoint.x;
		float64 cRadicand = cSquareNumerator / cSquareRootDenom;
		if (cRadicand < 0.01f) cRadicand = 0.0f;
		float32 cCoef = (float32)sqrt(cRadicand);
		if (isLarge == isCounterclockwiseSweep) cCoef = -cCoef;
		Vector2f transformedCenter(
			cCoef * ((rx*transformedPoint.y) / ry),
			cCoef * (-(ry*transformedPoint.x) / rx));

		cx = cos(theta)*transformedCenter.x
			- sin(theta)*transformedCenter.y + ((p0.x + p1.x) / 2.0f);
		cy = sin(theta)*transformedCenter.x
			+ cos(theta)*transformedCenter.y + ((p0.y + p1.y) / 2.0f);

		Vector2f startVector((p0.x - cx) / rx, (p0.y - cy) / ry);
		startAngle = VectorAngleBetweenVectors(Vector2f(1.0f, 0.0f), startVector);
		if (startVector.y > 0.0f)
			startAngle = m2PI - startAngle;
		Vector2f endVector((p1.x - cx) / rx, (p1.y - cy) / ry);
		endAngle = VectorAngleBetweenVectors(Vector2f(1.0f, 0.0f), endVector);
		if (endVector.y > 0.0f)
			endAngle = m2PI - endAngle;
	}
	Vector2f Geometry::Bezier2(Vector2f p0, Vector2f p1, Vector2f p2, float32 t)
	{
		return (1.0f - t)*((1.0f - t)*p0 + (2.0f*t)*p1) + (t*t)*p2;
	}
	void Geometry::AdjustVertical(float32 *y)
	{
		const float32 dy = 0.125f;
		uint32 idx = (uint32)(abs(*y) * 8.0f);
		float32 scan = copysign(idx * dy, *y);
		if (ScalarNearEqual(*y, scan, 1e-4f)
			|| ScalarNearEqual(*y, scan + dy, 1e-4f))
			*y += 1e-4f;
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
	void Geometry::TransformGeometry(float32 *data, uint32 count)
	{
		Vector2f prevUntransformed;
		uint32 geometryPathOffset = 1;
		prevUntransformed.x = data[geometryPathOffset];
		prevUntransformed.y = data[geometryPathOffset + 1];
		TransformPoint(data + geometryPathOffset);
		AdjustVertical(data + geometryPathOffset + 1);
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
				AdjustVertical(data + geometryPathOffset + 2);
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
				AdjustVertical(data + geometryPathOffset + 2);
				TransformPoint(data + geometryPathOffset + 3);
				AdjustVertical(data + geometryPathOffset + 4);
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
				AdjustVertical(data + geometryPathOffset + 10);
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
				data[geometryPathOffset + 1] *= hypot(transform[0][0], transform[0][1]);
				data[geometryPathOffset + 2] *= hypot(transform[1][0], transform[1][1]);
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
				AdjustVertical(data + geometryPathOffset + 2);
				xMin = Min(xMin, data[geometryPathOffset + 1]);
				xMax = Max(xMax, data[geometryPathOffset + 1]);
				yMin = Min(yMin, data[geometryPathOffset + 2]);
				yMax = Max(yMax, data[geometryPathOffset + 2]);
				geometryPathOffset += 3;
			}
		}
	}
	void Geometry::PushDirectedCoord(
		float32 x,
		float32 yTangent,
		int32 scanRow,
		std::vector<std::vector<float32>> &xtable)
	{
		x += 100000.0f;
		int32 scanColumn = 0;
		while (scanColumn < xtable[scanRow].size()
			&& x < abs(xtable[scanRow][scanColumn]))
			scanColumn++;
		x = copysign(x, yTangent);
		if (!isCounterclockwiseFace) Negate(x);
		xtable[scanRow].insert(xtable[scanRow].begin() + scanColumn, x);
	}
	void Geometry::AdvanceLine(
		Vector2f p0,
		Vector2f p1,
		std::vector<std::vector<float32>> &xtable)
	{
		const float32 dy = 0.125f;
		float32 xi1, scan, dpr = (p1.x - p0.x) / (p1.y - p0.y);
		uint32 idx = (uint32)((Min(p0.y, p1.y) - (float32)xtableStart) * 8.0f),
			idxMax = (uint32)((Max(p0.y, p1.y) - (float32)xtableStart) * 8.0f);
		scan = (float32)xtableStart + idx * dy + dy;
		while (idx < idxMax)
		{
			xi1 = p0.x + (scan - p0.y)*dpr;
			if (idx >= 0 && idx < xtableHeight)
			{
				PushDirectedCoord(xi1, p1.y - p0.y, idx, xtable);
				xtableWidth = Max(xtableWidth, xtable[idx].size());
			}
			idx++;
			scan += dy;
		}
	}
	void Geometry::AdvanceBezier(
		Vector2f p0,
		Vector2f p1,
		Vector2f p2,
		std::vector<std::vector<float32>> &xtable)
	{
		const float32 dy = 0.125f;
		int32 idx0 = (int32)((p0.y - (float32)xtableStart) * 8.0f),
			idx1 = (int32)((p2.y - (float32)xtableStart) * 8.0f);
		bool startIncremental = (p1.y >= p0.y),
			endIncremental = (p2.y >= p1.y);
		if (!startIncremental) idx0--;
		if (endIncremental) idx1--;
		float32 scan = (float32)xtableStart + idx0 * dy + dy;
		float32 a = p0.y - 2.0f*p1.y + p2.y;
		if (abs(a) < 0.001f) a = copysign(0.001f, a);
		float32 ar = 0.5f / a,
			b = 2.0f*p1.y - 2.0f*p0.y,
			c, d, xi1, xi2;
		while (true)
		{
			if (startIncremental == endIncremental
				&& (startIncremental && idx0 > idx1
					|| !startIncremental && idx0 < idx1)) break;
			c = p0.y - scan;
			d = b * b - 4.0f*a*c;
			if (d < 0.0f) break;
			d = sqrt(d);
			xi1 = (-d - b)*ar;
			xi2 = (d - b)*ar;
			if (xi2 < xi1) Swap(xi1, xi2);
			if (xi1 < 0.0f || xi1 > 1.0f) Swap(xi1, xi2);
			if (idx0 >= 0 && idx0 < xtableHeight)
			{
				PushDirectedCoord(Bezier2(p0, p1, p2, xi1).x, (startIncremental ? 1.0f : -1.0f), idx0, xtable);
				xtableWidth = Max(xtableWidth, xtable[idx0].size());
			}
			if (startIncremental)
			{
				idx0++;
				scan += dy;
			}
			else
			{
				idx0--;
				scan -= dy;
			}
		}
		if (startIncremental != endIncremental)
		{
			while (true)
			{
				if (endIncremental && idx0 > idx1
					|| !endIncremental && idx0 < idx1) break;
				c = p0.y - scan;
				d = b * b - 4.0f*a*c;
				if (d >= 0.0f)
				{
					d = sqrt(d);
					xi1 = (-d - b)*ar;
					xi2 = (d - b)*ar;
					if (xi2 > xi1) Swap(xi1, xi2);
					if (xi1 < 0.0f || xi1 > 1.0f) Swap(xi1, xi2);
					if (idx0 >= 0 && idx0 < xtableHeight)
					{
						PushDirectedCoord(Bezier2(p0, p1, p2, xi1).x, (endIncremental ? 1.0f : -1.0f), idx0, xtable);
						xtableWidth = Max(xtableWidth, xtable[idx0].size());
					}
				}
				if (endIncremental)
				{
					idx0++;
					scan += dy;
				}
				else
				{
					idx0--;
					scan -= dy;
				}
			}
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
		float32 startAngle,
		float32 endAngle,
		bool isCounterclockwisesweep,
		std::vector<std::vector<float32>>& xtable)
	{
		const float32 dy = 0.125f;
		int32 idx0 = (int32)((p0.y - (float32)xtableStart) * 8.0f),
			idx1 = (int32)((p1.y - (float32)xtableStart) * 8.0f);
		bool startIncremental = (startAngle >= mPIdiv2 && startAngle < mPI + mPIdiv2),
			endIncremental = (endAngle >= mPIdiv2 && endAngle < mPI + mPIdiv2);
		if (!startIncremental) idx0--;
		if (endIncremental) idx1--;
		float32 sinTheta = -sin(rotation),
			cosTheta = cos(rotation),
			a = cosTheta * cosTheta / (rx*rx) + sinTheta * sinTheta / (ry*ry),
			bk = 2.0f*cosTheta*sinTheta*(1.0f / (rx*rx) - 1.0f / (ry*ry)),
			ck = (sinTheta*sinTheta / (rx*rx) + cosTheta * cosTheta / (ry*ry)),
			ar = 0.5f / a,
			b, c, d, xi1;
		float32 scan = (float32)xtableStart + idx0 * dy + dy - cy;
		while (true)
		{
			if (startIncremental == endIncremental
				&& (startIncremental && idx0 > idx1
					|| !startIncremental && idx0 < idx1)) break;
			b = bk * scan;
			c = ck * scan*scan - 1.0f;
			d = b * b - 4.0f*a*c;
			if (d < 0.0f) break;
			d = sqrt(d);
			if(startIncremental) xi1 = (-d - b)*ar + cx;
			else xi1 = (d - b)*ar + cx;
			if (idx0 >= 0 && idx0 < xtableHeight)
			{
				PushDirectedCoord(xi1, ((startIncremental == isCounterclockwisesweep) ? 1.0f : -1.0f), idx0, xtable);
				xtableWidth = Max(xtableWidth, xtable[idx0].size());
			}
			if (startIncremental)
			{
				idx0++;
				scan += dy;
			}
			else
			{
				idx0--;
				scan -= dy;
			}
		}
		if (startIncremental != endIncremental)
		{
			while (true)
			{
				if (endIncremental && idx0 > idx1
					|| !endIncremental && idx0 < idx1) break;
				b = bk * scan;
				c = ck * scan*scan - 1.0f;
				d = b * b - 4.0f*a*c;
				if (d >= 0.0f)
				{
					d = sqrt(d);
					if(endIncremental) xi1 = (-d - b)*ar + cx;
					else xi1 = (d - b)*ar + cx;
					if (idx0 >= 0 && idx0 < xtableHeight)
					{
						PushDirectedCoord(xi1, ((endIncremental == isCounterclockwisesweep) ? 1.0f : -1.0f), idx0, xtable);
						xtableWidth = Max(xtableWidth, xtable[idx0].size());
					}
				}
				if (endIncremental)
				{
					idx0++;
					scan += dy;
				}
				else
				{
					idx0--;
					scan -= dy;
				}
			}
		}
	}
	void Geometry::GetTangent(
		Vector2f p0,
		Vector2f p1,
		float32 lineWidth,
		Vector2f *tang1,
		Vector2f *tang2,
		Vector2f *tang1Reverse,
		Vector2f *tang2Reverse)
	{
		if (!isCounterclockwiseFace) Negate(lineWidth);
		Vector2f ortho = p1 - p0;
		ortho = lineWidth * VectorNormalize(Vector2f(-ortho.y, ortho.x));
		*tang1 = p0 + ortho;
		*tang2 = p1 + ortho;
		*tang1Reverse = p0 - ortho;
		*tang2Reverse = p1 - ortho;
	}
	bool Geometry::GetLinesJoint(
		Vector2f line1Point1,
		Vector2f line1Point2,
		Vector2f line2Point1,
		Vector2f line2Point2,
		Vector2f *joint)
	{
		if (VectorAngleBetweenVectors(
			line1Point1 - line1Point2,
			line2Point2 - line2Point1) < mPI / 15.0f)
			return false;
		return VectorLinesIntersect(
			line1Point1,
			line1Point2,
			line2Point1,
			line2Point2,
			joint);
	}
	void Geometry::Reset()
	{
		if (ready)
		{
			GpuDevice *device;
			QueryGpuDevice(&device);
			device->DeallocateMemory(xtableOffset);
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

		std::vector<std::vector<float32>> xtable(8 * (uint32)(yMax - yMin));
		Vector2f p0, p1, p2;
		p0.x = pathData[1];
		p0.y = pathData[2];
		xtableStart = (int32)yMin;
		xtableHeight = 8 * (int32)(yMax - yMin);
		xtableWidth = 0;
		uint32 pathOffset = 3;
		for (uint32 i = 1; i < fillPath.count; i++)
		{
			if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeLine)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				if (p0.y != p1.y)
					AdvanceLine(p0, p1, xtable);
				p0 = p1;
				pathOffset += 3;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeQuadraticBezier)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				p2.x = pathData[pathOffset + 3];
				p2.y = pathData[pathOffset + 4];
				AdvanceBezier(p0, p1, p2, xtable);
				p0 = p2;
				pathOffset += 5;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeMove)
			{
				p0.x = pathData[pathOffset + 1];
				p0.y = pathData[pathOffset + 2];
				pathOffset += 3;
			}
			else
			{
				Vector2f p0t = p0, p1t(pathData[pathOffset + 9], pathData[pathOffset + 10]);
				float32 startAngle = pathData[pathOffset + 6], endAngle = pathData[pathOffset + 7];
				p0 = p1t;
				if (Reinterpret<uint32>(pathData[pathOffset + 8]) == 0)
				{
					Swap(p0t, p1t);
					Swap(startAngle, endAngle);
				}
				AdvanceArc(
					p0t,
					p1t,
					pathData[pathOffset + 4],
					pathData[pathOffset + 5],
					pathData[pathOffset + 1],
					pathData[pathOffset + 2],
					pathData[pathOffset + 3],
					startAngle,
					endAngle,
					(Reinterpret<uint32>(pathData[pathOffset + 8]) == 0) ? false : true,
					xtable);
				pathOffset += 11;
			}
		}

		if (xtableWidth == 0 || xtableHeight == 0)
			return false;
		GpuDevice *device;
		QueryGpuDevice(&device);
		xtableOffset = device->AllocateMemory(
			xtableWidth*xtableHeight * sizeof(float32));
		void *mapped;
		device->MapMemory(
			xtableOffset,
			xtableWidth*xtableHeight * sizeof(float32),
			&mapped);
		for (uint32 i = 0; i < xtable.size(); i++)
		{
			while (xtable[i].size() < xtableWidth)
				xtable[i].insert(xtable[i].begin(), FLT_MAX);
			memcpy(
				(float32 *)mapped + xtableWidth * i,
				xtable[i].data(),
				xtableWidth * sizeof(float32));
		}
		device->UnmapMemory();

		ready = true;
		return true;
	}
	Geometry::Geometry()
	{
		isCounterclockwiseFace = true;
		MatrixSetIdentity(&transform);
		decay = 1.0f;
		ready = false;
	}
	Geometry::~Geometry()
	{
		Reset();
	}
	void Geometry::SetFaceOrientation(bool counterclockwiseFlag)
	{
		if (isCounterclockwiseFace != counterclockwiseFlag)
		{
			Reset();
			isCounterclockwiseFace = counterclockwiseFlag;
		}
	}
	bool Geometry::IsCounterclockwiseFace()
	{
		return isCounterclockwiseFace;
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
	void Geometry::SetDecay(float32 xRatio, float32 yRatio)
	{
		decay = Vector2f(xRatio, yRatio);
	}
	void Geometry::GetDecay(float32 *xRatio, float32 *yRatio)
	{
		*xRatio = decay.x;
		*yRatio = decay.y;
	}
	void Geometry::Offset(float32 offset)
	{

	}
	void Geometry::DrawLine(
		Vector2f a,
		Vector2f b,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(a);
		path.PushLine(b);
		DrawGeometry(path, false, lineWidth);
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
		path.PushLine(b);
		path.PushLine(c);
		DrawGeometry(path, true, lineWidth);
	}
	void Geometry::FillTriangle(
		Vector2f a,
		Vector2f b,
		Vector2f c)
	{
		Reset();
		GeometryPath path;
		path.Move(a);
		path.PushLine(b);
		path.PushLine(c);
		FillGeometry(path);
	}
	void Geometry::DrawRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(x + width, y));
		path.PushLine(Vector2f(x, y));
		path.PushLine(Vector2f(x, y + height));
		path.PushLine(Vector2f(x + width, y + height));
		DrawGeometry(path, true, lineWidth);
	}
	void Geometry::FillRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(x + width, y));
		path.PushLine(Vector2f(x, y));
		path.PushLine(Vector2f(x, y + height));
		path.PushLine(Vector2f(x + width, y + height));
		FillGeometry(path);
	}
	void Geometry::DrawRoundedRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 rx,
		float32 ry,
		float32 lineWidth)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(x + width, y + ry));
		path.PushArcSegment(Vector2f(x + width - rx, y), rx, ry, 0, false, true);
		path.PushLine(Vector2f(x + rx, y));
		path.PushArcSegment(Vector2f(x, y + ry), rx, ry, 0, false, true);
		path.PushLine(Vector2f(x, y + height - ry));
		path.PushArcSegment(Vector2f(x + rx, y + height), rx, ry, 0, false, true);
		path.PushLine(Vector2f(x + width - rx, y + height));
		path.PushArcSegment(Vector2f(x + width, y + height - ry), rx, ry, 0, false, true);
		DrawGeometry(path, true, lineWidth);
	}
	void Geometry::FillRoundedRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 rx,
		float32 ry)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(x + width, y + ry));
		path.PushArcSegment(Vector2f(x + width - rx, y), rx, ry, 0, false, true);
		path.PushLine(Vector2f(x + rx, y));
		path.PushArcSegment(Vector2f(x, y + ry), rx, ry, 0, false, true);
		path.PushLine(Vector2f(x, y + height - ry));
		path.PushArcSegment(Vector2f(x + rx, y + height), rx, ry, 0, false, true);
		path.PushLine(Vector2f(x + width - rx, y + height));
		path.PushArcSegment(Vector2f(x + width, y + height - ry), rx, ry, 0, false, true);
		FillGeometry(path);
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
		path.PushArcSegment(Vector2f(center.x - rx, center.y), rx, ry, 0, true, true);
		path.PushArcSegment(Vector2f(center.x + rx, center.y), rx, ry, 0, true, true);
		DrawGeometry(path, true, lineWidth);
	}
	void Geometry::FillEllipse(
		Vector2f center,
		float32 rx,
		float32 ry)
	{
		Reset();
		GeometryPath path;
		path.Move(Vector2f(center.x + rx, center.y));
		path.PushArcSegment(Vector2f(center.x - rx, center.y), rx, ry, 0, true, true);
		path.PushArcSegment(Vector2f(center.x + rx, center.y), rx, ry, 0, true, true);
		FillGeometry(path);
	}
	void Geometry::DrawGeometry(
		GeometryPath &path,
		bool isClosed,
		float32 lineWidth)
	{
		if (path.count < 2) return;
		Reset();
		lineWidth /= 2.0f;
		if (isClosed)
		{
			path.PushLine(Vector2f(path.data[1], path.data[2]));
			if (Reinterpret<uint32>(path.data[3]) == GeometryPath::geometryTypeLine)
				path.PushLine(Vector2f(path.data[4], path.data[5]));
			else if (Reinterpret<uint32>(path.data[3]) == GeometryPath::geometryTypeQuadraticBezier)
				path.PushQuadraticBezier(Vector2f(path.data[4], path.data[5]), Vector2f(path.data[6], path.data[7]));
			else if(Reinterpret<uint32>(path.data[3]) == GeometryPath::geometryTypeMove)
				path.PushLine(Vector2f(path.data[4], path.data[5]));
			else path.PushArcSegment(
				Vector2f(path.data[12], path.data[13]),
				path.data[4], path.data[5], path.data[6],
				(Reinterpret<uint32>(path.data[9]) == 0 ? false : true),
				(Reinterpret<uint32>(path.data[10]) == 0 ? false : true));
		}
		GeometryPath stroke, strokeReverse;
		std::vector<float32> &pathData = path.data;
		uint32 pathIter = 1,
			pathOffset = 3,
			prevGeometry = GeometryPath::geometryTypeMove;
		Vector2f p0(pathData[1], pathData[2]), p1, p2, joint, jointReverse,
			tang1, tang2, tang1Prev, tang2Prev,
			tang1Reverse, tang2Reverse, tang1PrevReverse, tang2PrevReverse;
		while (pathIter != path.count)
		{
			tang1Prev = tang1;
			tang2Prev = tang2;
			tang1PrevReverse = tang1Reverse;
			tang2PrevReverse = tang2Reverse;
			if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeLine)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				GetTangent(p0, p1, lineWidth, &tang1, &tang2, &tang1Reverse, &tang2Reverse);
				if (prevGeometry == GeometryPath::geometryTypeMove)
				{
					stroke.Move(tang1Reverse);
					stroke.PushLine(tang1);
					jointReverse = tang1Reverse;
				}
				else
				{
					if (GetLinesJoint(tang1Prev, tang2Prev, tang1, tang2, &joint))
					{
						stroke.PushLine(joint);
						GetLinesJoint(tang1PrevReverse, tang2PrevReverse, tang1Reverse, tang2Reverse, &joint);
						strokeReverse.Move(joint);
						strokeReverse.PushLine(jointReverse);
						jointReverse = joint;
					}
					else
					{
						stroke.PushLine(tang1);
						strokeReverse.Move(tang1Reverse);
						strokeReverse.PushLine(jointReverse);
						jointReverse = tang1Reverse;
					}
				}
				stroke.PushLine(tang2);
				strokeReverse.Move(tang2Reverse);
				strokeReverse.PushLine(jointReverse);
				jointReverse = tang2Reverse;
				p0 = p1;
				pathOffset += 3;
				prevGeometry = GeometryPath::geometryTypeLine;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeQuadraticBezier)
			{
				p1.x = pathData[pathOffset + 1];
				p1.y = pathData[pathOffset + 2];
				p2.x = pathData[pathOffset + 3];
				p2.y = pathData[pathOffset + 4];
				GetTangent(p0, p1, lineWidth, &tang1, &tang2, &tang1Reverse, &tang2Reverse);
				if (prevGeometry == GeometryPath::geometryTypeMove)
				{
					stroke.Move(tang1Reverse);
					stroke.PushLine(tang1);
					jointReverse = tang1Reverse;
				}
				else
				{
					if (GetLinesJoint(tang1Prev, tang2Prev, tang1, tang2, &joint))
					{
						stroke.PushLine(joint);
						GetLinesJoint(tang1PrevReverse, tang2PrevReverse, tang1Reverse, tang2Reverse, &joint);
						strokeReverse.Move(joint);
						strokeReverse.PushLine(jointReverse);
						jointReverse = joint;
					}
					else
					{
						stroke.PushLine(tang1);
						strokeReverse.Move(tang1Reverse);
						strokeReverse.PushLine(jointReverse);
						jointReverse = tang1Reverse;
					}
				}
				stroke.PushLine(tang1);
				strokeReverse.Move(tang1Reverse);
				strokeReverse.PushLine(jointReverse);
				jointReverse = tang1Reverse;
				tang1Prev = tang1;
				tang2Prev = tang2;
				tang1PrevReverse = tang1Reverse;
				tang2PrevReverse = tang2Reverse;
				GetTangent(p1, p2, lineWidth, &tang1, &tang2, &tang1Reverse, &tang2Reverse);
				if (VectorLinesIntersect(tang1Prev, tang2Prev, tang1, tang2, &joint))
				{
					stroke.PushQuadraticBezier(joint, tang2);
					VectorLinesIntersect(tang1PrevReverse, tang2PrevReverse, tang1Reverse, tang2Reverse, &joint);
					strokeReverse.Move(tang2Reverse);
					strokeReverse.PushQuadraticBezier(joint, jointReverse);
					jointReverse = tang2Reverse;
				}
				else
				{
					stroke.PushLine(tang2);
					strokeReverse.Move(tang2Reverse);
					strokeReverse.PushLine(jointReverse);
					jointReverse = tang2Reverse;
				}
				p0 = p2;
				pathOffset += 5;
				prevGeometry = GeometryPath::geometryTypeQuadraticBezier;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeMove)
			{
				p0.x = pathData[pathOffset + 1];
				p0.y = pathData[pathOffset + 2];
				stroke.PushLine(jointReverse);
				pathOffset += 3;
				prevGeometry = GeometryPath::geometryTypeMove;
			}
			else
			{
				p1.x = pathData[pathOffset + 9];
				p1.y = pathData[pathOffset + 10];
				float32 cx,
					cy,
					rx = pathData[pathOffset + 1],
					ry = pathData[pathOffset + 2],
					rotation = pathData[pathOffset + 3],
					sinTheta = sin(rotation),
					cosTheta = cos(rotation),
					startAngle,
					endAngle,
					tangentLineWidth = lineWidth;
				bool isLarge = true, isCounterclockwiseSweep = true;
				if (Reinterpret<uint32>(pathData[pathOffset + 6]) == 0) isLarge = false;
				if (Reinterpret<uint32>(pathData[pathOffset + 7]) == 0) isCounterclockwiseSweep = false;
				ConvertArc(
					p0,
					p1,
					rx,
					ry,
					rotation,
					isLarge,
					isCounterclockwiseSweep,
					cx,
					cy,
					startAngle,
					endAngle);
				if (!isCounterclockwiseSweep)
					tangentLineWidth = -tangentLineWidth;
				startAngle -= rotation;
				endAngle -= rotation;
				GetTangent(
					p0,
					p0 + Vector2f(-rx * sin(startAngle)*cosTheta - ry * cos(startAngle)*sinTheta,
						-(-rx * sin(startAngle)*sinTheta + ry * cos(startAngle)*cosTheta)),
					tangentLineWidth,
					&tang1,
					&tang2,
					&tang1Reverse,
					&tang2Reverse);
				if (prevGeometry == GeometryPath::geometryTypeMove)
				{
					stroke.Move(tang1Reverse);
					stroke.PushLine(tang1);
					jointReverse = tang1Reverse;
				}
				else
				{
					if (GetLinesJoint(tang1Prev, tang2Prev, tang1, tang2, &joint))
					{
						stroke.PushLine(joint);
						GetLinesJoint(tang1PrevReverse, tang2PrevReverse, tang1Reverse, tang2Reverse, &joint);
						strokeReverse.Move(joint);
						strokeReverse.PushLine(jointReverse);
						jointReverse = joint;
					}
					else
					{
						stroke.PushLine(tang1);
						strokeReverse.Move(tang1Reverse);
						strokeReverse.PushLine(jointReverse);
						jointReverse = tang1Reverse;
					}
				}
				stroke.PushLine(tang1);
				strokeReverse.Move(tang1Reverse);
				strokeReverse.PushLine(jointReverse);
				jointReverse = tang1Reverse;
				GetTangent(
					p1,
					p1 + Vector2f(-rx * sin(endAngle)*cosTheta - ry * cos(endAngle)*sinTheta,
						-(-rx * sin(endAngle)*sinTheta + ry * cos(endAngle)*cosTheta)),
					tangentLineWidth,
					&tang1,
					&tang2,
					&tang1Reverse,
					&tang2Reverse);
				if (isCounterclockwiseSweep == isCounterclockwiseFace)
				{
					rx += lineWidth;
					ry += lineWidth;
				}
				else
				{
					rx -= lineWidth;
					ry -= lineWidth;
				}
				stroke.PushArcSegment(tang1, rx, ry, rotation, isLarge, isCounterclockwiseSweep);
				if (isCounterclockwiseSweep == isCounterclockwiseFace)
				{
					rx -= 2.0f*lineWidth;
					ry -= 2.0f*lineWidth;
				}
				else
				{
					rx += 2.0f*lineWidth;
					ry += 2.0f*lineWidth;
				}
				strokeReverse.Move(tang1Reverse);
				strokeReverse.PushArcSegment(jointReverse, rx, ry, rotation, isLarge, !isCounterclockwiseSweep);
				jointReverse = tang1Reverse;
				p0 = p1;
				pathOffset += 11;
				prevGeometry = GeometryPath::geometryTypeArc;
			}
			pathIter++;
		}
		stroke.PushLine(jointReverse);
		fillPath.Append(stroke);
		fillPath.Append(strokeReverse);
	}
	void Geometry::FillGeometry(GeometryPath &path)
	{
		if (path.IsEmpty()) return;
		Reset();
		fillPath.Append(path);
		std::vector<float32> &pathData = path.data;
		Vector2f p0, ps;
		p0 = Vector2f(pathData[1], pathData[2]);
		ps = p0;
		uint32 pathOffset = 3;
		for (uint32 i = 1; i < path.count; i++)
		{
			if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeLine)
			{
				p0 = Vector2f(pathData[pathOffset + 1], pathData[pathOffset + 2]);
				pathOffset += 3;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeQuadraticBezier)
			{
				p0 = Vector2f(pathData[pathOffset + 3], pathData[pathOffset + 4]);
				pathOffset += 5;
			}
			else if (Reinterpret<uint32>(pathData[pathOffset]) == GeometryPath::geometryTypeMove)
			{
				if (p0 != ps)
				{
					fillPath.Move(p0);
					fillPath.PushLine(ps);
				}
				p0 = Vector2f(pathData[pathOffset + 1], pathData[pathOffset + 2]);
				ps = p0;
				pathOffset += 3;
			}
			else
			{
				p0 = Vector2f(pathData[pathOffset + 9], pathData[pathOffset + 10]);
				pathOffset += 11;
			}
		}
		if (p0 != ps)
		{
			fillPath.Move(p0);
			fillPath.PushLine(ps);
		}
	}
}
