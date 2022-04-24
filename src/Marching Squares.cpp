#include "Marching Squares.h"

using namespace std;
using namespace ci;

namespace MS
{
	void marchingSquares(const float* amplitudeBuffer, const vector<float>& magSpectrum, const Rectf& bounds)
	{
		gl::color(255.0f, 255.0f, 255.0f);
		int height = abs(bounds.getHeight());
		//height = min(height, CANVAS_HEIGHT);
		int width = abs(bounds.getWidth());
		//width = min(width, CANVAS_WIDTH);

		Grid* myGrid = new Grid(width, height, bounds.x1, bounds.y2);
		myGrid->draw();



		delete myGrid;
	}
}