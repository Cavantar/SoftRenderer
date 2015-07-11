#pragma once
#include <SDL.h>
#include <jpb/Vector.h>
#include <vector>
#include "main.h"

struct TextureBuffer {
  uint32* pixelData;
  Vec2f textureDimensions;
  int32 pitch;
  
  void setPixel(uint32 x, uint32 y, const Vec3f& color);
};

typedef std::vector<Vec3f> VerticesVector;

class Cube {
public:
  
  Cube(Vec3f centerPosition = Vec3f(), float sideLength = 1.0f, Vec3f color = Vec3f(1.0f, 0, 0)) :
    centerPosition(centerPosition), sideLength(sideLength), color(color) {}

  VerticesVector getVertices(real32 rotAngle) const ;
  Vec3f getColor() const { return color; }
private:
  Vec3f centerPosition;
  float sideLength;
  Vec3f color;
};

class SoftwareRenderer {
public:
  
  void drawLine(Vec2f p1, Vec2f p2, Vec3f color, TextureBuffer* texture) const ;
  void drawSquare(Vec2f pos, float sideLength, Vec3f color, TextureBuffer* texture) const ;
  void drawCubeInPerspective(const Cube& cube, TextureBuffer* texture, real32 rotAngle = 0);
};

class Game
{
public:
  void start();
  void update(TextureBuffer* screenBuffer, Input* input, float lastDeltaMs);
  void cleanUp();
  
private:
  Vec2f offset;
  void fillScreen(TextureBuffer* screenBuffer);
};


