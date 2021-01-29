#include "ofApp.h"
#include <iostream>
#include <string>
#include <math.h>

#include "../../common/pitchColours.h"


PlayedMesh::PlayedMesh(PianoKey p) {
  x = p.x;
  targetX = p.targetX;
  y = p.y;
  targetY = p.targetY;
  w = p.w;
  h = p.h;
  white = p.white;
  active = p.active;
  copyNote(p.note);
  int basePitch = PITCH_COLOURS[note.pitch % 12];
  float multiplier = floor(note.pitch / 12);
  hexColor = basePitch * multiplier;
  direction = note.pitch >= 60 ? 1 : -1;
  alpha = 200.0;
  xIntercept = ofGetWidth() / 2;
  yIntercept = ofGetHeight() / 2;
  yShift = (yIntercept - y) * LINE_SPEED / (xIntercept - x - w);
  sphere.setRadius(0);
  sphere.setResolution(32);
  theta = ((float) rand() / RAND_MAX) * 2 * PI;
  phi = ((float) rand() / RAND_MAX) * 2 * PI;
  deltaTheta = (((float) rand() / RAND_MAX) - 1) * SPHERE_ANGLE_THETA;
  deltaPhi = (((float) rand() / RAND_MAX) - 1) * SPHERE_ANGLE_THETA;
  this->update(false);
}

bool PlayedMesh::update(bool playbackRequested) {
  if (!replaying && w > 0.0) {
    int num_xs = 0;
    int num_ys = 0;
    mesh.clear();
    for (float ypos = y; ypos < y + h; ypos += MESH_GRANULARITY) {
      num_ys++;
      for (float xpos = x; xpos < x + w; xpos += MESH_GRANULARITY) {
        auto meshX = xpos;
        auto meshY = ypos;
        auto meshZ = 0.0;
        mesh.addVertex(ofPoint(meshX, meshY, meshZ));
        mesh.addColor(ofColor::fromHex(hexColor, alpha));
        if (ypos == y) num_xs++;
      }
    }
    for (int j = 0; j < num_ys - 1; ++j) {
      for (int i = 0; i < num_xs - 1; ++i) {
        mesh.addIndex(i + j * num_xs);
        mesh.addIndex((i + 1) + j * num_xs);
        mesh.addIndex(i + (j + 1) * num_xs);
        mesh.addIndex((i + 1) + j * num_xs);
        mesh.addIndex((i + 1) + (j + 1) * num_xs);
        mesh.addIndex(i + (j + 1) * num_xs);
      }
    }
  }
  theta += deltaTheta;
  phi += deltaPhi;
  sphereX = xIntercept + OUTER_SPHERE_RADIUS * sin(phi) * cos(theta);
  sphereY = yIntercept + OUTER_SPHERE_RADIUS * sin(phi) * sin(theta);
  sphereZ = OUTER_SPHERE_RADIUS * cos(phi);
  sphere.setPosition(sphereX, sphereY, sphereZ);
  if (!replaying && x < xIntercept - w) {
    if (playbackRequested) {
      x += LINE_SPEED * 10;
      y += yShift * 10;
    } else {
      x += LINE_SPEED;
      y += yShift;
    }
  } else if (!replaying && w > 0.0) {
    growing = true;
    drawSphere = true;
    float delta = x - xIntercept + w;
    w -= delta;
    sphere.setRadius(std::min(sphere.getRadius() + delta * SPHERE_MULTIPLIER,
                              MAX_SPHERE_RADIUS));
    if (playbackRequested) {
      x += LINE_SPEED * 6;
    } else {
      x += LINE_SPEED;
    }
  } else if (replaying) {
    float radius = finalSphereRadius * (1.0 - (ofGetElapsedTimef() - note.time) / note.duration);
    if (radius < 0.001) {
      radius = 0.0;
      drawSphere = false;
    }
    sphere.setRadius(radius);
  } else {
    growing = false;
    return playbackRequested;
  }
  return false;
}

void PlayedMesh::draw() {
  if (!replaying) {
    mesh.draw();
  }
  ofSetHexColor(hexColor);
  if (growing) {
    ofPoint center;
    center.set(xIntercept, yIntercept, 0);
    ofPoint end;
    end.set(sphereX, sphereY, sphereZ);
    ofPolyline line;
    line.addVertex(center);
    line.addVertex(end);
    line.draw();
  } else if (replaying && drawSphere) {
    ofPoint start;
    start.set(sphereX, sphereY, sphereZ);
    ofPoint end;
    end.set(targetX, targetY, 0.0);
    ofPolyline line;
    line.addVertex(start);
    line.addVertex(end);
    line.draw();
  }
  if (drawSphere) {
    sphere.draw();
  }
}

void Echo::update() {
  bool callStartPlayback = true;
  for (size_t i = 0; i < playedMeshes.size(); ++i) {
    callStartPlayback &= playedMeshes[i].update(playbackRequested);
    if (playingBack && playedMeshes[i].sphere.getRadius() == 0.0) {
      playedMeshes.erase(playedMeshes.begin() + i);
    }
  }
  if (playbackRequested && callStartPlayback) {
    playingBack = true;
    playbackRequested = false;
    startPlayback(ofGetElapsedTimef(), reverseRequested, negativeRequested);
  }
}

void Echo::addNote(PianoKey p) {
  if (playingBack) return;
  if (firstNote < 0.0) firstNote = p.note.time;
  playedMeshes.emplace_back(p);
  timeDelta = p.note.time;
};

void Echo::startPlayback(float t, bool reverse, bool negative) {
  if (playingBack) {
    for (size_t i = 0; i < playedMeshes.size(); ++i) {
      if (reverse) {
        playedMeshes[i].note.time = t * 2 - playedMeshes[i].note.time; // - firstNote;
      } else {
        playedMeshes[i].note.time += t + 1.0 - firstNote;
      }
      if (negative) {
        // Hardcoded, parameterize.
        // float pivot = 63.5;  // C pivot
        // float pivot = 68.5;  // F pivot
        float pivot = 58.5;  // G pivot
        playedMeshes[i].note.pitch = int(pivot * 2) - playedMeshes[i].note.pitch;
      }
    }
  } else {
    playbackRequested = true;
    reverseRequested = reverse;
    negativeRequested = negative;
  }
}

std::vector<Note> Echo::getNotes(float time) {
  std::vector<Note> notesToPlay;
  if (!playingBack) return notesToPlay;
  for (size_t i = 0; i < playedMeshes.size(); ++i) {
    auto& note = playedMeshes[i].note;
    if (!playedMeshes[i].replaying && time > note.time) {
      notesToPlay.emplace_back(note.time, true, note.pitch, note.velocity);
      notesToPlay.emplace_back(note.time + note.duration, false, note.pitch, note.velocity);
      playedMeshes[i].replaying = true;
      playedMeshes[i].finalSphereRadius = playedMeshes[i].sphere.getRadius();
    }
  }
  return notesToPlay;
};

void Echo::draw() {
  for (auto& k : playedMeshes) {
    k.draw();
  }
}

//--------------------------------------------------------------
void ofApp::setup(){
  ofSetVerticalSync(true);
  ofBackground(ofColor::white);
  ofSetLogLevel(OF_LOG_NOTICE);
  ofDisableAntiAliasing();
  setupKeyboards();

  receiver.setup(IN_PORT);
  sender.setup(HOST, OUT_PORT);

  ofSetSmoothLighting(true);
  pointLight1.setDiffuseColor(ofFloatColor(.85, .85, .55) );
  pointLight1.setSpecularColor(ofFloatColor(1.f, 1.f, 1.f));
  pointLight2.setDiffuseColor(ofFloatColor( 238.f/255.f, 57.f/255.f, 135.f/255.f ));
  pointLight2.setSpecularColor(ofFloatColor(.8f, .8f, .9f));
  pointLight3.setDiffuseColor(ofFloatColor(19.f/255.f,94.f/255.f,77.f/255.f));
  pointLight3.setSpecularColor(ofFloatColor(18.f/255.f,150.f/255.f,135.f/255.f));

  glBlendEquation(GL_ADD);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ofApp::setupKeyboards() {
  float whiteWidth = ofGetHeight() / 60;
  float whiteHeight = whiteWidth * 4;
  backboard.w = whiteHeight + KEY_SEPARATION;
  backboard.h = whiteWidth * 54 + KEY_SEPARATION * 26;
  backboard.y = (ofGetHeight() - backboard.h) / 2;
  backboard.x = 0.0;
  backboard2.w = whiteHeight + KEY_SEPARATION;
  backboard2.h = whiteWidth * 54 + KEY_SEPARATION * 26;
  backboard2.y = (ofGetHeight() - backboard.h) / 2;
  float rightX = ofGetWidth() - backboard2.w;
  backboard2.x = rightX - KEY_SEPARATION / 2;
  float yPos = backboard.y + whiteWidth;
  float yPos2 = backboard2.y + backboard2.h - whiteWidth * 2;
  int whiteKeyPos = 0;
  int i = 0;
  while (i < 88) {
    // Left keyboard
    pianoKeys[i].active = false;
    pianoKeys[i].white = true;
    pianoKeys[i].x = 0;
    pianoKeys[i].y = yPos;
    pianoKeys[i].h = whiteWidth - KEY_SEPARATION;
    pianoKeys[i].w = whiteHeight - KEY_SEPARATION / 2;
    // Right keyboard
    pianoKeys2[i].active = false;
    pianoKeys2[i].white = true;
    pianoKeys2[i].x = rightX;
    pianoKeys2[i].y = yPos2;
    pianoKeys2[i].h = whiteWidth - KEY_SEPARATION;
    pianoKeys2[i].w = whiteHeight - KEY_SEPARATION / 2;
    i++;
    // We need to special case the lone black key in the lowest partial octave.
    // After that we can do simple mod arithmetic to figure it out.
    int keyPos = (whiteKeyPos - 2) % 7;
    if (whiteKeyPos == 0 || (i < 88 && whiteKeyPos >= 2 && keyPos != 2 && keyPos != 6)) {
      // Left keyboard
      pianoKeys[i].active = false;
      pianoKeys[i].white = false;
      pianoKeys[i].x = whiteHeight / 2;
      pianoKeys[i].y = yPos + KEY_SEPARATION + whiteWidth / 2;
      pianoKeys[i].h = (whiteWidth + KEY_SEPARATION)/ 2;
      pianoKeys[i].w = (whiteHeight - KEY_SEPARATION) / 2;
      // Right keyboard
      pianoKeys2[i].active = false;
      pianoKeys2[i].white = false;
      pianoKeys2[i].x = rightX;
      pianoKeys2[i].y = yPos2 - KEY_SEPARATION - whiteWidth / 4;
      pianoKeys2[i].h = (whiteWidth + KEY_SEPARATION)/ 2;
      pianoKeys2[i].w = (whiteHeight - KEY_SEPARATION) / 2;
      i++;
    }
    yPos += whiteWidth + KEY_SEPARATION / 2;
    yPos2 -= whiteWidth + KEY_SEPARATION / 2;
    whiteKeyPos++;
  }
}

void ofApp::sendPlayNote(int note, int velocity) {
  ofxOscMessage m;
  m.setAddress("/playnote");
  m.addIntArg(note);
  m.addIntArg(velocity);
  sender.sendMessage(m);
}

void ofApp::sendStopNote(int note, int velocity) {
  ofxOscMessage m;
  m.setAddress("/stopnote");
  m.addIntArg(note);
  m.addIntArg(velocity);
  sender.sendMessage(m);
}

void ofApp::sendCCMessage(int channel, int control, int value) {
}

//--------------------------------------------------------------
void ofApp::update() {
  // check for OSC messages
  while(receiver.hasWaitingMessages()) {
    ofxOscMessage m;
    receiver.getNextMessage(m);
    if ((m.getAddress() == "/noteon" || m.getAddress() == "/noteoff") &&
        (!echo.playingBack)) {
      Note note;
      note.time = ofGetElapsedTimef();
      note.pitch = m.getArgAsInt(0);
      note.velocity = m.getArgAsInt(1);
      note.onset = m.getAddress() == "/noteon";
      int pitchPos = note.pitch - 21;
      pianoKeys[pitchPos].active = note.onset;
      pianoKeys2[pitchPos].active = note.onset;
      if (note.onset) {
        playingKeys[pitchPos] = PianoKey(pianoKeys[pitchPos]);
        playingKeys[pitchPos].copyNote(note);
        playingKeys[pitchPos].x += pianoKeys[pitchPos].w;
        playingKeys[pitchPos].w = 0.0;
      } else {
        float duration = ofGetElapsedTimef() - playingKeys[pitchPos].note.time;
        float time = playingKeys[pitchPos].note.time;
        playingKeys[pitchPos].copyNote(note);
        playingKeys[pitchPos].note.time = time;
        playingKeys[pitchPos].note.duration = duration;
        playingKeys[pitchPos].targetX = pianoKeys2[pitchPos].x;
        playingKeys[pitchPos].targetY = pianoKeys2[pitchPos].y;
        echo.addNote(playingKeys[pitchPos]);
        playingKeys[pitchPos].active = false;
      }
    }
  }
  pointLight1.setPosition((ofGetWidth()*.5)+ cos(ofGetElapsedTimef()*.5)*(ofGetWidth()*.3), ofGetHeight()/2, 500);
  pointLight2.setPosition((ofGetWidth()*.5)+ cos(ofGetElapsedTimef()*.15)*(ofGetWidth()*.3),
              ofGetHeight()*.5 + sin(ofGetElapsedTimef()*.7)*(ofGetHeight()), -300);

  pointLight3.setPosition(
              cos(ofGetElapsedTimef()*1.5) * ofGetWidth()*.5,
              sin(ofGetElapsedTimef()*1.5f) * ofGetWidth()*.5,
              cos(ofGetElapsedTimef()*.2) * ofGetWidth()
  );
  auto currTime = ofGetElapsedTimef();
  echo.update();
  // Check if there are any notes from the echo module.
  auto echoNotes = echo.getNotes(currTime);
  noteQueue.insert(noteQueue.end(), echoNotes.begin(), echoNotes.end());
  // Go through current notes in queue to see if any need to be played.
  for (size_t i = 0; i < noteQueue.size(); ++i) {
    auto& note = noteQueue[i];
    if (note.time <= currTime) {
      int pitchPos = note.pitch - 21;
      if (note.onset) {
        sendPlayNote(note.pitch, note.velocity);
        pianoKeys2[pitchPos].active = true;
        timeDelta = note.time;
      } else {
        sendStopNote(note.pitch, note.velocity);
        pianoKeys2[pitchPos].active = false;
      }
      noteQueue.erase(noteQueue.begin() + i);
    }
  }
  // Go through active notes to draw.
  for (size_t i = 0; i < 88; ++i) {
    if (playingKeys[i].active) {
      playingKeys[i].update();
    }
  }
}

void ofApp::draw() {
  ofSetHexColor(0x000000);
  ofFill();
  backboard.draw();
  backboard2.draw();
  // We have to go through the keys twice to make sure we draw the black ones
  // last.
  // Left keyboard
  for (uint i = 0; i < 88; ++i) {
    if (!pianoKeys[i].white) continue;
    if (pianoKeys[i].active) {
      int basePitch = PITCH_COLOURS[(i - 3) % 12];
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
      int basePitch = PITCH_COLOURS[(i - 3) % 12];
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
  // Right keyboard
  for (uint i = 0; i < 88; ++i) {
    if (!pianoKeys2[i].white) continue;
    if (pianoKeys2[i].active) {
      int basePitch = PITCH_COLOURS[(i - 3) % 12];
      float multiplier = floor((i + 21) / 12);
      ofSetHexColor(basePitch * multiplier);
    } else {
      ofSetHexColor(0xffffff);
    }
    ofFill();
    pianoKeys2[i].draw();
  }
  for (uint i = 0; i < 88; ++i) {
    if (pianoKeys2[i].white) continue;
    if (pianoKeys2[i].active) {
      int basePitch = PITCH_COLOURS[(i - 3) % 12];
      float multiplier = floor((i + 21) / 12);
      ofSetHexColor(basePitch * multiplier);
    } else {
      ofSetHexColor(0x000000);
    }
    ofFill();
    pianoKeys2[i].draw();
  }
  ofEnableLighting();
  pointLight1.enable();
  pointLight2.enable();
  pointLight3.enable();
  echo.draw();
  ofDisableLighting();
}

void ofApp::reset() {
  echo.reset();
  noteQueue.clear();
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
  int testPitch = int(rand() % 88);
  switch(key) {
    case 'r':
      reset();
      break;
    case 'p':
      echo.startPlayback(ofGetElapsedTimef(), false, false);
      break;
    case 'q':
      echo.startPlayback(ofGetElapsedTimef(), true, false);
      break;
    case 'v':
      echo.startPlayback(ofGetElapsedTimef(), false, true);
      break;
    case 'x':
      echo.startPlayback(ofGetElapsedTimef(), true, true);
      break;
    case 'n':
      pianoKeys[testPitch].active = true;
      playingKeys[testPitch] = PianoKey(pianoKeys[testPitch]);
      playingKeys[testPitch].x += pianoKeys[testPitch].w;
      playingKeys[testPitch].w = 0.0;
      playingKeys[testPitch].note.time = ofGetElapsedTimef();
      playingKeys[testPitch].note.onset = true;
      playingKeys[testPitch].note.pitch = testPitch + 21;
      playingKeys[testPitch].note.velocity = 60;
      break;
    case 'm':
      for (size_t i = 0; i < 88; ++i) {
        if (pianoKeys[i].active) {
          pianoKeys[i].active = false;
          echo.addNote(playingKeys[i]);
          playingKeys[i].active = false;
        }
      }
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
