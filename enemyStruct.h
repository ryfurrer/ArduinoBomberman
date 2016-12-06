#ifndef _enemy_func_
#define  _enemy_func_

struct enemy {
  int yx;
  uint8_t dir;
  unsigned long timeOfLastMove;
  unsigned long timeToNextMove;
  uint8_t spacesToMove;
  bool active;
  enemy();
  void setXY(uint8_t x, uint8_t y);
  void setStatus(bool tf);
  int getX();
  int getY();
  int getYXandStat();
  void changeDirection();
};
#endif
