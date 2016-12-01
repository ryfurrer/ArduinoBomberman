// mystruct.h
#ifndef TREE_H
#define TREE_H

struct tree {
  uint8_t xy;

  tree();
  void setXY(int x, int y);
  int getX();
  int getY();

};

#endif
