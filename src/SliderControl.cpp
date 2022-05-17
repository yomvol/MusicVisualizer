#include "SliderControl.h"

using namespace ci;

SliderControl::SliderControl(float level, Rectf area) : mLevel{level}, mArea{area}
{}

void SliderControl::drawSlider()
{
	float tickCenterX = mArea.x1 + mArea.getWidth() * (mLevel / 2.0f);
	float tickCenterY = mArea.getCenter().y;
	float tickX1 = tickCenterX - (mArea.getHeight() / 2) / 2;
	float tickX2 = tickCenterX + (mArea.getHeight() / 2) / 2;
	float tickY1 = tickCenterY + mArea.getHeight();
	float tickY2 = tickCenterY - mArea.getHeight();
	Rectf tick(tickX1, tickY1, tickX2, tickY2);
	gl::drawSolidRect(mArea);
	gl::drawSolidRect(tick);
}

void SliderControl::setLevel(float level)
{
	mLevel = level;
}

Rectf SliderControl::getArea()
{
	return mArea;
}

void SliderControl::setArea(Rectf area)
{
	mArea = area;
}