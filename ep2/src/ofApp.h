#pragma once
#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxBox2d.h"
#include "ofxBox2dParticleSystem.h"

#define IN_PORT 12345
// This needs to coincide with SuperCollider settings.
#define OUT_PORT 57121
#define HOST "localhost"
#define GRAVITY 10.0
#define KEY_SEPARATION 2
#define MAX_PARTICLES 5000
#define PARTICLE_DELAY 0.25
#define TRIAD_DELAY 0.05
#define OCTAVE_DELAY 2.0
#define SCALE_SWITCH_PROBABILITY 0.3
#define RESET_THRESHOLD 2.0
#define DO_TRIADS true

struct PianoKey {
  ofxBox2dRect key;
  bool white;
  bool active = false;
  bool inTriad = false;
  float lastDraw = -1.0;
  glm::vec2 origPos;
  float targetY;
};

class ofApp : public ofBaseApp {

public:

  void setup();
  void update();
  void draw();
  void reset() { }

  void addNote(int pitch);
  void dropNote(int pitch);
  void sendNote(int pitch, bool onset);

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void resized(int w, int h);

  PianoKey pianoKeysTop[88];
  PianoKey pianoKeysBottom[88];
  PianoKey pianoKeysBottomBacking[88];
  ofxBox2dRect backboardTop;
  ofxBox2dRect backboardBottomLeft;
  ofxBox2dRect backboardBottomRight;
  ofxBox2d box2d;  // the box2d world
  ofxBox2dParticleSystem::ParticleSystem particleSystem;
  vector <shared_ptr<ofxBox2dRect>> rects;

private:
  ofxOscReceiver receiver;
  ofxOscSender sender;
  void setupKeyboard();
  void updateTriad();
  void allNotesOff();
  void newScale();
  void updateOnScale();
  float whiteWidth;
  float whiteHeight;
  int keyColours[88];
  int root = -1;
  int triad[3] = {-1, -1, -1};
  int scaleNumber = 59;
  float lastTriadSelect = -1;
  float lastOctaveSelect = -1;
  int pedalOnStart = -1.0;
  std::string bitString;
  bool onScale[88];
  ofTrueTypeFont font;
};
