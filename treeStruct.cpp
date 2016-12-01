#include <Arduino.h>
#include "treeStruct.h"

uint8_t xy;

tree::tree() {
  xy = 0;
}

//only use nubers x:0-15, y:4-19
void tree::setXY(int x, int y) {
  y = y - 4;
  y = y << 4;
  xy = x + y;

}

int tree::getX() {
  int t = (xy&(15));
  return t;
}

int tree::getY() {
  int t = xy >> 4;
  t += 4;
  return t;
}
