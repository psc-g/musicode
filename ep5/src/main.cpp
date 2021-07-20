#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){
  ofGLFWWindowSettings settings;
  settings.setGLVersion(3, 2); //we define the OpenGL version we want to use
  settings.setSize(1280, 10000);
  ofCreateWindow(settings);
  ofRunApp(new ofApp());
}

