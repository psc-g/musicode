#pragma once
#include "ofMain.h"
#include "ofxMidi.h"
#include "../../common/constants.h"

#define MOON_RADIUS 200.0
#define STARTING_RADIUS 50.0
#define THETA_INC 0.01
#define SQUARE_WIDTH 50.0
#define MAX_DEPTH 10
#define SWITCH_DIRECTION_PROB 0.05
#define LINE_PROB 0.1
#define RANDOM_SPEED 2
#define TEMPO 2
#define REBEL_ID "11"
#define BREAKAWAY_PROB 0.35
#define BEAT_TEMPO 0.3
#define CIRCLE_DISPLACEMENT 0.5
#define NUM_STARS 12
#define WANDERING_TIME 2.0
#define STAR_PLAY_PROB 0.01
#define STAR_PLAY_DURATION 2.0


class Moon {
  public:
    Moon(float x, float y, float r);
    bool update();
    void draw(glm::vec2 posi, bool start); 
    void activate(int pos);
    void deactivate(int pos);
    void startShiftingBeat(int pos);
    bool isBeatAtEdge();
    void grow();
    bool fullSize() { return currentRadius >= radius; }
    glm::vec2 shiftingBeatPos();
    void addNewBeat(glm::vec2 pos);
    int beatShiftDir = 1;
    void reduceAlpha(float m) { alpha -= m; if (alpha < 0.0) alpha = 0.0; }

  private:
    float x;
    float y;
    float radius;
    float currentRadius = 0.0;
    std::vector<glm::vec2> beats;
    std::vector<bool> onBeats;
    std::vector<bool> activeBeats;
    std::vector<float> beatTimeElapsed;
    float lastBeat = 0.0;
    ofShader shader;
    int shiftingBeat = -1;
    float alpha = 1.0;
};

class Star {
  public:
    Star(float x, float y, float r, int n);
    int update();
    void draw();
    glm::vec2 getPos() { return glm::vec2(x, y); }
    void reduceAlpha(float m) { alpha -= m; if (alpha < 0.0) alpha = 0.0; }

  private:
    float x;
    float y;
    float radius;
    int note;
    //bool shining = true;
    float lastPlayed;
    ofShader shader;
    float alpha = 1.0;
};


class Circle {
  public:
    static bool calm;
    static bool wandering;
    static float rebelX;
    static float rebelY;
    static float rebelTargetX;
    static float rebelTargetY;
    static float rebelTheta;
    static float rebelRadius;
    static float timeWandering;
    static float rebelChasingX;
    static float rebelChasingY;
    static float alpha;
    static bool born;
    static bool hasRebel;
    static bool reHide;
    static bool becomingBeat;
    static void drawRebel();
    static void updateRebel(glm::vec2 starPos);
    static int rebelIntersectsMoon(float radius);
    static void reduceAlpha(float m) { alpha -= m; if (alpha < 0.0) alpha = 0.0; }

    Circle(float x, float y, float r, int d, std::string id) :
      x(x),
      y(y),
      r(r),
      direction(d),
      id(id),
      n(1),
      theta(0.0),
      radiusSum(r) {}

    void setPos(float new_x, float new_y);
    void activate() { active = true; }
    void deactivate() { active = false; }
    bool isActive() { return active; }

    void update();
    void draw() const;
    void split();
    void toggleCalm();
    void setCalm(bool c);
    bool prune();
    ofPoint getPoint() const { return ofPoint(x, y); }
    glm::vec2 getPos() const { return glm::vec2(x, y); }
    bool intersect(glm::vec2 pt);
    void signalBreakAway();


  private:
    float x;
    float y;
    float r;
    int direction;
    std::string id;
    bool isRebel;
    int n;
    float theta;
    float radiusSum;
    int numSplits = 0;
    float lastStep = 0.0;
    bool splitting = true;
    int targetSplits = 1;
    std::vector<Circle> children;
    bool active = true;
};


enum Mode { start, growMoon, circleOrbit, circleSplit1, circleSplit2,
            breakAway, callingBack, callBack, circlePrune1, circlePrune2,
            circleRehide, makeNewBeat, newBeat, circleReBorn, circleReSplit1,
            circleReSplit2, preFade, fadeAway };


class ofApp : public ofBaseApp, ofxMidiListener {

public:

  void setup();
  void update();
  void draw();
  void reset() { }

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void resized(int w, int h);

  void newMidiMessage(ofxMidiMessage& eventArgs);
  void sendPlayNote(int note, int velocity);
  void sendStopNote(int note, int velocity);
  void sendCCMessage(int channel, int control, int value);

private:
  ofxMidiIn disklavierIn;
  ofxMidiOut disklavierOut;
  Moon* moon;
  std::vector<Star> stars;
  Circle* c;
  float orbitRadius;
  float orbitAngle;
  glm::vec2 center;
  bool step = false;
  float lastMoonPlay = 0.0;
  Mode storyMode = start;
  bool pedalOn = false;
  int numReturns = 0;
  void cycleModes();
  float halfWidth;
  float halfHeight;
};
