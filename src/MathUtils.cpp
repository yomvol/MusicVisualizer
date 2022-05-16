#include "MathUtils.h"

Point getNewCoordsFromCartesianToCartesian(Point newOrigin, Point pointCoordsInPrevFrame)
{
	// It`s probably wrong, but works fine, if a difference between origins is positive
	Point pointCoordsInNewFrame;
	pointCoordsInNewFrame.first = pointCoordsInPrevFrame.first - newOrigin.first;
	pointCoordsInNewFrame.second = newOrigin.second - pointCoordsInPrevFrame.second;
	return pointCoordsInNewFrame;
}

Point fromCartesianToPolar(Point pointCoordsInCartFrame)
{
	Point pointCoordsInPolarFrame;
	double tolerance = 0.01;
	pointCoordsInPolarFrame.first = sqrt(pow(pointCoordsInCartFrame.first, 2.0) + pow(pointCoordsInCartFrame.second, 2.0));
	if (pointCoordsInPolarFrame.first < tolerance) // In case point is very near origin
		return Point(0.0, 0.0);

	if (pointCoordsInCartFrame.first >= 0 && pointCoordsInCartFrame.second > 0) // Quadrant I
		pointCoordsInPolarFrame.second = atan(pointCoordsInCartFrame.second / pointCoordsInCartFrame.first);
	else if (pointCoordsInCartFrame.first < tolerance && pointCoordsInCartFrame.second > 0)
		pointCoordsInPolarFrame.second = MATH_PI / 2;
	/*else if (pointCoordsInCartFrame.first < 0 && pointCoordsInCartFrame.second >= 0) // Quandrant II
		pointCoordsInPolarFrame.second = MATH_PI + atan(pointCoordsInCartFrame.second / pointCoordsInCartFrame.first);
	else if (pointCoordsInCartFrame.first < 0 && pointCoordsInCartFrame.second < 0) // Quadrant III
		pointCoordsInPolarFrame.second = MATH_PI + atan(pointCoordsInCartFrame.second / pointCoordsInCartFrame.first);*/
	else if (pointCoordsInCartFrame.first < 0) // Converged solution
		pointCoordsInPolarFrame.second = MATH_PI + atan(pointCoordsInCartFrame.second / pointCoordsInCartFrame.first);
	else if (pointCoordsInCartFrame.first < tolerance && pointCoordsInCartFrame.second < 0)
		pointCoordsInPolarFrame.second = 3 * MATH_PI / 2;
	else if (pointCoordsInCartFrame.first >= 0 && pointCoordsInCartFrame.second < 0) // Quadrant IV
		pointCoordsInPolarFrame.second = 2 * MATH_PI + atan(pointCoordsInCartFrame.second / pointCoordsInCartFrame.first);

	return pointCoordsInPolarFrame;
}