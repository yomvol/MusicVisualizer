/*
 Copyright (c) 2014, The Cinder Project

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "AudioDrawUtils.h"

#include "cinder/audio/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Shader.h"

using namespace std;
using namespace ci;

RGBMask::RGBMask(float num) // Compute RGB mask once and use it until the end
{
	int numOfBins = (int)num;
	if (numOfBins <= 0)
		return;
	red_exp = make_unique<vector<double>>();
	green_norm = make_unique<vector<double>>();
	blue_exp = make_unique<vector<double>>();
	for (int bin = 0; bin < numOfBins; bin++)
	{
		int A = 255; // Max number in RGB
		double T = 500.0; // Damping factor
		int k = 150000; // Amplifying coeff
		double mean = numOfBins / 2;
		double deviation = 230.0; // To be picked experimentally
		blue_exp.get()->push_back( A * exp(-(double)bin / T));
		red_exp.get()->push_back( A * exp(double(bin - numOfBins) / T));
		double power = -0.5 * pow(((bin - mean) / deviation), 2);
		double divisor = 1 / (deviation * sqrt(2 * M_PI));
		double bell = k * divisor * exp(power); // A slappy implementation. Peak is about 260 with this k.
		if (bell > 255)
			bell = 255;
		green_norm.get()->push_back(bell); // Green channel is plotted like a normal distribution (Bell curve)
	}
}

void drawColorfulFlash(const audio::Buffer& buffer, const vector<float>& magSpectrum, const Rectf& bounds, const RGBMask& mask)
{
	// Define color of the lightning based on dominating frequency bins
	// Bins are represented from 0 to Nyquist frequency (samplerate / 2)
	ColorA color;
	if (&magSpectrum == nullptr)
		color = Color(255.0f, 255.0f, 255.0f);
	else
	{
		const int NUM_OF_BINS = magSpectrum.size();
		float red = 0.0f, green = 0.0f, blue = 0.0f;
		for (int bin = 0; bin < NUM_OF_BINS; bin++)
		{
			float weight = magSpectrum[bin];
			weight = audio::linearToDecibel(weight) / 100;
			red += weight * mask.red_exp.get()->at(bin);
			green += weight * mask.green_norm.get()->at(bin);
			blue += weight * mask.blue_exp.get()->at(bin);
		}
		red = glm::mod(red, 255.0f);
		green = glm::mod(green, 255.0f);
		blue = glm::mod(blue, 255.0f);
		red /= 255.0f + 0.2f; // Normalizing from (0; 255) to (0; 1) and adding minimal brightness
		green /= 255.0f + 0.2f;
		blue /= 255.0f + 0.2f;
		color = Color(red, green, blue); // Constructor works correctly only with 0 <= float <= 1
	}
	gl::ScopedGlslProg glslScope(getStockShader(gl::ShaderDef().color()));
	gl::color(color);

	const float waveHeight = bounds.getHeight();
	const float xScale = bounds.getWidth() / (float)buffer.getNumFrames();

	float yOffset = bounds.y1;
	
	PolyLine2f waveform;
	const float* channel = buffer.getChannel(0); // left channel
	float x = bounds.x1;
	for (size_t i = 0; i < buffer.getNumFrames(); i++) {
		x += xScale;
		float y = (1 - (channel[i] * 0.5f + 0.5f)) * waveHeight + yOffset;
		waveform.push_back(vec2(x, y));
	}

	if (!waveform.getPoints().empty())
		gl::draw(waveform);
	gl::color(255.0f, 255.0f, 255.0f); // Restoring default color for other elements to draw
}

void drawConcentricShapes(const audio::Buffer& buffer, const vector<float>& magSpectrum, const Rectf& bounds, vec2 windowCenter)
{
	// loudness influences amount of "circles", frequency spetrcum influence shapes based on Skew Factor 0 - 100
	// the more spread out frequency bins are, the bigger is the margin between contours
	//  diamond 0 - 33, 34 - 66 ellipse, 67 - 100 heart

	const int NUM_OF_BINS = magSpectrum.size();
	const int NUMBER_OF_GROUPS = 32; // there are 32 groups on the x axis of the histogram
	int numberOfBinsInGroup = NUM_OF_BINS / NUMBER_OF_GROUPS;
	deque<HistogramGroup> frequencyBinHistogram;
	double histMean = 0.0, histMedian = 0.0, histStandardDeviation = 0.0; // estimating the mean, median and standard deviation
	int groupCounter = 0;
	double groupAverage = 0.0;
	double summarizedPresence = 0.0;
	vector<double> peaks;
	double maxWeight = 45.0; // this value is open for tuning
	for (int bin = 0; bin < NUM_OF_BINS; bin++)
	{
		double weight = magSpectrum[bin];
		weight = audio::linearToDecibel(weight); // value between [0;100]
		if (weight >= maxWeight)
		{
			if (weight - maxWeight >= 10.0)
			{
				maxWeight = weight;
				peaks.clear();
			}
			peaks.push_back(bin);
		}
		groupAverage += weight;
		if ((bin + 1) % numberOfBinsInGroup == 0)
		{
			groupAverage = groupAverage / numberOfBinsInGroup;
			HistogramGroup group;
			group.presence = groupAverage;
			group.firstBin = groupCounter * numberOfBinsInGroup;
			group.lastBin = (groupCounter + 1) * numberOfBinsInGroup - 1;
			frequencyBinHistogram.push_back(group);
			groupAverage = 0.0;
			groupCounter++;
			histMean += group.presence * (group.firstBin + group.lastBin) / 2;
			summarizedPresence += group.presence;
		}
	}

	histMean = histMean / summarizedPresence;
	double medianClassValue = summarizedPresence / 2;
	int medianClass = medianClassValue / numberOfBinsInGroup; // this number equals the number of the group which has the median
	double presenceUpToMedianClass = 0.0;
	groupCounter = 0;
	for (HistogramGroup gr : frequencyBinHistogram)
	{
		if (groupCounter == medianClass)
		{
			histMedian = gr.firstBin + (gr.lastBin - gr.firstBin) * ((medianClassValue - presenceUpToMedianClass) / gr.presence);
		}
		histStandardDeviation += gr.presence * pow((gr.firstBin + gr.lastBin) / 2 - histMean, 2.0);
		presenceUpToMedianClass += gr.presence;
		groupCounter++;
	}
	histStandardDeviation /= summarizedPresence - 1;
	histStandardDeviation = sqrt(histStandardDeviation);

	double skew = 0.0; // alternative Pearson Mode Skewness.
	//A symmetrical distribution has a skew of zero. A positive result means that data is right skewed.
	//A negative results means that data is left skewed.
	if (histMean >= 1) // if mean doesn`t equal zero
		skew = 3 * (histMean - histMedian) / histStandardDeviation;
	int skewFactor = (skew - (-0.2)) * 100 / (1 - (-0.2)) - 0.2;
	// skew usually ranges from -0.2 to 1, Skew Fsctor must range from 0 to 100.
	if (skewFactor < 0)
		skewFactor = 0;
	if (skewFactor > 100)
		skewFactor = 100;

	// for ellipse A and B from 1.0 to 10.0, for diamond around 80 and 200, for heart around 80 to 100
	double geomFactorA, geomFactorB; // A is associated with x-es, B is for y-es
	//skewFactor = 10;
	double (*implicitFunction)(vec2, vec2, double, double);
	bool isReversed = false;
	double coefficientA = 0.0, coefficientB = 0.0, summarizedPeaksDistA = 0.0, summarizedPeaksDistB = 0.0;
	int counter = 0;
	if (peaks.size() >= 2)
	{
		for (auto peak : peaks)
		{
			if (counter < peaks.size() / 2)
			{
				// lower freq => bigger x-es, smaller y-es
				summarizedPeaksDistA += peak - histMedian;
			}
			else // if counter >=
			{
				// higher freq => smaller x-es, bigger y-es
				summarizedPeaksDistB += peak - histMedian;
			}
			counter++;
		}
		coefficientA = abs(summarizedPeaksDistA) / 1023 / 6;
		coefficientB = abs(summarizedPeaksDistB) / 1023 / 6;
	}
	if (skewFactor <= 33)
	{
		implicitFunction = &getVerticeValueDiamond;
		geomFactorA = 80.0 + 120.0 * coefficientA;
		geomFactorB = 120.0 + 80 * coefficientB;
	}
	else if (skewFactor <= 66)
	{
		implicitFunction = &getVerticeValueEllipse;
		geomFactorA = 1.0 + 5.0 * coefficientA;
		geomFactorB = 1.0 + 5.0 * coefficientB;
	}
	else
	{
		implicitFunction = &getVerticeValueHeart;
		isReversed = true;
		geomFactorA = 80.0 + 20.0 * coefficientA;
		geomFactorB = 80.0 + 20.0 * coefficientB;
	}

	const float* channel = buffer.getChannel(0);
	// transforming standard deviation in range [0;1023] to a number in range [1;1.5]
	double rangeFactor = 1.0;
	if (histStandardDeviation >= 0)
		rangeFactor = histStandardDeviation * (1.5 - 1) / 400 + 1; // let standard deviation be [0;400] approximatelly
	float contourMargin = 20.0f * rangeFactor; // 20 and 30 pixels are okay values, using 20.0 as the base
	int maxAmountOfContours = abs(bounds.getHeight() / 2 / 2 / contourMargin);
	float mean = 0.0f;
	for (size_t i = 0; i < buffer.getNumFrames(); i++) {
		mean += audio::linearToDecibel(channel[i]);
	}
	mean /= buffer.getNumFrames();
	double numOfCountoursCoeff = (mean - 20.0) / (40.0 - 20.0); // mean usually hangs out around 20 ~ 40, coeff must be [0;1]
	int numOfCountours = std::round(numOfCountoursCoeff * maxAmountOfContours);
	//string str = "Max: " + to_string(maxAmountOfContours) + " Mean: " + to_string(mean) + " NumCont: " + to_string(numOfCountours)
	//	+ " Skew: " + to_string(skew);
	//gl::drawString(str, vec2(20, 40), ColorA(1, 1, 1, 1), Font("Helvetica", 30.0f)); // useful for debugging

	int height = abs(bounds.getHeight());
	//height = min(height, CANVAS_HEIGHT); // useful for debugging
	int width = abs(bounds.getWidth());
	//width = min(width, CANVAS_WIDTH);

	MS::Grid* myGrid = new MS::Grid(width, height, bounds.x1, bounds.y2);
	//myGrid->draw();

	double scaleFactor = 1.0;
	float initialHeight;
	for (int j = 0; j < numOfCountours; j++)
	{
		float height = MS::marchingSquares(myGrid, implicitFunction, windowCenter, scaleFactor, geomFactorA, geomFactorB, isReversed);
		if (j == 0)
			initialHeight = height;
		scaleFactor = (height + 2 * contourMargin) / initialHeight;
	}
	delete myGrid;
}

double getVerticeValueEllipse(vec2 vertice, vec2 origin, double a, double b)
{
	// Ellipse: (y-y_0)^2 / a + (x-x_0)^2 / b = r^2 -- the most basic implicit function
	// r^2 / ((y-y_0)^2 / a + (x-x_0)^2 / b) = 1
	double r = 100.0;
	double result = pow(r, 2.0) / (pow(vertice.y - origin.y, 2) / b + pow(vertice.x - origin.x, 2) / a);
	return result;
}

double getVerticeValueHeart(vec2 vertice, vec2 origin, double a, double b)
{
	// Heart implicit equation:  (x/a)^2+[y/b-((x/a)^2)^(1/3)]^2 = 1, 
	/*const double a = 100.0;
	const double b = 100.0;*/
	double result = pow(abs(vertice.x - origin.x) / a, 2) + pow((vertice.y - origin.y) / b - pow(abs(vertice.x - origin.x) / a, 0.66), 2);
	return result;
}

double getVerticeValueDiamond(vec2 vertice, vec2 origin, double a, double b)
{
	double x = vertice.x - origin.x;
	double y = vertice.y - origin.y;
	//double a = 80.0; // size coefficients
	//double b = 120.0;
	double result = abs(x) / a + abs(y) / b;
	return result;
}

// ----------------------------------------------------------------------------------------------------
// MARK: - SpectrumPlot
// ----------------------------------------------------------------------------------------------------

SpectrumPlot::SpectrumPlot()
	: mScaleDecibels( true ), mBorderEnabled( true ), mBorderColor( 0.5f, 0.5f, 0.5f, 1 )
{
}

void SpectrumPlot::draw( const vector<float> &magSpectrum )
{
	if( magSpectrum.empty() )
		return;

	gl::ScopedGlslProg glslScope( getStockShader( gl::ShaderDef().color() ) );

	ColorA bottomColor( 0, 0, 0.7f, 1 );

	float width = mBounds.getWidth();
	float height = mBounds.getHeight();
	size_t numBins = magSpectrum.size();
	float padding = 0;
	float binWidth = ( width - padding * ( numBins - 1 ) ) / (float)numBins;

	gl::VertBatch batch( GL_TRIANGLE_STRIP );

	size_t currVertex = 0;
	float m;
	Rectf bin( mBounds.x1, mBounds.y1, mBounds.x1 + binWidth, mBounds.y2 );
	for( size_t i = 0; i < numBins; i++ ) {
		m = magSpectrum[i];
		if( mScaleDecibels )
			m = audio::linearToDecibel( m ) / 100;

		bin.y1 = bin.y2 - m * height;

		batch.color( bottomColor );
		batch.vertex( bin.getLowerLeft() );
		batch.color( 0, m, 0.7f );
		batch.vertex( bin.getUpperLeft() );

		bin += vec2( binWidth + padding, 0 );
		currVertex += 2;
	}

	batch.color( bottomColor );
	batch.vertex( bin.getLowerLeft() );
	batch.color( 0, m, 0.7f );
	batch.vertex( bin.getUpperLeft() );

	gl::color( 0, 0.9f, 0 );

	batch.draw();

	if( mBorderEnabled ) {
		gl::color( mBorderColor );
		gl::drawStrokedRect( mBounds );
	}
	gl::color(255.0f, 255.0f, 255.0f);
}
