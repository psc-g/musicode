#version 150

out vec4 outputColor;
uniform int mode;

void main()
{
  float windowWidth = 2048.0;
  float windowHeight = 768.0;
  float r = 0.1137;
  float g = 0.2078;
  float b = 0.34117;
  if (mode == 1) {
    r = 0.6588;
    g = 0.8549;
    b = 0.8627;
  } else if (mode == 2) {
    b = 0.6588;
    r = 0.8549;
    g = 0.8627;
  } else if (mode == 3) {
    g = 0.6588;
    b = 0.8549;
    r = 0.8627;
  }
  outputColor = vec4(r, g, b, 1.0);
}

