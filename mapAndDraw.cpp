#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h> // hardware-specific library
#include <SPI.h>
#include <treeStruct.h>

// Map Dimensions
#define MAP_WIDTH 16
#define MAP_HEIGHT 20

// Tile Dimensions
#define TILE_WIDTH 8
#define TILE_HEIGHT 8

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define TREE     0x0340
#define STONE     0x94B2
#define BROWN     0x6A20
#define WHITE    0xFFFF
#define EXPLOSION 0xFBA0

#define TFT_CS   6  // Chip select line for TFT display
#define TFT_DC   7  // Data/command line for TFT
#define TFT_RST  8  // Reset line for TFT (or connect to +5V)

const uint8_t PIN = 13;// for reading voltage for getRandom


//TFT specific setup
void setupTFT(Adafruit_ST7735 *tft) {
  (*tft).initR(INITR_BLACKTAB);   // initialize a ST7735R chip, black tab
  (*tft).fillScreen(BLACK);
}

//More random but has delay
//only for numbers less than 2^5-1 = 31
uint8_t getRandom(uint8_t max){
  uint8_t rand_num;
  uint8_t b;
  uint8_t key = 0;
  uint8_t modifier = 6300/max;
  for (uint8_t i = 0; i < 6; i++) {
    rand_num = analogRead(PIN);
    b = 1 << i;

    if ((rand_num&1)!=0) {
      key = key + (b);
    }

    delay(80);//5*80=400ms
  }

  rand_num = (key*100) / modifier;

  if (rand_num > max) {
    rand_num = max;
  }
  return rand_num;
}

//Print title and options of the start screen
void printStartItem(Adafruit_ST7735 *tft, uint8_t i) {
  if (i==0) {
    (*tft).setCursor(29, 10*8);
    (*tft).print("Play Game!");
  } else if (i==1) {
    (*tft).setCursor(24, 11*8);
    (*tft).print("Instructions");
  } else {
    (*tft).setTextSize(2);
    (*tft).setCursor(8, 8);
    (*tft).print("Bomberman!");
    (*tft).setTextSize(1);
  }
}

//Prints the message to player informing them of their death
void printDeath(Adafruit_ST7735 *tft) {
  (*tft).setCursor(0,10*8);
  (*tft).setTextSize(1);
  (*tft).setTextColor(ST7735_WHITE, ST7735_BLACK);
  (*tft).print("Oh dear, you are dead!");
  (*tft).setCursor(0, 12*8);
  (*tft).print("Press select to return to start");
}

//Prints the message to player informing them of their death
void printWin(Adafruit_ST7735 *tft) {
  (*tft).setCursor(0,10*8);
  (*tft).setTextSize(1);
  (*tft).setTextColor(ST7735_WHITE, ST7735_BLACK);
  (*tft).print("Level Cleared!");
  (*tft).setCursor(0, 12*8);
  (*tft).print("Press select to go to the next level.");
}

//sets all array values to 0
void init_map(uint8_t map[20][16]) {
  for (uint8_t i = 0; i < MAP_HEIGHT; i++) {
    for (uint8_t t = 0; t < MAP_WIDTH; ++t) {
      map[i][t] = 0;
    }
  }
}

// Tiles
// x and y are the tile's coordinates

//Used for all 8x8 rectangle drawing
void drawTile(Adafruit_ST7735 *tft, uint8_t x, uint8_t y, int color){
  (*tft).fillRect(x*TILE_WIDTH, y*TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, color);
}

//slightly smaller than tile drawing
void drawBomb(Adafruit_ST7735 *tft, uint8_t x, uint8_t y) {
  (*tft).fillRect(x*TILE_WIDTH+1, y*TILE_HEIGHT+1, TILE_WIDTH-2, TILE_HEIGHT-2, BLACK);
}

//draws plus sign shaped objects
//will need to change to show objects block explosions
void drawExplosion(Adafruit_ST7735 *tft, uint8_t x, uint8_t y, uint8_t explo_size) {
  (*tft).fillRect(x*TILE_WIDTH-TILE_WIDTH*explo_size, y*TILE_HEIGHT, TILE_WIDTH*(2*explo_size+1), TILE_HEIGHT, EXPLOSION);
  (*tft).fillRect(x*TILE_WIDTH, y*TILE_HEIGHT-TILE_HEIGHT*explo_size, TILE_WIDTH, TILE_HEIGHT*(2*explo_size+1), EXPLOSION);
}

//draws a light green background and a triangle over top
void drawTree(Adafruit_ST7735 *tft, uint8_t x, uint8_t y, int color){
  drawTile(tft, x, y, GREEN);
  //vertecs coordinates then color
  (*tft).fillTriangle(x*TILE_WIDTH + TILE_WIDTH, y*TILE_HEIGHT + TILE_HEIGHT, x*TILE_WIDTH-1, y*TILE_HEIGHT + TILE_HEIGHT, x*TILE_WIDTH + TILE_WIDTH/2, y*TILE_HEIGHT, color);
}

//draws a circle
//do not use without coloring the background before (due to tile corners)
void drawCircle(Adafruit_ST7735 *tft, uint8_t x, uint8_t y, int color){
  (*tft).fillCircle(x*TILE_WIDTH + TILE_WIDTH/2-1, y*TILE_HEIGHT + TILE_HEIGHT/2-1, 3, color);
}

//converts map position and number into a colored tile on the screen
void drawTile(Adafruit_ST7735 *tft, uint8_t map1[20][16], uint8_t x, uint8_t y) {
  // Serial.println(map1[y][x]);
  if (map1[y][x] == 0) {
    drawTile(tft, x, y, GREEN);
  }
  else if (map1[y][x] == 1) {
    drawTile(tft, x, y, BROWN);
  }
  else if (map1[y][x] == 2) {
    drawTile(tft, x, y, BLUE);
  }
  else if (map1[y][x] == 3 || map1[y][x] == 5) {
    drawTree(tft, x, y, TREE);
  }
  else if (map1[y][x] == 4) {
    drawTile(tft, x, y, STONE);
  }

  else if (map1[y][x] == 6) {
    drawTile(tft, x, y, WHITE);
  }
}

//pass in array ie: map1
void addGridOfBlocks(uint8_t map[20][16], uint8_t space, uint8_t num){
  for (uint8_t i = space; i < MAP_HEIGHT; i += (space+1)) {
    for (uint8_t x = space; x < MAP_WIDTH; x+= (space+1)) {
      map[i][x] = num;
    }
  }
}

//pass in array ie: map1
void addSemiRandomBlocks(uint8_t map[20][16], uint8_t type){
  uint8_t num = random(45);
  for (uint8_t i = 0; i < num; i++) {
    uint8_t x = random(15)+1;
    uint8_t y = random(19)+1;
    map[y][x] = type;
  }
}

// Draw an entire map
void loadMap(Adafruit_ST7735 *tft, uint8_t map[20][16]) {
  uint8_t x;
  uint8_t y;
  for (y = 0; y < MAP_HEIGHT; y++) {
    for (x = 0; x < MAP_WIDTH; x++) {
      drawTile(tft, map, x, y);
    }
  }
}

void removeTree(uint8_t map[20][16], uint8_t y, uint8_t x, uint8_t num) {
  map[y][x] = num;
}


//checks for trees in the explosion
void removeTrees(uint8_t map[20][16], uint8_t x, uint8_t y, uint8_t explo_size) {
  for (uint8_t i = 1; i <= explo_size; i++) {
    if (map[y][x+i] == 3 && x <= (15-i)){
      removeTree(map, y,x+i, 1);
    }
    else if (map[y][x+i] == 5 && x <= (15-i)){
      removeTree(map, y,x+i, 6);
    }

    if (map[y][x-i] == 3 && x>=i) {
      removeTree(map, y,x-i, 1);
    }
     else if (map[y][x-i] == 5 && x>=i){
      removeTree(map, y,x-i, 6);
    }

    if (map[y+i][x] == 3 && y<=(19-i)){
      removeTree(map, y+i,x, 1);
    }
    else if (map[y+i][x] == 5 && y<=(19-i)){
      removeTree(map, y+i,x, 6);
    }

    if (map[y-i][x] == 3 && y>=i) {
      removeTree(map, y-i,x, 1);
    }
    else if (map[y-i][x] == 5 && y>=i)
    {
      removeTree(map, y-i,x, 6);
    }
  }
}

//adds 30 trees (3) to the map and stores their location in trees
void addTrees(tree trees[40], uint8_t map[20][16]){
  uint8_t t = getRandom(29);
  for (uint8_t i = 0; i < 40; i++) {
    uint8_t y, x;
    do {
      x = random(15);//range: 0-15
      y = random(16)+4;// range: 4-19
    } while(map[y][x]==3 || map[y][x]==5);//do until no tree at this location

    trees[i].setXY(x,y);  //stores to a single 8 bit number
    if (x != trees[i].getX()) {
      Serial.print("ERROR X ");
      Serial.print(trees[i].getX());
      Serial.print(" != ");
      Serial.println(x);
    }

    if (y != trees[i].getY()) {
      Serial.print("ERROR Y ");
      Serial.print(trees[i].getY());
      Serial.print(" != ");
      Serial.println(y);
    }

    if (i == t)
    {
      map[y][x] = 5;
    } else
    {
      map[y][x] = 3;
    }
  }
}

//unexplodes trees and moves the powerup;
void resetTrees(tree trees[40], uint8_t map[20][16]) {
  uint8_t t = getRandom(39);
  for (uint8_t i = 0; i < 40; i++) {
    if (i == t)
    {
      map[trees[i].getY()][trees[i].getX()] = 5;
    } else
    {
      map[trees[i].getY()][trees[i].getX()] = 3;
    }
  }
}
