#include "SliderControl.h"

using namespace ci;

SliderControl::SliderControl(float level)
{
	mLevel = level;
}

void SliderControl::drawSlider(Rectf area)
{
	float tickCenterX = area.x1 + area.getWidth() * (mLevel / 2.0f);
	float tickCenterY = area.getCenter().y;
	float tickX1 = tickCenterX - (area.getHeight() / 2) / 2;
	float tickX2 = tickCenterX + (area.getHeight() / 2) / 2;
	float tickY1 = tickCenterY + area.getHeight();
	float tickY2 = tickCenterY - area.getHeight();
	Rectf tick(tickX1, tickY1, tickX2, tickY2);
	gl::drawSolidRect(area);
	gl::drawSolidRect(tick);
}

void SliderControl::setLevel(float level)
{
	mLevel = level;
}