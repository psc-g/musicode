#pragma once

#include "ofMain.h"
#include "ofxOsc.h"

#define HOST "localhost"
#define IN_PORT 12345
// This needs to coincide with SuperCollider settings.
#define OUT_PORT 57120
#define KEY_SEPARATION 2
#define KEYS_OUTER_RADIUS 450
#define KEYS_INNER_RADIUS 300

class ofApp : public ofBaseApp {

  public:

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

    void renderText();
    void setupKeyboard();
    void checkOSCMessages();
    void sendOSCMessage(ofxOscMessage m);
    void sendNote(int pitch, bool onset);
    void addBasslineNote(int key);
    void reset();

    string text;
    ofTrueTypeFont font;
    ofFbo fbo;
    ofBoxPrimitive box;
    ofEasyCam cam;
    bool rotate = false;

    ofMesh keys, border, innerCircle, outerCircle;
    ofMesh keysLoop, borderLoop;
    std::vector<ofMesh> keysGroove;
    std::vector<ofMesh> borderGroove;
    int basslineCounter = 0;
    int basslinePlayerCounter = 0;
    std::vector<int> endGrooveCounter;
    int endGrooveLoops = 0;
    float startTheta = 0.0;
    bool activeKeys[88];
    ofxOscReceiver receiver;
    ofxOscSender sender;

    ofShader shader;
    int mode = 0;
    std::map<int, int> noteMappings;
    static int drumbeatNotes[];
    int charCount = 0;
    bool listeningToChars = true;
    int bassNotePointers[88];
    bool activeBassNotes[36];  // Acttually more than we need.
};
