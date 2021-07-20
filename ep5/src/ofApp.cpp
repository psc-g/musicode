#include "ofApp.h"
#include "../../common/pitchColours.h"

int ofApp::drumbeatNotes[] = {
  0, 87, 87, 86, 87,
  0, 87, 87, 86, 87,
  0, 87, 87, 86, 87,
  0, 87, 87, 86, 87,
  0, 87, 87, 86, 87,
  0, 87, 87, 85, 87,
};

void makeCircularKeyboard(int numKeys, float startTheta, float innerRadius,
                          float baseOuterRadius, bool animateKeys,
                          bool* activeKeys, int frameNum, ofMesh* keys,
                          ofMesh* border, ofMesh* innerCircle,
                          ofMesh* outerCircle, float xShift, float yShift) {
  float phi = PI / 2;
  float deltaTheta = 2.0 * PI / numKeys;
  float theta = startTheta;
  for (int keyPos = 0; keyPos < numKeys; ++keyPos) {
    float outerRadius = baseOuterRadius;
    vector<glm::vec3> vertices;
    vector<glm::vec3> innerCircleVertices;
    vector<glm::vec3> outerCircleVertices;
    if (animateKeys && activeKeys[keyPos]) {
      outerRadius += ofMap(ofNoise(keyPos, frameNum * 0.01),
                           0, 1, outerRadius / 4, outerRadius / 2);
    }
    vertices.push_back(glm::vec3(xShift + innerRadius * sin(phi) * cos(theta),
                                 yShift + innerRadius * sin(phi) * sin(theta),
                                 innerRadius * cos(phi)));
    vertices.push_back(glm::vec3(xShift + outerRadius * sin(phi) * cos(theta),
                                 yShift + outerRadius * sin(phi) * sin(theta),
                                 outerRadius * cos(phi)));
    vertices.push_back(
        glm::vec3(xShift + outerRadius * sin(phi) * cos(theta + deltaTheta),
                  yShift + outerRadius * sin(phi) * sin(theta + deltaTheta),
                  outerRadius * cos(phi)));
    vertices.push_back(
        glm::vec3(xShift + innerRadius * sin(phi) * cos(theta + deltaTheta),
                  yShift + innerRadius * sin(phi) * sin(theta + deltaTheta),
                  innerRadius * cos(phi)));
    keys->addVertices(vertices);
    border->addVertices(vertices);
    if (innerCircle != NULL) {
      innerCircleVertices.push_back(glm::vec3(xShift, yShift, 0.0));
      innerCircleVertices.push_back(
          glm::vec3(xShift + innerRadius * sin(phi) * cos(theta),
                    yShift + innerRadius * sin(phi) * sin(theta),
                    innerRadius * cos(phi)));
      innerCircleVertices.push_back(
          glm::vec3(xShift + innerRadius * sin(phi) * cos(theta + deltaTheta),
                    yShift + innerRadius * sin(phi) * sin(theta + deltaTheta),
                    innerRadius * cos(phi)));
      innerCircleVertices.push_back(glm::vec3(xShift, yShift, 0.0));
      innerCircle->addVertices(innerCircleVertices);
    }
    if (outerCircle != NULL) {
      outerCircleVertices.push_back(
          glm::vec3(xShift + outerRadius * sin(phi) * cos(theta),
                    yShift + outerRadius * sin(phi) * sin(theta),
                    outerRadius * cos(phi)));
      outerCircleVertices.push_back(
          glm::vec3(xShift + outerRadius * sin(phi) * cos(theta) * 10,
                    yShift + outerRadius * sin(phi) * sin(theta) * 10,
                    outerRadius * cos(phi) * 10));
      outerCircleVertices.push_back(
          glm::vec3(xShift + outerRadius * sin(phi) * cos(theta + deltaTheta) * 10,
                    yShift + outerRadius * sin(phi) * sin(theta + deltaTheta) * 10,
                    outerRadius * cos(phi) * 10));
      outerCircleVertices.push_back(
          glm::vec3(xShift + outerRadius * sin(phi) * cos(theta + deltaTheta),
                    yShift + outerRadius * sin(phi) * sin(theta + deltaTheta),
                    outerRadius * cos(phi)));
      outerCircle->addVertices(outerCircleVertices);
    }

    keys->addIndex(keys->getNumVertices() - 1);
    keys->addIndex(keys->getNumVertices() - 2);
    keys->addIndex(keys->getNumVertices() - 3);
    keys->addIndex(keys->getNumVertices() - 1);
    keys->addIndex(keys->getNumVertices() - 3);
    keys->addIndex(keys->getNumVertices() - 4);

    border->addIndex(keys->getNumVertices() - 1);
    border->addIndex(keys->getNumVertices() - 2);
    border->addIndex(keys->getNumVertices() - 2);
    border->addIndex(keys->getNumVertices() - 3);
    border->addIndex(keys->getNumVertices() - 3);
    border->addIndex(keys->getNumVertices() - 4);
    border->addIndex(keys->getNumVertices() - 4);
    border->addIndex(keys->getNumVertices() - 1);

    for (int i = 0; i < vertices.size(); ++i) {
      if (numKeys == 88) {
        int octavePos = (keyPos - 3) % 12;
        if (keyPos == 0 || keyPos == 2 ||
            (keyPos > 2 && octavePos != 1 && octavePos != 3 &&
             octavePos != 6 && octavePos != 8 && octavePos != 10)) {
          keys->addColor(ofColor(239));
          border->addColor(ofColor(39));
        } else {
          keys->addColor(ofColor(39));
          border->addColor(ofColor(239));
        }
      } else {
        if (keyPos % 2 == 0) {
          keys->addColor(ofColor(239));
          border->addColor(ofColor(39));
        } else {
          keys->addColor(ofColor(39));
          border->addColor(ofColor(239));
        }
      }
    }

    if (innerCircle != NULL) {
      innerCircle->addIndex(innerCircle->getNumVertices() - 1);
      innerCircle->addIndex(innerCircle->getNumVertices() - 2);
      innerCircle->addIndex(innerCircle->getNumVertices() - 3);
      innerCircle->addIndex(innerCircle->getNumVertices() - 1);
      innerCircle->addIndex(innerCircle->getNumVertices() - 3);
      innerCircle->addIndex(innerCircle->getNumVertices() - 4);
      for (int i = 0; i < innerCircleVertices.size(); ++i) {
        innerCircle->addColor(ofColor(255));
      }
    }
    if (outerCircle != NULL) {
      outerCircle->addIndex(outerCircle->getNumVertices() - 1);
      outerCircle->addIndex(outerCircle->getNumVertices() - 2);
      outerCircle->addIndex(outerCircle->getNumVertices() - 3);
      outerCircle->addIndex(outerCircle->getNumVertices() - 1);
      outerCircle->addIndex(outerCircle->getNumVertices() - 3);
      outerCircle->addIndex(outerCircle->getNumVertices() - 4);
      for (int i = 0; i < outerCircleVertices.size(); ++i) {
        outerCircle->addColor(ofColor(255));
      }
    }
    theta += deltaTheta;
  }
}

//--------------------------------------------------------------
void ofApp::setup() {
  receiver.setup(IN_PORT);
  sender.setup(HOST, OUT_PORT);
  ofBackground(29, 53, 87);
  font.load("Courier New Bold.ttf", 32);
  fbo.allocate(ofGetWidth(), ofGetHeight());
  reset();
  ofSetLineWidth(2);
  ofEnableDepthTest();
  border.setMode(ofPrimitiveMode::OF_PRIMITIVE_LINES);
  setupKeyboard();
  shader.load("shaders/bg");
}

void ofApp::reset() {
  rotate = false;
  text = "";
  renderText();
  box.set(ofGetWidth() / 4);
  box.setPosition(0, 0, 0.0);
  box.mapTexCoordsFromTexture(fbo.getTexture());
  for (uint i = 0; i < 88; ++i) {
    activeKeys[i] = false;
    bassNotePointers[i] = -1;
    if (i < 36) activeBassNotes[i] = false;
  }
  listeningToChars = true;
  charCount = 0;
}

void ofApp::checkOSCMessages() {
  while(receiver.hasWaitingMessages()) {
    ofxOscMessage m;
    receiver.getNextMessage(m);
    int note = m.getArgAsInt(0);
    int velocity = m.getArgAsInt(1);
    if (m.getAddress() == "/noteon") {
      activeKeys[note - 21] = true;
    } else if (m.getAddress() == "/noteoff") {
        activeKeys[note - 21] = false;
    } else if (m.getAddress() == "/kickOn") {
      mode = 1;
    } else if (m.getAddress() == "/snareOn") {
      mode = 2;
    } else if (m.getAddress() == "/snare2On") {
      mode = 3;
    } else if (m.getAddress() == "/kickOff" || m.getAddress() == "/snareOff" ||
               m.getAddress() == "/snare2Off") {
      mode = 0;
    } else if (m.getAddress() == "/resetCube") {
      reset();
    } else if (m.getAddress() == "/increaseBassline") {
      basslineCounter++;
      renderText();
    } else if (m.getAddress() == "/bassOn") {
      bassNotePointers[note] = basslinePlayerCounter;
      activeBassNotes[bassNotePointers[note]] = true;
      basslinePlayerCounter = (basslinePlayerCounter + 1) % basslineCounter;
      renderText();
    } else if (m.getAddress() == "/bassOff") {
      if (bassNotePointers[note] >= 0)
        activeBassNotes[bassNotePointers[note]] = false;
      bassNotePointers[note] = -1;
      renderText();
    } else if (m.getAddress() == "/increaseEndGroove") {
      if (endGrooveCounter.size() == 0) {
        keysGroove.emplace_back();
        borderGroove.emplace_back();
        endGrooveCounter.emplace_back(0);
      }
      keysGroove[keysGroove.size() - 1].clear();
      borderGroove[borderGroove.size() - 1].clear();
      int xDir = endGrooveLoops % 2 == 0 ? -1 : 1;
      int yDir = endGrooveLoops < 2 == 0 ? -1 : 1;
      float xOffset = xDir * ofGetWidth() * 3 / 8;
      float yOffset = yDir * ofGetHeight() * 3 / 8;
      makeCircularKeyboard(endGrooveCounter[endGrooveCounter.size() - 1]++,
                           0.0, KEYS_INNER_RADIUS * 0.2,
                           KEYS_OUTER_RADIUS * 0.2, false, activeBassNotes,
                           ofGetFrameNum(), &keysGroove[keysGroove.size() - 1],
                           &borderGroove[borderGroove.size() - 1],
                           NULL, NULL, xOffset, yOffset);
    } else if (m.getAddress() == "/newNewGroove") {
      keysGroove.emplace_back();
      borderGroove.emplace_back();
      endGrooveCounter.emplace_back(0);
      endGrooveLoops++;
    }
  }
}

void ofApp::sendNote(int pitch, bool onset) {
  if (pitch < 0) return;
  ofxOscMessage m;
  if (onset) m.setAddress("/playnote");
  else m.setAddress("/stopnote");
  m.addIntArg(pitch + 21);
  m.addIntArg(60);
  sender.sendMessage(m);
}


void ofApp::setupKeyboard() {
  keys.clear();
  border.clear();
  innerCircle.clear();
  outerCircle.clear();
  makeCircularKeyboard(88, startTheta, KEYS_INNER_RADIUS, KEYS_OUTER_RADIUS,
                       true, activeKeys, ofGetFrameNum(), &keys, &border,
                       &innerCircle, &outerCircle, 0.0, 0.0);
}

void ofApp::update() {
  startTheta += 0.01;
  checkOSCMessages();
  setupKeyboard();
  if (rotate) {
    box.rotate(0.5,
               (float) rand() / RAND_MAX,
               (float) rand() / RAND_MAX,
               (float) rand() / RAND_MAX);
  }
}

void ofApp::draw() {
  cam.begin();
  shader.begin();
  shader.setUniform1i("mode", mode);
  if (mode == 1) {
    innerCircle.draw();
  } else if (mode == 2) {
    outerCircle.draw();
  }
  shader.end();
  fbo.getTexture().bind();
  ofEnableDepthTest();
  box.draw();
  ofDisableDepthTest();
  fbo.getTexture().unbind();
  keys.draw();
  border.drawWireframe();
  if (endGrooveCounter.size() > 0) {
    for (uint i = 0; i < keysGroove.size(); ++i) {
      keysGroove[i].draw();
      borderGroove[i].draw();
    }
  }
  cam.end();
}

void ofApp::renderText() {
  fbo.begin();
  ofClear(0, 0, 0, 255);
  ofSetColor(ofColor::green);
  font.drawString(text, 20, 50);
  keysLoop.clear();
  borderLoop.clear();
  makeCircularKeyboard(basslineCounter, 0.0, KEYS_INNER_RADIUS * 0.8,
                       KEYS_OUTER_RADIUS * 0.6, true, activeBassNotes,
                       ofGetFrameNum(), &keysLoop, &borderLoop,
                       NULL, NULL, 600.0, 500.0);
  keysLoop.draw();
  borderLoop.drawWireframe();
  fbo.end();
}

void ofApp::addBasslineNote(int key) {
  if (!listeningToChars) return;
  if (noteMappings.find(key) == noteMappings.end() ||
      noteMappings[key] < 0) {
    text += key;
    noteMappings[key] = drumbeatNotes[charCount % 30];
    sendNote(noteMappings[key], true);
    activeKeys[noteMappings[key]] = true;
    charCount++;
  }
}

void ofApp::keyPressed (int key) {
  if (key == OF_KEY_BACKSPACE) {
    if (text.size()) {
      text.pop_back();
    }
  } else if (key == 33) {
    reset();
    ofxOscMessage m;
    m.setAddress("/reset");
    m.addIntArg(0);
    m.addIntArg(0);
    sender.sendMessage(m);
  } else if (key == OF_KEY_RETURN) {
    text += "\n";
    rotate = !rotate;
    ofxOscMessage m;
    m.setAddress("/playdrumBeat");
    m.addIntArg(0);
    m.addIntArg(0);
    sender.sendMessage(m);
    listeningToChars = false;
  } else if (key != 3680 && key != OF_KEY_SHIFT) {
    addBasslineNote(key);
  }
  renderText();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
  if (key != 3680 && key != OF_KEY_SHIFT && key != OF_KEY_BACKSPACE && key != 33) {
    sendNote(noteMappings[key], false);
    activeKeys[noteMappings[key]] = false;
    noteMappings[key] = -1;
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
