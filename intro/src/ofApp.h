#pragma once

#include "ofxBox2d.h"
#include "ofMain.h"
#include "ofxMidi.h"
#include "../../common/constants.h"

// Physics constants
#define DRIFT 0.1
#define GRAVITY 10.0
#define ANCHOR_RADIUS 0
#define BALL_RADIUS 8
#define BALL_DENSITY 0.3
#define MAX_BALL_TRAIL 100
#define JOINT_LENGTH 100
#define START_TANGENT_DRAW 10
#define TANGENT_LENGTH 200
#define TOGETHER_THRESHOLD 0.1


// Image paths
#define FULL_LOGO_FILE "images/fullLogo.png"
#define M_FILE "images/M.png"
#define U_FILE "images/U.png"
#define S_FILE "images/S.png"
#define I_FILE "images/I.png"
#define C_FILE "images/C.png"
#define O_FILE "images/O.png"
#define D_FILE "images/D.png"
#define E_FILE "images/E.png"

// Animation constants
#define SPIN_DELTA 5.0
#define ROTATE_STEPS 100

#define VERBOSE false

struct BezierPoints {
  float x0;
  float y0;
  float x1;
  float y1;
  float x2;
  float y2;
  float x3;
  float y3;
};

struct Note {
  int pitch;
  bool active;
  int prevNoteIndex;
  int nextNoteIndex;
  int positionInChain;
  float onsetTime;
  float offsetTime;
  float velocity;
  bool onset;
  shared_ptr<ofxBox2dCircle> circle;
  shared_ptr<ofxBox2dJoint> joint;
  ofPolyline polyline;
  vector <glm::vec3> points;
  glm::vec3 center;
  vector<std::pair<glm::vec3, glm::vec3>> lines;
  vector<BezierPoints> beziers;
  ofPath path;
};

class PianoKey {
  public:
    float x;
    float y;
    float w;
    float h;
    bool white;
    bool active;
    bool sustained;
    Note note;
    char letter;
    float time;

    void draw() {
      ofDrawRectangle(x, y, w, h);
    }
};


class ofApp : public ofBaseApp, ofxMidiListener {

  public:
    std::vector<ofImage> letterImages;
    std::vector<ofPixels> letterPixels;
    std::vector<ofPlanePrimitive> letterPlanes;
    std::vector<int> letterColours;
    ofxBox2d box2d;  // the box2d world
    ofxBox2dCircle anchor;  // fixed anchor
    vector <Note> notes;
    vector <int> activeNoteIds;
    int lastActiveNoteIndex;

    void setup();
    void setupKeyboard();
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
    void newMidiMessage(ofxMidiMessage& eventArgs);
    void reset();
    void addPendulumNote(int pitch);
    void dropPendulumNote(int pitch);
    void addNewLetter(int pitch);

private:
    std::vector<glm::vec2> rotationDirections;
    std::vector<float> cumulativeDeltas;
    std::vector<float> targetDelta;
    std::vector<bool> drawLetter;
    std::vector<float> offsets;
    ofxMidiIn nanoKontrolIn;
    ofxMidiIn disklavierIn;
    ofxMidiOut disklavierOut;
    PianoKey backboard;
    PianoKey pianoKeys[88];
    PianoKey playingKeys[88];
    int sustainValue;
    float xDir = 1.0;
    float yDir = 0.0;
    float zDir = 0.0;
    std::string dirText = "x";
    float spin = 0.0;
    std::string spinText = "x-spin: 0.0";
    bool finalRotation = false;
    float finalRotateAngle = 0.0;
    int mode;
};
