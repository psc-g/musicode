#version 150

out vec4 fragColor;
uniform float height;
uniform vec3 gradients;
uniform float time;
uniform sampler2DRect tex0;

void main(){
  vec2 uv = gl_FragCoord.xy;
  uv.y = height - uv.y;
  float displacementHeight = 100.0;
  float displacementX = cos(5 * gradients.x * time + (uv.x / 300.0)) * displacementHeight;
  float displacementY = sin(5 * gradients.y * time + (uv.x / 300.0)) * displacementHeight;
  uv.x += displacementX;
  uv.y += displacementY;
  fragColor = texture(tex0, uv);
}
