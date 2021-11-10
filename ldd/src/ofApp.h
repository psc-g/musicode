#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxCvHaarFinder.h"

#define HOST "localhost"
#define IN_PORT 12345
// This needs to coincide with SuperCollider settings.
#define OUT_PORT 57120

#define THRESHOLD 0.001
#define MIN_TIME_BW_CHORDS 2.0
#define LOSS_MULTIPLIER 0.05

#define BATCH_SIZE 64

#define KNOT_T_INC 0.1
#define KNOT_LIM 100.0

struct PianoKey {
  ofRectangle key;
  bool white;
  bool active;
};

class LissajousKnot {
  public:
    LissajousKnot();
    ~LissajousKnot() {};

    void setValues(int nx, int ny, int nz, float px, float py, float pz);
    void draw();
    void update();
    float gradientStep();
    void redrawLearnedPath();
    void computeGradients(bool t) { doGradients = t; };
    bool isReady() { return ready; };
    void reset() {
      ready = false;
      loss = 100.0;
      doGradients = false;
    };
    std::vector<float> getGradients() { return gradients; };
    float getLoss() { return loss; };

  private:
    bool ready = false;
    glm::vec3 n;
    glm::vec3 p;
    std::vector<float> ts;
    glm::vec3 learnedN;
    glm::vec3 learnedP;
    ofPath truePath;
    ofPath learnedPath;
    float learning_rate = 0.025;
    bool doGradients = false;
    float loss = 100.0;
    std::vector<float> gradients;
};

class LearnedLine {
  public:
    LearnedLine(int nx, float lr) : num_x(nx), learning_rate(lr) {};
    ~LearnedLine() {};

    void addCoeff(float c) {
      coeffs.push_back(c);
    };

    void setup();
    void place(float xscale, float xoff, float yscale, float yoff);
    void rotate(float r);
    void draw();
    float gradientStep();
    void setLearning(bool b) {
      learning = b;
      if (!learning) notes.clear();
    };
    bool isLearning() { return learning; };
    void addNote(int note) { notes.push_back(note); };
    void deleteNote(int pos) { notes.erase(notes.begin() + pos); };
    std::vector<int> getNotes() { return notes; };
    std::vector<float> getGradients() { return gradients; };

  private:
    int num_x;
    float learning_rate;
    std::vector<int> notes;
    float loss = 100.0;
    int alpha = 255;
    float rotation = 0.0;
    bool learning = true;
    glm::vec2 xLims;
    glm::vec2 yLims;
    float drawMidX;
    float drawMidY;
    std::vector<float> xs;
    std::vector<float> ys;
    std::vector<float> learnedYs;
    std::vector<float> drawXs;
    std::vector<float> drawYs;
    std::vector<float> drawLearnedYs;
    std::vector<float> coeffs;
    std::vector<float> theta;
    std::vector<float> gradients;
    float xOffset, yOffset, xScale, yScale;
};

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

    void checkOSCMessages();
    ofxOscReceiver receiver;
    ofxOscSender sender;

    void addLearnedLine();
    void reset();
    std::vector<LearnedLine> learned_lines;
    int mode = 0;
    float time_of_last_chord = -MIN_TIME_BW_CHORDS;
    std::vector<int> active_bass_notes;
    void toggleMode();

    int lissajous_pos = 1;
    LissajousKnot lissajous;

    ofEasyCam cam;
    bool orbit = false;
    float orbitRad = 0.0;

    ofVideoGrabber webcam;
    ofxCvColorImage image;
    ofxCvColorImage component_images[3];
    ofxCvColorImage red_filter;
    ofxCvColorImage green_filter;
    ofxCvColorImage blue_filter;

    std::vector<glm::vec3> circlePositions;

    bool started = true;
    bool listening_to_pedal = true;

    ofShader shader;
};
