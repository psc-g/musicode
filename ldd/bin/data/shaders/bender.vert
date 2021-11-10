#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
uniform vec3 gradients;
uniform float time;

void main() {
  vec4 modifiedPosition = modelViewProjectionMatrix * position;
  float displacementHeight = 100.0;
  float displacementX = 0.0;
  float displacementY = 0.0;
  // displacementX = sin((gradients.x + gradients.y) * time) * 1000;
  // displacementY = sin((gradients.y + gradients.z) * time) * 1000;
  displacementX = cos(5 * gradients.x * time + (position.x / 300.0)) * displacementHeight;
  displacementY = sin(5 * gradients.y * time + (position.x / 300.0)) * displacementHeight;
  modifiedPosition.x += displacementX;
  modifiedPosition.y += displacementY;
  gl_Position = modifiedPosition;
}

