#version 150

out vec4 outputColor;
uniform float time;
uniform float redWeight;
uniform int mode;

void main()
{
  float windowWidth = 1024.0;
  float windowHeight = 768.0;
  float r = 0.0;
  float g = 0.0;
  float b = 0.0;
  if (mode == 0) {
    float yPos = gl_FragCoord.y / windowHeight;
    r = (1 - redWeight) * 0.1 + redWeight * yPos;
    g = 0.1;
    b = redWeight * 0.1 + (1 - redWeight) * yPos;
  } else if (mode == 1) {
    int divider = 100;
    float x = int(gl_FragCoord.x) % divider;
    float y = int(gl_FragCoord.y) % divider;
    float xmulty = int(x * y) % divider;
    r = 0.25 * mod(sin(time + (x * y)) + x / windowWidth, 2);
    g = 0.01 * mod(cos(time + (x * y)) + y / windowHeight, 5);
    b = 0.1; // gl_FragCoord.y / windowHeight;
  } else if (mode == 2) {
    r = 0.25 * mod(sin(time + (gl_FragCoord.x)) + gl_FragCoord.x / windowWidth, 2);
    g = 0.25 * mod(cos(time + (gl_FragCoord.y)) + gl_FragCoord.y / windowHeight, 2);
    // b = 0.25 * mod(sin(time + (gl_FragCoord.x)) + gl_FragCoord.y / windowHeight, 2);
    b = gl_FragCoord.y / windowHeight;
  } else {
    r = int(time + sin(gl_FragCoord.x / 100.0)) % 2 == 0 ? 0.25 : 0.0;
    g = int(time + cos(gl_FragCoord.y / 100.0)) % 2 == 0 ? 0.25 : 0.0;
    b = 0.25 * int(time + sin(gl_FragCoord.x * gl_FragCoord.y) / 100.0);
  }
  float a = 1.0;
  outputColor = vec4(r, g, b, a);
}

