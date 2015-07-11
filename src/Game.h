#pragma once
#include <SDL.h>
#include <jpb/Vector.h>
#include "main.h"

class Cube {
public:
  
  Cube(Vec3f centerPosition = Vec3f(), float sideLength = 1.0f, Vec3f color = Vec3f(1.0f, 0, 0)) :
    centerPosition(centerPosition), sideLength(sideLength), color(color) {}
  
private:
  Vec3f centerPosition;
  float sideLength;
  Vec3f color;
};

class SoftwareRenderer {
public:
  
  void drawLine(Vec2f p1, Vec2f p2, Vec3f color, TextureBuffer* texture) const ;
  void drawSquare(Vec2f pos, float sideLength, Vec3f color, TextureBuffer* texture) const ;
};

class Game
{
public:
  void start();
  void update(TextureBuffer* screenBuffer, Input* input, float lastDeltaMs);
  void cleanUp();
  
private:
  Vec2f offset;
  SDL_Texture* screenBuffer = NULL;
  void fillScreen(TextureBuffer* screenBuffer);
};


