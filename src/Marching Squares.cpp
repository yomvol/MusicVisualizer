#include "Marching Squares.h"

using namespace std;
using namespace ci;

namespace MS
{
	void marchingSquares(const Rectf& bounds, double (*implicitFunction)(vec2, vec2), vec2 drawingOrigin)
	{
		int height = abs(bounds.getHeight());
		//height = min(height, CANVAS_HEIGHT); // useful for debugging
		int width = abs(bounds.getWidth());
		//width = min(width, CANVAS_WIDTH);

		Grid* myGrid = new Grid(width, height, bounds.x1, bounds.y2);
		myGrid->draw();

		gl::color(200.0f, 200.0f, 0);
		gl::drawSolidCircle(drawingOrigin, 5);

		gl::color(0, 255.0f, 0); // With which color do we draw?
		vector<vec2> contourPoints;
		marchingProcessing(*myGrid, drawingOrigin, contourPoints, implicitFunction);

		// Drawing here with resulting points
		Path2d contour;
		auto pointIter = contourPoints.begin();
		contour.moveTo(*pointIter);
		pointIter++;
		for (pointIter; pointIter < contourPoints.end(); ++pointIter)
		{
			contour.lineTo(*pointIter);
		}
		gl::draw(contour);

		delete myGrid;
		gl::color(255.0f, 255.0f, 255.0f);
	}

	vec2 getEndpointByLinearInterpolation(vec2 vert0, vec2 vert1, std::function<double(vec2, vec2)> implicitFunction,
		vec2 drawingOrigin)
	{
		double x, y;
		if (vert0.x == vert1.x)
		{
			x = vert0.x;
			y = vert0.y + (vert1.y - vert0.y) *
				((1.0 - implicitFunction(vert0, drawingOrigin)) /
					(implicitFunction(vert1, drawingOrigin) - implicitFunction(vert0, drawingOrigin)));
			return vec2(x, y);
		}
		else if (vert0.y == vert1.y)
		{
			y = vert0.y;
			x = vert0.x + (vert1.x - vert0.x) *
				((1.0 - implicitFunction(vert0, drawingOrigin)) /
					(implicitFunction(vert1, drawingOrigin) - implicitFunction(vert0, drawingOrigin)));
			return vec2(x, y);
		}
		else
			throw;
	}

	void marchingProcessing(Grid& grid, vec2 drawingOrigin, vector<vec2>& curvePoints,
		std::function<double(glm::vec2, glm::vec2)> implicitFunction)
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

					double value = implicitFunction(vertice, drawingOrigin);
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
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
					/*p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 2:
					/*p0.x = cell.x2;
					p0.y = cell.getCenter().y;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 3:
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.getCenter().y;
					p1.x = cell.x2;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 4:
					/*p0.x = cell.getCenter().x;
					p0.y = cell.y2;
					p1.y = cell.getCenter().y;
					p1.x = cell.x2;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction, drawingOrigin);
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
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					p2 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					p3 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					//gl::drawLine(p2, p3);
					break;
				case 6:
					/*p0.x = cell.getCenter().x;
					p0.y = cell.y2;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 7: // 0111
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.y2;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 8: // 1000
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.y2;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 9: // 1001
					/*p0.x = cell.getCenter().x;
					p0.y = cell.y2;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
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
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					p2 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					p3 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					//gl::drawLine(p2, p3);
					break;
				case 11: // 1011
					/*p0.x = cell.x2;
					p0.y = cell.getCenter().y;
					p1.y = cell.y2;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 12: // 1100
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.getCenter().y;
					p1.x = cell.x2;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getUpperLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getUpperRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 13: // 1101
					/*p0.x = cell.x2;
					p0.y = cell.getCenter().y;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getLowerLeft(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getUpperRight(), cell.getLowerRight(), implicitFunction, drawingOrigin);
					//gl::drawLine(p0, p1);
					break;
				case 14: // 1110
					/*p0.x = cell.x1;
					p0.y = cell.getCenter().y;
					p1.y = cell.y1;
					p1.x = cell.getCenter().x;*/
					p0 = getEndpointByLinearInterpolation(cell.getUpperLeft(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
					p1 = getEndpointByLinearInterpolation(cell.getLowerRight(), cell.getLowerLeft(), implicitFunction, drawingOrigin);
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