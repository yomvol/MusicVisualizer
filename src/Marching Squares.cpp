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
		const unsigned int min_per_thread = 50; // One thread works out minimum {number} cells
		const unsigned int max_threads = (myGrid->mResolutionX * myGrid->mResolutionY) / min_per_thread;
		const unsigned int hardware_threads = thread::hardware_concurrency();
		unsigned int num_threads = min(hardware_threads != 0 ? hardware_threads : 4, max_threads);
		unsigned int num_threads_P2 = 1; // Number of threads adjusted to the nearest lesser power of two
		while (num_threads >>= 1) // While(`k` >>= 1) n = n << 1 double `k` and divide `n` in half till it becomes 0
			num_threads_P2 = num_threads_P2 << 1;
		vector<thread> threads;
		vector<vec2> points;
		computeRecursiveSquares(myGrid, num_threads_P2, threads, implicitFunction, drawingOrigin, points);

		list<ContourPoint> toBeSortedPoints;

		for (auto iter = threads.begin(); iter < threads.end(); ++iter)
		{
			iter->join();
		}

		// There must be some kind of sorting, because points append into the collection in a random order
		
		for (auto iter = points.begin(); iter < points.end(); ++iter)
		{
			ContourPoint cPoint;
			cPoint.screenCartCoords.x = iter->x;
			cPoint.screenCartCoords.y = iter->y;
			Point result = getNewCoordsFromCartesianToCartesian(Point(drawingOrigin.x, drawingOrigin.y),
				Point(cPoint.screenCartCoords.x, cPoint.screenCartCoords.y));

			Point polarForm = fromCartesianToPolar(result);
			cPoint.pointPolarCoords.x = polarForm.first; // Radius-vector
			cPoint.pointPolarCoords.y = polarForm.second; // Angle
			toBeSortedPoints.push_back(cPoint);
		}

		toBeSortedPoints.sort(comparator);

		// Drawing here with resulting points
		Path2d contour;
		auto pointIter = toBeSortedPoints.begin();
		contour.moveTo(pointIter->screenCartCoords);
		pointIter++;
		for (pointIter; pointIter != toBeSortedPoints.end(); ++pointIter)
		{
			contour.lineTo(pointIter->screenCartCoords);
		}

		delete myGrid;
		gl::color(255.0f, 255.0f, 255.0f);
	}

	bool comparator(ContourPoint first, ContourPoint second)
	{
		if (first.pointPolarCoords.y <= second.pointPolarCoords.y)
			return true;
		else
			return false;
	}

	int scrollCipher(int input)
	{
		if (input == 0) // 00
			return 10; // 10
		else if (input == 10)
			return 1; // 01
		else if (input == 1)
			return 11; // 11
		else if (input == 11)
			return 0; // back to 00
		else
			return 999; // sign of unwanted input 
	}

	void fromCipherToCellNumber(int pointCipher, pair<int, int>& result, int cellsInClusterX, int cellsInClusterY)
	{
		switch (pointCipher)
		{
		case 0: // 00
			result.first = 0; // x
			result.second = 0; // y
			break;
		case 1: // 01
			result.first = 0; // x
			result.second = cellsInClusterY; // y
			break;
		case 2: // 02
			result.first = 0; // x
			result.second = 2 * cellsInClusterY; // y
			break;
		case 10: // 10
			result.first = cellsInClusterX; // x
			result.second = 0; // y
			break;
		case 11: // 11
			result.first = cellsInClusterX;
			result.second = cellsInClusterY;
			break;
		case 12: // 12
			result.first = cellsInClusterX; // x
			result.second = 2 * cellsInClusterY; // y
			break;
		case 20: // 20 I won`t use this case, but include it for the sake of completeness
			result.first = 2 * cellsInClusterX; // x
			result.second = 0; // y
			break;
		case 21:
			result.first = 2 * cellsInClusterX; // x
			result.second = cellsInClusterY; // y
			break;
		case 22:
			result.first = 2 * cellsInClusterX; // x
			result.second = 2 * cellsInClusterY; // y
			break;
		default:
			throw;
		}
	}

	class MutexWrapper // A mutex passed over simply by reference to a thread function produces undefined behavior
	{
	private:
		mutex* m;

	public:
		MutexWrapper() {
			m = new mutex;
		}
		inline mutex* getMutex() { return m; }
		~MutexWrapper() { delete m; }
	};

	void computeRecursiveSquares(Grid* grid, int num_of_threads, vector<thread>& threads,
		std::function<double(vec2, vec2)> implicitFunction, vec2 drawingOrigin, vector<vec2>& curvePoints)
	{
		const unsigned int num_of_clusters_on_side = 2; // Entire screen is always split on 4 clusters
		int startCellX = 0, startCellY = 0, endCellX = 0, endCellY = 0;
		int cellsInClusterX = grid->mResolutionX / num_of_clusters_on_side;
		int remainderX = grid->mResolutionX % num_of_clusters_on_side;
		int cellsInClusterY = grid->mResolutionY / num_of_clusters_on_side;
		int remainderY = grid->mResolutionY % num_of_clusters_on_side;
		
		int startCornerCode = 0;
		pair<int, int> coords;
		if (num_of_threads > 4) // If there are more than 4 threads, every cluster has sub-clusters
		{
			int reduced_num = num_of_threads - 4;
			if (reduced_num < 1)
				return;
			computeRecursiveSquares(grid, reduced_num, threads, implicitFunction, drawingOrigin, curvePoints);
		}
		else
		{
			for (int i = 0; i < num_of_threads; i++)
			{
				fromCipherToCellNumber(startCornerCode, coords, cellsInClusterX, cellsInClusterY);
				startCellX = coords.first;
				startCellY = coords.second;
				fromCipherToCellNumber(startCornerCode + 11, coords, cellsInClusterX, cellsInClusterY);
				endCellX = coords.first;
				endCellY = coords.second;
				if (i == (num_of_threads - 1))
				{
					endCellX += remainderX;
					endCellY += remainderY;
				}
				startCornerCode = scrollCipher(startCornerCode);

				std::function<double(vec2, vec2)> wrapper = implicitFunction;
				MutexWrapper muWrapper;
				thread t(marchingProcessing, std::ref(*grid), drawingOrigin, std::ref(muWrapper), std::ref(curvePoints),
					startCellX, startCellY, endCellX, endCellY, wrapper);
				threads.push_back(move(t));
			}
		}
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

	void marchingProcessing(Grid& grid, vec2 drawingOrigin, MutexWrapper& muWrapper, vector<vec2>& curvePoints,
		int startCellX, int startCellY, int endCellX, int endCellY, std::function<double(glm::vec2, glm::vec2)> implicitFunction)
	{
		ci::ThreadSetup threadSetup;
		int i = startCellX, j = startCellY;
		for (i; i < endCellX; i++)
		{
			for (j; j < endCellY; j++)
			{
				muWrapper.getMutex()->lock();
				Rectf cell = grid.getGrid(i, j);
				muWrapper.getMutex()->unlock();
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
					muWrapper.getMutex()->lock();
					curvePoints.push_back(p0);
					curvePoints.push_back(p1);
					if (p2.length != 0)
					{
						curvePoints.push_back(p2);
						curvePoints.push_back(p3);
					}
					muWrapper.getMutex()->unlock();
				}
			}
		}
	}
}