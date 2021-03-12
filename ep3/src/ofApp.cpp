#include "ofApp.h"
#include <algorithm>
#include <math.h>

#include "../../common/pitchColours.h"

glm::vec3 randomMove(float x, float y, float theta, float speed) {
  if (((float) rand() / RAND_MAX) < SWITCH_DIRECTION_PROB)
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
  float new_x = x + speed * cos(theta);
  if (new_x < 0.0) {
    x = -1 * new_x;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
  } else if (new_x > ofGetWidth()) {
    x = 2 * ofGetWidth() - new_x;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
  } else {
    x = new_x;
  }
  float new_y = y + speed * sin(theta);
  if (new_y < 0.0) {
    y = -1 * new_y;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
  } else if (new_y > ofGetHeight()) {
    y = 2 * ofGetHeight() - new_y;
    theta = ((float) rand() / RAND_MAX) * 2 * PI;
  } else {
    y = new_y;
  }
  return glm::vec3(x, y, theta);
}

//--------------------------------------------------------------
Moon::Moon(float x, float y, float r) : x(x), y(y), radius(r) {
  for (int i = 0; i < 3; ++i) {
    auto angle = PI / 2 + i * 2 * PI / 3;
    beats.emplace_back(glm::vec2(x + r * cos(angle) / 2,
                                 y + r * sin(angle) / 2));
    onBeats.push_back(false);
    activeBeats.push_back(true);  // All beats active initially.
    beatTimeElapsed.push_back(0.0);
  }
  onBeats[0] = true;
  shader.load("shaders/moon");
}

void Moon::addNewBeat(glm::vec2 pos) {
  beats.emplace_back(pos);
  onBeats.push_back(false);
  activeBeats.push_back(true);
  beatTimeElapsed.push_back(0.0);
}

void Moon::draw(glm::vec2 pos, bool start) {
  if (currentRadius == 0.0) return;
  shader.begin();
  shader.setUniform1f("time", ofGetElapsedTimef());
  shader.setUniform1f("alpha", alpha);
  if (start) shader.setUniform1i("start", 1);
  else shader.setUniform1i("start", 0);
  if (currentRadius < radius) shader.setUniform2f("circlePos", glm::vec2(-1.0, -1.0));
  else shader.setUniform2f("circlePos", pos);
  for (uint i = 0; i < beats.size(); ++i) {
    if (onBeats[i]) {
      if (i == 0) shader.setUniform1i("isGreen", 0);
      else shader.setUniform1i("isGreen", 1);
      if (activeBeats[i]) {
        shader.setUniform2f("beatPos", beats[i]);
        shader.setUniform1f("circleRadiusMultiplier", beatTimeElapsed[i]);
      } else {
        shader.setUniform2f("beatPos", beats[i]);
        shader.setUniform1f("circleRadiusMultiplier", 0.0);
      }
      break;
    }
  }
  ofSetCircleResolution(100);
  ofDrawCircle(x, y, currentRadius);
  shader.end();
}

bool Moon::update() {
  if (currentRadius == 0.0) return false;
  else if (currentRadius < radius) {
    currentRadius = std::min(radius, (float) (currentRadius + 1.0));
    return false;
  }
  float currTime = ofGetElapsedTimef();
  float timeDelta = currTime - lastBeat;
  bool play = false;
  if (shiftingBeat >= 0 && activeBeats[shiftingBeat]) {
    beats[shiftingBeat].y += beatShiftDir * CIRCLE_DISPLACEMENT;
    auto targetY = ofGetHeight() / 2 + MOON_RADIUS * sin(PI / 2) / 2;
    if (beatShiftDir < 0 && beats[shiftingBeat].y < targetY) {
      beats[shiftingBeat].y = targetY;
      shiftingBeat = -1;
      beatShiftDir = 1;
    }
  }
  if (timeDelta > BEAT_TEMPO) {
    lastBeat = currTime;
    int pos = 0;
    for (pos = 0; pos < onBeats.size(); ++pos) {
      if (onBeats[pos]) {
        beatTimeElapsed[pos] = 0.0;
        onBeats[pos++] = false;
        break;
      }
    }
    onBeats[pos % onBeats.size()] = true;
    play = pos % onBeats.size(); // != shiftingBeat;
  } else {
    for (uint i = 0; i < onBeats.size(); ++i) {
      if (onBeats[i]) beatTimeElapsed[i] = timeDelta / BEAT_TEMPO;
    }
  }
  return play;
}

void Moon::activate(int pos) {
  if (pos < 0 || pos >= activeBeats.size()) return;
  activeBeats[pos] = true;
}

void Moon::deactivate(int pos) {
  if (pos < 0 || pos >= activeBeats.size()) return;
  activeBeats[pos] = false;
}

void Moon::startShiftingBeat(int pos) {
  if (pos < 0 || pos >= activeBeats.size()) return;
  shiftingBeat = pos;
}

bool Moon::isBeatAtEdge() {
  if (shiftingBeat < 0) return false;
  return beats[shiftingBeat].y > y + radius;
}

glm::vec2 Moon::shiftingBeatPos() {
  if (shiftingBeat < 0) return glm::vec2(0.0, 0.0);
  return beats[shiftingBeat];
}

void Moon::grow() {
  currentRadius = 0.1;
}

//--------------------------------------------------------------
Star::Star(float x, float y, float r, int n) : x(x), y(y), radius(r), note(n) {
  lastPlayed = 0.0;
  shader.load("shaders/star");
}

int Star::update() {
  float currTime = ofGetElapsedTimef();
  if (currTime - lastPlayed > STAR_PLAY_DURATION * 4) {
    if ((float) rand() / RAND_MAX < STAR_PLAY_PROB) {
      lastPlayed = currTime;
      return note;
    }
  } else if (currTime - lastPlayed > STAR_PLAY_DURATION) {
    return -note;
  }
  return 0;
}

void Star::draw() {
  shader.begin();
  shader.setUniform1f("time", ofGetElapsedTimef());
  shader.setUniform1f("alpha", alpha);
  auto starPos = getPos();
  starPos.y = ofGetHeight() - starPos.y;
  shader.setUniform2f("starPos", starPos); //glm::vec2(starPosX, starPosY));
  //ofPushMatrix();
  //ofTranslate(halfWidth, ofGetHeight() / 2);
  ofSetCircleResolution(100);
  ofDrawCircle(x, y, radius);
  //ofPopMatrix();
  shader.end();
}

//--------------------------------------------------------------
bool Circle::calm = true;
bool Circle::wandering = false;
float Circle::rebelX = 0.0;
float Circle::rebelY = 0.0;
float Circle::rebelTargetX = 0.0;
float Circle::rebelTargetY = 0.0;
float Circle::rebelTheta = 0.0;
float Circle::rebelRadius = 0.0;
float Circle::timeWandering = 0.0;
float Circle::rebelChasingX = 0.0;
float Circle::rebelChasingY = 0.0;
bool Circle::born = false;
bool Circle::hasRebel = true;
bool Circle::reHide = false;
bool Circle::becomingBeat = false;
float Circle::alpha = 1.0;

void Circle::draw() const {
  if (!active) return;
  int drawAlpha = 128 * alpha;
  if (!calm && ((float) rand() / RAND_MAX) < LINE_PROB) {
    ofSetColor(ofColor::white, drawAlpha);
    ofPolyline line;
    line.addVertex(getPoint());
    line.addVertex(ofPoint(rebelX, rebelY));
    line.draw();
  } else {
    if (id == REBEL_ID) ofSetColor(ofColor::green, drawAlpha);
    else ofSetColor(ofColor::red, drawAlpha);
  }
  if ((!wandering && calm) || id != REBEL_ID) ofDrawCircle(x, y, r / (n * 2));
  for (const auto& c : children) {
    c.draw();
  }
}

void Circle::drawRebel() {
  if (wandering || (!wandering && !calm)) {
    ofSetColor(ofColor::green, (int)(128 * alpha));
    ofDrawCircle(rebelX, rebelY, rebelRadius);
  }
}

int Circle::rebelIntersectsMoon(float moonRadius) {
  return int(sqrt(pow(rebelX - ofGetWidth() / 2, 2.0) +
                  pow(rebelY - ofGetHeight() / 2, 2.0)) < moonRadius);
}

void Circle::update() {
  theta += direction * THETA_INC;
  float c_theta = theta;
  float delta = 2 * PI / n;
  for (uint i = 0; i < children.size(); ++i) {
    children[i].setPos(x + r * cos(c_theta), y + r * sin(c_theta));
    children[i].update();
    c_theta += delta;
  }
}

void Circle::setPos(float new_x, float new_y) {
  x = new_x;
  y = new_y;
  if (id == REBEL_ID && hasRebel) {
    if (calm && !wandering) {
      rebelX = x;
      rebelY = y;
      rebelRadius = r / (n * 2);
    }
    rebelTargetX = x;
    rebelTargetY = y;
  }
}

void Circle::updateRebel(glm::vec2 starPos) {
  if ((!calm && !wandering) || becomingBeat) {
    double dist = sqrt(pow(rebelTargetX - rebelX, 2.0) + pow(rebelTargetY - rebelY, 2.0));
    double frac = std::min(1.0, RANDOM_SPEED / dist);
    timeWandering = ofGetElapsedTimef();
    rebelX += frac * (rebelTargetX - rebelX);
    rebelY += frac * (rebelTargetY - rebelY);
    if (abs(rebelX - rebelTargetX) < 0.001 && abs(rebelY - rebelTargetY) < 0.001) {
      calm = true;
    }
    return;
  }
  if (wandering) {
    auto newPos = randomMove(rebelX, rebelY, rebelTheta, RANDOM_SPEED * 5.0);
    rebelX = newPos.x;
    rebelY = newPos.y;
    rebelTheta = newPos.z;
    rebelChasingX = starPos.x;
    rebelChasingY = starPos.y;
  }
}

void Circle::split() {
  if (!calm) return;
  int old_n = n;
  n++;
  radiusSum += r / n;
  float delta = 2 * PI / n;
  float c_theta = theta;
  if (old_n == 1) {
    old_n = 0;
  } else {
    for (uint i = 0; i < old_n; ++i) {
      children[i].setPos(x + r * cos(c_theta), y + r * sin(c_theta));
      children[i].split();
      c_theta += delta;
    }
  }
  for (uint i = old_n; i < n; ++i) {
    children.emplace_back(x + r * cos(c_theta),
                          y + r * sin(c_theta), 
                          r / 2,
                          -direction,
                          id + std::to_string(i));
    c_theta += delta;
  }
  numSplits++;
}

bool Circle::prune() {
  // if (n == 1 || !calm) return true;
  if (n == 1) return true;
  bool shouldPrune = true;
  for (uint i = 0; i < children.size(); ++i) {
    shouldPrune &= children[i].prune();
  }
  radiusSum -= r / n;
  n--;
  if (shouldPrune) {
    children.clear();
    n = 1;
  }
  numSplits--;
  return false;
}

bool Circle::intersect(glm::vec2 pt) {
  return sqrt(pow(x - pt.x, 2.0) + pow(y - pt.y, 2.0)) < radiusSum;
}

//--------------------------------------------------------------
void ofApp::setup() {
  halfWidth = ofGetWidth() / 2;
  halfHeight = ofGetHeight() / 2;

  ofSetVerticalSync(true);
  ofBackground(ofColor::black);
  ofSetLogLevel(OF_LOG_NOTICE);
  ofDisableAntiAliasing();
  ofEnableAlphaBlending();

  disklavierIn.listInPorts();
  disklavierOut.listOutPorts();
  // You'll want to open the appropriate ports based on the
  // output of the above two commands.
  disklavierIn.openPort(DISKLAVIER_PORT);
  // don't ignore sysex, timing, & active sense messages,
  // these are ignored by default
  disklavierIn.ignoreTypes(false, false, false);
  // add ofApp as a listener
  disklavierIn.addListener(this);
  disklavierOut.openPort(DISKLAVIER_PORT);

  // Star notes.
  int starNotes[] = {84, 86, 88, 89, 91, 93, 95, 96, 98, 100, 101, 103, 105, 107};

  orbitRadius = MOON_RADIUS * 2;
  orbitAngle = 3 * PI / 2;
  center = glm::vec2(halfWidth, halfHeight);
  moon = new Moon(center.x, center.y, MOON_RADIUS);
  for (uint i = 0; i < NUM_STARS; ++i) {
    float xPos = ((float) rand() / RAND_MAX) * ofGetWidth();
    float yPos = ((float) rand() / RAND_MAX) * ofGetHeight();
    if (sqrt(pow(xPos - halfWidth, 2.0) + pow(yPos - halfHeight, 2.0)) < MOON_RADIUS * 1.5) {
      int flip = rand() % 2 + 1;
      float xLower = flip == 1 ? 0.0 : halfWidth + MOON_RADIUS * 2.0;
      float xHigher = flip == 1 ? halfWidth - MOON_RADIUS * 2.0 : ofGetWidth();
      float xRange = xHigher - xLower;
      xPos = ((float) rand() / RAND_MAX) * xRange + xLower;
      flip = rand() % 2 + 1;
      float yLower = flip == 1 ? 0.0 : halfHeight + MOON_RADIUS * 2.0;
      float yHigher = flip == 1 ? halfHeight - MOON_RADIUS * 2.0 : ofGetHeight();
      float yRange = yHigher - yLower;
      yPos = ((float) rand() / RAND_MAX) * yRange + yLower;
    }
    stars.emplace_back(xPos, yPos, MOON_RADIUS, starNotes[i]);
  }
  c = new Circle(center.x + orbitRadius * cos(orbitAngle),
                 center.y + orbitRadius * sin(orbitAngle),
                 STARTING_RADIUS, 1, "");

  c->deactivate();
}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
  if(msg.status < MIDI_SYSEX) {
    if (msg.control == 67) {
      if (pedalOn && msg.value == 0) {
        pedalOn = false;
        cycleModes();
      } else if (msg.value > 0) {
        pedalOn = true;
      }
    }
    if (msg.status == MIDI_NOTE_ON || msg.status == MIDI_NOTE_OFF) {
    }
  }
}

void ofApp::sendPlayNote(int note, int velocity) {
  disklavierOut.sendNoteOn(1, note, velocity);
}

void ofApp::sendStopNote(int note, int velocity) {
  disklavierOut.sendNoteOff(1, note, velocity);
}

void ofApp::sendCCMessage(int channel, int control, int value) {
  disklavierOut.sendControlChange(channel, control, value);
}

//--------------------------------------------------------------
void ofApp::update() {
  if (storyMode == fadeAway) {
    moon->reduceAlpha(0.001);
    c->reduceAlpha(0.001);
  }
  if (c->isActive() && c->calm) {
    auto const& pos = c->getPos();
    if (!c->born && pos.y >= center.y - orbitRadius) {
      c->setPos(pos.x, pos.y - CIRCLE_DISPLACEMENT);
    } else if (c->reHide &&
               pos.x <= halfWidth + 5.0 && pos.x >= halfWidth - 5.0 &&
               pos.y < halfHeight) {
      c->setPos(halfWidth, pos.y + CIRCLE_DISPLACEMENT);
      if (c->getPos().y > center.y - MOON_RADIUS) {
        c->deactivate();
        moon->beatShiftDir = -1;
        moon->activate(0);
      }
    } else {
      c->born = true;
      orbitAngle += 0.01;
      c->setPos(center.x + orbitRadius * cos(orbitAngle),
                center.y + orbitRadius * sin(orbitAngle));
    }
  }
  bool playMoon = moon->update();
  if (playMoon && c->calm && storyMode != fadeAway) {
    int velocity = storyMode == preFade ? 80 : 60;
    sendPlayNote(33, velocity);
    lastMoonPlay = ofGetElapsedTimef();
  } else {
    float currTime = ofGetElapsedTimef();
    if (currTime - lastMoonPlay > BEAT_TEMPO / 2) {
      sendStopNote(33, 40);
      lastMoonPlay = currTime;
    }
  }
  for (auto& star : stars) {
    int n = star.update();
    if (n > 0) sendPlayNote(n, 30);
    else if (n < 0) sendStopNote(-n, 30);
  }
  if (!c->isActive() && moon->isBeatAtEdge()) {
    c->activate();
    // The shader thing flips the y-axis, so correct for it.
    c->setPos(moon->shiftingBeatPos().x, center.y - MOON_RADIUS);
    moon->deactivate(0);
  }
  c->update();
  c->updateRebel(stars[rand() % stars.size()].getPos());
}


//--------------------------------------------------------------
void ofApp::draw() {
  for (auto star : stars) star.draw();
  if ((storyMode == callingBack || storyMode == callBack)  && (!c->calm)) {
    moon->draw(glm::vec2(-1.0, -1.0), c->isActive());
  } else {
    moon->draw(c->getPos(), c->isActive());
  }
  c->draw();
  c->drawRebel();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
}

void ofApp::cycleModes() {
  auto angle = 2 * PI;
  auto newX = center.x + MOON_RADIUS * sin(angle) / 2;
  auto newY = center.y;
  switch(storyMode) {
    case start:
      moon->grow();
      storyMode = growMoon;
      break;
    case growMoon:
      if (moon->fullSize()) {
        storyMode = circleOrbit;
        moon->startShiftingBeat(0);
      }
      break;
    case circleOrbit:
      c->split();
      storyMode = circleSplit1;
      break;
    case circleSplit1:
      c->split();
      storyMode = circleSplit2;
      break;
    case circleSplit2:
      c->wandering = true;
      c->timeWandering = ofGetElapsedTimef();
      storyMode = breakAway;
      break;
    case breakAway:
      c->calm = numReturns >= 1;
      storyMode = numReturns < 1 ? callingBack : circlePrune1;
      break;
    case callingBack:
      storyMode = callBack;
      c->wandering = false;
      break;
    case callBack:
      c->calm = true;
      storyMode = circlePrune1;
      break;
    case circlePrune1:
      c->prune();
      storyMode = circlePrune2;
      break;
    case circlePrune2:
      c->prune();
      storyMode = numReturns < 1 ? circleOrbit : circleRehide;
      numReturns++;
      break;
    case circleRehide:
      c->reHide = true;
      storyMode = makeNewBeat;
      break;
    case makeNewBeat:
      c->rebelTargetX = newX;
      c->rebelTargetY = newY + MOON_RADIUS * cos(angle) / 2;
      c->becomingBeat = true;
      storyMode = newBeat;
      break;
    case newBeat:
      moon->addNewBeat(glm::vec2(newX, newY - MOON_RADIUS * cos(angle) / 2));
      storyMode = circleReBorn;
      break;
    case circleReBorn:
      moon->startShiftingBeat(0);
      c->reHide = false;
      c->born = false;
      c->hasRebel = false;
      storyMode = circleReSplit1;
      break;
    case circleReSplit1:
      c->split();
      storyMode = circleReSplit2;
      break;
    case circleReSplit2:
      c->split();
      storyMode = preFade;
      break;
    case preFade:
      storyMode = fadeAway;
      break;
    case fadeAway:
      break;
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
  if (key == 'n') {
    c->split();
  } else if (key == 'p') {
    c->prune();
  } else if (key == 'j') {
    c->calm = !c->calm;
  } else if (key == 's') {
    moon->startShiftingBeat(0);
  } else if (key == 'b') {
    c->wandering = true;
    c->timeWandering = ofGetElapsedTimef();
  } else if (key == 'g') {
    step = true;
  } else if (key == 'a') {
    cycleModes();
  }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ) {
	
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
	
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
}

//--------------------------------------------------------------
void ofApp::resized(int w, int h){
}

