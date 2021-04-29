#version 150

out vec4 outputColor;

void main()
{
  float windowWidth = 1280.0;
  float windowHeight = 1000.0;
  float r = gl_FragCoord.x / (2.0 * windowWidth) + 0.5;
  float g = 0.25 * ((gl_FragCoord.x / windowWidth) + (gl_FragCoord.y / windowHeight)) + 0.5;
  float b = gl_FragCoord.y / (2.0 * windowHeight) + 0.5;
  float a = 1.0;
  outputColor = vec4(r, g, b, a);
}

