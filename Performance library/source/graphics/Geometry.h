// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel/kernel.h"
#include "math/VectorMath.h"
#include "graphics/GeometryPath.h"
#include <vector>

namespace graphics
{
	class Geometry
	{
		friend class gpu::RenderTarget;
	protected:
		GeometryPath fillPath;
		Matrix3x2f transform;
		float32 xMin;
		float32 xMax;
		float32 yMin;
		float32 yMax;
		int32 scanlineStart;
		int32 scanlineWidth;
		int32 scanlineHeight;
		msize scanlineOffset;
		bool ready;

		void ConvertArc(
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
			float32 &endAngle);
		Vector2f Bezier2(
			Vector2f p0,
			Vector2f p1,
			Vector2f p2,
			float32 t);
		void TransformPoint(float32 *point);
		void AdjustVertical(float32 &y);
		void TransformGeometry(float32 *data, uint32 count);
		void AdvanceLine(
			Vector2f p0,
			Vector2f p1,
			std::vector<std::vector<float32>> &scanline);
		void AdvanceBezier(
			Vector2f p0,
			Vector2f p1,
			Vector2f p2,
			std::vector<std::vector<float32>> &scanline);
		void AdvanceArc(
			Vector2f p0,
			Vector2f p1,
			float32 cx,
			float32 cy,
			float32 rx,
			float32 ry,
			float32 rotation,
			std::vector<std::vector<float32>> &scanline);
		void Reset();
		bool Prepare();
	public:
		Geometry();
		~Geometry();
		void SetTransform(Matrix3x2f &transform);
		Matrix3x2f GetTransform();
		void DrawLine(
			Vector2f a,
			Vector2f b,
			float32 lineWidth = 1.0f);
		void DrawTriangle(
			Vector2f a,
			Vector2f b,
			Vector2f c,
			float32 lineWidth = 1.0f);
		void FillTriangle(
			Vector2f a,
			Vector2f b,
			Vector2f c);
		void DrawRectangle(
			Rectf &rect,
			float32 lineWidth = 1.0f);
		void FillRectangle(Rectf &rect);
		void DrawRoundedRectangle(
			Rectf &rect,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f);
		void FillRoundedRectangle(
			Rectf &rect,
			float32 rx,
			float32 ry);
		void DrawEllipse(
			Vector2f center,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f);
		void FillEllipse(
			Vector2f center,
			float32 rx,
			float32 ry);
		void DrawGeometry(
			GeometryPath *path,
			bool isClosed,
			float32 lineWidth = 1.0f);
		void FillGeometry(GeometryPath *path);
	};
}