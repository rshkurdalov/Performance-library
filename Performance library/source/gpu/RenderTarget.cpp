// Copyright (c) 2017-2018, Roman Shkurdalov
// This file is under The Clear BSD License, see LICENSE.txt

#include "gpu\RenderTarget.h"
#include "gpu\GpuDevice.h"
#include "gpu\SwapChain.h"
#include "gpu\Buffer.h"
#include "gpu\CommandBuffer.h"
#include "gpu\Pipeline.h"
#include "gpu\Bitmap.h"

namespace gpu
{
	RenderTarget::RenderTarget(
		GpuDevice *device,
		SwapChain *swapChain,
		Buffer *vertexBuffer,
		CommandBuffer *cmdBuffer,
		Pipeline *pipeline)
	{
		device->AddRef();
		this->device = device;
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
		device->Unref();
		swapChain->Unref();
		vertexBuffer->Unref();
		cmdBuffer->Unref();
		pipeline->Unref();
	}
	void RenderTarget::PushVertex(Vector2f vertex)
	{
		vertex.x /= fc.decayX;
		vertex.y /= fc.decayY;
		float32 p0 = vertex.x;
		vertex.x = vertex.x * fc.transform[0][0]
			+ vertex.y * fc.transform[1][0]
			+ fc.transform[2][0];
		vertex.y = vertex.y * fc.transform[1][1]
			+ p0 * fc.transform[0][1]
			+ fc.transform[2][1];
		vertex.x = vertex.x*projX - 1.0f;
		vertex.y = vertex.y*projY - 1.0f;
		vertices[currentVertex++] = vertex;
	}
	HResult RenderTarget::CreateBitmap(
		uint32 width,
		uint32 height,
		Bitmap **ppBitmap)
	{
		return device->CreateBitmap(width, height, ppBitmap);
	}
	HResult RenderTarget::CreateGradientCollection(
		GradientStop *stops,
		uint32 count,
		GradientCollection **ppGradientCollection)
	{
		return device->CreateGradientCollection(stops, count, ppGradientCollection);
	}
	void RenderTarget::Begin()
	{
		cmdBuffer->Begin();
		cmdBuffer->BeginRenderPass(swapChain);
		cmdBuffer->SetViewport(0, 0, swapChain->GetWidth(), swapChain->GetHeight(), 0.0f, 1.0f);
		cmdBuffer->BindPipeline(pipeline);
		cmdBuffer->BindDescriptorSet(device->vkDescSet);
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
		cmdBuffer->EndRenderPass();
		cmdBuffer->End();
		cmdBuffer->Submit(swapChain);
		swapChain->Present();
	}
	void RenderTarget::Resize(uint32 width, uint32 height)
	{
		swapChain->Resize(width, height);
	}
	void RenderTarget::SetSolidColorBrush(Color color)
	{
		fc.colorMode = colorModeSolidColor;
		fc.paramf[0] = color.r / 255.0f;
		fc.paramf[1] = color.g / 255.0f;
		fc.paramf[2] = color.b / 255.0f;
		cmdBuffer->PushConstants(
			&fc,
			0,
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
			0,
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
			0,
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
			0,
			10 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	void RenderTarget::SetOpacity(float32 opacity)
	{
		if (fc.opacity == opacity) return;
		fc.opacity = opacity;
		cmdBuffer->PushConstants(
			&fc,
			31 * sizeof(float32),
			sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	float32 RenderTarget::GetOpacity()
	{
		return fc.opacity;
	}
	void RenderTarget::SetColorInterpolationMode(ColorInterpolationMode value)
	{
		if (fc.interpolationMode == value) return;
		fc.interpolationMode = value;
		cmdBuffer->PushConstants(
			&fc,
			30 * sizeof(float32),
			sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
	ColorInterpolationMode RenderTarget::GetColorInterpolationMode()
	{
		return (ColorInterpolationMode)fc.interpolationMode;
	}
	void RenderTarget::PushScissor(
		float32 x,
		float32 y,
		float32 width,
		float32 height)
	{
		cmdBuffer->PushScissor(x, y, width, height);
	}
	void RenderTarget::PopScissor()
	{
		cmdBuffer->PopScissor();
	}
	void RenderTarget::RenderGeometry(
		Geometry &geometry,
		float32 translateX,
		float32 translateY,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		if (!geometry.ready && !geometry.Prepare()) return;
		fc.renderMode = renderModeGeometry;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.transform[2][0] += round(translateX);
		fc.transform[2][1] += round(translateY);
		fc.xtableOffset = geometry.xtableOffset >> 2;
		fc.xtableStart = geometry.xtableStart;
		fc.xtableHeight = geometry.xtableHeight;
		fc.xtableWidth = geometry.xtableWidth;
		fc.decayX = geometry.decay.x;
		fc.decayY = geometry.decay.y;
		cmdBuffer->PushConstants(
			&fc,
			17 * sizeof(float32),
			13 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(geometry.xMax, geometry.yMin),
			v2(geometry.xMin, geometry.yMin),
			v3(geometry.xMin, geometry.yMax),
			v4(geometry.xMax, geometry.yMax);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::DrawLine(
		Vector2f a,
		Vector2f b,
		float32 lineWidth,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		fc.renderMode = renderModeLine;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = a.x;
		fc.paramf[8] = a.y;
		fc.paramf[9] = b.x;
		fc.paramf[10] = b.y;
		fc.paramf[11] = lineWidth;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(Max(a.x, b.x) + lineWidth, Min(a.y, b.y) - lineWidth),
			v2(Min(a.x, b.x) - lineWidth, Min(a.y, b.y) - lineWidth),
			v3(Min(a.x, b.x) - lineWidth, Max(a.y, b.y) + lineWidth),
			v4(Max(a.x, b.x) + lineWidth, Max(a.y, b.y) + lineWidth);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::DrawRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 lineWidth,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		float32 lineWidthDiv2 = 0.5f*lineWidth;
		width = round(x + width + lineWidthDiv2);
		x = round(x - lineWidthDiv2) + lineWidthDiv2;
		width -= x + lineWidthDiv2;
		height = round(y + height + lineWidthDiv2);
		y = round(y - lineWidthDiv2) + lineWidthDiv2;
		height -= y + lineWidthDiv2;
		fc.renderMode = renderModeRectangleOutline;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = x;
		fc.paramf[8] = y;
		fc.paramf[9] = x + width;
		fc.paramf[10] = y + height;
		fc.paramf[11] = lineWidth;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(x + width + lineWidth, y - lineWidth),
			v2(x - lineWidth, y - lineWidth),
			v3(x - lineWidth, y + height + lineWidth),
			v4(x + width + lineWidth, y + height + lineWidth);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::FillRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		width = round(x + width);
		x = round(x);
		width -= x;
		height = round(y + height);
		y = round(y);
		height -= y;
		fc.renderMode = renderModeRectangle;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = x;
		fc.paramf[8] = y;
		fc.paramf[9] = x + width;
		fc.paramf[10] = y + height;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(x + width, y),
			v2(x, y),
			v3(x, y + height),
			v4(x + width, y + height);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::DrawRoundedRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 rx,
		float32 ry,
		float32 lineWidth,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		float32 lineWidthDiv2 = 0.5f*lineWidth;
		width = round(x + width + lineWidthDiv2);
		x = round(x - lineWidthDiv2) + lineWidthDiv2;
		width -= x + lineWidthDiv2;
		height = round(y + height + lineWidthDiv2);
		y = round(y - lineWidthDiv2) + lineWidthDiv2;
		height -= y + lineWidthDiv2;
		fc.renderMode = renderModeRoundedRectangleOutline;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = x;
		fc.paramf[8] = y;
		fc.paramf[9] = x + width;
		fc.paramf[10] = y + height;
		fc.paramf[11] = rx;
		fc.paramf[12] = ry;
		fc.paramf[13] = lineWidth;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(x + width + lineWidth, y - lineWidth),
			v2(x - lineWidth, y - lineWidth),
			v3(x - lineWidth, y + height + lineWidth),
			v4(x + width + lineWidth, y + height + lineWidth);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::FillRoundedRectangle(
		float32 x,
		float32 y,
		float32 width,
		float32 height,
		float32 rx,
		float32 ry,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		width = round(x + width);
		x = round(x);
		width -= x;
		height = round(y + height);
		y = round(y);
		height -= y;
		fc.renderMode = renderModeRoundedRectangle;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = x;
		fc.paramf[8] = y;
		fc.paramf[9] = x + width;
		fc.paramf[10] = y + height;
		fc.paramf[11] = rx;
		fc.paramf[12] = ry;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(x + width, y),
			v2(x, y),
			v3(x, y + height),
			v4(x + width, y + height);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::DrawEllipse(
		Vector2f center,
		float32 rx,
		float32 ry,
		float32 lineWidth,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		fc.renderMode = renderModeEllispeOutline;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = center.x;
		fc.paramf[8] = center.y;
		fc.paramf[9] = rx;
		fc.paramf[10] = ry;
		fc.paramf[11] = lineWidth;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(center.x + rx + lineWidth, center.y - ry - lineWidth),
			v2(center.x - rx - lineWidth, center.y - ry - lineWidth),
			v3(center.x - rx - lineWidth, center.y + ry + lineWidth),
			v4(center.x + rx + lineWidth, center.y + ry + lineWidth);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
	void RenderTarget::FillEllipse(
		Vector2f center,
		float32 rx,
		float32 ry,
		float32 rotation,
		float32 originX,
		float32 originY)
	{
		fc.renderMode = renderModeEllipse;
		MatrixRotate2d(rotation, originX, originY, &fc.transform);
		fc.decayX = 1.0f;
		fc.decayY = 1.0f;
		fc.paramf[7] = center.x;
		fc.paramf[8] = center.y;
		fc.paramf[9] = rx;
		fc.paramf[10] = ry;
		cmdBuffer->PushConstants(
			&fc,
			10 * sizeof(float32),
			16 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(center.x + rx, center.y - ry),
			v2(center.x - rx, center.y - ry),
			v3(center.x - rx, center.y + ry),
			v4(center.x + rx, center.y + ry);
		PushVertex(v1);
		PushVertex(v2);
		PushVertex(v3);
		PushVertex(v4);
		cmdBuffer->Draw(4, 1, currentVertex - 4, 0);
	}
}
