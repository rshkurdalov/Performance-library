#include "Scene.h"
#include "graphics\TextLayout.h"

Scene::Scene(Application *app)
	: UIObject(app->window)
{
	this->app = app;

	std::vector<Vector2f> points =
	{
		Vector2f(650,100),
		Vector2f(100,100),
		Vector2f(100,300),
		Vector2f(500,300),
		Vector2f(500,500),
		Vector2f(650,500)
	};
	CreateDistanceGeometry(
		points.data(),
		points.size(),
		&geometry,
		&Vector2f(400, 270),
		0.0f);

	points =
	{
		Vector2f(600,50),
		Vector2f(400,50),
		Vector2f(150,50),
		Vector2f(150,150),
		Vector2f(50,75),
		Vector2f(50,200),
		Vector2f(150,350),
		Vector2f(200,200),
		Vector2f(250,300),
		Vector2f(220,350),
		Vector2f(200,300),
		Vector2f(200,400),
		Vector2f(300,400),
		Vector2f(250,200),
		Vector2f(600,200),
		Vector2f(600,250),
		Vector2f(550,250),
		Vector2f(550,300),
		Vector2f(700,300),
		Vector2f(700,150),
		Vector2f(200,150),
		Vector2f(200,100),
		Vector2f(600,100),
	};
	CreateDistanceGeometry(
		points.data(),
		points.size(),
		&geometry2,
		&Vector2f(600, 50),
		0.0f);

	GradientStop gradientStops[6];
	gradientStops[0].color = Color(Color::Blue);
	gradientStops[0].offset = 0.1;
	gradientStops[1].color = Color(Color::Yellow);
	gradientStops[1].offset = 0.15;
	gradientStops[2].color = Color(Color::Red);
	gradientStops[2].offset = 0.2;
	gradientStops[3].color = Color(Color::Green);
	gradientStops[3].offset = 0.3;
	gradientStops[4].color = Color(Color::Purple);
	gradientStops[4].offset = 0.5;
	gradientStops[5].color = Color(Color::White);
	gradientStops[5].offset = 0.9;
	app->device->CreateGradientCollection(gradientStops, ARRAYSIZE(gradientStops), &gradientCollection);
}

void Scene::RenderImpl()
{
	app->cmdBuffer->Begin();
	app->cmdBuffer->BeginRenderPass(app->swapChain);
	app->cmdBuffer->SetViewport(0, 0, window->GetWidth(), window->GetHeight(), 0.0f, 1.0f);

	app->renderTarget->Begin();

	GeometryPath path;
	path.Move(Vector2f(600, 200));
	path.AddLine(Vector2f(500, 300));
	path.AddArcSegment(Vector2f(400, 400), 150, 50, mPI / 6, true, true);
	path.AddLine(Vector2f(800, 550));
	app->renderTarget->SetSolidColorBrush(Color(Color::White));
	Geometry g;
	g.FillGeometry(&path);
	app->renderTarget->Render(&g);

	app->renderTarget->SetColorInterpolationMode(ColorInterpolationModeFlat);

	// Uncomment to test text rendering

	/*app->renderTarget->SetSolidColorBrush(Color(Color::White));
	app->renderTarget->FillRectangle(Rectf(0, 0, 800, 600));
	app->renderTarget->SetSolidColorBrush(Color(Color::Black));
	std::u16string str = u"abcdefgh ijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	TextLayout tl;
	uint32 size = 20;
	tl.SetText((char16 *)str.data(), str.size(), app->font, size);
	for (uint32 i = 0; i < 30; i++)
		tl.Render(Vector2f(0, size*i), app->renderTarget);*/

	app->renderTarget->End();

	app->cmdBuffer->EndRenderPass();
	app->cmdBuffer->End();
	app->cmdBuffer->Submit(app->swapChain);
	app->swapChain->Present();
}
