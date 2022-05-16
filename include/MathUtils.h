#pragma once
#include <utility>

const double MATH_PI = 3.1415926535;

typedef std::pair<double, double> Point; // first must represent x and second must represent y for Cartesian coordinate frames
// first must represent radius-vector and second must represent angle in radians for Polar coordinate frames
Point getNewCoordsFromCartesianToCartesian(Point newOrigin, Point pointCoordsInPrevFrame);
Point fromCartesianToPolar(Point pointCoordsInCartFrame);