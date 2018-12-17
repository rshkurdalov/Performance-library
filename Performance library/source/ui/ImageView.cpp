#include "ui\ImageView.h"
#include "gpu\Bitmap.h"

namespace ui
{
	ImageView::ImageView(Bitmap *bitmap)
	{
		SetWidthDesc(autosize());
		SetHeightDesc(autosize());
		SetPadding(Rect<UISize>(0.0f, 0.0f, 0.0f, 0.0f));
		bitmap->AddRef();
		this->bitmap = bitmap;
	}
	ImageView::~ImageView()
	{
		bitmap->Unref();
	}
	Vector2f ImageView::EvaluateContentSizeImpl(
		float32 *viewportWidth,
		float32 *viewportHeight)
	{
		return Vector2f(
			(float32)bitmap->GetWidth(),
			(float32)bitmap->GetHeight());
	}
	void ImageView::RenderImpl(RenderTarget *rt, Vector2f p)
	{
		rt->PushScissor(
			p.x + viewport.left,
			p.y + viewport.top,
			viewport.right - viewport.left,
			viewport.bottom - viewport.top);
		rt->SetBitmapBrush(bitmap, p.x, p.y);
		rt->FillRectangle(
			p.x + viewport.left,
			p.y + viewport.top,
			(float32)bitmap->GetWidth(),
			(float32)bitmap->GetHeight());
		rt->PopScissor();
	}
}
