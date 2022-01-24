#include "ofApp.h"
#include <algorithm>
#include "../../common/pitchColours.h"

int sign(float x) {
  return (x > 0.0f) - (x < 0.0f);
}

FlameStrip::FlameStrip(float x, int s) : xPos(x), seed(s) {
  for (int i = 0; i < numPaths; ++i) paths.emplace_back();
}

void FlameStrip::setNumPaths(int np) {
  numPaths = np;
  paths.clear();
  for (int i = 0; i < numPaths; ++i) paths.emplace_back();
}

void FlameStrip::draw() {
  if (lifespan < 0.0) return;
  for (int i = 0; i < numPaths; ++i) {
    paths[i].setFilled(false);
    paths[i].setStrokeColor(ofColor(red, green, blue, lifespan * alphaMultiplier * 255 / numPaths));
    paths[i].setStrokeWidth(1);
    paths[i].draw();
  }
}

void FlameStrip::update() {
  if (lifespan < 0.0) return;
  float startY = -500; //-yLength / 2;
  float yDelta = yLength / numPoints;
  float time = ofGetElapsedTimef() * timeMultiplier * yLength;
  float width = xWidth * yLength;
  coarseXWidth = yLength / 5;
  coarseYDivider = yLength * coarseYDivider;
  coarseXFreq = coarseXFreq / yLength;
  for (int i = 0; i < numPaths; ++i) {
    paths[i].clear();
    float y = startY;
    float xOffset = (ofNoise(glm::vec2(y / coarseYDivider, seed * time * coarseXFreq)) - 0.5) * coarseXWidth;
    float x = xPos + xOffset;
    paths[i].moveTo(x, y);
    for (int j = 0; j < numPoints; ++j) {
      y += yDelta;
      xOffset = (ofNoise(glm::vec2(y / coarseYDivider, seed * time * coarseXFreq)) - 0.5) * coarseXWidth;
      float localOffset = width * i * (ofNoise(glm::vec3(x, seed * y / 10, time)) - 0.5) / numPaths;
      //localOffset *= (numPoints - std::abs(numPoints / 2 - j))  / numPoints;
      if (j < numPoints / 10 || j > (numPoints - numPoints / 10)) {
        float multiplier = float(j);
        multiplier = std::min(std::abs(multiplier), std::abs(float(numPoints) - multiplier));
        multiplier *= 10.0 / float(numPoints);
        localOffset *= multiplier;
      }
      x = xPos + xOffset + localOffset;
      paths[i].lineTo(x, y);
    }
  }
  lifespan -= 0.008;
}


//--------------------------------------------------------------
WavyCircle::WavyCircle(float r, int c) : radius(r), color(c) {
  for (int i = 0; i < numPaths; ++i) paths.emplace_back();
  reset();
}

void WavyCircle::reset() {
  for (int i = 0; i < numPaths; ++i) {
    paths[i].clear();
    paths[i].moveTo(0, 0);
    paths[i].arc(0, 0, radius, radius, 0, 360);
  }
  timeActive = 0.0;
}

void WavyCircle::setActive(bool a) {
  active = a;
  if (active) startActive = ofGetElapsedTimef();
}

void WavyCircle::update() {
  if (!active && zMultiplier < zThreshold) return;
  if (active) {
    zMultiplier = 1.0;
  } else {
    zMultiplier -= zDelta;
    if (zMultiplier < zThreshold) {
      reset();
      return;
    }
  }
  for (int i = 0; i < numPaths; ++i) {
    paths[i].clear();
    for (int j = 0; j < 360; ++j) {
      float theta = (j * PI) / 180;
      float phi = (j * 10 * PI) / 180;
      float offset = 20 * (i / numPaths) - (numPaths / 2);
      if (j == 0) {
        paths[i].moveTo(offset + cos(theta) * radius,
                        offset + sin(theta) * radius,
                        zMultiplier * (offset + cos(phi * ofGetElapsedTimef()) * radius / 4));
      } else {
        paths[i].lineTo(offset + cos(theta) * radius,
                        offset + sin(theta) * radius,
                        zMultiplier * (offset + cos(phi * ofGetElapsedTimef()) * radius / 4));
                        //offset + timeActive * j * sin(ofGetElapsedTimef()) + cos(phi * ofGetElapsedTimef()) * radius / 4);
      }
    }
  }
  timeActive += ofGetElapsedTimef() - startActive;
}

void WavyCircle::draw() {
  for (int i = 0; i < numPaths; ++i) {
    paths[i].setFilled(false);
    if (useColors) paths[i].setStrokeHexColor(color);
    else paths[i].setStrokeColor(ofColor(245, 167, 66, 255 / numPaths));
    paths[i].setStrokeWidth(1);
    paths[i].draw();
  }
}

//--------------------------------------------------------------
void ofApp::setup() {
  ofBackground(0, 0, 0);
  gui.setup();
  gui.add(uiIncludeCircles.set("circles", false));
  gui.add(uiIncludeFlames.set("flames", true));
  gui.add(uiNumPaths.set("num_paths", 100, 1, 1000));
  gui.add(uiNumPoints.set("num_points", 200, 1, 1000));
  gui.add(uiYLength.set("y_length", 1000, 100, 10000));
  gui.add(uiXWidth.set("x_width", 0.2, 0.01, 2));
  gui.add(uiTimeMultiplier.set("time_multiplier", 0.1, 0.01, 10));
  gui.add(uiRed.set("red", 245, 0, 255));
  gui.add(uiGreen.set("green", 167, 0, 255));
  gui.add(uiBlue.set("blue", 66, 0, 255));
  gui.add(uiAlphaMultiplier.set("alpha", 4.0, 0.01, 10.0));
  gui.add(uiCoarseXWidth.set("coarseXWidth", 10.0, 1.0, 100.0));
  gui.add(uiCoarseXFreq.set("coarseXFreq", 1.0, 0.01, 10.0));
  gui.add(uiCoarseYDivider.set("coarseYDivider", 0.1, 0.2, 50.0));
  gui.add(uiStartXPos.set("startX", -500, -1000.0, 1000.0));
  gui.add(uiOnsetThreshold.set("onsetThresh", 0.1, 0.1, 10.0));
  gui.add(uiTextureScale.set("scale", -1.0, -10.0, 10.0));
  gui.add(uiRotation.set("rotation", ofVec4f(0), ofVec4f(-2 * PI), ofVec4f(2 * PI)));
  gui.add(uiEndRotation.set("endRotation", true));
  for (int i = 0; i < 88; ++i) {
    int basePitch = PITCH_COLOURS[(i + 10) % 12];
    float multiplier = floor((i + 10) / 12);
    int hexColor = basePitch * multiplier;
    circles.emplace_back(WavyCircle((88 - i) * 5, hexColor));
  }

  vidGrabber.listDevices();
  vidGrabber.setDeviceID(1);
  vidGrabber.setup(VIDEO_WIDTH, VIDEO_HEIGHT, true);
  const auto cellWidth = ofGetWidth() / GRID_SIZE;
  const auto cellHeight = ofGetHeight() / GRID_SIZE;
  pixelCellWidth = vidGrabber.getWidth() / GRID_SIZE;
  pixelCellHeight = vidGrabber.getHeight() / GRID_SIZE;
  float xOffset = (ofGetWidth() - VIDEO_WIDTH) / 2.0;
  float yOffset = (ofGetHeight() - VIDEO_HEIGHT) / 2.0;
  for (int i = 1; i <= GRID_SIZE; ++i) {
    std::vector<ofPlanePrimitive> planeCol;
    std::vector<glm::vec3> planePos;
    std::vector<glm::quat> planeOrient;
    std::vector<glm::quat> planeOrientDelta;
    std::vector<std::vector<unsigned char>> pixelsCol;
    std::vector<ofTexture> textureCol;
    for (int j = 1; j <= GRID_SIZE; ++j) {
      ofPlanePrimitive p;
      glm::vec3 pos = glm::vec3(xOffset + pixelCellWidth * i, yOffset + pixelCellHeight * j, 0);
      p.setPosition(pos);
      planePos.push_back(pos);
      auto q = glm::quat(0.0, 0.0, 0.0, 0.0);
      p.setGlobalOrientation(q);
      planeOrient.push_back(q);
      auto q_delta = glm::quat(
          0.0,
          0.01 * ((float)(2 * (rand() % 2)) - 1.0),
          0.01 * ((float)(2 * (rand() % 2)) - 1.0),
          0.01 * ((float)(2 * (rand() % 2)) - 1.0));
      planeOrientDelta.push_back(q_delta);
      planeCol.push_back(p);
      pixelsCol.push_back(
          std::vector<unsigned char>(pixelCellWidth * pixelCellHeight * 3));
      ofTexture tmpTexture;
      tmpTexture.allocate(pixelCellWidth, pixelCellHeight, GL_RGB);
      textureCol.push_back(tmpTexture);
    }
    planes.push_back(planeCol);
    origPlanePositions.push_back(planePos);
    planeOrientations.push_back(planeOrient);
    planeOrientationDeltas.push_back(planeOrientDelta);
    splitPixels.push_back(pixelsCol);
    splitTextures.push_back(textureCol);
  }

  receiver.setup(IN_PORT);
  sender.setup(HOST, OUT_PORT);
}

//--------------------------------------------------------------
void ofApp::update() {
  checkOSCMessages();
  uiIncludeCircles = (mode == 1 || mode == 2);
  uiIncludeFlames = mode == 0;
  if (mode == 0) {
    circlesAngle = 90;
    for (int i = 0; i < flameStrips.size(); ++i) {
      if (uiNumPaths != flameStrips[i].numPaths) flameStrips[i].setNumPaths(uiNumPaths);
      if (uiNumPoints != flameStrips[i].numPoints) flameStrips[i].numPoints = uiNumPoints;
      if (uiYLength != flameStrips[i].yLength) flameStrips[i].yLength = uiYLength;
      if (uiXWidth != flameStrips[i].xWidth) flameStrips[i].xWidth = uiXWidth;
      if (uiTimeMultiplier != flameStrips[i].timeMultiplier) flameStrips[i].timeMultiplier = uiTimeMultiplier;
      if (uiRed != flameStrips[i].red) flameStrips[i].red = uiRed;
      if (uiGreen != flameStrips[i].green) flameStrips[i].green = uiGreen;
      if (uiBlue != flameStrips[i].blue) flameStrips[i].blue = uiBlue;
      if (uiAlphaMultiplier != flameStrips[i].alphaMultiplier) flameStrips[i].alphaMultiplier = uiAlphaMultiplier;
      if (uiCoarseXWidth != flameStrips[i].coarseXWidth) flameStrips[i].coarseXWidth = uiCoarseXWidth;
      if (uiCoarseXFreq != flameStrips[i].coarseXFreq) flameStrips[i].coarseXFreq = uiCoarseXFreq;
      if (uiCoarseYDivider != flameStrips[i].coarseYDivider) flameStrips[i].coarseYDivider = uiCoarseYDivider;
      flameStrips[i].updateXPos(flameStrips[i].xPos + 10);
      flameStrips[i].update();
    }
  } else if (mode == 1 || mode == 2) {
    if (circlesAngle != 90 || mode == 2) circlesAngle += 0.5;
    if (circlesAngle >= 360) circlesAngle -= 360;
    for (int i = 0; i < 88; ++i) {
      circles[i].setUseColors(mode == 1 && circlesAngle == 90);
      if (notesActive[i]) {
        circles[i].setActive(true);
      } else {
        if (pedalOn) circles[i].setZDelta(0.01);
        else circles[i].setZDelta(0.1);
        circles[i].setActive(false);
      }
      circles[i].update();
    }
  } else if (mode >= 3) { // && mode <= 7) {
    if (mode <= 4) {
      if (mode == 3) {
        targetBlowup = 0.0;
        uiEndRotation = true;
      } else {
        if (std::abs(currBlowup - targetBlowup) < 0.01) {
          targetBlowup = 1000 - targetBlowup;
          for (int x = 0; x < planes.size(); ++x) {
            for (int y = 0; y < planes[x].size(); ++y) {
              planeOrientationDeltas[x][y] = glm::quat(
                  0.0,
                  0.01 * (float)((2 * rand() % 2) - 1.0),
                  0.01 * (float)((2 * rand() % 2) - 1.0),
                  0.01 * (float)((2 * rand() % 2) - 1.0));
            }
          }
        }
      }
      if (currBlowup < targetBlowup) currBlowup += 1.0;
      else if (currBlowup > targetBlowup) currBlowup -= 1.0;
      if (std::abs(currBlowup - targetBlowup) < 100.0f) {
        uiEndRotation = true;
      }
      else {
        uiEndRotation = false;
        rotate = true;
      }
      int numBlocks = planes.size();
      for (int x = 0 ; x < numBlocks; ++x) {
        int numCols = planes[x].size();
        for (int y = 0; y < numCols; ++y) {
          auto pos = planes[x][y].getPosition();
          auto xOff = currBlowup * 2 * std::abs((float)x - (float)numBlocks / 2) / numBlocks;
          if (x < (float)numBlocks / 2) xOff *= -1;
          pos.x = origPlanePositions[x][y].x + xOff;
          auto yOff = currBlowup * 2 * std::abs((float)y - (float)numCols / 2) / numCols;
          if (y < (float)numCols / 2) yOff *= -1;
          pos.y = origPlanePositions[x][y].y + yOff;
          planes[x][y].setPosition(pos);
        }
      }
    } else {
      int numInfinityPoints = splitTextures.size() * splitTextures[0].size();
      float t = startT;
      float deltaT = 2 * PI / numInfinityPoints;
      int numBlocks = planes.size();
      float xOffset = (ofGetWidth() - VIDEO_WIDTH) / 2.0;
      float yOffset = (ofGetHeight() - VIDEO_HEIGHT) / 2.0;
      bool allArrived = true;
      for (int x = 0 ; x < numBlocks; ++x) {
        int numCols = planes[x].size();
        for (int y = 0; y < numCols; ++y) {
          auto targetPos = origPlanePositions[x][y];
          if (mode == 5) {
            targetPos = glm::vec3(2 * xOffset + 500 * cos(t),
                                  2 * yOffset + 500 * sin(t) * cos(t),
                                  0.0);
          } else if (mode == 6) {
            int num_points = 6;
            int m = 4;
            float numerator = cos((m + 1) * PI / (2 * num_points));
            float denominator = cos((2 * asin(cos(num_points * t)) + PI * m) / (2 * num_points));
            float radius = numerator / denominator;
            targetPos = glm::vec3(2 * xOffset + 500 * radius * cos(t),
                                  2 * yOffset + 500 * radius * sin(t),
                                  0.0);
          } else if (mode == 7) {
            int p = 2;
            int q = -3;
            float radius = cos(q * t) + 2;
            targetPos = glm::vec3(2 * xOffset + 150 * radius * cos(p * t),
                                  2 * yOffset + 150 * radius * sin(p * t),
                                  0.0);
          }
          auto currPos = planes[x][y].getPosition();
          auto deltaPos = targetPos - currPos;
          float xDelta = 5.0;
          if (deltaPos.x > xDelta || deltaPos.y > xDelta) {
            if (targetPos.x < currPos.x) xDelta *= -1.0;
            auto newPos = glm::vec3(xDelta, xDelta * (deltaPos.y / deltaPos.x), 0.0);
            planes[x][y].setPosition(currPos + newPos);
            allArrived = false;
          } else {
            planes[x][y].setPosition(targetPos);
          }
          t += deltaT;
        }
      }
      if (allArrived) startT += 0.01;
    }
    vidGrabber.update();
    if (vidGrabber.isFrameNew()) {
      unsigned char* pixelData = vidGrabber.getPixels().getData();
      for (int j = 0; j < vidGrabber.getHeight(); ++j) {
        int blockRow = j / pixelCellHeight;
        int y = j % pixelCellHeight;
        for (int i = 0; i < vidGrabber.getWidth(); ++i) {
          int blockCol = i / pixelCellWidth;
          for (int rgb = 0; rgb < 3; ++rgb) {
            int x = (i * 3 + rgb) % (pixelCellWidth * 3);
            int splitPos = y * pixelCellWidth * 3 + x;
            int bytePos = j * VIDEO_WIDTH * 3 + i * 3 + rgb;
            splitPixels[blockCol][blockRow][splitPos] = pixelData[bytePos];
          }
        }
      }
      for (int i = 0; i < splitTextures.size(); ++i) {
        for (int j = 0; j < splitTextures[i].size(); ++j) {
          splitTextures[i][j].loadData(&splitPixels[i][j][0],
                                       pixelCellWidth,
                                       pixelCellHeight,
                                       GL_RGB);
          planes[i][j].resizeToTexture(splitTextures[i][j], uiTextureScale);
        }
      }
    }
    int currBlock = planes.size();
    int numBlocks = currBlock;
    if (rotate) {
      for (int i = 0; i < splitTextures.size(); ++i) {
        for (int j = 0; j < splitTextures[i].size(); ++j) {
          auto delta = planeOrientationDeltas[i][j];
          planeOrientations[i][j] += delta;
          // Before flipping deltas, check if we want to stop rotation.
          if (uiEndRotation) {
            if (sign(planeOrientations[i][j].x) !=
                sign(planeOrientations[i][j].x - planeOrientationDeltas[i][j].x)) {
              planeOrientations[i][j].x = 0.0;
              planeOrientationDeltas[i][j].x = 0.0;
            }
            if (sign(planeOrientations[i][j].y) !=
                sign(planeOrientations[i][j].y - planeOrientationDeltas[i][j].y)) {
              planeOrientations[i][j].y = 0.0;
              planeOrientationDeltas[i][j].y = 0.0;
            }
            if (sign(planeOrientations[i][j].z) !=
                sign(planeOrientations[i][j].z - planeOrientationDeltas[i][j].z)) {
              planeOrientations[i][j].z = 0.0;
              planeOrientationDeltas[i][j].z = 0.0;
            }
          }
          if (std::abs(planeOrientations[i][j].x) > 1.0f) {
            planeOrientationDeltas[i][j].x *= -1.0f;
          }
          planeOrientations[i][j].x = std::max(
              -1.0f, std::min(1.0f, planeOrientations[i][j].x));
          if (std::abs(planeOrientations[i][j].y) > 1.0f) {
            planeOrientationDeltas[i][j].y *= -1.0f;
          }
          planeOrientations[i][j].y = std::max(
              -1.0f, std::min(1.0f, planeOrientations[i][j].y));
          if (std::abs(planeOrientations[i][j].z) > 1.0f) {
            planeOrientationDeltas[i][j].z *= -1.0f;
          }
          planeOrientations[i][j].z = std::max(
              -1.0f, std::min(1.0f, planeOrientations[i][j].z));
          planes[i][j].setGlobalOrientation(planeOrientations[i][j]);
        }
      }
    }
  }
}

void ofApp::reset() {
  for (int i = 0; i < 88; ++i) {
    circles[i].reset();
  }
  flameStrips.clear();
}

//--------------------------------------------------------------
void ofApp::draw() {
  ofPushMatrix();
  cam.begin();
  if (uiIncludeCircles) {
    ofNoFill();
    ofPushMatrix();
    ofRotateDeg(circlesAngle, 1.0, 0.0, 0.0);
    for (int i = 0; i < 88; ++i) circles[i].draw();
    ofPopMatrix();
  }
  if (uiIncludeFlames) {
    ofNoFill();
    ofSetColor(ofColor(255, 60, 60));
    ofPushMatrix();
    for (int i = 0; i < flameStrips.size(); ++i) flameStrips[i].draw();
    ofPopMatrix();
  }
  cam.end();
  ofPopMatrix();

  if (mode >= 3) {
    ofPushMatrix();
    ofSetColor(ofColor::white);
    float xOffset = (ofGetWidth() - VIDEO_WIDTH) / 2.0;
    float yOffset = (ofGetHeight() - VIDEO_HEIGHT) / 2.0;
    int numBlocks = planes.size();
    for (int x = 0 ; x < numBlocks; ++x) {
      int numCols = planes[x].size();
      for (int y = 0; y < numCols; ++y) {
        splitTextures[numBlocks - x - 1][y].bind();
        planes[x][y].draw();
        splitTextures[numBlocks - x - 1][y].unbind();
      }
    }
    ofPopMatrix();
  }

  if (drawGui) gui.draw();
}

void ofApp::checkOSCMessages() {
  while(receiver.hasWaitingMessages()) {
    ofxOscMessage m;
    receiver.getNextMessage(m);
    auto addr = m.getAddress();
    if (addr == "/mode") {
      if (m.getArgAsInt(0) > 4) readyForMode3 = true;
      if (m.getArgAsInt(0) == 3) {
        mode = readyForMode3 ? 3 : 4;
      } else {
        mode = m.getArgAsInt(0);
      }
    } else if (addr == "/noteon" || addr == "/noteoff") {
      int pitch = m.getArgAsInt(0);
      int velocity = m.getArgAsInt(1);
      notesActive[pitch - 21] = (addr == "/noteon");
      if (notesActive[pitch - 21] &&
          ((ofGetElapsedTimef() - firstOnset) > uiOnsetThreshold) &&
          mode == 0 && pitch < 50) {
        firstOnset = ofGetElapsedTimef();
        flameStrips.emplace_back(uiStartXPos, (velocity % 10) + float(rand()) / RAND_MAX);
      }
    } else if (addr == "/cc" && m.getArgAsInt(0) == 64) {
      pedalOn = (m.getArgAsInt(1) > 0);
    }
  }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
  if (key == 'a') {
    notesActive[60] = true;
  } else if (key == 'w') {
    notesActive[61] = true;
  } else if (key == 's') {
    notesActive[62] = true;
  } else if (key == 'e') {
    notesActive[63] = true;
  } else if (key == 'd') {
    notesActive[64] = true;
  } else if (key == 'f') {
    notesActive[65] = true;
  } else if (key == 'l') {
    flameStrips.emplace_back(uiStartXPos, (rand() % 10) + float(rand()) / RAND_MAX);
  } else if (key == '0') {
    mode = 0;
  } else if (key == '1') {
    mode = 1;
  } else if (key == '2') {
    mode = 2;
  } else if (key == '3') {
    mode = 3;
  } else if (key == '4') {
    mode = 4;
  } else if (key == '5') {
    mode = 5;
  } else if (key == '6') {
    mode = 6;
  } else if (key == '7') {
    mode = 7;
  } else if (key == '8') {
    mode = 8;
  } else if (key == '9') {
    mode = 9;
  }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
  if (key == 'a') {
    notesActive[60] = false;
  } else if (key == 'w') {
    notesActive[61] = false;
  } else if (key == 's') {
    notesActive[62] = false;
  } else if (key == 'e') {
    notesActive[63] = false;
  } else if (key == 'd') {
    notesActive[64] = false;
  } else if (key == 'f') {
    notesActive[65] = false;
  } else if (key == 'g') {
    drawGui = !drawGui;
  } else if (key == 'r') {
    reset();
  } else if (key  == 'p') {
    rotate = !rotate;
  }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

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
