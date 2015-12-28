#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "ofAddons.h"

#include "ofxTiledTexture.h"

class testApp : public ofSimpleApp {

	public:
		void setup();
		void update();
		void draw();

		void mouseMoved(int x, int y);
		
    private:
        int             mouseX;
        int             mouseY;
        ofxTiledTexture tex;
        ofVideoGrabber  video;
		
};

#endif
