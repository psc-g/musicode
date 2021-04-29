#include "ofApp.h"
#include <iostream>
#include <string>
#include <math.h>


// Colors for pitches.
int PITCH_COLORS[] = {
    0x210000, 0x211000, 0x212100, 0x102100, 0x002100, 0x002110,
    0x002121, 0x001021, 0x000021, 0x100021, 0x210021, 0x210010};


glm::vec4 randomMove(float x, float y, float theta, float targetTheta, float speed) {
  if (((float) rand() / RAND_MAX) < SWITCH_DIRECTION_PROB)
    targetTheta = ((float) rand() / RAND_MAX) * 2 * PI;
  theta += (targetTheta - theta) * THETA_SPEED;
  float new_x = x + speed * cos(theta);
  if (new_x < 0.0) {
    x = -1 * new_x;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
    targetTheta = theta;
  } else if (new_x > ofGetWidth()) {
    x = 2 * ofGetWidth() - new_x;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
    targetTheta = theta;
  } else {
    x = new_x;
  }
  float new_y = y + speed * sin(theta);
  if (new_y < 0.0) {
    y = -1 * new_y;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
    targetTheta = theta;
  } else if (new_y > ofGetHeight()) {
    y = 2 * ofGetHeight() - new_y;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
    targetTheta = theta;
  } else {
    y = new_y;
  }
  return glm::vec4(x, y, theta, targetTheta);
}

//--------------------------------------------------------------
void Circle::update() {
  angle += direction * 2 * PI * frequency / 60.0;  // TODO: set with actual freq.
  if (brokeAway && targetX >= 0.0 && targetY >= 0.0) {
    float dist = std::min(
        1.0, TRAVEL_SPEED / sqrt(pow(targetX - x, 2) + pow(targetY - y, 2)));
    x += (targetX - x) * dist;
    y += (targetY - y) * dist;
    /*
    radius *= 0.5;
    if (child != NULL) child->updateRadius(0.1);
    */
  }
  if (child != NULL) {
    child->update(getChildPos());
  } else {
    linePath.emplace_back(getChildPos());
    if (brokeAway && linePath.size() > MAX_LINE_LENGTH) {
      linePath.erase(linePath.begin());
    }
  }
}

glm::vec2 Circle::getChildPos() {
  return glm::vec2(x + cos(angle) * radius, y + sin(angle) * radius);
}

void Circle::update(glm::vec2 newPos) {
  x = newPos.x;
  y = newPos.y;
  update();
}

void Circle::draw() {
  ofNoFill();
  if (!brokeAway) {
    ofSetColor(ofColor::white);
    if (brokeAway) ofSetLineWidth(1);
    else ofSetLineWidth(2);
    ofCircle(x, y, radius);
    ofPolyline line;
    line.addVertex(x, y);
    auto childPos = getChildPos();
    line.addVertex(childPos.x, childPos.y);
    line.draw();
  }
  if (child != NULL) child->draw();
  else {
    ofSetColor(color);
    if (brokeAway) ofSetLineWidth(1);
    else ofSetLineWidth(4);
    ofPolyline drawing;
    for (auto p : linePath) {
      drawing.addVertex(p.x, p.y);
    }
    drawing.draw();
  }
  ofFill();
}

void Circle::addChild() {
  float r = ((float) rand() / RAND_MAX) * MAX_RADIUS;
  float a = ((float) rand() / RAND_MAX) * 2 * PI;
  float f = ((float) rand() / RAND_MAX) * MAX_FREQUENCY;
  int dir = rand() % 2 == 0 ? 1 : -1;
  addChild(dir, r, a, f);
}

void Circle::addChild(int d, float r, float a, float f) {
  if (child != NULL) {
    child->addChild(d, r, a, f);
  } else {
    glm::vec2 childPos = getChildPos();
    child = new Circle(d, childPos.x, childPos.y, r, a, f, color);
  }
}

void Circle::clear() {
  if (child != NULL) child->clear();
  linePath.clear();
  child = NULL;
}


//--------------------------------------------------------------
void Jellyfish::update() {
  if (tails.size() == 0) return;
  auto newPos = randomMove(x, y, theta, targetTheta, RANDOM_SPEED);
  x = newPos.x;
  y = newPos.y;
  theta = newPos.z;
  targetTheta = newPos.w;
  rePlacePoints();
}

void Jellyfish::draw() {
  ofCircle(x, y, POLYGON_POINT_RADIUS);
  ofPolyline outer;
  for (int i = 0; i <= cyclingPos; ++i) {
    auto& tail = tails[i];
    ofPolyline line;
    line.addVertex(x, y);
    line.addVertex(tail.x, tail.y);
    outer.addVertex(tail.x, tail.y);
    ofSetColor(ofColor::white);
    line.draw();
    tail.draw();
  }
  if (tails.size() > 0) outer.addVertex(tails[0].x, tails[0].y);
  ofSetColor(ofColor::white);
  outer.draw();
}

void Jellyfish::rePlacePoints() {
  float angleDelta = PI * 2 / tails.size();
  float angle = theta;
  for (uint i = 0; i < tails.size(); ++i) {
    tails[i].update(x + cos(angle) * radius, y + sin(angle) * radius);
    angle += angleDelta;
  }
}

void Jellyfish::addPoint() {
  tails.emplace_back(x, y);
  rePlacePoints();
  cyclingPos++;
}

void Jellyfish::cyclePos() {
  cyclingPos = (cyclingPos + 1) % tails.size();
}


JellyTail::JellyTail(float x, float y) : x(x), y(y) {
  shader.load("shaders/tail");
}

void JellyTail::update(float nx, float ny) {
  x = nx;
  y = ny;
  tail.emplace_back(glm::vec2(x, y));
  if (tail.size() > MAX_TAIL_LENGTH) {
    tail.erase(tail.begin());
  }
}

void JellyTail::draw() {
  ofPolyline line;
  line.addVertex(x, y);
  for (const auto& t : tail) line.addVertex(t.x, t.y);
  ofSetColor(ofColor::white);
  glLineWidth(4);
  shader.begin();
  shader.setUniform1f("time", ofGetElapsedTimef());
  shader.setUniform2f("tailStart", glm::vec2(x, y));
  line.draw();
  shader.end();
}


//--------------------------------------------------------------
void ofApp::setup(){
  receiver.setup(IN_PORT);
  ofSetVerticalSync(true);
  ofBackground(ofColor::black);
  ofSetLogLevel(OF_LOG_NOTICE);
  ofDisableAntiAliasing();

  gridX = ofGetWidth() / (GRID_SIZE + 1);
  gridXInc = gridX;
  gridY = ofGetHeight() / (GRID_SIZE + 1);
  gridYInc = gridY;

  colors.emplace_back(ofColor::magenta);
  colors.emplace_back(ofColor::aquamarine);
  colors.emplace_back(ofColor::chartreuse);
  colors.emplace_back(ofColor::cyan);
  colors.emplace_back(ofColor::lawnGreen);
  colors.emplace_back(ofColor::goldenRod);
  colors.emplace_back(ofColor::lemonChiffon);
  colors.emplace_back(ofColor::lightBlue);
  colors.emplace_back(ofColor::gold);
  colors.emplace_back(ofColor::lime);

  for (uint i = 0; i < 88; ++i) notesOn[i] = 0.0;

  glBlendEquation(GL_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  ofEnableAlphaBlending();

  jellyfish.emplace_back(ofGetWidth() / 2, ofGetHeight() / 2, POLYGON_RADIUS);

  shader.load("shaders/bg");
}

void ofApp::sendPlayNote(int note, int velocity) {
  //
}

void ofApp::sendStopNote(int note, int velocity) {
  //
}

void ofApp::sendCCMessage(int channel, int control, int value) {
  //
}

//--------------------------------------------------------------
void ofApp::update() {
  // check for OSC messages
  while(receiver.hasWaitingMessages()) {
      ofxOscMessage m;
      receiver.getNextMessage(m);
      int note = m.getArgAsInt(0);
      int velocity = m.getArgAsInt(1);
      if (m.getAddress() == "/jellyleg") {
        jellyfish[jellyfish.size() - 1].addPoint();
      } else if (m.getAddress() == "/newjelly") {
        jellyfish.emplace_back(ofGetWidth() / 2, ofGetHeight() / 2, POLYGON_RADIUS);
      } else if (m.getAddress() == "/jellycycle1") {
        if (jellyfish.size() > 0) {
          jellyfish[0].cyclePos();
        }
      } else if (m.getAddress() == "/jellycycle2") {
        if (jellyfish.size() > 1) {
          jellyfish[1].cyclePos();
        }
      } else if (m.getAddress() == "/newscribble") {
        breakAway();
      } else if (m.getAddress() == "/scribble") {
        lastNoteOff = ofGetElapsedTimef();
        float r = MAX_RADIUS * (1 - note / 127.0);
        float a = 2 * PI * velocity / 128.0;
        float f = MAX_FREQUENCY * note / 108.0;
        int dir = rand() % 2 == 0 ? 1 : -1;
        if (circles.size() == 0 || circles[circles.size() - 1].isBrokeAway()) {
          circles.emplace_back(
              Circle(dir, ofGetWidth() / 2, ofGetHeight() / 2, r, a, f, colors[colorPos]));
          colorPos = (colorPos + 1) % colors.size();
        }
        else circles[circles.size() - 1].addChild(dir, r, a, f);
      } else if (m.getAddress() == "/chordcycle") {
        if (ofGetElapsedTimef() - lastChordCycle > CHORD_DELAY) {
          mode = (mode + 1) % 4;
          mode = mode == 0 ? 1 : mode;
          lastChordCycle = ofGetElapsedTimef();
        }
      } else if (m.getAddress() == "/redShift") {
        redWeight += redDirection * 0.01;
        if (redWeight > 1.0) {
          redWeight = 1.0;
          redDirection *= -1;
        } else if (redWeight < 0.0) {
          redWeight = 0.0;
          redDirection *= -1;
        }
      } else if (m.getAddress() == "/reset") {
        reset();
      }
  }
  for (uint i = 0; i < circles.size(); ++i) circles[i].update();
  for (uint i = 0; i < jellyfish.size(); ++i) jellyfish[i].update();
}

//--------------------------------------------------------------
void ofApp::draw() {
  if (flash) {
    ofSetColor(ofColor::white);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    flash = false;
    return;
  }
  ofSetColor(ofColor::black);
  shader.begin();
  shader.setUniform1f("time", ofGetElapsedTimef());
  shader.setUniform1i("mode", mode);
  shader.setUniform1f("redWeight", redWeight);
  ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
  shader.end();
  for (uint i = 0; i < circles.size(); ++i) circles[i].draw();
  for (uint i = 0; i < jellyfish.size(); ++i) jellyfish[i].draw();
}

void ofApp::reset() {
  for (uint i = 0; i < circles.size(); ++i) circles[i].clear();
  circles.clear();
  jellyfish.clear();
  jellyfish.emplace_back(ofGetWidth() / 2, ofGetHeight() / 2, POLYGON_RADIUS);
  mode = 0;
  scribbleRow = 1;
  scribbleCol = 1;
  redWeight = 0.0;
  redDirection = 1;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
}

void ofApp::addCircle() {
  if (circles.size() == 0 || circles[circles.size() - 1].isBrokeAway()) {
    circles.emplace_back(
        Circle(1, ofGetWidth() / 2, ofGetHeight() / 2, MAX_RADIUS / 2,
          0.0, 1, colors[colorPos]));
    colorPos = (colorPos + 1) % colors.size();
  }
  else circles[circles.size() - 1].addChild();
}

void ofApp::breakAway() {
  if (circles.size() == 0) return;
  if (circles.size() == GRID_SIZE * GRID_SIZE) {
    circles.erase(circles.begin());
  }
  circles[circles.size() - 1].breakAway(scribbleCol * gridX, scribbleRow * gridY);
  scribbleCol += 1;
  if (scribbleCol > GRID_SIZE) {
    scribbleCol = 1;
    scribbleRow += 1;
  }
  if (scribbleRow > GRID_SIZE) scribbleRow = 1;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
  switch(key) {
    case 'f':
      flash = true;
      break;
    case 'n':
      addCircle();
      break;
    case 'r':
      reset();
      break;
    case 'b':
      breakAway();
      break;
    case 'a':
      jellyfish[jellyfish.size() - 1].addPoint();
      break;
    case 'c':
      jellyfish[jellyfish.size() - 1].cyclePos();
      break;
    case 'j':
      jellyfish.emplace_back(ofGetWidth() / 2, ofGetHeight() / 2, POLYGON_RADIUS);
      break;
    case '0':
      mode = 0;
      break;
    case '1':
      mode = 1;
      break;
    case '2':
      mode = 2;
      break;
    case '3':
      mode = 3;
      break;
  }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
