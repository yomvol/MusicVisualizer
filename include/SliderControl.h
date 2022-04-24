#pragma once
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Rect.h"


class SliderControl
{
public:
	SliderControl(float level, ci::Rectf area);
	void drawSlider();
	void setLevel(float level);
	ci::Rectf getArea();
	void setArea(ci::Rectf area);

private:
	float mLevel; // from 0 to 2.0 representing 200% max volume
	ci::Rectf mArea;
};