#include "ofApp.h"
#include <iostream>
#include <string>
#include <math.h>

#include "../../common/nanokontrol.h"


// Colors for pitches.
int PITCH_COLORS[] = {
    0x210000, 0x211000, 0x212100, 0x102100, 0x002100, 0x002110,
    0x002121, 0x001021, 0x000021, 0x100021, 0x210021, 0x210010};


//--------------------------------------------------------------
void ofApp::setup(){
  mode = 1;
  ofSetVerticalSync(true);
  ofBackground(ofColor::black);
  ofSetLogLevel(OF_LOG_NOTICE);
  ofDisableAntiAliasing();
  setupKeyboard();

  std::string letterFiles[] = {
    M_FILE, U_FILE, S_FILE, I_FILE, C_FILE, O_FILE, D_FILE, E_FILE};
  ofImage fullLogo;
  fullLogo.load(FULL_LOGO_FILE);
  float xOffset = (ofGetWidth() - (fullLogo.getWidth() / 4.0)) / 2.0;
  float yOffset = (ofGetHeight() - fullLogo.getHeight()) / 4.0;
  // The xOffsets computed seem to vary by letter, so we manually adjust.
  float letterMultipliers[] = {0.95, 0.95, 1.0, 1.5, 1.4, 1.3, 1.1, 1.3};
  for (int i = 0; i < 8; ++i) {
    ofImage tmpImage;
    tmpImage.load(letterFiles[i]);
    ofPlanePrimitive p;
    p.setPosition(xOffset, yOffset, 0);
    p.resizeToTexture(tmpImage.getTexture(), 1.0);
    // Make sure the letters start upright.
    p.rotateDeg(180.0, 1.0, 0.0, 0.0);
    if (i >= 5) p.rotateDeg(-90.0, 0.0, 0.0, 1.0);
    xOffset += p.getWidth() * letterMultipliers[i];
    letterImages.push_back(tmpImage);
    letterPlanes.push_back(p);
    drawLetter.push_back(false);
    offsets.push_back(0.0);
    letterColours.push_back(0xffffff);
  }
  // Now put the letters in their initial rotations and set target angles.
  for (int i = 0; i < letterPlanes.size(); ++i) {
    glm::vec2 rotDir;
    if (i % 2 == 0) {
      rotDir = glm::vec2(1.0, 0.0);
    } else {
      rotDir = glm::vec2(0.0, 1.0);
    }
    letterPlanes[i].rotateDeg(90.0, rotDir.x, rotDir.y, 0.0);
    rotationDirections.push_back(rotDir);
    cumulativeDeltas.push_back(0.0);
    targetDelta.push_back(90.0);
  }

  disklavierIn.listInPorts();
  disklavierOut.listOutPorts();
  // You'll want to open the appropriate ports based on the
  // output of the above two commands.
  disklavierIn.openPort(DISKLAVIER_PORT);
  nanoKontrolIn.openPort(NANOKONTROL_PORT);
  // don't ignore sysex, timing, & active sense messages,
  // these are ignored by default
  disklavierIn.ignoreTypes(false, false, false);
  nanoKontrolIn.ignoreTypes(false, false, false);
  // add ofApp as a listener
  disklavierIn.addListener(this);
  nanoKontrolIn.addListener(this);
  disklavierOut.openPort(DISKLAVIER_PORT);

  glBlendEquation(GL_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Set up Box2d
  box2d.init();
  box2d.setGravity(0, GRAVITY);
  box2d.setFPS(60.0);
  box2d.registerGrabbing();
  anchor.setPhysics(0.0, 0.0, 0.0);
  anchor.setup(box2d.getWorld(), ofGetWidth()*3/8, ofGetHeight()*5/8, ANCHOR_RADIUS);
  // We start with a single pendulum.
  Note note;
  note.pitch = -1;
  note.active = true;
  note.prevNoteIndex = -1;
  note.nextNoteIndex = -1;
  note.circle = make_shared<ofxBox2dCircle>();
  note.circle->setPhysics(BALL_DENSITY, 0.53, 0.0);
  auto xPos = ofGetWidth()/2 - JOINT_LENGTH;
  auto yPos = ofGetHeight()/2 - JOINT_LENGTH;
  note.circle->setup(box2d.getWorld(), xPos, yPos, BALL_RADIUS * 0.5);
  note.circle->enableGravity(false);
  // now connect with a joint
  note.joint = make_shared<ofxBox2dJoint>(box2d.getWorld(), anchor.body, note.circle->body);
  note.joint->setLength(JOINT_LENGTH * 1.5);
  note.joint->setFrequency(0);
  note.joint->setDamping(0);
  notes.push_back(note);
  notes[0].circle->addForce(ofVec2f(0.0, 1.0), 20.0);

  lastActiveNoteIndex = 0;
  ofEnableAlphaBlending();
}

void ofApp::setupKeyboard() {
  float whiteWidth = ofGetWidth() / 60;
  float whiteHeight = whiteWidth * 4;
  backboard.y = 0;
  backboard.h = whiteHeight + KEY_SEPARATION;
  backboard.w = whiteWidth * 54 + KEY_SEPARATION * 26;
  backboard.x = (ofGetWidth() - backboard.w) / 2;
  // float xPos = ofGetWidth() - 6 * whiteWidth;
  float xPos = ofGetWidth() - (backboard.x + 2 * whiteWidth);
  int whiteKeyPos = 0;
  int i = 0;
  while (i < 88) {
    pianoKeys[i].time = 0.0;
    pianoKeys[i].active = false;
    pianoKeys[i].white = true;
    pianoKeys[i].x = xPos;
    pianoKeys[i].y = KEY_SEPARATION;
    pianoKeys[i].w = whiteWidth - KEY_SEPARATION;
    pianoKeys[i++].h = whiteHeight - KEY_SEPARATION / 2;
    // We need to special case the lone black key in the lowest partial octave.
    // After that we can do simple mod arithmetic to figure it out.
    int keyPos = (whiteKeyPos - 2) % 7;
    if (whiteKeyPos == 0 || (i < 88 && whiteKeyPos >= 2 && keyPos != 2 && keyPos != 6)) {
      pianoKeys[i].time = 0.0;
      pianoKeys[i].active = false;
      pianoKeys[i].white = false;
      pianoKeys[i].y = KEY_SEPARATION + whiteHeight / 2;
      pianoKeys[i].x = xPos - whiteWidth * 1 / 4;
      pianoKeys[i].w = (whiteWidth - KEY_SEPARATION) / 2;
      pianoKeys[i++].h = (whiteHeight - KEY_SEPARATION) / 2;
    }
    playingKeys[i] = PianoKey(pianoKeys[i]);
    playingKeys[i].y += pianoKeys[i].h;
    playingKeys[i].w = 0.0;
    xPos -= whiteWidth + KEY_SEPARATION / 2;
    whiteKeyPos++;
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
  box2d.update();
  for (int i = 0; i < drawLetter.size(); ++i) {
    if (!drawLetter[i]) continue;
    if (cumulativeDeltas[i] >= targetDelta[i]) continue;
    cumulativeDeltas[i] += SPIN_DELTA;
    letterPlanes[i].rotateDeg(-SPIN_DELTA,
                              rotationDirections[i].x,
                              rotationDirections[i].y,
                              0.0);
  }
  if (finalRotation) {
    if (finalRotateAngle < PI / 2) {
      finalRotateAngle += PI / ROTATE_STEPS;
    } else {
      return;
    }
    const auto& refPos = letterPlanes[4].getPosition();
    for (int i = 5; i < letterPlanes.size(); ++i) {
      if (offsets[i] == 0.0) offsets[i] = letterPlanes[i].getPosition().x - refPos.x;
      letterPlanes[i].setPosition(refPos.x + offsets[i] * cos(finalRotateAngle),
                                  refPos.y + offsets[i] * sin(finalRotateAngle),
                                  0.0);
      letterPlanes[i].rotateDeg(180.0 / ROTATE_STEPS, 0.0, 0.0, 1.0);
    }
  }
}

void ofApp::addPendulumNote(int pitch) {
  int idx = 0;
  int newPosition = 1;
  float currTime = ofGetElapsedTimef();
  for (int i = activeNoteIds.size() - 1; i >= 0; --i) {
    if (currTime - notes[activeNoteIds[i]].onsetTime < TOGETHER_THRESHOLD) {
      idx = activeNoteIds[i];
      newPosition = notes[idx].positionInChain + 1;
      break;
    }
  }
  float joint_l = pitch * 2 / newPosition;
  Note note;
  note.pitch = pitch;
  note.active = true;
  note.onsetTime = ofGetElapsedTimef();
  note.positionInChain = newPosition;
  note.circle = make_shared<ofxBox2dCircle>();
  note.circle->setPhysics(BALL_DENSITY, 0.53, 0.0);
  int noteName = (pitch - 24) % 12;
  float angle = noteName * glm::two_pi<float>() / 12;
  auto xPos = notes[0].circle->getPosition().x + joint_l * cos(angle);
  auto yPos = notes[0].circle->getPosition().y + joint_l * sin(angle);
  note.circle->setup(box2d.getWorld(), xPos, yPos, BALL_RADIUS);
  note.circle->enableGravity(false);
  note.circle->addForce(ofVec2f(sin(angle), cos(angle)), 100.0 / newPosition);
  note.polyline.curveTo(xPos, yPos);
  note.path.moveTo(ofPoint(xPos, yPos));
  note.path.setStrokeWidth(3.0);
  note.joint = make_shared<ofxBox2dJoint>(box2d.getWorld(),
                                          notes[idx].circle->body,
                                          note.circle->body);
  note.joint.get()->setLength(joint_l);
  note.joint.get()->setFrequency(0);
  notes.push_back(note);
  activeNoteIds.push_back(notes.size() - 1);
  // Set active note on keyboard.
  pianoKeys[pitch - 21].active = true;
}

void ofApp::dropPendulumNote(int pitch) {
  int idx = -1;
  for (int i = 0; i < activeNoteIds.size(); ++i) {
    if (notes[activeNoteIds[i]].pitch == pitch) {
      idx = activeNoteIds[i];
      activeNoteIds.erase(activeNoteIds.begin() + i);
      break;
    }
  }
  if (idx > 0) {
    notes[idx].active = false;
    notes[idx].offsetTime = ofGetElapsedTimef();
    //notes[idx].circle->enableGravity(true);
    notes[idx].joint->destroy();
  }
  pianoKeys[pitch - 21].active = false;
}

void ofApp::addNewLetter(int pitch) {
  if (pitch < 62) return;
  for (int i = 0; i < drawLetter.size(); ++i) {
    if (drawLetter[i]) continue;
    drawLetter[i] = true;
    int basePitch = PITCH_COLORS[(pitch - 24) % 12];
    float multiplier = floor((pitch - 3) / 12);
    letterColours[i] = basePitch * multiplier;
    return;
  }
  finalRotation = true;
}

void ofApp::newMidiMessage(ofxMidiMessage& msg) {
  if(msg.status < MIDI_SYSEX) {
    if (msg.status == MIDI_CONTROL_CHANGE) {
      if (VERBOSE) {
        cout << "*** Control message ***" << endl;
        cout << "status: " << msg.status << endl;
        cout << "channel: " << msg.channel << endl;
        cout << "pitch: " << msg.pitch << endl;
        cout << "velocity: " << msg.velocity << endl;
        cout << "control: " << msg.control << endl;
        cout << "value: " << msg.value << endl;
        cout << "portNum: " << msg.portNum << endl;
        cout << "portName: " << msg.portName << endl;
      }
      if (msg.portNum == NANOKONTROL_PORT) {
        if (msg.control == NANO_SET && msg.value == 127) {
          this->reset();
        }
      } else if (msg.portNum == DISKLAVIER_PORT) {
        if (msg.control == SUSTAIN_CTL) {
          sustainValue = msg.value;
          if (sustainValue == 0) {
            for (size_t i = 0; i < 88; ++i) {
              pianoKeys[i].sustained = false;
            }
          }
        }
      }
    } else if (msg.status == MIDI_NOTE_ON || msg.status == MIDI_NOTE_OFF) {
      Note note;
      note.pitch = msg.pitch;
      note.velocity = msg.velocity;
      note.onset = msg.status == MIDI_NOTE_ON;
      if (note.onset) {
        note.onsetTime = ofGetElapsedTimef();
        addPendulumNote(msg.pitch);
        addNewLetter(msg.pitch);
      } else {
        dropPendulumNote(msg.pitch);
      }
      int pitchPos = msg.pitch - 21;
      pianoKeys[pitchPos].active = note.onset;
      pianoKeys[pitchPos].time = note.onsetTime;
    }
  }
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofSetColor(ofColor::white);
  for (int i = 0; i < letterPlanes.size(); ++i) {
    if (!drawLetter[i]) continue;
    ofSetHexColor(letterColours[i]);
    letterImages[i].getTexture().bind();
    letterPlanes[i].draw();
    letterImages[i].getTexture().unbind();
  }
  ofSetHexColor(0x000000);
  ofFill();
  backboard.draw();
  // We have to go through the keys twice to make sure we draw the black ones
  // last.
  for (uint i = 0; i < 88; ++i) {
    if (!pianoKeys[i].white) continue;
    if (pianoKeys[i].active) {
      int basePitch = PITCH_COLORS[(i - 3) % 12];
      float multiplier = floor((i + 21) / 12);
      ofSetHexColor(basePitch * multiplier);
    } else {
      ofSetHexColor(0xffffff);
    }
    ofFill();
    pianoKeys[i].draw();
    // Go through active notes to draw.
    if (playingKeys[i].active) {
      playingKeys[i].draw();
    }
  }
  for (uint i = 0; i < 88; ++i) {
    if (pianoKeys[i].white) continue;
    if (pianoKeys[i].active) {
      int basePitch = PITCH_COLORS[(i - 3) % 12];
      float multiplier = floor((i + 21) / 12);
      ofSetHexColor(basePitch * multiplier);
    } else {
      ofSetHexColor(0x000000);
    }
    ofFill();
    pianoKeys[i].draw();
    // Go through active notes to draw.
    if (playingKeys[i].active) {
      playingKeys[i].draw();
    }
  }
  for (auto &note : notes) {
    ofFill();
    int basePitch = PITCH_COLORS[(note.pitch - 24) % 12];
    float multiplier = floor(note.pitch / 12);
    if (note.pitch >= 0) {
      float elapsedTime = ofGetElapsedTimef() - note.offsetTime;
      if (note.active) {
        ofVec2f currPos = note.circle->getPosition();
        note.polyline.curveTo(currPos[0], currPos[1]);
        vector<glm::vec3> vertices = note.polyline.getVertices();
        int i = vertices.size() - 4;
        if (i >= 0) {
          glm::vec3 vertex = vertices[i];
          glm::vec3 tangent = note.polyline.getTangentAtIndex(i) * TANGENT_LENGTH;
          note.lines.push_back(std::pair<glm::vec3, glm::vec3>(vertex - tangent / 2, vertex + tangent / 2));
          note.path.bezierTo(
                vertices[i + 1].x, vertices[i + 1].y,
                vertices[i + 2].x, vertices[i + 2].y,
                vertices[i + 3].x, vertices[i + 3].y);
        }
      }
      float alpha = note.active ? 200 : std::max(float(0.0), 200 - elapsedTime * float(10.0));
      if (mode == 0) {
        for (auto &line : note.lines) {
          if (!note.active) {
            float zOffset = elapsedTime / 4;
            line.first.z -= zOffset;
            line.second.z -= zOffset;
          }
          ofSetColor(ofColor::fromHex(basePitch * multiplier, alpha));
          ofDrawLine(line.first, line.second);
        }
      } else {
        note.path.setColor(ofColor::fromHex(basePitch * multiplier, alpha));
        note.path.draw();
      }
    }
  }
}

void ofApp::reset() {
  for (size_t i = 0; i < 88; ++i) {
    pianoKeys[i].active = false;
    playingKeys[i].active = false;
    sendStopNote(i + 21, 60);
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
  switch(key) {
    case 'm':
      drawLetter[0] = true;
      break;
    case 'u':
      drawLetter[1] = true;
      break;
    case 's':
      drawLetter[2] = true;
      break;
    case 'i':
      drawLetter[3] = true;
      break;
    case 'c':
      drawLetter[4] = true;
      break;
    case 'o':
      drawLetter[5] = true;
      break;
    case 'd':
      drawLetter[6] = true;
      break;
    case 'e':
      drawLetter[7] = true;
      break;
    case 'q':
      finalRotation = true;
      break;
    case 'x':
      xDir = 1.0;
      yDir = 0.0;
      zDir = 0.0;
      dirText = "x";
      break;
    case 'y':
      xDir = 0.0;
      yDir = 1.0;
      zDir = 0.0;
      dirText = "y";
      break;
    case 'z':
      xDir = 0.0;
      yDir = 0.0;
      zDir = 1.0;
      dirText = "z";
      break;
    case '-':
      spin -= SPIN_DELTA;
      spinText = dirText + "-spin: " + std::to_string(spin);
      break;
    case '+':
      spin += SPIN_DELTA;
      spinText = dirText + "-spin: " + std::to_string(spin);
      break;
    case 'v':
      mode = 1 - mode;
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
