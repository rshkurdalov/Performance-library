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
		static const uint32 colorModeSolidColor = 0;
		static const uint32 colorModeBitmap = 1;
		static const uint32 colorModeLinearGradient = 2;
		static const uint32 colorModeRadialGradient = 3;
		static const uint32 colorModeDistanceBased = 5;

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
		float32 projX;
		float32 projY;
		SwapChain *swapChain;
		Buffer *vertexBuffer;
		CommandBuffer *cmdBuffer;
		Pipeline *pipeline;
		Vector2f *vertices;
		uint32 currentVertex;

		RenderTarget(
			SwapChain *swapChain,
			Buffer *vertexBuffer,
			CommandBuffer *cmdBuffer,
			Pipeline *pipeline);
		~RenderTarget();
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
		void Render(Geometry *geometry);
	};
}
