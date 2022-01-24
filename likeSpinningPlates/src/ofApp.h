#pragma once

#include "ofxGui.h"
#include "ofMain.h"
#include "ofxOsc.h"
#include "../../common/constants.h"

#define HOST "localhost"
#define IN_PORT 12345
#define OUT_PORT 57120
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define GRID_SIZE 8

class FlameStrip {
  public:
    FlameStrip(float x, int s);
    ~FlameStrip() {}
    void draw();
    void update();
    void setNumPaths(int np);
    void updateXPos(int x) { xPos = x; };

    float lifespan = 1.0;
    float xPos;
    int seed;
    int numPaths = 100;
    int numPoints = 100;
    int red = 245;
    int green = 167;
    int blue = 66;
    float alphaMultiplier = 1.0;
    float yLength = 1000;
    float xWidth = 20;
    float timeMultiplier = 0.1;
    float coarseXWidth = 10.0;
    float coarseXFreq = 0.1;
    float coarseYDivider = 10.0;
    std::vector<ofPath> paths;
};

class WavyCircle {
  public:
    WavyCircle(float r, int c);
    ~WavyCircle() {}
    void reset();
    void update();
    void setActive(bool a);
    void draw();
    void setZDelta(float z) { zDelta = z; };
    void setUseColors(bool b) { useColors = b; }

  private:
    float radius;
    int color;
    bool active = false;
    bool useColors = true;
    int numPaths = 10;
    float startActive = 0.0;
    float timeActive = 0.0;
    float zMultiplier = 0.0;
    float zDelta = 0.01;
    float zThreshold = 0.1;
    std::vector<ofPath> paths;
};

class ofApp : public ofBaseApp {
  public:
    void setup();
    void update();
    void draw();
    void reset();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void checkOSCMessages();

    ofxPanel gui;
    ofParameter<bool> uiIncludeCircles;
    ofParameter<bool> uiIncludeFlames;
    ofParameter<int> uiNumPaths;
    ofParameter<int> uiNumPoints;
    ofParameter<float> uiYLength;
    ofParameter<float> uiXWidth;
    ofParameter<float> uiTimeMultiplier;
    ofParameter<int> uiRed;
    ofParameter<int> uiGreen;
    ofParameter<int> uiBlue;
    ofParameter<float> uiAlphaMultiplier;
    ofParameter<float> uiCoarseXWidth;
    ofParameter<float> uiCoarseXFreq;
    ofParameter<float> uiCoarseYDivider;
    ofParameter<float> uiStartXPos;
    ofParameter<float> uiOnsetThreshold;
    ofParameter<float> uiTextureScale;
    ofParameter<ofVec4f> uiRotation;
    ofParameter<bool> uiEndRotation;
    bool pedalOn = false;
    bool drawGui = false;

    ofEasyCam cam;
    float noteAngles[88];
    bool notesActive[88];
    float firstOnset = 0.0;
    float circlesAngle = 90.0;

    std::vector<WavyCircle> circles;
    std::vector<FlameStrip> flameStrips;

    std::vector<std::vector<ofPlanePrimitive>> planes;
    std::vector<std::vector<glm::vec3>> origPlanePositions;
    std::vector<std::vector<glm::quat>> planeOrientations;
    std::vector<std::vector<glm::quat>> planeOrientationDeltas;
    std::vector<std::vector<std::vector<unsigned char>>> splitPixels;
    std::vector<std::vector<ofTexture>> splitTextures;
    ofVideoGrabber vidGrabber;
    ofPixels vidPixels;
    int pixelCellWidth;
    int pixelCellHeight;
    bool rotate = false;
    float targetBlowup = 1000.0;
    float currBlowup = 0.0;

    float startT = -0.5 * PI;
    std::vector<std::vector<glm::vec3>> infinityPath;

    ofxOscReceiver receiver;
    ofxOscSender sender;
    int mode = 0;
    bool readyForMode3 = false;
};
