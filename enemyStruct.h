#ifndef _enemy_func_
#define  _enemy_func_

struct enemy {
  uint8_t x;
  uint8_t y;
  uint8_t dir;
  unsigned long timeOfLastMove;
  bool active;
  enemy();
  void setXY(uint8_t x, uint8_t y);
  void setStatus(bool tf);
  void changeDirection();
  bool isTouching(uint8_t x, uint8_t y);
};
#endif
