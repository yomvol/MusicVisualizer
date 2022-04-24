#pragma once
#include <vector>
#include "cinder/Rect.h"
#include "cinder/gl/gl.h"

namespace MS {
	const int CANVAS_HEIGHT = 1024;
	const int CANVAS_WIDTH = 1024;
	const int GRID_SIZE = 20;

	class Grid
	{
	private:
		int mResolutionX, mResolutionY;
		std::vector<std::vector<ci::Rectf>> mGrid;

	public:
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

		ci::Rectf getGrid(int i, int j) { return mGrid[i][j]; }
	};


	void marchingSquares(const float* amplitudeBuffer, const std::vector<float>& magSpectrum, const ci::Rectf& bounds);
	
}