#pragma once
#include "ui\UIObject.h"
#include "Application.h"
#include "graphics/GeometryPath.h"
#include "graphics/Geometry.h"

using namespace ui;

class Scene : public UIObject
{
public:
	Application *app;
	Matrix4x4f projection;
	Matrix4x4f view;
	Matrix4x4f model;
	Matrix4x4f MVP;
	DistanceGeometry geometry, geometry2;
	GradientCollection *gradientCollection;

	Scene(Application *app);
	void RenderImpl();
};
