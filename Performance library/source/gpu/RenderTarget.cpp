// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\RenderTarget.h"
#include "gpu\GpuDevice.h"
#include "gpu\SwapChain.h"
#include "gpu\Buffer.h"
#include "gpu\DescriptorSet.h"
#include "gpu\CommandPool.h"
#include "gpu\CommandBuffer.h"
#include "gpu\Pipeline.h"
#include "gpu\Bitmap.h"
#include "graphics\Geometry.h"

namespace gpu
{
	RenderTarget::RenderTarget(
		SwapChain *swapChain,
		Buffer *vertexBuffer,
		CommandBuffer *cmdBuffer,
		Pipeline *pipeline)
	{
		swapChain->AddRef();
		this->swapChain = swapChain;
		vertexBuffer->AddRef();
		this->vertexBuffer = vertexBuffer;
		cmdBuffer->AddRef();
		this->cmdBuffer = cmdBuffer;
		pipeline->AddRef();
		this->pipeline = pipeline;
		currentVertex = 0;
	}
	RenderTarget::~RenderTarget()
	{
		swapChain->Release();
		vertexBuffer->Release();
		cmdBuffer->Release();
		pipeline->Release();
	}
	void RenderTarget::Begin()
	{
		cmdBuffer->BindDescriptorSets(pipeline, vertexBuffer->device->descSet);
		cmdBuffer->BindPipeline(pipeline);
		projX = 2.0f / (float32)swapChain->GetWidth();
		projY = 2.0f / (float32)swapChain->GetHeight();
		vertexBuffer->MapMemory(0, vertexBufferSize, (void **)&vertices);
		cmdBuffer->BindVertexBuffer(0, vertexBuffer);
		SetSolidColorBrush(Color(Color::Black));
		SetColorInterpolationMode(ColorInterpolationModeSmooth);
		SetOpacity(1.0f);
	}
	void RenderTarget::End()
	{
		vertexBuffer->UnmapMemory();
		currentVertex = 0;
	}
	void RenderTarget::SetSolidColorBrush(Color color)
	{
		fc.colorMode = colorModeSolidColor;
		fc.paramf[0] = color.r / 255.0f;
		fc.paramf[1] = color.g / 255.0f;
		fc.paramf[2] = color.b / 255.0f;
		cmdBuffer->PushConstants(
			&fc,
			64,
			6 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	void RenderTarget::SetLinearGradientBrush(
		GradientCollection *gradientCollection,
		Vector2f startPoint,
		Vector2f endPoint)
	{
		fc.colorMode = colorModeLinearGradient;
		fc.colorOffset = gradientCollection->memOffset >> 2;
		fc.colorCount = gradientCollection->count;
		fc.paramf[0] = startPoint.x;
		fc.paramf[1] = startPoint.y;
		fc.paramf[2] = endPoint.x;
		fc.paramf[3] = endPoint.y;
		cmdBuffer->PushConstants(
			&fc,
			64,
			7 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	void RenderTarget::SetRadialGradientBrush(
		GradientCollection *gradientCollection,
		Vector2f center,
		float32 rx,
		float32 ry,
		Vector2f offset)
	{
		fc.colorMode = colorModeRadialGradient;
		fc.colorOffset = gradientCollection->memOffset >> 2;
		fc.colorCount = gradientCollection->count;
		fc.paramf[0] = center.x;
		fc.paramf[1] = center.y;
		fc.paramf[2] = rx;
		fc.paramf[3] = ry;
		fc.paramf[4] = center.x + offset.x;
		fc.paramf[5] = center.y + offset.y;
		cmdBuffer->PushConstants(
			&fc,
			64,
			9 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	void RenderTarget::SetBitmapBrush(
		Bitmap *bitmap,
		float32 x,
		float32 y)
	{
		fc.colorMode = colorModeBitmap;
		fc.colorOffset = bitmap->memOffset >> 2;
		fc.colorCount = bitmap->width;
		Vector2f startPoint = Vector3f(x, y, 1.0f)*bitmap->transform,
			aw = (Vector3f(x + bitmap->width, y, 1.0f)*bitmap->transform) - startPoint,
			ah = (Vector3f(x, y + bitmap->height, 1.0f)*bitmap->transform) - startPoint;
		fc.paramf[0] = startPoint.x;
		fc.paramf[1] = startPoint.y;
		fc.paramf[2] = aw.x;
		fc.paramf[3] = aw.y;
		fc.paramf[4] = ah.x;
		fc.paramf[5] = ah.y;
		fc.paramf[6] = (float32)bitmap->height;
		cmdBuffer->PushConstants(
			&fc,
			64,
			10 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	void RenderTarget::SetOpacity(float32 opacity)
	{
		fc.opacity = opacity;
		cmdBuffer->PushConstants(
			&fc,
			64 + 15 * sizeof(float32),
			sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	float32 RenderTarget::GetOpacity()
	{
		return fc.opacity;
	}
	void RenderTarget::SetColorInterpolationMode(ColorInterpolationMode value)
	{
		fc.interpolationMode = value;
		cmdBuffer->PushConstants(
			&fc,
			64 + 14 * sizeof(float32),
			sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	ColorInterpolationMode RenderTarget::GetColorInterpolationMode()
	{
		return (ColorInterpolationMode)fc.interpolationMode;
	}
	void RenderTarget::PushScissor(
		int32 x,
		int32 y,
		uint32 width,
		uint32 height)
	{
		cmdBuffer->PushScissor(x, y, width, height);
	}
	void RenderTarget::PopScissor()
	{
		cmdBuffer->PopScissor();
	}
	void RenderTarget::Render(Geometry *geometry)
	{
		if (!geometry->ready && !geometry->Prepare())
			return;
		fc.scanlineOffset = geometry->scanlineOffset >> 2;
		fc.scanlineStart = geometry->scanlineStart;
		fc.scanlineHeight = geometry->scanlineHeight;
		fc.scanlineWidth = geometry->scanlineWidth;
		cmdBuffer->PushConstants(&fc, 0, 4 * sizeof(float32), VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(geometry->xMax, geometry->yMin),
			v2(geometry->xMin, geometry->yMin),
			v3(geometry->xMin, geometry->yMax),
			v4(geometry->xMax, geometry->yMax);
		v1.x = v1.x*projX - 1.0f;
		v1.y = v1.y*projY - 1.0f;
		v2.x = v2.x*projX - 1.0f;
		v2.y = v2.y*projY - 1.0f;
		v3.x = v3.x*projX - 1.0f;
		v3.y = v3.y*projY - 1.0f;
		v4.x = v4.x*projX - 1.0f;
		v4.y = v4.y*projY - 1.0f;
		vertices[currentVertex++] = v1;
		vertices[currentVertex++] = v2;
		vertices[currentVertex++] = v3;
		vertices[currentVertex++] = v4;
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
}
