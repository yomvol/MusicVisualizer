#pragma once
#include <vector>
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"
#include <list>
#include <utility>

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
		ci::Rectf  inline getCell(int i, int j) { return mGrid[i][j]; }
	};

	static glm::vec2 origin_for_comparator;
	bool comparator(glm::vec2 a, glm::vec2 b); // Returns true if the first argument goes before the second argument
	//in the strict weak ordering it defines, and false otherwise.
	float marchingSquares(Grid* grid, double (*implicitFunction)(glm::vec2, glm::vec2), glm::vec2 windowCenter, double _scale,
		bool IsReversalNeeded = false);
	glm::vec2 getEndpointByLinearInterpolation(glm::vec2 vert0, glm::vec2 vert1,
		std::function<double(glm::vec2, glm::vec2)> implicitFunction, glm::vec2 drawingOrigin);
	void marchingProcessing(Grid& grid, glm::vec2 drawingOrigin, std::list<glm::vec2>& curvePoints,
		std::function<double(glm::vec2, glm::vec2)> implicitFunction);
}