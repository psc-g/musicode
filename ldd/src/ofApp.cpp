#include "ofApp.h"
#include "../../musicode/common/pitchColours.h"

#include <math.h>

LissajousKnot::LissajousKnot() {
  for (float t = 0; t <= KNOT_LIM; t += KNOT_T_INC) {
    ts.push_back(t);
  }
}

void LissajousKnot::setValues(int nx, int ny, int nz, float px, float py, float pz) {
  float mult = min(ofGetHeight(), ofGetWidth()) / 2;
  n = glm::vec3(nx, ny, nz);
  p = glm::vec3(px, py, pz);
  truePath.clear();
  truePath.setColor(ofColor::white);
  truePath.setStrokeWidth(50.0);
  truePath.setFilled(false);
  auto x = cos(p.x) * mult; //  + mult / 2;
  auto y = cos(p.y) * mult; //  + mult / 2;
  auto z = cos(p.z) * mult; //  + mult / 2;
  truePath.moveTo(x, y, z);
  for (uint i = 0; i < ts.size(); ++i) {
    x = cos(n.x * ts[i] + p.x) * mult; //  + mult / 2;
    y = cos(n.y * ts[i] + p.y) * mult; //  + mult / 2;
    z = cos(n.z * ts[i] + p.z) * mult; //  + mult / 2;
    truePath.lineTo(x, y, z);
  }
  learnedP = glm::vec3(float(rand()) / RAND_MAX,
                       float(rand()) / RAND_MAX,
                       float(rand()) / RAND_MAX);
  learnedN = n;
  redrawLearnedPath();
  doGradients = false;
  ready = true;
  loss = 100.0;
}

void LissajousKnot::redrawLearnedPath() {
  float mult = min(ofGetHeight(), ofGetWidth()) / 2;
  learnedPath.clear();
  learnedPath.setColor(ofColor::red);
  learnedPath.setStrokeWidth(50.0);
  learnedPath.setFilled(false);
  auto x = cos(learnedP.x) * mult; //  + mult / 2;
  auto y = cos(learnedP.y) * mult; //  + mult / 2;
  auto z = cos(learnedP.z) * mult; //  + mult / 2;
  learnedPath.moveTo(x, y, z);
  for (uint i = 1; i < ts.size(); ++i) {
    x = cos(learnedN.x * ts[i] + learnedP.x) * mult; //  + mult / 2;
    y = cos(learnedN.y * ts[i] + learnedP.y) * mult; //  + mult / 2;
    z = cos(learnedN.z * ts[i] + learnedP.z) * mult; //  + mult / 2;
    learnedPath.lineTo(x, y, z);
  }
}

void LissajousKnot::update() {
  if (doGradients) gradientStep();
}

void LissajousKnot::draw() {
  if (!ready) return;
  truePath.draw();
  learnedPath.draw();
}

float LissajousKnot::gradientStep() {
  if (loss < 0.000001) {
    return 0.0;
  }
  gradients.clear();
  gradients.resize(3, 0.0);
  float signed_loss = 0.0;
  loss = 0.0;
  std::vector<uint> batch_ids;
  for (uint i = 0; i < BATCH_SIZE; ++i) {
    batch_ids.push_back(rand() % ts.size());
  }
  for (uint i = 0; i < batch_ids.size(); ++i) {
    auto x = cos(n.x * ts[batch_ids[i]] + p.x);
    auto y = cos(n.y * ts[batch_ids[i]] + p.y);
    auto z = cos(n.z * ts[batch_ids[i]] + p.z);
    auto learnedX = cos(learnedN.x * ts[batch_ids[i]] + learnedP.x);
    auto learnedY = cos(learnedN.y * ts[batch_ids[i]] + learnedP.y);
    auto learnedZ = cos(learnedN.z * ts[batch_ids[i]] + learnedP.z);
    float xLoss = learnedX - x;
    float yLoss = learnedY - y;
    float zLoss = learnedZ - z;
    loss += 0.5 * (pow(xLoss, 2.0) + pow(yLoss, 2.0) + pow(zLoss, 2.0));
    signed_loss += xLoss + yLoss + zLoss;
    gradients[0] += xLoss *
                   -sin(learnedN.x * ts[batch_ids[i]] + learnedP.x) / BATCH_SIZE;
    gradients[1] += yLoss *
                    -sin(learnedN.y * ts[batch_ids[i]] + learnedP.y) / BATCH_SIZE;
    gradients[2] += zLoss *
                    -sin(learnedN.z * ts[batch_ids[i]] + learnedP.z) / BATCH_SIZE;
  }
  loss /= BATCH_SIZE;
  signed_loss /= BATCH_SIZE;
  learnedP.x -= learning_rate * gradients[0];
  learnedP.y -= learning_rate * gradients[1];
  learnedP.z -= learning_rate * gradients[2];
  redrawLearnedPath();
  return signed_loss;
}


// ------------------------------------------------------------------

void LearnedLine::setup() {
  for (uint i = 0; i < coeffs.size(); ++i) {
    theta.push_back(((float) rand() * 2 / RAND_MAX) - 1);
  }
  for (uint i = 0; i < num_x; ++i) {
    xs.push_back(((float) rand() * 2 / RAND_MAX) - 1);
    drawXs.push_back(xs[i]);
  }
  std::sort(xs.begin(), xs.end());
  xLims = glm::vec2(xs[0], xs[xs.size() - 1]);
  float yMin = 0.0;
  float yMax = 0.0;
  for (auto x : xs) {
    float y = 0.0;
    float learnedY = 0.0;
    for (uint j = 0; j < coeffs.size(); ++j) {
      y += coeffs[j] * pow(x, coeffs.size() - 1 - j);
      learnedY += theta[j] * pow(x, theta.size() - 1 - j);
    }
    if (y > yMax) yMax = y;
    if (y < yMin) yMin = y;
    if (learnedY > yMax) yMax = learnedY;
    if (learnedY < yMin) yMin = learnedY;
    yLims = glm::vec2(yMin, yMax);
    ys.push_back(y);
    drawYs.push_back(ys[ys.size() - 1]);
    learnedYs.push_back(learnedY);
    drawLearnedYs.push_back(learnedYs[learnedYs.size() - 1]);
  }
}

void LearnedLine::rotate(float r) {
  if (!learning) {
    rotation += r;
    if (r > 0.0) {
      alpha *= 0.999;
    }
  }
  auto midX = (xLims[1] - xLims[0]) / 2;
  auto midY = (yLims[1] - yLims[0]) / 2;
  drawMidX = xScale * midX;
  drawMidY = yScale * midY;
  for (uint i = 0; i < xs.size(); ++i) {
    drawXs[i] = xOffset + xScale * (xs[i] - xLims[0]) / (xLims[1] - xLims[0]) - drawMidX;
    drawXs[i] *= cos(rotation);
    drawXs[i] += drawMidX; 
    drawYs[i] = yOffset + yScale * (ys[i] - yLims[0]) / (yLims[1] - yLims[0]) - drawMidY;
    drawYs[i] += drawYs[i] * sin(rotation) + drawMidY;
    drawLearnedYs[i] = yOffset + yScale * (learnedYs[i] - yLims[0]) / (yLims[1] - yLims[0]) - drawMidY;
    drawLearnedYs[i] += drawLearnedYs[i] * sin(rotation) + drawMidY;
  }
}

void LearnedLine::place(float xscale, float xoff, float yscale, float yoff) {
  xScale = xscale;
  xOffset = xoff;
  yScale = yscale;
  yOffset = yoff;
  rotate(0.0);
}

void LearnedLine::draw() {
  ofPath truePath;
  ofPath learnedPath;
  if (!learning) {
    truePath.setColor(ofColor(80, 80, 80, alpha));
  } else {
    truePath.setColor(ofColor(255, 255, 255, alpha));
  }
  truePath.moveTo(drawXs[0], drawYs[0]);
  learnedPath.setColor(ofColor(255, 0, 0, alpha));
  learnedPath.moveTo(drawXs[0], drawLearnedYs[0]);
  learnedPath.setStrokeWidth(5.0);
  learnedPath.setFilled(false);
  truePath.setStrokeWidth(5.0);
  truePath.setFilled(false);
  for (uint i = 0; i < drawXs.size(); ++i) {
    if (!learning) {
      ofSetColor(ofColor(80, 80, 80, alpha));
    } else {
      ofSetColor(ofColor(255, 255, 255, alpha));
    }
    ofDrawCircle(drawXs[i], drawYs[i], 5);
    ofPoint truePt;
    truePt.set(drawXs[i], drawYs[i]);
    if (!learning) {
      ofSetColor(ofColor(80, 0, 0, alpha));
    } else {
      ofSetColor(ofColor(255, 0, 0, alpha));
    }
    ofDrawCircle(drawXs[i], drawLearnedYs[i], 5);
    ofPolyline line2;
    line2.addVertex(truePt);
    ofPoint learnedPt;
    learnedPt.set(drawXs[i], drawLearnedYs[i]);
    line2.addVertex(learnedPt);
    line2.draw();
    if (i < drawXs.size() - 4) {
      truePath.bezierTo(drawXs[i+1], drawYs[i+1],
                        drawXs[i+2], drawYs[i+2],
                        drawXs[i+3], drawYs[i+3]);
      learnedPath.bezierTo(drawXs[i+1], drawLearnedYs[i+1],
                           drawXs[i+2], drawLearnedYs[i+2],
                           drawXs[i+3], drawLearnedYs[i+3]);
    } else if (i < drawXs.size() - 1) {
      truePath.lineTo(drawXs[i+1], drawYs[i+1]);
      learnedPath.lineTo(drawXs[i+1], drawLearnedYs[i+1]);
    }
  }
  truePath.draw();
  learnedPath.draw();
}

float LearnedLine::gradientStep() {
  if (!learning || loss < THRESHOLD) {
    return 0.0;
  }
  gradients.clear();
  gradients.resize(theta.size(), 0.0);
  float signed_loss = 0.0;
  loss = 0.0;
  for (uint i = 0; i < xs.size(); ++i) {
    float local_loss = learnedYs[i] - ys[i];
    loss += 0.5 * pow(local_loss, 2.0);
    signed_loss += local_loss;
    for (uint j = 0; j < theta.size(); ++j) {
      gradients[j] += local_loss * pow(xs[i], theta.size() - 1 - j) / xs.size();
    }
  }
  loss /= xs.size();
  signed_loss /= xs.size();
  for (uint i = 0; i < theta.size(); ++i) {
    theta[i] -= learning_rate * gradients[i];
  }
  for (uint i = 0; i < xs.size(); ++i) {
    float learnedY = 0.0;
    for (uint j = 0; j < theta.size(); ++j) {
      learnedY += theta[j] * pow(xs[i], theta.size() - 1 - j);
    }
    learnedYs[i] = learnedY;
  }
  return signed_loss;
}

//--------------------------------------------------------------
void ofApp::setup() {
  ofBackground(0, 0, 0);

  cam.setDistance(3000);

  receiver.setup(IN_PORT);
  sender.setup(HOST, OUT_PORT);

  webcam.setDeviceID(1);
  webcam.setup(ofGetWidth(), ofGetHeight(), true);
  image.allocate(webcam.getWidth(), webcam.getHeight());
  for (int i = 0; i < 3; ++i) {
    component_images[i].allocate(webcam.getWidth(), webcam.getHeight());
  }
  red_filter.allocate(webcam.getWidth(), webcam.getHeight());
  red_filter.set(0, 255, 255);
  green_filter.allocate(webcam.getWidth(), webcam.getHeight());
  green_filter.set(255, 0, 255);
  blue_filter.allocate(webcam.getWidth(), webcam.getHeight());
  blue_filter.set(255, 255, 0);

  shader.load("shaders/bender");
  lissajous.reset();
}

void ofApp::addLearnedLine() {
  learned_lines.emplace_back(100, 0.01);
  for (int i = 0; i < 4; ++i) {
    learned_lines[learned_lines.size() - 1].addCoeff(
        ((float) rand() * 2 / RAND_MAX) - 1);
  }
  learned_lines[learned_lines.size() - 1].setup();
  learned_lines[learned_lines.size() - 1].gradientStep();
  float xScale = ofGetWidth();
  learned_lines[learned_lines.size() - 1].place(
      xScale, 0, ofGetHeight(), 0);
}

void ofApp::reset() {
  ofBackground(0, 0, 0);
  started = false;
  learned_lines.clear();
  lissajous.reset();
}

void ofApp::update() {
  webcam.update();
  if (webcam.isFrameNew()) {
    if (mode == 2) {
      image.setFromPixels(webcam.getPixels());
      component_images[0] = image;
      component_images[0] -= red_filter;
      component_images[1] = image;
      component_images[1] -= green_filter;
      component_images[2] = image;
      component_images[2] -= blue_filter;
    }
  }

  std::vector<int> to_erase;
  for (int i = 0; i < circlePositions.size(); ++i) {
    circlePositions[i].z -= 0.2;
    if (circlePositions[i].z < 0) to_erase.push_back(i);
  }
  for (int i : to_erase) circlePositions.erase(circlePositions.begin() + i);

  checkOSCMessages();
  if (mode == 1 || mode == 2) {
    float loss = lissajous.gradientStep();
    ofxOscMessage m;
    m.setAddress("/detuneChord");
    m.addFloatArg(loss);
    for (int n : active_bass_notes) m.addIntArg(n);
    for (int i = active_bass_notes.size(); i < 3; ++i) m.addIntArg(-1);
    sender.sendMessage(m);
    lissajous.update();
  } else {
    for (int i = 0; i < learned_lines.size(); ++i) {
      if (i < learned_lines.size() - 1) {
        learned_lines[i].gradientStep();
      } else if (learned_lines[i].getNotes().size() > 0) {
        float loss = learned_lines[i].gradientStep();
        for (auto n : learned_lines[i].getNotes()) {
          ofxOscMessage m;
          m.setAddress("/detune");
          m.addIntArg(n);
          m.addFloatArg(loss * LOSS_MULTIPLIER);
          sender.sendMessage(m);
        }
      }
      learned_lines[i].rotate(0.05);
    }
  }
}

void ofApp::checkOSCMessages() {
  while(receiver.hasWaitingMessages()) {
    ofxOscMessage m;
    receiver.getNextMessage(m);
    int note = m.getArgAsInt(0);
    int velocity = m.getArgAsInt(1);
    if (m.getAddress() == "/noteon") {
      started = true;
      if (mode == 0 || mode == 3) {
        if (note < 48 || learned_lines.size() == 0) {
          addLearnedLine();
          circlePositions.clear();
        } else if (note >= 48) {
          circlePositions.emplace_back(ofGetWidth() * (float)rand() / RAND_MAX,
                                       ofGetHeight() * (float)rand() / RAND_MAX,
                                       webcam.getHeight() / 8);
        }
        for (int i = 0; i < learned_lines.size(); ++i) {
          if (i < learned_lines.size() - 1) {
            learned_lines[i].setLearning(false);
          } else {
            if (note >= 48) learned_lines[i].addNote(note);
            float loss = learned_lines[i].gradientStep();
            learned_lines[i].rotate(0.05);
            for (auto n : learned_lines[i].getNotes()) {
              ofxOscMessage m;
              m.setAddress("/detune");
              m.addIntArg(n);
              m.addFloatArg(loss * LOSS_MULTIPLIER);
              sender.sendMessage(m);
            }
          }
        }
      } else {
        if (note < 60) {
          active_bass_notes.push_back(note);
          if (ofGetElapsedTimef() - time_of_last_chord > MIN_TIME_BW_CHORDS) {
            time_of_last_chord = ofGetElapsedTimef();
            lissajous.setValues(rand() % 9, rand() % 9, rand() % 9,
                                (float)rand() / RAND_MAX,
                                (float)rand() / RAND_MAX,
                                (float)rand() / RAND_MAX);
            orbit = true;
          }
        }
      }
    } else if (m.getAddress() == "/noteoff") {
      if ((mode == 0 || mode == 3) && learned_lines.size() > 0) {
        int to_erase = -1;
        auto notes = learned_lines[learned_lines.size() - 1].getNotes();
        for (int i = 0; i < notes.size(); ++i) {
          if (notes[i] == note) {
            to_erase = i;
            break;
          }
        }
        if (to_erase >= 0) learned_lines[learned_lines.size() - 1].deleteNote(to_erase);
      } else if ((mode == 1 || mode == 2) && note < 60) {
        active_bass_notes.clear();
      }
    } else if (m.getAddress() == "/ccEvent" && note == 67) {
      int channel = m.getArgAsInt(2);
      if (listening_to_pedal && velocity > 0) {
        toggleMode();
        listening_to_pedal = false;
      } else if (velocity == 0) {
        listening_to_pedal = true;
      }
    }
  }
}

void ofApp::draw() {
  if (!started) return;

  glm::vec2 center = glm::vec2((ofGetWidth() - webcam.getWidth()) / 2,
                               (ofGetHeight() - webcam.getHeight()) / 2);
  if (mode == 1 || mode == 2) {
    std::vector<float> displacements(3, 0.0);
    if (lissajous.isReady()) {
      displacements = lissajous.getGradients();
    }
    if (mode == 2) {
      ofSetColor(ofColor(255, 255, 255, 100));
      float p = (float)rand() / RAND_MAX;
      if (p < lissajous.getLoss() * 100) {
        int shift = rand() % 3;
        for (int i = shift; i < shift + 3; ++i) {
          int pos = i % 3;
          component_images[pos].draw(
              center.x + displacements[pos] * 100, center.y + displacements[pos] * 100);
        }
      } else {
        image.draw(center.x, center.y);
      }
    }
  } else if (mode == 3) {
    ofSetColor(255, 255, 255, 80);
    webcam.draw(0, 0, ofGetWidth(), ofGetHeight());
    std::vector<float> displacements(3, 0.0);
    if (learned_lines.size() > 0) {
      displacements = learned_lines[learned_lines.size() - 1].getGradients();
      for (int i = 0; i < 3; ++i) {
        if (displacements[i] < -0.1) displacements[i] = -0.1;
        else if (displacements[i] > 0.1) displacements[i] = 0.1;
      }
    }
    shader.begin();
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform3f("gradients",
                        glm::vec3(displacements[0],
                                  displacements[1],
                                  displacements[2]));
    shader.setUniform1f("height", webcam.getHeight());
    shader.setUniformTexture("tex0", webcam.getTexture(), 0);
    for (auto circle : circlePositions) {
      ofSetColor(255, 255, 255, circle.z);
      ofDrawCircle(circle.x, circle.y, circle.z);
    }
    shader.end();
  }

  cam.begin();
  ofTranslate(0, 0);
  if (mode == 0 || mode == 3) {
    for (int i = 0; i < learned_lines.size(); ++i) {
      learned_lines[i].draw();
    }
  }
  ofPushMatrix();
  if (orbit) {
    ofTranslate(glm::vec3(500, 500, 0));
    orbitRad += 1.0;
    ofRotateXDeg(orbitRad);
    ofRotateZDeg(orbitRad);
  }
  lissajous.draw();
  ofPopMatrix();
  cam.end();
}

void ofApp::toggleMode() {
  mode = (mode + 1) % 4;
  if (mode == 1 || mode == 2) {
    orbit = false;
    learned_lines.clear();
    circlePositions.clear();
    cam.reset();
    cam.setPosition(599.875, 696.504, 2934.58);
  } else {
    lissajous.reset();
  }
}

void ofApp::keyPressed (int key) {
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){ 
  if (key == 'f') {
    ofxOscMessage m;
    m.setAddress("/fit");
    m.addIntArg(0);
    m.addIntArg(0);
    sender.sendMessage(m);
  } else if (key == 'n') {
    addLearnedLine();
  } else if (key == 'r') {
    reset();
  } else if (key == 'p' || key == 'q') {
    int dir = key == 'p' ? 1 : -1;
    int last_pos = learned_lines.size() - 1;
    float xScale = ofGetWidth() / 2;
    float xOffset = ofGetWidth() / 4;
    learned_lines[last_pos].place(
        xScale, 0, ofGetHeight() / 2, 0);
        //xScale, xOffset, ofGetHeight() / 2, ofGetHeight() / 4);
  } else if (key == 'm') {
    toggleMode();
  } else if (key == 'j') {
    if (lissajous_pos == 0) {
      lissajous.setValues(3, 4, 7, 0.1, 0.7, 0.0);
    } else if (lissajous_pos == 1) {
      lissajous.setValues(3, 2, 7, 0.7, 0.2, 0.0);
    } else if (lissajous_pos == 2) {
      lissajous.setValues(3, 2, 5, 1.7, 0.2, 0.0);
    } else if (lissajous_pos == 3) {
      lissajous.setValues(3, 5, 7, 0.7, 1.2, 0.0);
    } else if (lissajous_pos == 4) {
      lissajous.setValues(3, 5, 7, 0.7, 1.2, 0.0);
    } else if (lissajous_pos == 5) {
      lissajous.setValues(4, 5, 41, 0.01, 0.16, 0.0);
    }
    lissajous_pos = (lissajous_pos + 1) % 6;
  } else if (key == 'l') {
    lissajous.setValues(rand() % 9, rand() % 9, rand() % 9,
                        (float)rand() / RAND_MAX,
                        (float)rand() / RAND_MAX,
                        (float)rand() / RAND_MAX);
    orbit = true;
  } else if (key == 'k') {
    lissajous.computeGradients(true);
  } else if (key == 'c') {
    orbit = !orbit;
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
