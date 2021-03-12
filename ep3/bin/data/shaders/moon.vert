#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;
uniform float time;
uniform vec2 circlePos;
uniform int start;

void main() {
  vec4 modifiedPosition = modelViewProjectionMatrix * position;
  if (start > 0) {
    float displacementHeight = 100.0;
    float displacementX = 0.0;
    float displacementY = 0.0;
    if (circlePos.x >= 0.0 && circlePos.y >= 0.0) {
      float distance = sqrt(pow(circlePos.x - position.x, 2.0) + pow(circlePos.y - position.y, 2.0));
      float weight = max(0.0, 1.0 - distance / 400.0);
      displacementX = weight * (circlePos.x - position.x) + (1 - weight) * displacementX / 2.0;
      displacementY = weight * (position.y - circlePos.y) + (1 - weight) * displacementY / 2.0;
    } else {
      displacementX = cos(5 * time + (position.x / 300.0)) * displacementHeight;
      displacementY = sin(5 * time + (position.x / 300.0)) * displacementHeight;
    }
    modifiedPosition.x += displacementX;
    modifiedPosition.y += displacementY;
  }
  gl_Position = modifiedPosition;
}
