#include <Arduino.h>
#include <enemyStruct.h>

uint8_t x;
uint8_t y;
uint8_t dir;
bool active;
unsigned long timeOfLastMove;
unsigned long timeToNextMove;
uint8_t spacesToMove;

enemy::enemy(){
  dir = 0;//0 left, 1 right, 2 up, 3 down
  x = 1;
  y = 0;
  active = false;
}

void enemy::setXY(uint8_t x, uint8_t y){
  this->x = x;
  this->y = y;
}

bool enemy::isTouching(uint8_t playerX, uint8_t playerY) {
  if (playerX == x && playerY == y && active) {
    return true;
  }
  return false;
}

void enemy::setStatus(bool tf) {
  active = tf;
}
