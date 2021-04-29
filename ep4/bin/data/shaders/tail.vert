#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
uniform float time;
uniform vec2 tailStart;

void main() {
  float displacementX = (position.x - tailStart.x);
  float displacementY = (position.y - tailStart.y);
  vec4 modifiedPosition = modelViewProjectionMatrix * position;
  displacementX *= cos(5 * time + (position.x / 300.0));
  displacementY *= sin(5 * time + (position.x / 300.0));
  modifiedPosition.x += displacementX;
  modifiedPosition.y += displacementY;
  gl_Position = modifiedPosition;
}

