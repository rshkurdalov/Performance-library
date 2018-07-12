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
#include "graphics\GeometryPath.h"
#include <algorithm>

namespace gpu
{
	float32 RenderTarget::geometryData[geometryBufferSize / sizeof(float32)];

	RenderTarget::RenderTarget(
		SwapChain *swapChain,
		Buffer *vertexBuffer,
		CommandBuffer *cmdBuffer,
		Pipeline *pipeline,
		msize geometryMemory)
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
		this->geometryMemory = geometryMemory;
		geometryOffset = 0;
		MatrixSetIdentity(&transform);
		fc.opacity = 1.0f;
	}
	RenderTarget::~RenderTarget()
	{
		swapChain->Release();
		vertexBuffer->Release();
		cmdBuffer->Release();
		pipeline->Release();
	}
	void RenderTarget::ConvertArc(
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
	Vector2f RenderTarget::Bezier2(Vector2f p0, Vector2f p1, Vector2f p2, float32 t)
	{
		return (1.0f - t)*((1.0f - t)*p0 + (2.0f*t)*p1) + (t*t)*p2;
	}
	void RenderTarget::TransformPoint(float32 *point)
	{
		float32 p0 = point[0];
		point[0] = point[0] * transform[0][0]
			+ point[1] * transform[1][0]
			+ transform[2][0];
		point[1] = point[1] * transform[1][1]
			+ p0 * transform[0][1]
			+ transform[2][1];
	}
	void RenderTarget::TransformGeometry(
		float32 *data,
		uint32 count,
		float32 &xMin,
		float32 &xMax,
		float32 &yMin,
		float32 &yMax)
	{
		Vector2f prevUntransformed;
		msize geometryPathOffset = 0;
		prevUntransformed.x = data[geometryPathOffset];
		prevUntransformed.y = data[geometryPathOffset + 1];
		TransformPoint(data + geometryPathOffset);
		xMin = Min(xMin, data[geometryPathOffset]);
		xMax = Max(xMax, data[geometryPathOffset]);
		yMin = Min(yMin, data[geometryPathOffset + 1]);
		yMax = Max(yMax, data[geometryPathOffset + 1]);
		geometryPathOffset += 2;
		for (uint32 i = 0; i < count; i++)
		{
			if (Reinterpret<uint32>(data[geometryPathOffset]) == GeometryPath::geometryTypeLine)
			{
				prevUntransformed.x = data[geometryPathOffset + 1];
				prevUntransformed.y = data[geometryPathOffset + 2];
				TransformPoint(data + geometryPathOffset + 1);
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
				TransformPoint(data + geometryPathOffset + 3);
				xMin = Min(xMin, data[geometryPathOffset + 1], data[geometryPathOffset + 3]);
				xMax = Max(xMax, data[geometryPathOffset + 1], data[geometryPathOffset + 3]);
				yMin = Min(yMin, data[geometryPathOffset + 2], data[geometryPathOffset + 4]);
				yMax = Max(yMax, data[geometryPathOffset + 2], data[geometryPathOffset + 4]);
				geometryPathOffset += 5;
			}
			else if(Reinterpret<uint32>(data[geometryPathOffset]) == GeometryPath::geometryTypeArc)
			{
				/*Vector2f dpUntransformed(
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
				GeometryPath::ConvertArc(
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
				geometryPathOffset += 11;*/
			}
			else
			{
				prevUntransformed.x = data[geometryPathOffset + 1];
				prevUntransformed.y = data[geometryPathOffset + 2];
				TransformPoint(data + geometryPathOffset + 1);
				xMin = Min(xMin, data[geometryPathOffset + 1]);
				xMax = Max(xMax, data[geometryPathOffset + 1]);
				yMin = Min(yMin, data[geometryPathOffset + 2]);
				yMax = Max(yMax, data[geometryPathOffset + 2]);
				geometryPathOffset += 3;
			}
		}
	}
	void RenderTarget::AdvanceLine(
		Vector2f p0,
		Vector2f p1,
		std::vector<std::vector<float32>> &scanline)
	{
		const float32 dy = 0.125f;
		float32 xi1, scan, dpr = (p1.x - p0.x) / (p1.y - p0.y);
		uint32 idx = (uint32)((Min(p0.y, p1.y) - (float32)fc.scanlineStart) * 8.0f),
			idxMax = (uint32)((Max(p0.y, p1.y) - (float32)fc.scanlineStart) * 8.0f);
		scan = (float32)fc.scanlineStart + idx * dy + 0.5f*dy;
		while (idx < idxMax)
		{
			xi1 = p0.x + (scan - p0.y)*dpr;
			scanline[idx].push_back(xi1);
			fc.scanlineWidth = Max(fc.scanlineWidth, scanline[idx].size());
			idx++;
			scan += dy;
		}
	}
	void RenderTarget::AdvanceBezier(
		Vector2f p0,
		Vector2f p1,
		Vector2f p2,
		std::vector<std::vector<float32>> &scanline)
	{
		const float32 dy = 0.125f;
		float32 yMin = Min(p0.y, p1.y, p2.y),
			yMax = Max(p0.y, p1.y, p2.y);
		uint32 idx = (uint32)((yMin - (float32)fc.scanlineStart) * 8.0f),
			idxMax = (uint32)((yMax - (float32)fc.scanlineStart) * 8.0f);
		float32 scan = (float32)fc.scanlineStart + idx * dy + 0.5f*dy;
		if (yMin == p0.y)
		{
			scanline[idx].push_back(p0.x);
			if (idx == (uint32)((p2.y - (float32)fc.scanlineStart) * 8.0f))
				scanline[idx].push_back(p2.x);
			idx++;
			scan += dy;
		}
		else if (yMin == p2.y)
		{
			scanline[idx].push_back(p2.x);
			if (idx == (uint32)((p0.y - (float32)fc.scanlineStart) * 8.0f))
				scanline[idx].push_back(p0.x);
			idx++;
			scan += dy;
		}
		if (p0.y > p1.y && p0.y < p2.y)
			p0.y -= 0.5f*dy;
		else if (p2.y > p1.y && p2.y < p0.y)
			p2.y -= 0.5f*dy;
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
					fc.scanlineWidth = Max(fc.scanlineWidth, scanline[idx].size());
				}
				if (xi2 >= 0.0f && xi2 <= 1.0f)
				{
					scanline[idx].push_back(Bezier2(p0, p1, p2, xi2).x);
					fc.scanlineWidth = Max(fc.scanlineWidth, scanline[idx].size());
				}
			}
			idx++;
			scan += dy;
		}
	}
	void RenderTarget::Begin()
	{
		cmdBuffer->BindDescriptorSets(pipeline, vertexBuffer->device->descSet);
		cmdBuffer->BindPipeline(pipeline);
		projX = 2.0f / (float32)swapChain->GetWidth();
		projY = 2.0f / (float32)swapChain->GetHeight();
		vertexBuffer->MapMemory(0, vertexBufferSize, (void **)&vertices);
		cmdBuffer->BindVertexBuffer(0, vertexBuffer);
		MatrixSetIdentity(&transform);
		SetSolidColorBrush(Color(Color::Black));
		SetColorInterpolationMode(ColorInterpolationModeSmooth);
		SetOpacity(1.0f);
	}
	void RenderTarget::End()
	{
		vertexBuffer->UnmapMemory();
		currentVertex = 0;
		void *mappedData;
		vertexBuffer->device->MapMemory(
			geometryMemory,
			geometryOffset * sizeof(float32),
			&mappedData);
		memcpy(
			mappedData,
			geometryData,
			geometryOffset * sizeof(float32));
		vertexBuffer->device->UnmapMemory();
		geometryOffset = 0;
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
	void RenderTarget::SetTransform(Matrix3x2f &transform)
	{
		this->transform = transform;
	}
	Matrix3x2f RenderTarget::GetTransform()
	{
		return transform;
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
	/*void RenderTarget::DrawLine(
		Vector2f a,
		Vector2f b,
		float32 lineWidth)
	{
		GeometryPath path;
		path.Begin(a);
		path.AddLine(b);
	}*/
	/*void RenderTarget::DrawTriangle(
		Vector2f a,
		Vector2f b,
		Vector2f c,
		float32 lineWidth)
	{
		GeometryPath path;
		path.Begin(a);
		path.AddLine(b);
		path.AddLine(c);
	}*/
	void RenderTarget::FillTriangle(
		Vector2f a,
		Vector2f b,
		Vector2f c)
	{
		GeometryPath path;
		path.Begin(a);
		path.AddLine(b);
		path.AddLine(c);
		FillGeometry(&path);
	}
	/*void RenderTarget::DrawRectangle(
		Rectf& rect,
		float32 lineWidth)
	{
		GeometryPath path;
		path.Begin(Vector2f(rect.right, rect.top));
		path.AddLine(Vector2f(rect.left, rect.top));
		path.AddLine(Vector2f(rect.left, rect.bottom));
		path.AddLine(Vector2f(rect.right, rect.bottom));
	}*/
	void RenderTarget::FillRectangle(Rectf& rect)
	{
		GeometryPath path;
		path.Begin(Vector2f(rect.right, rect.top));
		path.AddLine(Vector2f(rect.left, rect.top));
		path.AddLine(Vector2f(rect.left, rect.bottom));
		path.AddLine(Vector2f(rect.right, rect.bottom));
		FillGeometry(&path);
	}
	/*void RenderTarget::DrawRoundedRectangle(
		Rectf &rect,
		float32 rx,
		float32 ry,
		float32 lineWidth)
	{
		GeometryPath path;
		path.Begin(Vector2f(rect.right, rect.top + ry));
		path.AddArcSegment(Vector2f(rect.right - rx, rect.top), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left + rx, rect.top));
		path.AddArcSegment(Vector2f(rect.left, rect.top + ry), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left, rect.bottom - ry));
		path.AddArcSegment(Vector2f(rect.left + rx, rect.bottom), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.right - rx, rect.bottom));
		path.AddArcSegment(Vector2f(rect.right, rect.bottom - ry), rx, ry, 0, false, true);
	}*/
	void RenderTarget::FillRoundedRectangle(
		Rectf &rect,
		float32 rx,
		float32 ry)
	{
		GeometryPath path;
		path.Begin(Vector2f(rect.right, rect.top + ry));
		path.AddArcSegment(Vector2f(rect.right - rx, rect.top), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left + rx, rect.top));
		path.AddArcSegment(Vector2f(rect.left, rect.top + ry), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.left, rect.bottom - ry));
		path.AddArcSegment(Vector2f(rect.left + rx, rect.bottom), rx, ry, 0, false, true);
		path.AddLine(Vector2f(rect.right - rx, rect.bottom));
		path.AddArcSegment(Vector2f(rect.right, rect.bottom - ry), rx, ry, 0, false, true);
	}
	/*void RenderTarget::DrawEllipse(
		Vector2f center,
		float32 rx,
		float32 ry,
		float32 lineWidth)
	{
		GeometryPath path;
		path.Begin(Vector2f(center.x + rx, center.y));
		path.AddArcSegment(Vector2f(center.x - rx, center.y), rx, ry, 0, true, true);
		path.AddArcSegment(Vector2f(center.x + rx, center.y), rx, ry, 0, true, true);
	}*/
	void RenderTarget::FillEllipse(
		Vector2f center,
		float32 rx,
		float32 ry)
	{
		GeometryPath path;
		path.Begin(Vector2f(center.x + rx, center.y));
		path.AddArcSegment(Vector2f(center.x - rx, center.y), rx, ry, 0, true, true);
		path.AddArcSegment(Vector2f(center.x + rx, center.y), rx, ry, 0, true, true);
		FillGeometry(&path);
	}
	void RenderTarget::FillGeometry(GeometryPath *path)
	{
		if (path->IsEmpty()) return;
		std::vector<float32> pathData = path->data;
		float32 xMin = FLT_MAX, xMax = -FLT_MAX,
			yMin = FLT_MAX, yMax = -FLT_MAX;
		TransformGeometry(
			pathData.data(),
			path->count,
			xMin,
			xMax,
			yMin,
			yMax);
		xMin = floor(xMin);
		xMax = ceil(xMax);
		yMin = floor(yMin);
		yMax = ceil(yMax);
		
		std::vector<std::vector<float32>> scanline(8 * (msize)(yMax - yMin));
		Vector2f p0, p1, p2, ps;
		p0.x = pathData[0];
		p0.y = pathData[1];
		ps = p0;
		uint32 pathOffset = 2;
		fc.scanlineStart = (int32)yMin;
		fc.scanlineHeight = 8 * (int32)(yMax - yMin);
		fc.scanlineWidth = 0;
		for (uint32 i = 0; i < path->count; i++)
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
				// not impl
				pathOffset += 11;
			}
		}
		AdvanceLine(p0, ps, scanline);

		fc.scanlineOffset = geometryOffset;
		for (msize i = 0; i < scanline.size(); i++)
		{
			std::sort(scanline[i].begin(), scanline[i].end(), std::greater<>());
			while (scanline[i].size() < fc.scanlineWidth)
				scanline[i].insert(scanline[i].begin(), FLT_MAX);
			memcpy(
				geometryData + geometryOffset,
				scanline[i].data(),
				fc.scanlineWidth * sizeof(float32));
			geometryOffset += fc.scanlineWidth;
		}

		cmdBuffer->PushConstants(&fc, 0, 4 * sizeof(float32), VK_SHADER_STAGE_FRAGMENT_BIT);
		Vector2f v1(xMax, yMin),
			v2(xMin, yMin),
			v3(xMin, yMax),
			v4(xMax, yMax);
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
	void RenderTarget::FillDistanceGeometry(
		DistanceGeometry *geometry,
		GradientCollection *gradientCollection,
		float32 gradientDistance)
	{
		FragmentConstants fcTemp = fc;
		fc.colorMode = colorModeDistanceBased;
		fc.colorOffset = gradientCollection->memOffset >> 2;
		fc.colorCount = gradientCollection->count;
		cmdBuffer->PushConstants(
			&fc,
			64,
			3 * sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);

		fc.paramf[3] = gradientDistance;
		cmdBuffer->PushConstants(
			&fc,
			64 + 6 * sizeof(float32),
			sizeof(float32),
			VK_SHADER_STAGE_FRAGMENT_BIT);
		for (uint32 i = 0; i < geometry->geometryParts.size(); i++)
		{
			if (geometry->geometryParts[i].points.size() == 0) continue;
			fc.paramf[0] = geometry->geometryParts[i].points[0].x;
			fc.paramf[1] = geometry->geometryParts[i].points[0].y;
			TransformPoint(fc.paramf);
			fc.paramf[2] = geometry->geometryParts[i].initialDistance;
			cmdBuffer->PushConstants(
				&fc,
				64 + 3 * sizeof(float32),
				3 * sizeof(float32),
				VK_SHADER_STAGE_FRAGMENT_BIT);
			GeometryPath path;
			path.Begin(Vector2f(
				geometry->geometryParts[i].points[0].x,
				geometry->geometryParts[i].points[0].y));
			for (uint32 j = 1; j < geometry->geometryParts[i].points.size(); j++)
				path.AddLine(Vector2f(geometry->geometryParts[i].points[j]));
			FillGeometry(&path);
		}
		fc = fcTemp;
		cmdBuffer->PushConstants(
			&fc,
			64,
			64,
			VK_SHADER_STAGE_FRAGMENT_BIT);
	}
}
