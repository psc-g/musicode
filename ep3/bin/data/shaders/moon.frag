#version 150

out vec4 outputColor;
uniform vec2 circlePos;
uniform vec2 beatPos;
uniform float circleRadiusMultiplier;
uniform int isGreen;
uniform float alpha;

void main()
{
  float windowWidth = 1280.0;
  float windowHeight = 1000.0;
  float weight = 1.0 - min(1.0, 20 * circleRadiusMultiplier / sqrt(pow(beatPos.x - gl_FragCoord.x, 2.0) + pow(beatPos.y - gl_FragCoord.y, 2.0)));
  float r = weight * gl_FragCoord.x / windowWidth;
  float g = weight * 0.5;
  float b = weight * gl_FragCoord.y / windowHeight;
  float a = alpha;
  if (isGreen > 0) g += 1.0 - weight;
  else r += 1.0 - weight;
  if (circlePos.x < 0.0 && circlePos.y < 0.0) {
    r = gl_FragCoord.x / windowWidth;
    g = 0.5;
    b = gl_FragCoord.y / windowHeight;
  }
  outputColor = vec4(r, g, b, a);
}
