#pragma once
#include <vector>
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include <thread>
#include <functional>
#include <mutex>
#include "cinder/Thread.h"
#include "MathUtils.h"
#include <list>

namespace MS {
	/*
 * The standard approach in literature is to construct and use
 * lookup tables for all the possible cases. There are certain
 * ambiguous cases as well which will be dealt explicitly
 *
 * The numbering of the square is as follows:
 *
 *        Node: 3     Edge: 2     Node: 2
 *            o-----------------------o
 *            |                       |
 *            |                       |
 *            |                       |
 *  Edge: 3   |                       |   Edge: 1
 *            |                       |
 *            |                       |
 *            |                       |
 *            |                       |
 *            o-----------------------o
 *        Node: 0     Edge: 0     Node: 1
 *
 * We will represent the node by 1 if it is positive
 * Otherwise, we assign it the value 0
 *
 * All possible states of the nodes ( 2 ^ 4 = 16) can
 * be represented by a 4-bit id. This id, we will use
 * the following arbitrary order:
 *
 *      (Node0, Node1, Node2, Node3)
 *
 * So, for example, if node 3 is the only positive node,
 * we have,
 *
 *      id: 0001
 *
 * This binary converted to int corresponds to the lookup key
 *
 */

	const int CANVAS_HEIGHT = 1024;
	const int CANVAS_WIDTH = 1024;
	const int GRID_SIZE = 10;

	class Grid
	{
	private:
		std::vector<std::vector<ci::Rectf>> mGrid;

	public:
		int mResolutionX, mResolutionY;
		Grid(int width, int height)
		{
			mResolutionX = width / GRID_SIZE;
			mResolutionY = height / GRID_SIZE;
			mGrid.resize(mResolutionX);
			for (auto iter = mGrid.begin(); iter < mGrid.end(); ++iter)
				iter->resize(mResolutionY);
			for (int i = 0; i < mResolutionX; i++)
			{
				for (int j = 0; j < mResolutionY; j++)
				{
					mGrid[i][j].set(GRID_SIZE * i, GRID_SIZE * j, GRID_SIZE * i + GRID_SIZE, GRID_SIZE * j + GRID_SIZE);
				}
			}
		}

		Grid(int width, int height, int initialX, int initialY)
		{
			mResolutionX = width / GRID_SIZE;
			mResolutionY = height / GRID_SIZE;
			mGrid.resize(mResolutionX);
			for (auto iter = mGrid.begin(); iter < mGrid.end(); ++iter)
				iter->resize(mResolutionY);
			for (int i = 0; i < mResolutionX; i++)
			{
				for (int j = 0; j < mResolutionY; j++)
				{
					mGrid[i][j].set(initialX + GRID_SIZE * i, initialY + GRID_SIZE * j, 
						initialX + GRID_SIZE * i + GRID_SIZE, initialY + GRID_SIZE * j + GRID_SIZE);
				}
			}
		}

		void draw()
		{
			for (int i = 0; i < mResolutionX; i++)
			{
				for (int j = 0; j < mResolutionY; j++)
				{
					ci::gl::drawStrokedRect(mGrid[i][j]);
				}
			}
		}
		ci::Rectf  inline getGrid(int i, int j) { return mGrid[i][j]; }
	};

	class MutexWrapper;

	typedef struct ContourPoint
	{
		glm::vec2 screenCartCoords; // Cartesian coordiate frame used by Cinder to draw, originating in the upper left corner
		glm::vec2 pointPolarCoords;
	} ContourPoint;

	bool comparator(ContourPoint first, ContourPoint second);
	void marchingSquares(const ci::Rectf& bounds, double (*implicitFunction)(glm::vec2, glm::vec2), glm::vec2 drawingOrigin);
	glm::vec2 getEndpointByLinearInterpolation(glm::vec2 vert0, glm::vec2 vert1,
		std::function<double(glm::vec2, glm::vec2)> implicitFunction, glm::vec2 drawingOrigin);
	void marchingProcessing(Grid& grid, glm::vec2 drawingOrigin, MutexWrapper& muWrapper, std::vector<glm::vec2>& curvePoints,
		int startCellX, int startCellY, int endCellX, int endCellY,
		std::function<double(glm::vec2, glm::vec2)> implicitFunction); //The processing function, which is meant to run parallelly
	int scrollCipher(int input);
	void fromCipherToCellNumber(int pointCipher, std::pair<int, int>& result, int cellsInClusterX, int cellsInClusterY);
	void computeRecursiveSquares(Grid* grid, int num_of_threads, std::vector<std::thread>& threads,
		std::function<double(glm::vec2, glm::vec2)> implicitFunction, glm::vec2 drawingOrigin, std::vector<glm::vec2>& curvePoints);
}