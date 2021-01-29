#pragma once

#include <vector>
#include "../../common/constants.h"
#include "ofMain.h"
#include "ofxOsc.h"

#define IN_PORT 12345
// This needs to coincide with SuperCollider settings.
#define OUT_PORT 57121
#define HOST "localhost"

#define LINE_SPEED 2

#define MESH_GRANULARITY 0.5
#define OUTER_SPHERE_RADIUS 200
#define SPHERE_OFFSET 50
#define MAX_SPHERE_RADIUS 20.0
#define EPSILON 0.001  // To avoid 0.0 in modf
#define SPHERE_MULTIPLIER 0.3
#define SPHERE_ANGLE_THETA 0.01

#define VERBOSE false

class Note {
  public:
    Note() {};
    Note(float t, bool o, int p, int v) : time(t), onset(o), pitch(p),
         velocity(v) {};
    ~Note() {};

    float time;
    float duration;
    bool onset;
    int pitch;
    int velocity;
};

class PianoKey {
  public:
    float x;
    float targetX;
    float y;
    float targetY;
    float w;
    float h;
    bool white;
    bool active;
    Note note;

    void copyNote(const Note n) {
      note.time = n.time;
      note.duration = n.duration;
      note.onset = n.onset;
      note.pitch = n.pitch;
      note.velocity = n.velocity;
    }

    void draw() {
      ofDrawRectangle(x, y, w, h);
    }

    void update() {
      w += LINE_SPEED;
    }
};


class PlayedMesh : public PianoKey {
  public:
    PlayedMesh(PianoKey p);
    bool update(bool playbackRequested);
    void draw();

    ofMesh mesh;
    float targetX;
    float targetY;
    float originalH;
    int hexColor;
    int direction;
    float alpha;
    float xIntercept;
    float yIntercept;
    float yShift;
    float theta;
    float phi;
    float sphereX;
    float sphereY;
    float sphereZ;
    float deltaTheta;
    float deltaPhi;
    bool growing = false;
    bool drawSphere = false;
    bool replaying = false;
    ofSpherePrimitive sphere;
    float finalSphereRadius;
};


class Echo {
  public:
    bool playbackRequested = false;
    bool reverseRequested = false;
    bool negativeRequested = false;
    bool playingBack = false;
    float pivot = 63.5;
    Echo() {
      velocityMultiplier = 0.8;
    }
    Echo(float v) {
      velocityMultiplier = v;
    }

    void draw();
    void update();
    void addNote(PianoKey p);
    std::vector<Note> getNotes(float time);

    void startPlayback(float t, bool reverse, bool negative);

    void reset() {
      notes.clear();
      playedMeshes.clear();
      playbackRequested = false;
      playingBack = false;
      firstNote = -1;
      timeDelta = 0.0;
    }

    void setVelocityMultiplier(float v) {
      velocityMultiplier = v;
    };

  private:
    float velocityMultiplier;
    float firstNote = -1;
    float timeDelta = 0.0;
    std::vector<Note> notes;
    std::vector<PlayedMesh> playedMeshes; 
};

class ofApp : public ofBaseApp {

  public:
    std::vector<Note> noteQueue;
    Echo echo;
    ofLight pointLight1;
    ofLight pointLight2;
    ofLight pointLight3;

    void setup();
    void setupKeyboards();
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
    
private:
    PianoKey backboard;
    PianoKey backboard2;
    PianoKey pianoKeys[88];
    PianoKey pianoKeys2[88];
    PianoKey playingKeys[88];
    ofxOscReceiver receiver;
    ofxOscSender sender;
    float timeDelta = 0.0;
};
