#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "../../common/constants.h"

#define IN_PORT 12345

#define MAX_RADIUS 50.0
#define MAX_FREQUENCY 3.0
#define TRAVEL_SPEED 10
#define MAX_LINE_LENGTH 100
#define MAX_TAIL_LENGTH 30
#define GRID_SIZE 4
#define CUTOFF 21
#define PHRASE_PAUSE 0.3
#define VERBOSE false
#define POLYGON_POINT_RADIUS 5
#define POLYGON_RADIUS 30
#define SWITCH_DIRECTION_PROB 0.05
#define RANDOM_SPEED 5
#define THETA_SPEED 0.3
#define CHORD_DELAY 1.0

class Circle {
  public:
    Circle(int d, float x, float y, float r, float a, float f, ofColor c) :
      direction(d),
      x(x),
      y(y),
      radius(r),
      angle(a),
      frequency(f),
      color(c) { }

    void update();
    void update(glm::vec2 newPos);
    void updateRadius(float mult) { radius *= mult; }
    void draw();
    void addChild();
    void addChild(int d, float r, float a, float f);
    void clear();
    void breakAway(float tx, float ty) {
      targetX = tx;
      targetY = ty;
      brokeAway = true;
      if (child != NULL) child->setBreakAway(true);
    }
    void setBreakAway(bool b) {
      brokeAway = b;
      if (child != NULL) child->setBreakAway(b);
    }
    bool isBrokeAway() { return brokeAway; }

  private:
    int direction;
    float x;
    float y;
    float radius;
    float angle;
    float frequency;
    ofColor color;
    float targetX = -1.0;
    float targetY = -1.0;
    bool brokeAway = false;
    Circle* child = NULL;
    glm::vec2 getChildPos();
    std::vector<glm::vec2> linePath;
};

class JellyTail {
  public:
    JellyTail(float x, float y);

    void update(float nx, float ny);
    void draw();

    float x;
    float y;

  private:
    std::vector<glm::vec2> tail;
    ofShader shader;
};

class Jellyfish {
  public:
    Jellyfish(float x, float y, float r) :
      x(x),
      y(y),
      radius(r),
      theta(0.0),
      targetTheta(0.0) { }

    void update();
    void draw();
    void addPoint();
    void reset() {
      x = ofGetWidth() / 2;
      y = ofGetHeight() / 2;
      theta = 0.0;
      tails.clear();
    }
    void cyclePos();

  private:
    float x;
    float y;
    float radius;
    float theta;
    float targetTheta;
    std::vector<JellyTail> tails;
    void rePlacePoints();
    int cyclingPos = -1;
};


class ofApp : public ofBaseApp {

  public:
    vector <int> activeNoteIds;

    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void sendPlayNote(int note, int velocity);
    void sendStopNote(int note, int velocity);
    void sendCCMessage(int channel, int control, int value);
    void reset();
    void addCircle();
    void breakAway();

  private:
    std::vector<Circle> circles;
    std::vector<Jellyfish> jellyfish;
    float gridX;
    float gridY;
    float gridXInc;
    float gridYInc;
    int scribbleRow = 1;
    int scribbleCol = 1;
    int colorPos = 0;
    std::vector<ofColor> colors;
    bool pedalOn = false;
    float notesOn[88];
    float lastNoteOff = 0.0;
    ofShader shader;
    ofxOscReceiver receiver;
    int mode = 0;
    bool flash = false;
    float lastChordCycle = 0.0;
    float redWeight = 0.0;
    int redDirection = 1;
};
