#pragma once
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rect.h"


class SliderControl
{
public:
	SliderControl(float level);
	void drawSlider(ci::Rectf area);
	void setLevel(float level);

private:
	float mLevel;
};