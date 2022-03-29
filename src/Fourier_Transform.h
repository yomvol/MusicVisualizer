// include <iterator>
#include <list>
#include <complex>
//#include <cmath>
typedef std::complex<double> ComplexVal;

namespace FFT
{
	const double PI = 3.14159265358979323846;
	struct Point
	{
		double x;
		double y;
	};
	
	unsigned int findClosestPowerOfTwo (const unsigned double input)
	{
		unsigned int result = 1;
		while (result < input)
		{
			result *= 2;
		}
		return result;
	}
	
	std::list<Point> fastFourierTransform (const std::list<ComplexVal>& coeffs)
	{
		// P = [p0, p1, ... , p_n-1] <- coefficient representation of the given polynom
		//unsigned int n = findClosestPowerOfTwo(coeffs.size());
		if (n <= 1)
			return coeffs;
		// append nulls?
		std::list<ComplexVal> evens, odds;
		for (int i = 0; i < n; i++)
		{
			if(i % 2 == 0)
				evens.push_back(coeffs.at(i));
			else
				odds.push_back(coeffs.at(i));
		}
		std::list<Point> y_evens, y_odds;
		y_evens = fastFourierTransform(&evens);
		y_odds = fastFourierTransform(&odds);
		std::list<Point> output (n, {0, 0});
		for (int j = 0; j < n/2; j++)
		{
			ComplexVal omega = std::polar(1.0, 2 * PI * j / n);
			ComplexVal compPoint = ComplexVal(y_evens.x, y_evens.y) + omega * ComplexVal(y_odds.x, y_odds.y);
			output.at(j).x= compPoint.real;
			output.at(j).y= compPoint.imag;
			ComplexVal compPoint2 = ComplexVal(y_evens.x, y_evens.y) - omega * ComplexVal(y_odds.x, y_odds.y);
			output.at(j + n/2).x= compPoint2.real;
			output.at(j + n/2).y= compPoint2.imag;
		}
		return output;
	}
}












