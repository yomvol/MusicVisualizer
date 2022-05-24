#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"
#include "glm/common.hpp"

#include "../common/AudioDrawUtils.h"
#include "SliderControl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MusicVisualizerApp : public App {
public:
	void setup() override;
	void mouseDown(MouseEvent event) override;
	void update() override;
	void draw() override;
	void mouseWheel(MouseEvent event) override;
	void mouseDrag(MouseEvent event) override;

private:
	audio::ContextRef ctx;
	audio::GainNodeRef mVolumeControl;
	audio::BufferPlayerNodeRef mBufferPlayer;
	audio::MonitorNodeRef mPCMMonitor; // Pulse-Code Modulation monitor supplies samples in time domain
	audio::MonitorSpectralNodeRef mSpectrMonitor; // Monitor for analysing frequencies using Fast Fourier Transform
	vector<float> mMagSpectrum; // Spectrum of magnitudes. Frequencies after FFT are converted to complex polar coordinates.
	// Phases are usually neglegted for visualising purposes
	bool isPaused = false;
	double mPassedSec = 0.0;
	Rectf mBoundaryRect;
	unique_ptr<RGBMask> mMask;
	unique_ptr<SliderControl> mSlider;
};

void MusicVisualizerApp::setup()
{
	audio::SourceFileRef sourceFile;
	DataSourceRef file;
#define FILE_BROWSING 0
#if FILE_BROWSING
	vector<string> supportedExtensions;
	supportedExtensions.push_back("wav");
	supportedExtensions.push_back("mp3");
	supportedExtensions.push_back("mp4");
	supportedExtensions.push_back("m4a");
	supportedExtensions.push_back("flac");
	fs::path p = getOpenFilePath(getAppPath(), supportedExtensions);
	if (!p.empty())
		file = loadFile(p);
	else
	{
		getWindow()->close();
		terminate();
	}
#else
	file = loadAsset("watching-the-waves.wav");
#endif
	try {
		sourceFile = audio::load(file);
	}
	catch (audio::AudioFileExc)
	{
		console() << "Error occured opening the file" << endl;
	}

	ctx = audio::ContextRef(audio::Context::master());
	mBufferPlayer = ctx->makeNode(new audio::BufferPlayerNode()); // Design based on assumption that we want to preload a file
	// and access it from RAM quickly without file I/O
	mBufferPlayer->loadBuffer(sourceFile);
	mBufferPlayer->setLoopEnabled(true);
	console() << "Context samplerate is: " << ctx->getSampleRate() << endl; // works only with debugging tools
	console() << "File samplerate is: " << sourceFile->getSampleRateNative() << endl;
	mVolumeControl = ctx->makeNode(new audio::GainNode);
	mVolumeControl->setValue(1.0f);

	int rectLength = 150, rightXOffset = 50, rectHeight = 8, bottomYOffset = 30;
	mSlider = make_unique<SliderControl>(1.0f, Rectf(getWindowWidth() - rightXOffset - rectLength,
		getWindowHeight() - rectHeight - bottomYOffset, getWindowWidth() - rightXOffset,
		getWindowHeight() - bottomYOffset));

	auto amplMonitorFormat = audio::MonitorNode::Format().windowSize(1024);
	mPCMMonitor = ctx->makeNode(new audio::MonitorNode(amplMonitorFormat));
	// By providing an FFT size double that of the window size, we 'zero-pad' the analysis data, which gives
	// an increase in resolution of the resulting spectrum data.
	auto freqMonitorFormat = audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	// If sample rate of a sound is 44100, then by Nyquist`s theorem it should possess frequnces up to 22050.
	// Frequency of the 1024`th bin equals 1024 * sample rate / FFT size = 22050.
	mSpectrMonitor = ctx->makeNode(new audio::MonitorSpectralNode(freqMonitorFormat));
	mMask = make_unique<RGBMask>(mSpectrMonitor->getNumBins());

	mBufferPlayer >> mVolumeControl >> ctx->getOutput();
	mVolumeControl >> mPCMMonitor;
	mVolumeControl >> mSpectrMonitor;
	mBufferPlayer->start();
	ctx->enable();
	
	auto bounds = Display::getMainDisplay()->getBounds();
	getWindow()->setSize(bounds.getWidth() - 100, bounds.getHeight() - 100);
	getWindow()->setPos(bounds.getX1() + 30, bounds.getY1() + 40);
}

void MusicVisualizerApp::mouseDown(MouseEvent event)
{
	vec2 click = event.getPos();
	if (mSlider->getArea().contains(click))
	{
		float volume = 2.0f * (click.x - mSlider->getArea().x1) / (mSlider->getArea().x2 - mSlider->getArea().x1);
		mSlider->setLevel(volume);
		mVolumeControl->setValue(volume);
	}
	else
	{
		if (isPaused == true)
		{
			mBufferPlayer->start();
			mBufferPlayer->seekToTime(mPassedSec);
			isPaused = false;
		}
		else
		{
			double devident = ctx->getNumProcessedSeconds();
			double divisor = mBufferPlayer->getLoopEndTime();
			mPassedSec = glm::mod(devident, divisor);
			mBufferPlayer->stop();
			isPaused = true;
		}
	}
}

void MusicVisualizerApp::mouseDrag(MouseEvent event)
{
	vec2 click = event.getPos();
	if (mSlider->getArea().contains(click))
	{
		float volume = 2.0f * (click.x - mSlider->getArea().x1) / (mSlider->getArea().x2 - mSlider->getArea().x1);
		mSlider->setLevel(volume);
		mVolumeControl->setValue(volume);
	}
}

void MusicVisualizerApp::mouseWheel(MouseEvent event)
{
	float incr = event.getWheelIncrement();
	console() << incr << endl;
	float volume = mVolumeControl->getValue() + 0.1f * incr;
	if (volume > 2.0f)
		volume = 2.0f;
	else if (volume < 0.1f)
		volume = 0.1f;
	mSlider->setLevel(volume);
	mVolumeControl->setValue(volume);
}

void MusicVisualizerApp::update()
{
	if (mSpectrMonitor && mSpectrMonitor->getNumConnectedInputs())
		mMagSpectrum = mSpectrMonitor->getMagSpectrum();
	int rectLength = 150, rightXOffset = 50, rectHeight = 8, bottomYOffset = 30;
	mSlider->setArea(Rectf(getWindowWidth() - rightXOffset - rectLength,
		getWindowHeight() - rectHeight - bottomYOffset, getWindowWidth() - rightXOffset,
		getWindowHeight() - bottomYOffset));
}

void MusicVisualizerApp::draw()
{
	gl::clear(Color(0, 0, 0));
	
	const audio::Buffer& buffer = mPCMMonitor->getBuffer();
	console() << mPCMMonitor->getWindowSize() << endl << mPCMMonitor->getNumConnectedInputs() << endl;
	if (mPCMMonitor && mPCMMonitor->getNumConnectedInputs())
	{
		float boundaryPxX = 16.0f * 1;
		float boundaryPxY = 9.0f * 1;
		mBoundaryRect = Rectf(0.0f + boundaryPxX, getWindowHeight() - boundaryPxY - 100.0f, getWindowWidth() - boundaryPxX,
			0.0f + boundaryPxY);
		//drawColorfulFlash(buffer, mMagSpectrum, mBoundaryRect, *mMask);
		drawConcentricShapes(buffer, mMagSpectrum, mBoundaryRect, getWindowCenter());
	}

	vec2 hintPos(20.0f, getWindowHeight() - 50);
	gl::drawString("Press LMB to pause track, use mouse wheel to control sound volume", hintPos, ColorA(1, 1, 1, 1),
		Font("Helvetica", 30.0f));
	mSlider->drawSlider();
}

CINDER_APP(MusicVisualizerApp, RendererGl(RendererGl::Options().msaa(4)), [](App::Settings* settings) {
	settings->setMultiTouchEnabled(false);
	})