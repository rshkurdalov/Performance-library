// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"
#include "graphics\Color.h"
#include "algo\DistanceGeometry.h"
#include "gpu\GradientCollection.h"

namespace gpu
{
	class RenderTarget : public SharedObject
	{
		friend class GpuDevice;
	protected:
		static const uint32 vertexBufferSize = 10000 * 4 * sizeof(float32);
		static const uint32 geometryBufferSize = 10000 * 30 * 5 * sizeof(float32);
		static const uint32 colorModeSolidColor = 0;
		static const uint32 colorModeBitmap = 1;
		static const uint32 colorModeLinearGradient = 2;
		static const uint32 colorModeRadialGradient = 3;
		static const uint32 colorModeDistanceBased = 5;
		static float32 geometryData[geometryBufferSize / sizeof(float32)];

		struct FragmentConstants
		{
			int32 scanlineOffset;
			int32 scanlineStart;
			int32 scanlineHeight;
			int32 scanlineWidth;
			uint32 param[12];
			uint32 colorMode;
			uint32 colorOffset;
			uint32 colorCount;
			float32 paramf[11];
			uint32 interpolationMode;
			float32 opacity;
		} fc;
		Matrix3x2f transform;
		float32 projX;
		float32 projY;
		SwapChain *swapChain;
		Buffer *vertexBuffer;
		CommandBuffer *cmdBuffer;
		Pipeline *pipeline;
		Vector2f *vertices;
		uint32 currentVertex;
		msize geometryMemory;
		uint32 geometryOffset;

		RenderTarget(
			SwapChain *swapChain,
			Buffer *vertexBuffer,
			CommandBuffer *cmdBuffer,
			Pipeline *pipeline,
			msize geometryMemory);
		~RenderTarget();
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
		void TransformGeometry(
			float32 *data,
			uint32 count,
			float32 &xMin,
			float32 &xMax,
			float32 &yMin,
			float32 &yMax);
		void AdvanceLine(
			Vector2f p0,
			Vector2f p1,
			std::vector<std::vector<float32>> &scanline);
		void AdvanceBezier(
			Vector2f p0,
			Vector2f p1,
			Vector2f p2,
			std::vector<std::vector<float32>> &scanline);
	public:
		void Begin();
		void End();
		void SetSolidColorBrush(Color color);
		void SetLinearGradientBrush(
			GradientCollection *gradientCollection,
			Vector2f startPoint,
			Vector2f endPoint);
		void SetRadialGradientBrush(
			GradientCollection *gradientCollection,
			Vector2f center,
			float32 rx,
			float32 ry,
			Vector2f offset = Vector2f(0.0f, 0.0f));
		void SetBitmapBrush(
			Bitmap *bitmap,
			float32 x,
			float32 y);
		void SetTransform(Matrix3x2f &transform);
		Matrix3x2f GetTransform();
		void SetOpacity(float32 opacity);
		float32 GetOpacity();
		void SetColorInterpolationMode(ColorInterpolationMode value);
		ColorInterpolationMode GetColorInterpolationMode();
		void PushScissor(
			int32 x,
			int32 y,
			uint32 width,
			uint32 height);
		void PopScissor();
		/*void DrawLine(
			Vector2f a,
			Vector2f b,
			float32 lineWidth = 1.0f);*/
		/*void DrawTriangle(
			Vector2f a,
			Vector2f b,
			Vector2f c,
			float32 lineWidth = 1.0f);*/
		void FillTriangle(
			Vector2f a,
			Vector2f b,
			Vector2f c);
		/*void DrawRectangle(
			Rectf &rect,
			float32 lineWidth = 1.0f);*/
		void FillRectangle(Rectf &rect);
		/*void DrawRoundedRectangle(
			Rectf &rect,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f);*/
		void FillRoundedRectangle(
			Rectf &rect,
			float32 rx,
			float32 ry);
		/*void DrawEllipse(
			Vector2f center,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f);*/
		void FillEllipse(
			Vector2f center,
			float32 rx,
			float32 ry);
		void FillGeometry(GeometryPath *path);
		void FillDistanceGeometry(
			DistanceGeometry *geometry,
			GradientCollection *gradientCollection,
			float32 gradientDistance);
	};
}
