#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {
  receiver.setup(IN_PORT);
  ofSetVerticalSync(true);
  //ofBackgroundHex(0xfdefc2);
  ofBackgroundHex(0xffffff);
  ofSetLogLevel(OF_LOG_NOTICE);
  ofDisableAntiAliasing();
}

//--------------------------------------------------------------
void ofApp::update() {
  // check for OSC messages
  while(receiver.hasWaitingMessages()) {
      ofxOscMessage m;
      receiver.getNextMessage(m);
      int note = m.getArgAsInt(0);
      int velocity = m.getArgAsInt(1);
      if (m.getAddress() == "/noteon") {
          pianoPendulum.addNote(note);
      } else if (m.getAddress() == "/noteoff") {
          pianoPendulum.dropNote(note);
      } else if (m.getAddress() == "/ccevent") {
        // Check if it's the left pedal.
        if (velocity == 67 && note == 127) {
          pianoPendulum.cycleModes();
        }
      }
  }
  pianoPendulum.update();
}


//--------------------------------------------------------------
void ofApp::draw() {
  pianoPendulum.draw();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == 'h') pianoPendulum.drawBalls = !pianoPendulum.drawBalls;
  if (key == 't') ofToggleFullscreen();
  if (key == 'r') pianoPendulum.reset();
  if (key >= 48 && key <= 57) pianoPendulum.setMode(key - 48);
  if(key == 'a') {
    pianoPendulum.addNote(62);
  } else if (key == 's') {
    pianoPendulum.addNote(64);
  } else if (key == 'd') {
    pianoPendulum.addNote(66);
  } else if (key == 'f') {
    pianoPendulum.addNote(67);
  } else if (key == 'g') {
    pianoPendulum.addNote(69);
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
  if(key == 'a') {
    pianoPendulum.dropNote(62);
  } else if (key == 's') {
    pianoPendulum.dropNote(64);
  } else if (key == 'd') {
    pianoPendulum.dropNote(66);
  } else if (key == 'f') {
    pianoPendulum.dropNote(67);
  } else if (key == 'g') {
    pianoPendulum.dropNote(69);
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

