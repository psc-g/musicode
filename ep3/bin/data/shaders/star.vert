#version 150

uniform mat4 modelViewProjectionMatrix;
in vec4 position;

void main() {
  vec4 modifiedPosition = modelViewProjectionMatrix * position;
  gl_Position = modifiedPosition;
}
