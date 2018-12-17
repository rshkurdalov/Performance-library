// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#pragma once
#include "kernel\kernel.h"
#include "kernel\SharedObject.h"
#include "math\VectorMath.h"
#include "graphics\Color.h"
#include "algo\DistanceGeometry.h"
#include "gpu\GradientCollection.h"
#include "graphics\Geometry.h"

namespace gpu
{
	class RenderTarget : public SharedObject
	{
		friend class GpuDevice;
	protected:
		static const uint32 vertexBufferSize = 10000 * 4 * sizeof(float32);
		static const uint32 renderModeGeometry = 0;
		static const uint32 renderModeLine = 1;
		static const uint32 renderModeRectangleOutline = 2;
		static const uint32 renderModeRectangle = 3;
		static const uint32 renderModeRoundedRectangleOutline = 4;
		static const uint32 renderModeRoundedRectangle = 5;
		static const uint32 renderModeEllispeOutline = 6;
		static const uint32 renderModeEllipse = 7;
		static const uint32 colorModeSolidColor = 0;
		static const uint32 colorModeBitmap = 1;
		static const uint32 colorModeLinearGradient = 2;
		static const uint32 colorModeRadialGradient = 3;
		static const uint32 colorModeDistanceBased = 5;

		struct FragmentConstants
		{
			uint32 colorMode;
			uint32 colorOffset;
			uint32 colorCount;
			float32 paramf[14];
			uint32 renderMode;
			Matrix3x2f transform;
			float32 decayX;
			float32 decayY;
			int32 xtableOffset;
			int32 xtableStart;
			int32 xtableHeight;
			int32 xtableWidth;
			uint32 interpolationMode;
			float32 opacity;
		} fc;
		GpuDevice *device;
		SwapChain *swapChain;
		Buffer *vertexBuffer;
		CommandBuffer *cmdBuffer;
		Pipeline *pipeline;
		Vector2f *vertices;
		uint32 currentVertex;
		float32 projX;
		float32 projY;

		RenderTarget(
			GpuDevice *device,
			SwapChain *swapChain,
			Buffer *vertexBuffer,
			CommandBuffer *cmdBuffer,
			Pipeline *pipeline);
		~RenderTarget();
		void PushVertex(Vector2f vertex);
	public:
		HResult CreateBitmap(
			uint32 width,
			uint32 height,
			Bitmap **ppBitmap);
		HResult CreateGradientCollection(
			GradientStop *stops,
			uint32 count,
			GradientCollection **ppGradientCollection);
		void Begin();
		void End();
		void Resize(uint32 width, uint32 height);
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
		void SetOpacity(float32 opacity);
		float32 GetOpacity();
		void SetColorInterpolationMode(ColorInterpolationMode value);
		ColorInterpolationMode GetColorInterpolationMode();
		void PushScissor(
			float32 x,
			float32 y,
			float32 width,
			float32 height);
		void PopScissor();
		void RenderGeometry(
			Geometry &geometry,
			float32 translateX = 0.0f,
			float32 translateY = 0.0f,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void DrawLine(
			Vector2f a,
			Vector2f b,
			float32 lineWidth = 1.0f,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void DrawRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 lineWidth = 1.0f,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void FillRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void DrawRoundedRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void FillRoundedRectangle(
			float32 x,
			float32 y,
			float32 width,
			float32 height,
			float32 rx,
			float32 ry,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void DrawEllipse(
			Vector2f center,
			float32 rx,
			float32 ry,
			float32 lineWidth = 1.0f,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
		void FillEllipse(
			Vector2f center,
			float32 rx,
			float32 ry,
			float32 rotation = 0.0f,
			float32 originX = 0.0f,
			float32 originY = 0.0f);
	};
}
