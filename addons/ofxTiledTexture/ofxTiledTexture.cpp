#include "ofxTiledTexture.h"

//----------------------------------------------------------
ofxTiledTexture::ofxTiledTexture() {
    alphaMode = PLAIN;
    sizeMode = PLAIN;
    
    sizeOffset = 0;
    growing = true;
    seq = NULL;
    newSeq = false;
}

//----------------------------------------------------------
void ofxTiledTexture::allocate(int w, int h, int internalGlDataType) {
    allocate(w, h, internalGlDataType, true);
}
    
//----------------------------------------------------------
void ofxTiledTexture::allocate(int w, int h, int internalGlDataType, bool smooth) {
    
	// 	can pass in anything (320x240) (10x5)
	// 	here we make it power of 2 for opengl (512x256), (16x8)
    
	if (GLEE_ARB_texture_rectangle){
		tex_w = w;
		tex_h = h;
	} else {
		tex_w = ofNextPow2(w);
		tex_h = ofNextPow2(h);
	}
    
	if (GLEE_ARB_texture_rectangle){
		tex_t = w;
		tex_u = h;
	} else {
		tex_t = 1.0f;
		tex_u = 1.0f;
	}
    
	// attempt to free the previous bound texture, if we can:
	clear();
    
	glGenTextures(1, (GLuint *)textureName);   // could be more then one, but for now, just one
    
	glEnable(textureTarget);
    
    glBindTexture(textureTarget, (GLuint)textureName[0]);
    glTexImage2D(textureTarget, 0, internalGlDataType, tex_w, tex_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);  // init to black...
    glTexParameterf(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (smooth) {
        glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  
        glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    } else {
        glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
        glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
	glDisable(textureTarget);
    
	width = w;
	height = h;
	bFlipTexture = false;
}

//----------------------------------------------------------
void ofxTiledTexture::draw(float x, float y, float w, float h, float p, float q) {
    
	glEnable(textureTarget);
    
	// bind the texture
	glBindTexture( textureTarget, (GLuint)textureName[0] );
    
    GLint px0 = 0;		// up to you to get the aspect ratio right
    GLint py0 = 0;
    GLint px1 = (GLint)w;
    GLint py1 = (GLint)h;
    
    if (bFlipTexture == true){
        GLint temp = py0;
        py0 = py1;
        py1 = temp;
    }
    
    // for rect mode center, let's do this:
    if (ofGetRectMode() == OF_RECTMODE_CENTER){
        px0 = (GLint)-w/2;
        py0 = (GLint)-h/2;
        px1 = (GLint)+w/2;
        py1 = (GLint)+h/2;
    }
    
    // -------------------------------------------------
    // complete hack to remove border artifacts.
    // slightly, slightly alters an image, scaling...
    // to remove the border.
    // we need a better solution for this, but
    // to constantly add a 2 pixel border on all uploaded images
    // is insane..
    
    GLfloat offsetw = 0;
    GLfloat offseth = 0;
    
    if (textureTarget == GL_TEXTURE_2D){
        offsetw = 1.0f/(tex_w);
        offseth = 1.0f/(tex_h);
    }
    // -------------------------------------------------
    
    GLfloat tx0 = 0+offsetw;
    GLfloat ty0 = 0+offseth;
    GLfloat tx1 = tex_t - offsetw;
    GLfloat ty1 = tex_u - offseth;
    
    glPushMatrix();
    glTranslated(x, y, 0);
    
    if (sizeMode == SEQUENCE && newSeq) {
        // delete the old sequence, if any
        if (seq != NULL) {
            delete [] seq;
        }
        
        // create a new sequence
        numSeq = ofRandom(1, p*q);
        seq = new int[numSeq];
        for (int i=0; i < numSeq; i++) {
            seq[i] = ofRandom(0, p*q-1);
        }
        
        newSeq = false;
    }
    
    if (sizeMode == SMOOTH || sizeMode == SEQUENCE) {
        if (growing) {
            sizeOffset += 1;
        } else {
            sizeOffset -= 1;
        }
        
        if (sizeOffset > MAX_OFFSET) {
            sizeOffset = MAX_OFFSET;
            growing = false;
        } else if (sizeOffset < 0) {
            sizeOffset = 0;
            growing = true;
            newSeq = true;
        } 
    }
    
    float s = (px1-px0)/p;
    float t = (py1-py0)/q;
    for (int j = 0; j < q; j++) {
        for (int i = 0; i < p; i++) {
            switch (alphaMode) {
                case PLAIN:
                    break;
                    
                case RAINBOW:
                    ofSetColor(ofRandom(0, 255), ofRandom(0, 255), ofRandom(0, 255));
                    break;
                    
                case WAVE:
                    float a1 = fabsf(sinf(i/10.0f)*255);
                    ofSetColor(255, 255, 255, a1);
                    break;
                    
                case GRAD_WAVE:
                    float d = sqrt(p);
                    float a2 = fabsf(sinf(i)*i/d)*255/d;
                    ofSetColor(255, 255, 255, a2);
                    break;
                    
                case GRAD:
                    ofSetColor(255, 255, 255, i*255/p);
                    break;
            }
                    
            if (sizeMode == CRAZY && (ofGetFrameNum()*i*j)%2 == 0) {
                // flickering grid
                sizeOffset = ofRandom(0, ofGetWidth()/p);   
				glBegin(GL_QUADS);
                glTexCoord2f(tx0,ty0);          glVertex3i(px0+i*s     -sizeOffset,  py0+j*t     -sizeOffset,  0);
                glTexCoord2f(tx1,ty0);          glVertex3i(px0+(i+1)*s +sizeOffset,  py0+j*t     -sizeOffset,  0);
                glTexCoord2f(tx1,ty1);          glVertex3i(px0+(i+1)*s +sizeOffset,  py0+(j+1)*t +sizeOffset,  0);
                glTexCoord2f(tx0,ty1);          glVertex3i(px0+i*s     -sizeOffset,  py0+(j+1)*t +sizeOffset,  0);
				glEnd();
                
            } else if ((sizeMode == SMOOTH && (i+j)%2 == 0) || (sizeMode == SEQUENCE && isInSeq(j*p+i))) {
                glBegin(GL_QUADS);
                glTexCoord2f(tx0,ty0);          glVertex3i(px0+i*s     -sizeOffset,  py0+j*t     -sizeOffset,  0);
                glTexCoord2f(tx1,ty0);          glVertex3i(px0+(i+1)*s +sizeOffset,  py0+j*t     -sizeOffset,  0);
                glTexCoord2f(tx1,ty1);          glVertex3i(px0+(i+1)*s +sizeOffset,  py0+(j+1)*t +sizeOffset,  0);
                glTexCoord2f(tx0,ty1);          glVertex3i(px0+i*s     -sizeOffset,  py0+(j+1)*t +sizeOffset,  0);
				glEnd();
                
            } else {
                // normal grid
                glBegin(GL_QUADS);
                glTexCoord2f(tx0,ty0);          glVertex3i(px0+i*s,      py0+j*t,      0);
                glTexCoord2f(tx1,ty0);			glVertex3i(px0+(i+1)*s,  py0+j*t,      0);
                glTexCoord2f(tx1,ty1);			glVertex3i(px0+(i+1)*s,  py0+(j+1)*t,  0);
                glTexCoord2f(tx0,ty1);			glVertex3i(px0+i*s,      py0+(j+1)*t,  0);
                glEnd();
            }            
        }
    }
    
    glPopMatrix();
    
	glDisable(textureTarget);
}

//----------------------------------------------------------
bool ofxTiledTexture::isInSeq(int v) {
    for (int i=0; i < numSeq; i++) {
        if (seq[i] == v)
            return true;
    }
    return false;
}

//----------------------------------------------------------
void ofxTiledTexture::setAlphaMode(int m) {
    alphaMode = m;
}

//----------------------------------------------------------
void ofxTiledTexture::setSizeMode(int m) {
    sizeMode = m;
    if (sizeMode == SEQUENCE) {
        newSeq = true;
    }
}
