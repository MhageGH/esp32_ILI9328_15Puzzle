/* 
 *  Controller configuration:
 *  Buttons UP, RIGHT, DOWN, and LEFT are each assigned on characters '8', '6', '2', and '4' in the both case of SerialPort and WiFi UDP.
 */

const char ssid[] = "ESP32";
const char password[] = "esp32pass";
const int localPort = 10000;

#include "ili9328.h"
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "picture.h"
#define CS 5
#define RESET 17
WiFiUDP udp;

ili9328SPI tft(CS, RESET);
const int h = 60, w = 80;
uint16_t fracImage[h][w];
uint16_t hMoveImage[h][2 * w];
uint16_t vMoveImage[2 * h][w];
uint16_t backcolor = 0;
const byte empty = 15;
enum Dir { UP, RIGHT, DOWN, LEFT };
byte screen[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, empty, 12, 13, 14, 11};
byte initialScreen[16];

bool ChangeScreen(Dir dir) {
  int emptyPos;
  for (int i = 0; i < 16; ++i) if (screen[i] == empty) emptyPos = i;
  if (dir == UP) {
    if (emptyPos > 11) return false;
    screen[emptyPos] = screen[emptyPos + 4];
    screen[emptyPos + 4] = empty;
  }
  if (dir == RIGHT) {
    if (emptyPos % 4 == 0) return false;
    screen[emptyPos] = screen[emptyPos - 1];
    screen[emptyPos - 1] = empty;
  }
  if (dir == DOWN) {
    if (emptyPos < 4) return false;
    screen[emptyPos] = screen[emptyPos - 4];
    screen[emptyPos - 4] = empty;
  }
  if (dir == LEFT) {
    if (emptyPos % 4 == 3 ) return false;
    screen[emptyPos] = screen[emptyPos + 1];
    screen[emptyPos + 1] = empty;
  }
  return true;
}

void MoveImage(Dir dir) {
  int emptyPos, imagePos, imageNumber;
  for (int i = 0; i < 16; ++i) if (screen[i] == empty) emptyPos = i;
  const int numAnimation = 5;
  for (int t = 0; t < numAnimation + 1; ++t) {
    if (dir == UP) {
      imagePos = emptyPos - 4;
      imageNumber = screen[imagePos];
      for (int i = 0; i < 2 * h; ++i) for (int j = 0; j < w; ++j) vMoveImage[i][j] = 0;
      for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) vMoveImage[i + h - (h / numAnimation)*t][j] = picture[i + h * (imageNumber / 4)][j + w * (imageNumber % 4)];
      tft.fillImage(vMoveImage, w * (imagePos % 4), h * (imagePos / 4), w, 2 * h);
    }
    else if (dir == RIGHT) {
      imagePos = emptyPos + 1;
      imageNumber = screen[imagePos];
      for (int i = 0; i < h; ++i) for (int j = 0; j < 2 * w; ++j) hMoveImage[i][j] = 0;
      for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) hMoveImage[i][j + (w / numAnimation)*t] = picture[i + h * (imageNumber / 4)][j + w * (imageNumber % 4)];
      tft.fillImage(hMoveImage, w * (emptyPos % 4), h * (emptyPos / 4), 2 * w, h);
    }
    else if (dir == DOWN) {
      imagePos = emptyPos + 4;
      imageNumber = screen[imagePos];
      for (int i = 0; i < 2 * h; ++i) for (int j = 0; j < w; ++j) vMoveImage[i][j] = 0;
      for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) vMoveImage[i + (h / numAnimation)*t][j] = picture[i + h * (imageNumber / 4)][j + w * (imageNumber % 4)];
      tft.fillImage(vMoveImage, w * (emptyPos % 4), h * (emptyPos / 4), w, 2 * h);
    }
    else if (dir == LEFT) {
      imagePos = emptyPos - 1;
      imageNumber = screen[imagePos];
      for (int i = 0; i < h; ++i) for (int j = 0; j < 2 * w; ++j) hMoveImage[i][j] = 0;
      for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j) hMoveImage[i][j + w - (w / numAnimation)*t] = picture[i + h * (imageNumber / 4)][j + w * (imageNumber % 4)];
      tft.fillImage(hMoveImage, w * (imagePos % 4), h * (imagePos / 4), 2 * w, h);
    }
  }
}

bool Push(Dir dir) {
  if (!ChangeScreen(dir)) return false;
  MoveImage(dir);
  return true;
}

void setup() {
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  udp.begin(localPort);
  for (int i = 0; i < 100; ++i) while (!ChangeScreen((Dir)random(4)));
  for (int i = 0; i < 16; ++i) initialScreen[i] = screen[i];
  tft.begin();
  tft.fillScreen(backcolor);
  for (int k = 0; k < 16; ++k) {
    for (int i = 0; i < h; ++i) for (int j = 0; j < w; ++j)
        fracImage[i][j] = screen[k] == empty ? 0 : picture[i + h * (screen[k] / 4)][j + w * (screen[k] % 4)];
    tft.fillImage(fracImage, w * (k % 4), h * (k / 4), w, h);
  }
}

void loop() {
  char r;
  if (Serial.available()) r = Serial.read();
  else if (udp.parsePacket()) r = udp.read();
  else return;
  if (r == '8') Push(UP);
  else if (r == '6') Push(RIGHT);
  else if (r == '2') Push(DOWN);
  else if (r == '4') Push(LEFT);
  else return;
}
