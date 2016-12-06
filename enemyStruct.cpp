#include <Arduino.h>
#include <enemyStruct.h>

int yx;
uint8_t dir;
bool active;
unsigned long timeOfLastMove;
unsigned long timeToNextMove;
uint8_t spacesToMove;

enemy::enemy(){
  dir = 0;//0 left, 1 right, 2 up, 3 down
  yx = 1;
  active = false;
}

void enemy::setXY(uint8_t x, uint8_t y){
  yx = ( y<< 4) + x;
}

int enemy::getX() {
  int t = (yx&(15));
  return t;
}

int enemy::getY() {
  int t = yx >> 4;
  return t;
}

int enemy::getYXandStat() {
  if (!active) {
    return (yx+(1<<12));
  }
  return yx;
}

void enemy::setStatus(bool tf) {
  active = tf;
}
