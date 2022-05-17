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

//#include "cinder/CinderMath.h"
//#include "cinder/Triangulate.h"
#include "cinder/gl/gl.h"
//#include "cinder/gl/Batch.h"
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
		const int numOfBins = magSpectrum.size();
		float red = 0.0f, green = 0.0f, blue = 0.0f;
		for (int bin = 0; bin < numOfBins; bin++)
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

void drawConcentricShapes(const audio::Buffer& buffer, const vector<float>& magSpectrum, const Rectf& bounds)
{
	const float* channel = buffer.getChannel(0);
	

	const vec2 center = bounds.getCenter();
	const float radOffset = 20.0f;
	// loudness influences amount of "circles", frequency spetrcum influence shapes 0 - 1000
	// 0 - 20 triangle, 21 - 40 square, 41 - 60 pentagon, 61 - 80 hexagon, 81 - 100 circle
	
	double (*implicitFunction)(vec2, vec2) = &getVerticeValueHeart;
	vec2 circleOrigin(500, 500);
	MS::marchingSquares(bounds, implicitFunction, circleOrigin);
}

double getVerticeValueCircle(vec2 vertice, vec2 origin)
{
	// Circle: (y-y_0)^2 + (x-x_0)^2 = r^2 -- the most basic implicit function
	// r^2 / ((y-y_0)^2 + (x-x_0)^2) = 1
	double r = 100.0;
	double result = pow(r, 2.0) / (pow(vertice.y - origin.y, 2) + pow(vertice.x - origin.x, 2));
	return result;
}

double getVerticeValueHeart(vec2 vertice, vec2 origin)
{
	// Heart implicit equation:  (x/a)^2+[y/b-((x/a)^2)^(1/3)]^2 = 1, 
	const double a = 100.0;
	const double b = 100.0;
	double result = pow(abs(vertice.x - origin.x) / a, 2) + pow((vertice.y - origin.y) / b - pow(abs(vertice.x - origin.x) / a, 0.66), 2);
	return result;
}

double getVerticeValueDiamond(vec2 vertice, vec2 origin)
{
	double x = vertice.x - origin.x;
	double y = vertice.y - origin.y;
	double a = 100.0; // size coefficients
	double b = 100.0;
	double result = abs(x) / a + abs(y) / b;
	return result;
}

// ----------------------------------------------------------------------------------------------------
// MARK: - WaveformPlot
// ----------------------------------------------------------------------------------------------------



namespace {

inline void calcMinMaxForSection( const float *buffer, size_t samplesPerSection, float &max, float &min ) {
	max = 0;
	min = 0;
	for( size_t k = 0; k < samplesPerSection; k++ ) {
		float s = buffer[k];
		max = math<float>::max( max, s );
		min = math<float>::min( min, s );
	}
}

inline void calcAverageForSection( const float *buffer, size_t samplesPerSection, float &upper, float &lower ) {
	upper = 0;
	lower = 0;
	for( size_t k = 0; k < samplesPerSection; k++ ) {
		float s = buffer[k];
		if( s > 0 ) {
			upper += s;
		} else {
			lower += s;
		}
	}
	upper /= samplesPerSection;
	lower /= samplesPerSection;
}

} // anonymouse namespace

//void Waveform::load( const float *samples, size_t numSamples, const ci::ivec2 &waveSize, size_t pixelsPerVertex, CalcMode mode )
//{
//    float height = waveSize.y / 2.0f;
//    size_t numSections = waveSize.x / pixelsPerVertex + 1;
//    size_t samplesPerSection = numSamples / numSections;
//
//	vector<vec2> &points = mOutline.getPoints();
//	points.resize( numSections * 2 );
//
//    for( size_t i = 0; i < numSections; i++ ) {
//		float x = (float)i * pixelsPerVertex;
//		float yUpper, yLower;
//		if( mode == CalcMode::MIN_MAX ) {
//			calcMinMaxForSection( &samples[i * samplesPerSection], samplesPerSection, yUpper, yLower );
//		} else {
//			calcAverageForSection( &samples[i * samplesPerSection], samplesPerSection, yUpper, yLower );
//		}
//		points[i] = vec2( x, height - height * yUpper );
//		points[numSections * 2 - i - 1] = vec2( x, height - height * yLower );
//    }
//	mOutline.setClosed();
//
//	mMesh = gl::VboMesh::create( Triangulator( mOutline ).calcMesh() );
//}


//void WaveformPlot::load( const std::vector<float> &samples, const ci::Rectf &bounds, size_t pixelsPerVertex )
//{
//	mBounds = bounds;
//	mWaveforms.clear();
//
//	ivec2 waveSize = bounds.getSize();
//	mWaveforms.push_back( Waveform( samples, waveSize, pixelsPerVertex, Waveform::CalcMode::MIN_MAX ) );
//	mWaveforms.push_back( Waveform( samples, waveSize, pixelsPerVertex, Waveform::CalcMode::AVERAGE ) );
//}
//
//void WaveformPlot::load( const audio::BufferRef &buffer, const ci::Rectf &bounds, size_t pixelsPerVertex )
//{
//	mBounds = bounds;
//	mWaveforms.clear();
//
//	size_t numChannels = buffer->getNumChannels();
//	ivec2 waveSize = bounds.getSize();
//	waveSize.y /= numChannels;
//	for( size_t ch = 0; ch < numChannels; ch++ ) {
//		mWaveforms.push_back( Waveform( buffer->getChannel( ch ), buffer->getNumFrames(), waveSize, pixelsPerVertex, Waveform::CalcMode::MIN_MAX ) );
//		mWaveforms.push_back( Waveform( buffer->getChannel( ch ), buffer->getNumFrames(), waveSize, pixelsPerVertex, Waveform::CalcMode::AVERAGE ) );
//	}
//}
//
//void WaveformPlot::draw()
//{
//	auto &waveforms = getWaveforms();
//	if( waveforms.empty() ) {
//		return;
//	}
//
//	gl::ScopedGlslProg glslScope( getStockShader( gl::ShaderDef().color() ) );
//
//	gl::color( mColorMinMax );
//	gl::draw( waveforms[0].getMesh() );
//
//	gl::color( mColorAverage );
//	gl::draw( waveforms[1].getMesh() );
//
//	if( waveforms.size() > 2 ) {
//		gl::pushMatrices();
//		gl::translate( 0, getBounds().getHeight() / 2 );
//
//		gl::color( mColorMinMax );
//		gl::draw( waveforms[2].getMesh() );
//
//		gl::color( mColorAverage );
//		gl::draw( waveforms[3].getMesh() );
//		
//		gl::popMatrices();
//	}
//}

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
}
