#version 150

out vec4 outputColor;
uniform vec2 starPos;
uniform float time;
uniform float alpha;

void main()
{
  float threshold = 1.0;
  float length = 100.0;
  float distX = abs(starPos.x - gl_FragCoord.x);
  float distY = abs(starPos.y - gl_FragCoord.y);
  if (distX < threshold || distY < threshold) {
    length = 400.0;
  } else if (distX == distY) {
    length = 300.0;
  }
  float radiance = min(1.0, length / sqrt(pow(starPos.x - gl_FragCoord.x, 2.0) + pow(starPos.y - gl_FragCoord.y, 2.0)) / 10.0);
  float weight = (sin(2 * time + starPos.x * starPos.y) + 1.0) / 2.0 + 0.5;
  float r = weight * radiance; 
  float g = weight * radiance;
  float b = weight * radiance;
  float a = weight * radiance * alpha;
  outputColor = vec4(r, g, b, a);
}
