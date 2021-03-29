#pragma once
#include "ofxBox2d.h"

#define DRIFT 0.1

#define GRAVITY 10.0
#define ANCHOR_RADIUS 0
#define BALL_RADIUS 8
#define BALL_DENSITY 0.3
#define MAX_BALL_TRAIL 100

#define JOINT_LENGTH 100

#define KEY_SEPARATION 2

#define START_TANGENT_DRAW 10
#define TANGENT_LENGTH 200

#define TOGETHER_THRESHOLD 0.1

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
  shared_ptr<ofxBox2dCircle> circle;
  shared_ptr<ofxBox2dJoint> joint;
  ofPolyline polyline;
  vector <glm::vec3> points;
  glm::vec3 center;
  vector<std::pair<glm::vec3, glm::vec3>> lines;
  vector<BezierPoints> beziers;
  ofPath path;
};

struct PianoKey {
  ofxBox2dRect key;
  bool white;
  bool active;
};

class PianoPendulum {
public:
  PianoPendulum();
  ~PianoPendulum();

  void init();
  void clear();
  void reset();
  void update();
  void draw();
  void addNote(int pitch);
  void dropNote(int pitch);
  void cycleModes();
  void setMode(int newMode);

  ofxBox2d box2d;  // the box2d world
  ofxBox2dCircle anchor;  // fixed anchor
  ofxBox2dRect ofFloor;
  ofxBox2dRect ofCeiling;
  ofxBox2dRect leftWall;
  ofxBox2dRect rightWall;
  vector <Note> notes;
  vector <int> activeNoteIds;
  int lastActiveNoteIndex;
  int colors[12];  // Colors for pitches.
  bool drawBalls;

  // Variables for keyboard.
  PianoKey pianoKeys[88];
  ofxBox2dRect backboard;

  int mode;

private:
  void setupKeyboard();
  void printActiveNotes();
};
