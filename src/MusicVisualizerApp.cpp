#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class MusicVisualizerApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void MusicVisualizerApp::setup()
{
}

void MusicVisualizerApp::mouseDown( MouseEvent event )
{
}

void MusicVisualizerApp::update()
{
}

void MusicVisualizerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( MusicVisualizerApp, RendererGl )
