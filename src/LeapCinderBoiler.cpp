/* ******************************************************************************
 *  This Sample App is developed using the tutorials as a natural progression
 *  in the learning process of using Cinder Libraries in implementing
 *  simple programs to create simple windows with shapes, images and colors
 * *******************************************************************************/

//cinder header files used
#import <Foundation/Foundation.h>
#include "cinder/app/AppBasic.h"
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Quaternion.h"
#include "Cinder-LeapMotion.h"

#include "cinder/params/Params.h"
#include "AntTweakBar.h"

//Windows header files used
//#define WINVER 0x0500
//#include <iostream>
//#include <time.h>

using namespace LeapMotion;
using namespace Leap;
using namespace ci;
using namespace ci::app;

class LeapCinderBoilerApp : public ci::app::AppNative
{
 public:
	void draw();
    void drawHand(Leap::Hand &hand, Vec3f position);
    void drawHands();
	void setup();
    void shutdown();
    void setupGui();
    void update();
	void prepareSettings(Settings *settings);
    void keyDown( KeyEvent event );

private:
    NSView *cinderView;
	Frame                   mFrame;
	LeapMotion::DeviceRef	mLeap;
	void 					onFrame( Leap::Frame frame );

	ci::CameraPersp			mCamera;
    
    bool showParams;
    
	params::InterfaceGl	mParams;

    Quatf mObjOrientation;
    float mScale;
    Vec3f mTranslate;
    bool mStaticPosHand, mStaticOrientHand;
};

void LeapCinderBoilerApp::prepareSettings( Settings *settings){
	//setting the window size and the frame rate for processing
	settings->setWindowSize ( 1600, 900);
	settings->setFrameRate (100.0f);
}

void LeapCinderBoilerApp::shutdown()
{
}

void LeapCinderBoilerApp::drawHand(Leap::Hand &hand, Vec3f position)
{
    gl::translate(position);
    gl::scale(mScale, mScale, mScale);
    
    Leap::Vector handXBasis =  hand.palmNormal().cross(hand.direction());
    Leap::Vector handYBasis = -hand.palmNormal();
    Leap::Vector handZBasis = -hand.direction();
    Leap::Matrix handTransform = Leap::Matrix(handXBasis, handYBasis, handZBasis);
    handTransform = handTransform.rigidInverse();
    
    if (!mStaticOrientHand)
    {
        gl::pushMatrices();

        gl::multModelView(LeapMotion::toMatrix44f(handTransform));
    }
    
    gl::drawColorCube(Vec3f(0,0,0), Vec3f( 40, 10, 40 ) );
    
    if (!mStaticOrientHand)
        gl::popMatrices();
    
    Leap::Matrix fingerPosTransform = Leap::Matrix(handXBasis, handYBasis, handZBasis, hand.palmPosition());
    fingerPosTransform = fingerPosTransform.rigidInverse();
    
    const Leap::PointableList& pointables = hand.pointables();
    for ( Leap::PointableList::const_iterator pointIter = pointables.begin(); pointIter != pointables.end(); ++pointIter ) {
        const Leap::Pointable& pointable = *pointIter;
        
        float length	= pointable.length();
        
        Vec3f basePos	= - LeapMotion::toVec3f(fingerPosTransform.transformDirection(pointable.direction())) * length;
        
        Leap::Vector fingerTipOrigin =  pointable.tipPosition();
        if (mStaticOrientHand)
            fingerTipOrigin = fingerPosTransform.transformPoint(fingerTipOrigin);
        else
            fingerTipOrigin -= hand.palmPosition();
        
        gl::pushMatrices();
        
        gl::translate(LeapMotion::toVec3f(fingerTipOrigin));
        
        if (!mStaticOrientHand)
        {
            Leap::Vector fingerXBasis = hand.palmNormal().cross(pointable.direction());
            Leap::Vector fingerYBasis = handYBasis;
            Leap::Vector fingerZBasis = -pointable.direction();
            Leap::Matrix fingerTransform = Leap::Matrix(fingerXBasis, fingerYBasis, fingerZBasis);
            fingerTransform = fingerTransform.rigidInverse();
            gl::multModelView(LeapMotion::toMatrix44f(fingerTransform));
        }
        gl::drawColorCube(Vec3f(0,0,0), Vec3f( 10, 10, 10 ) );
        gl::color( ColorAf::gray( 0.8f ) );
        gl::drawLine( basePos, Vec3f(0,0,0) );
        gl::popMatrices();
    }
    
}

void LeapCinderBoilerApp::drawHands()
{
    gl::setMatrices( mCamera );
	
	gl::translate(mTranslate);
    gl::rotate(mObjOrientation);
    
	// Iterate through hands
	const Leap::HandList& hands = mFrame.hands();
	for ( Leap::HandList::const_iterator handIter = hands.begin(); handIter != hands.end(); ++handIter ) {
        
		// Pointables
        Leap::Hand hand = *handIter;
        gl::pushMatrices();
        
        if (mStaticPosHand)
        {
            if (hands.count() > 1)
            {
                if (hand.id() == hands.leftmost().id())
                    gl::translate(Vec3f(-100,0,0)*mScale);
                else
                    gl::translate(Vec3f(100,0,0)*mScale);
            }
        }
        else
            gl::translate(LeapMotion::toVec3f(hand.palmPosition()*mScale));
        
        drawHand(hand, Vec3f(0,0,0));
        
        gl::popMatrices();
	}
}

void LeapCinderBoilerApp::draw()
{
	// Clear window
	gl::setViewport( getWindowBounds() );
	gl::clear( ColorA(0, 0, 0, 0.0) );


    glPushAttrib(GL_ALL_ATTRIB_BITS);
    gl::pushMatrices();

    drawHands();
    
    gl::popMatrices();
    glPopAttrib();

	gl::color(ColorA(0, 0, 0,1));
    if (!showParams)
        return;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    mParams.draw();
    glPopAttrib();
}

void LeapCinderBoilerApp::update()
{
}

void LeapCinderBoilerApp::onFrame( Leap::Frame frame )
{
	mFrame = frame;

    if (!frame.hands().isEmpty()) {
		// Get the first hand
		const Hand hand = frame.hands()[0];
		const Hand hand2 = frame.hands()[1];
        
		// Get the hand's normal vector and direction
		const Vector normal = hand.palmNormal();
		const Vector direction = hand.direction();
        const Vector position = hand.palmPosition();
        
		const Vector normal2 = hand2.palmNormal();
		const Vector direction2 = hand2.direction();
        const Vector position2 = hand2.palmPosition();
    }
}

void LeapCinderBoilerApp::setupGui()
{
    mParams = params::InterfaceGl("Parameters", Vec2i(200,400));
    mParams.addParam ("Scene Rotation", &mObjOrientation);
    mParams.addParam ("Hand Translation", &mTranslate);
    mParams.addParam ("Hand Scale", &mScale);
    mParams.addParam ("Static Hand Orientation", &mStaticOrientHand);
    mParams.addParam ("Static Hand Position", &mStaticPosHand);

    TwDefine(" Parameters color='0 128 255' alpha=255 ");
    TwDefine(" Parameters/'Scene Rotation' opened=true ");
    TwDefine(" Parameters/'Hand Translation' opened=true ");
    
    
}


void LeapCinderBoilerApp::setup()
{
    mStaticOrientHand = NO;
    mStaticPosHand = NO;
    
    mScale = 1;
    mTranslate.set(0,0,0);
    
    showParams = TRUE;
    
    setupGui();

	// Set up OpenGL
	gl::enableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	
	// Set up camera
	mCamera = CameraPersp( getWindowWidth(), getWindowHeight(), 60.0f, 0.01f, 5000.0f );
	mCamera.lookAt( Vec3f( 0.0f, 125.0f, 500.0f ), Vec3f( 0.0f, 250.0f, 0.0f ) );
	
	// Start device
	mLeap = LeapMotion::Device::create();
	mLeap->connectEventHandler( &LeapCinderBoilerApp::onFrame, this );
}

void LeapCinderBoilerApp::keyDown( KeyEvent event )
{
    if(event.getChar() == 'g')
    {
        showParams = !showParams;
    }
}



CINDER_APP_NATIVE( LeapCinderBoilerApp, RendererGl )
