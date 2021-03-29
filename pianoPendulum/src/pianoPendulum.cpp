#include "pianoPendulum.h"
#include <math.h>

PianoPendulum::PianoPendulum() {
  mode = 2;
  drawBalls = true;
  box2d.init();
  box2d.setGravity(0, GRAVITY);
  box2d.setFPS(60.0);
  box2d.registerGrabbing();

  anchor.setPhysics(0.0, 0.0, 0.0);
  anchor.setup(box2d.getWorld(), ofGetWidth()/2, ofGetHeight()/2, ANCHOR_RADIUS);

  init();

  // Add the floor and walls.
  ofFloor.setPhysics(0.0, 0.53, 0.1);
  ofFloor.setup(box2d.getWorld(), ofGetWidth() / 2, ofGetHeight(), ofGetWidth(), 10);
  ofCeiling.setPhysics(0.0, 0.53, 0.1);
  ofCeiling.setup(box2d.getWorld(), ofGetWidth() / 2, 0.0, ofGetWidth(), 10);
  leftWall.setPhysics(0.0, 0.53, 0.1);
  leftWall.setup(box2d.getWorld(), 0, ofGetHeight() / 2, 10, ofGetHeight());
  rightWall.setPhysics(0.0, 0.53, 0.1);
  rightWall.setup(box2d.getWorld(), ofGetWidth(), ofGetHeight() / 2, 10, ofGetHeight());

  // Add the keyboard.
  setupKeyboard();

  colors[0] = 0x210000;
  colors[1] = 0x211000;
  colors[2] = 0x212100;
  colors[3] = 0x102100;
  colors[4] = 0x002100;
  colors[5] = 0x002110;
  colors[6] = 0x002121;
  colors[7] = 0x001021;
  colors[8] = 0x000021;
  colors[9] = 0x100021;
  colors[10] = 0x210021;
  colors[11] = 0x210010;
}

PianoPendulum::~PianoPendulum() {
  clear();
}

void PianoPendulum::init() {
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

void PianoPendulum::clear() {
  for (auto &note : notes) {
    note.circle->destroy();
    note.lines.clear();
    note.beziers.clear();
  }
  notes.clear();
}

void PianoPendulum::reset() {
  clear();
  init();
}

void PianoPendulum::update() {
  box2d.update();
}

void PianoPendulum::draw() {
  ofSetHexColor(0x000000);
  ofFill();
  backboard.draw();
  // We have to go through the keys twice to make sure we draw the black ones last.
  for (uint i = 0; i < 88; ++i) {
    if (!pianoKeys[i].white) continue;
    if (pianoKeys[i].active) {
      int basePitch = colors[(i - 2) % 12];
      float multiplier = floor(i / 12);
      ofSetHexColor(basePitch * multiplier);
    } else {
      ofSetHexColor(0xffffff);
    }
    ofFill();
    pianoKeys[i].key.draw();
  }
  for (uint i = 0; i < 88; ++i) {
    if (pianoKeys[i].white) continue;
    if (pianoKeys[i].active) {
      int basePitch = colors[(i - 2) % 12];
      float multiplier = floor(i / 12);
      ofSetHexColor(basePitch * multiplier);
    } else {
      ofSetHexColor(0x000000);
    }
    ofFill();
    pianoKeys[i].key.draw();
  }

  for (auto &note : notes) {
    ofFill();
    int basePitch = colors[(note.pitch - 24) % 12];
    float multiplier = floor(note.pitch / 12);
    if (note.pitch >= 0) {
      float elapsedTime = ofGetElapsedTimef() - note.offsetTime;
      if (note.active) {
        ofVec2f currPos = note.circle->getPosition();
        note.polyline.curveTo(currPos[0], currPos[1]);
        vector<glm::vec3> vertices = note.polyline.getVertices();
        int i = vertices.size() - 4;
        if (i >= 0 && mode >= 1) {
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
      if (mode == 1 || mode == 3) {
        for (auto &line : note.lines) {
          if (!note.active) {
            float zOffset = elapsedTime / 4;
            line.first.z -= zOffset;
            line.second.z -= zOffset;
          }
          ofSetColor(ofColor::fromHex(basePitch * multiplier, alpha));
          ofDrawLine(line.first, line.second);
        }
      } else if (mode == 2) {
        note.path.setColor(ofColor::fromHex(basePitch * multiplier, alpha));
        note.path.draw();
      } else if (mode == 4) {  // Disabled, really
        // This is borrowed from examples/3d/cameraRibbonExample
        ofSetColor(ofColor::fromHex(basePitch * multiplier, alpha));
        ofMesh mesh;
        mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
        for (uint i = 1; i < note.points.size(); ++i) {
          if (!note.active) {
            float zOffset = elapsedTime / 4;
            note.points[i-1].z -= zOffset;
            note.points[i].z -= zOffset;
          }
          glm::vec3 thisPoint = note.points[i-1];
          glm::vec3 nextPoint = note.points[i];
          // get the direction from one to the next.
          // the ribbon should fan out from this direction
          glm::vec3 direction = nextPoint - thisPoint;
          float distance = glm::length(direction);
          // get the normalized direction. normalized vectors always have a
          // length of one and are really useful for representing directions as
          // opposed to something with length
          glm::vec3 unitDirection = glm::normalize(direction);
          // find both directions to the left and to the right
          glm::vec3 toTheLeft = glm::rotate(unitDirection, -90.f, glm::vec3(0, 0, 1));
          glm::vec3 toTheRight = glm::rotate(unitDirection, 90.f, glm::vec3(0, 0, 1));
          // use the map function to determine the distance.
          // the longer the distance, the narrower the line.
          // this makes it look a bit like brush strokes
          float thickness = ofMap(distance, 0, 30, 20, 2, true);
          // calculate the points to the left and to the right by extending the
          // current point in the direction of left/right by the length
          glm::vec3 leftPoint = thisPoint + toTheLeft * thickness;
          glm::vec3 rightPoint = thisPoint + toTheRight * thickness;
          // add these points to the triangle strip
          mesh.addVertex(glm::vec3(leftPoint.x, leftPoint.y, leftPoint.z));
          mesh.addVertex(glm::vec3(rightPoint.x, rightPoint.y, rightPoint.z));
        }
        mesh.draw();
      }
    }
    if (drawBalls && mode == 0) {
      ofSetHexColor(basePitch * multiplier);
      note.circle->draw();
    }
    if (drawBalls) {
      ofSetHexColor(0x444342);
      note.joint->draw();
    }
  }
}

void PianoPendulum::printActiveNotes() {
  int noteIndex = 0;
  while (noteIndex >= 0) {
    cout << noteIndex << "(pitch, prev, next): "
         << notes[noteIndex].pitch << ", "
         << notes[noteIndex].prevNoteIndex << ", "
         << notes[noteIndex].nextNoteIndex << endl;
    noteIndex = notes[noteIndex].nextNoteIndex;
  }
}

void PianoPendulum::addNote(int pitch) {
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

void PianoPendulum::dropNote(int pitch) {
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

void PianoPendulum::cycleModes() {
  if (mode == 2) setMode(1);
  else if (mode == 1) setMode(3);
  else setMode(2);
}

void PianoPendulum::setMode(int newMode) {
  if (newMode >= 0 && newMode < 4) {
    mode = newMode;
    reset();
  }
  if (mode == 3) {
    ofBackgroundHex(0x000000);
    drawBalls = false;
  } else {
    ofBackgroundHex(0xffffff);
    drawBalls = true;
  }
}

void PianoPendulum::setupKeyboard() {
  float whiteWidth = ofGetHeight() / 60;
  float whiteHeight = whiteWidth * 4;
  backboard.setPhysics(0.0, 0.53, 0.1);
  backboard.setup(box2d.getWorld(),
                  whiteHeight / 2,
                  ofGetHeight() / 2,
                  whiteHeight + KEY_SEPARATION,
                  whiteWidth * 52 + KEY_SEPARATION * 26);
  float yPos = whiteHeight + KEY_SEPARATION * 1.5;
  int whiteKeyPos = 0;
  int i = 0;
  while (i < 88) {
    pianoKeys[i].active = false;
    pianoKeys[i].white = true;
    pianoKeys[i].key.setPhysics(0.0, 0.53, 0.1);
    pianoKeys[i++].key.setup(box2d.getWorld(),
                             (KEY_SEPARATION + whiteHeight) / 2,
                             yPos,
                             whiteHeight - KEY_SEPARATION,
                             whiteWidth - KEY_SEPARATION / 2);
    // We need to special case the lone black key in the lowest partial octave.
    // After that we can do simple mod arithmetic to figure it out.
    int keyPos = (whiteKeyPos - 2) % 7;
    if (whiteKeyPos == 0 || (i < 88 && whiteKeyPos >= 2 && keyPos != 2 && keyPos != 6)) {
      pianoKeys[i].active = false;
      pianoKeys[i].white = false;
      pianoKeys[i].key.setPhysics(0.0, 0.53, 0.1);
      pianoKeys[i++].key.setup(box2d.getWorld(),
                               (KEY_SEPARATION + 3 * whiteHeight) / 4,
                               yPos + (whiteWidth + KEY_SEPARATION) / 2,
                               (whiteHeight - KEY_SEPARATION) / 2,
                               (whiteWidth - KEY_SEPARATION) / 2);
    }
    yPos += whiteWidth + KEY_SEPARATION / 2;
    whiteKeyPos++;
  }
}
