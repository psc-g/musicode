#include "ofApp.h"
#include <algorithm>
#include <bitset>
#include <math.h>

#include "../../common/pitchColours.h"

//--------------------------------------------------------------
void ofApp::setup() {
  font.load("fonts/DejaVuSans.ttf", 32, false, false, true);
  box2d.init();
  box2d.setGravity(0, GRAVITY);
  box2d.setFPS(60.0);
  box2d.createBounds();
  box2d.registerGrabbing();

  particleSystem.init(box2d.getWorld());
  particleSystem.setMaxParticles(MAX_PARTICLES);
  particleSystem.setRadius(KEY_SEPARATION * 2);

  receiver.setup(IN_PORT);
  sender.setup(HOST, OUT_PORT);

  ofSetVerticalSync(true);
  //ofBackgroundHex(0xffffff);
  ofBackground(ofColor::black);
  ofSetLogLevel(OF_LOG_NOTICE);
  ofDisableAntiAliasing();
  ofEnableAlphaBlending();
  setupKeyboard();
  newScale();
}

void ofApp::setupKeyboard() {
  whiteWidth = ofGetWidth() / 60;
  whiteHeight = whiteWidth * 4;
  backboardTop.setPhysics(0.0, 0.53, 0.1);
  backboardTop.setup(box2d.getWorld(),
                     ofGetWidth() / 2,
                     whiteHeight / 2,
                     whiteWidth * 52 + KEY_SEPARATION * 26,
                     whiteHeight + KEY_SEPARATION);
  float xPos = ofGetWidth() / 2 - 26.75 * whiteWidth;
  float yPosBottom = ofGetHeight() - whiteHeight * 1.5;
  float yPosTop = whiteHeight / 2;
  backboardBottomLeft.setPhysics(0.0, 0.53, 0.1);
  backboardBottomLeft.setup(box2d.getWorld(),
                            (xPos - whiteWidth) / 2,
                            yPosBottom,
                            xPos,
                            whiteHeight + KEY_SEPARATION);
  backboardBottomRight.setPhysics(0.0, 0.53, 0.1);
  backboardBottomRight.setup(box2d.getWorld(),
                             ofGetWidth() - (xPos - whiteWidth) / 2,
                             yPosBottom,
                             xPos,
                             whiteHeight + KEY_SEPARATION);
  int whiteKeyPos = 0;
  int i = 0;
  while (i < 88) {
    onScale[i] = false;
    int basePitch = PITCH_COLOURS[(i - 2) % 12];
    float multiplier = floor(i / 12);
    keyColours[i] = basePitch * multiplier;

    pianoKeysBottom[i].active = false;
    pianoKeysBottom[i].white = true;
    pianoKeysBottom[i].key.setPhysics(0.0, 0.53, 0.1);
    pianoKeysBottom[i].key.setup(box2d.getWorld(),
                                   xPos,
                                   yPosBottom,
                                   whiteWidth - KEY_SEPARATION,
                                   whiteHeight - KEY_SEPARATION / 2);
    pianoKeysBottom[i].origPos = pianoKeysBottom[i].key.getPosition();
    pianoKeysBottom[i].targetY = pianoKeysBottom[i].origPos.y - whiteHeight;
    pianoKeysBottomBacking[i].active = false;
    pianoKeysBottomBacking[i].white = true;
    pianoKeysBottomBacking[i].key.setPhysics(0.0, 0.53, 0.1);
    pianoKeysBottomBacking[i].key.setup(box2d.getWorld(),
                                        xPos,
                                        yPosBottom,
                                        whiteWidth,
                                        whiteHeight);
    pianoKeysBottomBacking[i].origPos = pianoKeysBottomBacking[i].key.getPosition();
    pianoKeysBottomBacking[i].targetY = pianoKeysBottomBacking[i].origPos.y - whiteHeight;
    pianoKeysTop[i].active = false;
    pianoKeysTop[i].white = true;
    pianoKeysTop[i].key.setPhysics(0.0, 0.53, 0.1);
    pianoKeysTop[i].key.setup(box2d.getWorld(),
                                xPos,
                                yPosTop,
                                whiteWidth - KEY_SEPARATION,
                                whiteHeight - KEY_SEPARATION / 2);
    pianoKeysTop[i].origPos = pianoKeysTop[i].key.getPosition();
    i++;
    // We need to special case the lone black key in the lowest partial octave.
    // After that we can do simple mod arithmetic to figure it out.
    int keyPos = (whiteKeyPos - 2) % 7;
    if (whiteKeyPos == 0 || (i < 88 && whiteKeyPos >= 2 && keyPos != 2 && keyPos != 6)) {
      keyColours[i] = basePitch * multiplier;
      pianoKeysBottom[i].active = false;
      pianoKeysBottom[i].white = false;
      pianoKeysBottom[i].key.setPhysics(0.0, 0.53, 0.1);
      pianoKeysBottom[i].key.setup(box2d.getWorld(),
                                     xPos + (whiteWidth + KEY_SEPARATION) / 2,
                                     yPosBottom - whiteHeight / 4,
                                     (whiteWidth - KEY_SEPARATION) / 2,
                                     (whiteHeight - KEY_SEPARATION) / 2);
      pianoKeysBottom[i].origPos = pianoKeysBottom[i].key.getPosition();
      pianoKeysBottom[i].targetY = pianoKeysBottom[i].origPos.y - whiteHeight / 2;
      pianoKeysBottomBacking[i].active = false;
      pianoKeysBottomBacking[i].white = false;
      pianoKeysBottomBacking[i].key.setPhysics(0.0, 0.53, 0.1);
      pianoKeysBottomBacking[i].key.setup(box2d.getWorld(),
                                          xPos + (whiteWidth + KEY_SEPARATION) / 2,
                                          yPosBottom - whiteHeight / 4,
                                          whiteWidth / 2,
                                          whiteHeight / 2);
      pianoKeysBottomBacking[i].origPos = pianoKeysBottomBacking[i].key.getPosition();
      pianoKeysBottomBacking[i].targetY = pianoKeysBottomBacking[i].origPos.y - whiteHeight / 2;
      pianoKeysTop[i].active = false;
      pianoKeysTop[i].white = false;
      pianoKeysTop[i].key.setPhysics(0.0, 0.53, 0.1);
      pianoKeysTop[i].key.setup(box2d.getWorld(),
                                  xPos + (whiteWidth + KEY_SEPARATION) / 2,
                                  yPosTop - whiteHeight / 4,
                                  (whiteWidth - KEY_SEPARATION) / 2,
                                  (whiteHeight - KEY_SEPARATION) / 2);
      pianoKeysTop[i].origPos = pianoKeysTop[i].key.getPosition();
      i++;
    }
    xPos += whiteWidth + KEY_SEPARATION / 2;
    whiteKeyPos++;
  }
}

void ofApp::addNote(int pitch) {
  if (DO_TRIADS) {
    if (pitch < 36) {
      int newPitch = pitch - 21;  // Second octave from bottom.
      sendNote(root, false);
      root = newPitch;
      updateOnScale();
      if (((float) rand() / RAND_MAX) < SCALE_SWITCH_PROBABILITY) {
        newScale();
      }
      updateTriad();
    }
  } else {
    int newPitch = pitch - 21;
    auto keyPos = pianoKeysTop[newPitch].key.getPosition();
    particleSystem.addParticle(keyPos.x, whiteHeight * 1.1);
    pianoKeysTop[newPitch].lastDraw = ofGetElapsedTimef();
    pianoKeysTop[newPitch].active = true;
  }
  pianoKeysBottom[pitch - 21].active = true;
}

void ofApp::dropNote(int pitch) {
  // Set active note on keyboard.
  pianoKeysBottom[pitch - 21].active = false;
  if (!DO_TRIADS) pianoKeysTop[pitch - 21].active = false;
}

void ofApp::allNotesOff() {
  root = -1;
  triad[0] = triad[1] = triad[2] = -1;
  for (uint i = 0; i < 88; ++i) sendNote(i, false);
}

//--------------------------------------------------------------
void ofApp::update() {
  box2d.update();
  // check for OSC messages
  while (receiver.hasWaitingMessages()) {
    ofxOscMessage m;
    receiver.getNextMessage(m);
    int pitch = m.getArgAsInt(0);
    int velocity = m.getArgAsInt(1);
    if (m.getAddress() == "/noteon") {
      addNote(pitch);
    } else if (m.getAddress() == "/noteoff") {
      dropNote(pitch);
    } else if (velocity == 67) {
      if (pitch > 0) {
        if (pedalOnStart < 0.0) pedalOnStart = ofGetElapsedTimef();
      } else {
        float elapsed = ofGetElapsedTimef() - pedalOnStart;
        if (elapsed > RESET_THRESHOLD) {
          allNotesOff();
        } else {
          newScale();
        }
        pedalOnStart = -1.0;
      }
    }
  }
  for (uint i = 0; i < 88; ++i) {
    if (!DO_TRIADS && pianoKeysTop[i].active) {
      auto currTime = ofGetElapsedTimef();
      if (currTime - pianoKeysTop[i].lastDraw > PARTICLE_DELAY) {
        auto keyPos = pianoKeysTop[i].key.getPosition();
        particleSystem.addParticle(keyPos.x, whiteHeight * 1.1);
        pianoKeysTop[i].lastDraw = currTime;
      }
    } else if (pianoKeysTop[i].inTriad) {
      auto currTime = ofGetElapsedTimef();
      if (currTime - pianoKeysTop[i].lastDraw > PARTICLE_DELAY) {
        auto keyPos = pianoKeysTop[i].key.getPosition();
        particleSystem.addParticle(keyPos.x, whiteHeight * 1.1);
        pianoKeysTop[i].lastDraw = currTime;
      }
    }
    if (pianoKeysBottom[i].active && (!DO_TRIADS || onScale[i])) {
      auto pos = pianoKeysBottom[i].key.getPosition();
      auto yPos = std::max(pianoKeysBottom[i].targetY, pos.y - KEY_SEPARATION * 2);
      pianoKeysBottom[i].key.setPosition(pos.x, yPos);
      pos = pianoKeysBottomBacking[i].key.getPosition();
      yPos = std::max(pianoKeysBottomBacking[i].targetY, pos.y - KEY_SEPARATION * 2);
      pianoKeysBottomBacking[i].key.setPosition(pos.x, yPos);
    } else {
      auto pos = pianoKeysBottom[i].key.getPosition();
      auto yPos = std::min(pianoKeysBottom[i].origPos.y, pos.y + KEY_SEPARATION * 2);
      pianoKeysBottom[i].key.setPosition(pos.x, yPos);
      pos = pianoKeysBottomBacking[i].key.getPosition();
      yPos = std::min(pianoKeysBottomBacking[i].origPos.y, pos.y + KEY_SEPARATION * 2);
      pianoKeysBottomBacking[i].key.setPosition(pos.x, yPos);
    }
  }
  // Update tremoloing triads.
  if (DO_TRIADS) {
    if (root < 0) return;
    float currTime = ofGetElapsedTimef();
    if (currTime - lastTriadSelect > TRIAD_DELAY) {
      int selectedNote = rand() % 3;
      for (uint i = 0; i < 3; ++i) {
        if (i == selectedNote) sendNote(triad[i], true);
        else sendNote(triad[i], false);
      }
      lastTriadSelect = currTime;
    }
    if (currTime - lastOctaveSelect > OCTAVE_DELAY) {
      updateTriad();
      lastOctaveSelect = currTime;
    }
  }
}

void ofApp::sendNote(int pitch, bool onset) {
  if (pitch < 0) return;
  ofxOscMessage m;
  if (onset) m.setAddress("/playnote");
  else m.setAddress("/stopnote");
  m.addIntArg(pitch + 21);
  m.addIntArg(30);
  sender.sendMessage(m);
  pianoKeysTop[pitch].inTriad = onset;
}


//--------------------------------------------------------------
void ofApp::draw() {
  ofSetHexColor(0x000000);
  ofFill();
  backboardTop.draw();
  backboardBottomLeft.draw();
  backboardBottomRight.draw();
  // We have to go through the keys twice to make sure we draw the black ones last.
  for (uint i = 0; i < 88; ++i) {
    if (!pianoKeysBottom[i].white) continue;
    ofSetHexColor(0x000000);
    ofFill();
    pianoKeysBottomBacking[i].key.draw();
    if (!DO_TRIADS) {
      ofSetColor(ofColor::white);
    } else if (pianoKeysBottom[i].active && onScale[i]) {
      ofSetHexColor(keyColours[i]);
    } else if (!onScale[i]) {
      ofSetColor(ofColor::grey);
    } else {
      ofSetHexColor(0xffffff);
    }
    ofFill();
    pianoKeysBottom[i].key.draw();
    if (pianoKeysTop[i].active || pianoKeysTop[i].inTriad) {
      ofSetHexColor(keyColours[i]);
    } else {
      ofSetHexColor(0xffffff);
    }
    ofFill();
    pianoKeysTop[i].key.draw();
  }
  for (uint i = 0; i < 88; ++i) {
    if (pianoKeysBottom[i].white) continue;
    ofSetHexColor(0x000000);
    ofFill();
    pianoKeysBottomBacking[i].key.draw();
    if (!DO_TRIADS) {
      ofSetColor(ofColor::black);
    } else if (pianoKeysBottom[i].active && onScale[i]) {
      ofSetHexColor(keyColours[i]);
    } else if (!onScale[i]) {
      ofSetColor(ofColor::grey);
    } else {
      ofSetHexColor(0x000000);
    }
    ofFill();
    pianoKeysBottom[i].key.draw();
    if (pianoKeysTop[i].active || pianoKeysTop[i].inTriad) {
      ofSetHexColor(keyColours[i]);
    } else {
      ofSetHexColor(0x000000);
    }
    ofFill();
    pianoKeysTop[i].key.draw();
  }
  ofSetColor(ofColor::yellow);
  particleSystem.updateMesh();
  particleSystem.draw();


  if (root >= 0) {
    ofSetColor(ofColor::green);
    auto printStr = to_string(scaleNumber) + " = " + bitString;
    font.drawString(printStr,
                    (ofGetWidth() - font.stringWidth(printStr)) / 2,
                    ofGetHeight() - font.stringHeight(printStr));
  }
}

void ofApp::updateTriad() {
  if (!DO_TRIADS) return;
  for (uint i = 0; i < 3; ++i) sendNote(triad[i], false);
  int selectedOctave = rand() % 5 + 1;  // Don't use first octave.
  sendNote(root, true);
  int currPitch = root + 12 * selectedOctave;
  triad[0] = currPitch;
  int scalePos = 0;
  for (uint i = bitString.length() - 1; i >= 0; --i) {
    if (scalePos % 2 == 0) triad[scalePos / 2] = currPitch;
    currPitch += 1;
    scalePos += 1;
    if (bitString[i] == '1') currPitch += 1;
    if (scalePos > 4) break;
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == 't') ofToggleFullscreen();
  if (key == 'r') reset();
  if(key == 'a') {
    addNote(62);
  } else if (key == 's') {
    addNote(64);
  } else if (key == 'd') {
    addNote(66);
  } else if (key == 'f') {
    addNote(67);
  } else if (key == 'g') {
    addNote(69);
  }
}
 void ofApp::updateOnScale() {
  if (root < 0) return;
  for (uint i = 0; i < 88; ++i) onScale[i] = false;
  int currPitch = root;
  for (uint i = bitString.length() - 1; i >= 0; --i) {
    onScale[currPitch] = true;
    for (int octave = -1; octave < 7; ++octave) onScale[currPitch + 12 * octave] = true;
    if (currPitch - root >= 12) break;
    currPitch += 1;
    if (bitString[i] == '1') currPitch += 1;
  }
 }

void ofApp::newScale() {
  scaleNumber = rand() % 1024;
  bitString = std::bitset<12>(scaleNumber).to_string();
  updateOnScale();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
  if(key == 'a') {
    dropNote(62);
  } else if (key == 's') {
    dropNote(64);
  } else if (key == 'd') {
    dropNote(66);
  } else if (key == 'f') {
    dropNote(67);
  } else if (key == 'g') {
    dropNote(69);
  } else if (key == 'y') {
    newScale();
  } else if (key == 'z') {
    updateTriad();
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

