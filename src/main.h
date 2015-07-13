#pragma once
#include <jpb/Types.h>
#include <jpb/Vector.h>

struct TextureBuffer {
  uint32* pixelData;
  Vec2i dimensions;
  int32 pitch;
  
  void setPixel(uint32 x, uint32 y, const Vec3f& color);
  Vec3f getPixel(uint32 x, uint32 y);
};

const int32 KEYCOUNT = 256;

const int32 SCREEN_WIDTH = 1280;
const int32 SCREEN_HEIGHT = 720;

struct Input {
  bool keysDown[KEYCOUNT];
  bool keysPressed[KEYCOUNT];
  bool keysReleased[KEYCOUNT];
};



