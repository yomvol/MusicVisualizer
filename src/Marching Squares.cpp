#include "Marching Squares.h"

using namespace std;
using namespace ci;

namespace MS
{
	float marchingSquares(Grid* grid, double (*implicitFunction)(vec2, vec2, double, double), vec2 windowCenter, double _scale,
		double a, double b, bool IsReversalNeeded)
	{
		/*gl::color(200.0f, 200.0f, 0);
		gl::drawSolidCircle(windowCenter, 5);*/ // useful for debugging

		gl::color(0, 255.0f, 0); // With which color do we draw?
		list<vec2> contourPoints;
		marchingProcessing(*grid, windowCenter, contourPoints, implicitFunction, a, b);
		if (contourPoints.size() == 0)
		{
			gl::color(255.0f, 255.0f, 255.0f);
			return 0.0f;
		}
			
		// There must be some kind of sorting
		origin_for_comparator = windowCenter;
		contourPoints.sort(&comparator);

		// Drawing here with resulting points
		Path2d contour;
		auto pointIter = contourPoints.begin();
		contour.moveTo(*pointIter);
		pointIter++;
		for (pointIter; pointIter != contourPoints.end(); ++pointIter)
		{
			contour.lineTo(*pointIter);
		}

		contour.scale(vec2(_scale, _scale), windowCenter);
		if (IsReversalNeeded == true)
		{
			gl::pushMatrices();
			gl::rotate(M_PI, windowCenter.x, 0, 0);
			gl::translate(0, -2 * windowCenter.y);
			gl::draw(contour);
			gl::popMatrices();
		}
		else
		{
			gl::draw(contour);
		}

		/*gl::color(255, 0, 0); // useful for debugging
		for (auto p : contourPoints)
		{
			gl::drawSolidCircle(p, 2.5);
		}*/

		gl::color(255.0f, 255.0f, 255.0f);
		return contour.calcPreciseBoundingBox().getHeight();
	}

	bool comparator(vec2 a, vec2 b)
	{
		if (a.x - origin_for_comparator.x >= 0 && b.x - origin_for_comparator.x < 0)
			return true;
		if (a.x - origin_for_comparator.x < 0 && b.x - origin_for_comparator.x >= 0)
			return false;
		if (a.x - origin_for_comparator.x == 0 && b.x - origin_for_comparator.x == 0) {
			if (a.y - origin_for_comparator.y >= 0 || b.y - origin_for_comparator.y >= 0)
				return a.y > b.y;
			return b.y > a.y;
		}

		// compute the cross product of vectors (center -> a) x (center -> b)
		int det = (a.x - origin_for_comparator.x) * (b.y - origin_for_comparator.y) - (b.x - origin_for_comparator.x)
			* (a.y - origin_for_comparator.y);
		if (det < 0)
			return true;
		if (det > 0)
			return false;

		// points a and b are on the same line from the center
		// check which point is closer to the center
		int d1 = (a.x - origin_for_comparator.x) * (a.x - origin_for_comparator.x) + (a.y - origin_for_comparator.y)
			* (a.y - origin_for_comparator.y);
		int d2 = (b.x - origin_for_comparator.x) * (b.x - origin_for_comparator.x) + (b.y - origin_for_comparator.y)
			* (b.y - origin_for_comparator.y);
		return d1 > d2;
	}

	vec2 getEndpointByLinearInterpolation(vec2 vert0, vec2 vert1,
		std::function<double(vec2, vec2, double, double)> implicitFunction, vec2 drawingOrigin, double a, double b)
	{
		double x, y;
		if (vert0.x == vert1.x)
		{
			x = vert0.x;
			y = vert0.y + (vert1.y - vert0.y) *
				((1.0 - implicitFunction(vert0, drawingOrigin, a, b)) /
					(implicitFunction(vert1, drawingOrigin, a, b) - implicitFunction(vert0, drawingOrigin, a, b)));
			return vec2(x, y);
		}
		else if (vert0.y == vert1.y)
		{
			y = vert0.y;
			x = vert0.x + (vert1.x - vert0.x) *
				((1.0 - implicitFunction(vert0, drawingOrigin, a, b)) /
					(implicitFunction(vert1, drawingOrigin, a, b) - implicitFunction(vert0, drawingOrigin, a, b)));
			return vec2(x, y);
		}
		else
			throw;
	}

	void marchingProcessing(Grid& grid, vec2 drawingOrigin, list<vec2>& curvePoints,
		std::function<double(vec2, vec2, double, double)> implicitFunction, double a, double b)
	{
		for (int i = 0; i < grid.mResolutionX; i++)
		{
			for (int j = 0; j < grid.mResolutionY; j++)
			{
				Rectf cell = grid.getCell(i, j);
				vec2 vertice;
				unsigned int code = 0;
				for (int k = 0; k < 4; k++) // 4 sides of a cell
				{
					switch (k)
					{
					case 0:
						vertice = cell.getLowerLeft();
						break;
					case 1:
						vertice = cell.getLowerRight();
						break;
						break;
					case 2:
						vertice = cell.getUpperRight();
						break;
					case 3:
						vertice = cell.getUpperLeft();
						break;
					default:
						throw;
						break;
					}

					double value = implicitFunction(vertice, drawingOrigin, a, b);
					if (value >= 1)
					{
						code = code << 1;
						code |= 00000001;
						/*gl::color(255.0f, 0, 0); // these are useful for debugging
						gl::drawSolidCircle(vertice, 5);*/
					}
					else
					{
						code = code << 1;
						/*gl::color(0, 0, 255.0f);
						gl::drawSolidCircle(vertice, 5);*/
					}
				}

				vec2 p0, p1, p2, p3;
				// look-up table
				switch (code)
				{
				case 0: // 0000
					break;
				case 1: // 0001
					//p0.x = cell.x1; // The commented snippets are mid-point realization
					//p0.y = cell.getCenter().y;
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					/*p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 2:
					/*p0.x = cell.x2;
					p0.y = cell.getCenter().y;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 3:
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.getCenter().y;
					p1.x = cell.x2;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 4:
					/*p0.x = cell.getCenter().x;
					p0.y = cell.y2;
					p1.y = cell.getCenter().y;
					p1.x = cell.x2;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 5:
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.x = cell.getCenter().x;
					p1.y = cell.y2;
					p2.x = cell.getCenter().x;
					p2.y = cell.y1;
					p3.x = cell.x2;
					p3.y = cell.getCenter().y;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					p2 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					p3 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					//gl::drawLine(p2, p3);
					break;
				case 6:
					/*p0.x = cell.getCenter().x;
					p0.y = cell.y2;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 7: // 0111
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.y2;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 8: // 1000
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.y2;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 9: // 1001
					/*p0.x = cell.getCenter().x;
					p0.y = cell.y2;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 10: // 1010
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.x = cell.getCenter().x;
					p1.y = cell.y1;
					p2.x = cell.getCenter().x;
					p2.y = cell.y2;
					p3.x = cell.x2;
					p3.y = cell.getCenter().y;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					p2 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					p3 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					//gl::drawLine(p2, p3);
					break;
				case 11: // 1011
					/*p0.x = cell.x2;
					p0.y = cell.getCenter().y;
					p1.y = cell.y2;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 12: // 1100
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.getCenter().y;
					p1.x = cell.x2;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 13: // 1101
					/*p0.x = cell.x2;
					p0.y = cell.getCenter().y;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 14: // 1110
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction,
						drawingOrigin, a, b);
					//gl::drawLine(p0, p1);
					break;
				case 15: // 1111
					break;
				default:
					throw;
					break;
				}
				if (code != 0 && code != 15)
				{
					curvePoints.push_back(p0);
					curvePoints.push_back(p1);
					if (p2.x != 0 || p2.y != 0)
					{
						curvePoints.push_back(p2);
						curvePoints.push_back(p3);
					}
				}
			}
		}
	}
}