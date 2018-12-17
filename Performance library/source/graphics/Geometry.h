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
		bool isCounterclockwiseFace;
		Matrix3x2f transform;
		Vector2f decay;
		float32 xMin;
		float32 xMax;
		float32 yMin;
		float32 yMax;
		int32 xtableStart;
		int32 xtableWidth;
		int32 xtableHeight;
		uint32 xtableOffset;
		bool ready;

		void ConvertArc(
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
			float32 &endAngle);
		Vector2f Bezier2(
			Vector2f p0,
			Vector2f p1,
			Vector2f p2,
			float32 t);
		void AdjustVertical(float32 *y);
		void TransformPoint(float32 *point);
		void TransformGeometry(float32 *data, uint32 count);
		void PushDirectedCoord(
			float32 x,
			float32 xTangent,
			int32 scanRow,
			std::vector<std::vector<float32>> &xtable);
		void AdvanceLine(
			Vector2f p0,
			Vector2f p1,
			std::vector<std::vector<float32>> &xtable);
		void AdvanceBezier(
			Vector2f p0,
			Vector2f p1,
			Vector2f p2,
			std::vector<std::vector<float32>> &xtable);
		void AdvanceArc(
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
			std::vector<std::vector<float32>> &scanline);
		void GetTangent(
			Vector2f p0,
			Vector2f p1,
			float32 lineWidth,
			Vector2f *tang1,
			Vector2f *tang2,
			Vector2f *tang1Reverse,
			Vector2f *tang2Reverse);
		bool GetLinesJoint(
			Vector2f line1Point1,
			Vector2f line1Point2,
			Vector2f line2Point1,
			Vector2f line2Point2,
			Vector2f *joint);
		void Reset();
		bool Prepare();
		Geometry(Geometry &) {}
	public:
		Geometry();
		~Geometry();
		void SetFaceOrientation(bool counterclockwiseFlag);
		bool IsCounterclockwiseFace();
		void SetTransform(Matrix3x2f &transform);
		Matrix3x2f GetTransform();
		void SetDecay(float32 xRatio, float32 yRatio);
		void GetDecay(float32 *xRatio, float32 *yRatio);
		void Offset(float32 offset);
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
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 lineWidth = 1.0f);
		void FillRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height);
		void DrawRoundedRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f);
		void FillRoundedRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
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
			GeometryPath &path,
			bool isClosed,
			float32 lineWidth = 1.0f);
		void FillGeometry(GeometryPath &path);
	};
}
