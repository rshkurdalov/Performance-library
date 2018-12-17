#pragma once
#include "ui\UIObject.h"

namespace ui
{
	class ImageView : public UIObject
	{
	protected:
		Bitmap *bitmap;

		Vector2f EvaluateContentSizeImpl(
			float32 *viewportWidth,
			float32 *viewportHeight);
		void RenderImpl(RenderTarget *rt, Vector2f p);
	public:
		ImageView(Bitmap *bitmap);
		~ImageView();
	};
}
