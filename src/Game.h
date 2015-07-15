#pragma once
#include <SDL.h>
#include <jpb/Vector.h>
#include <vector>
#include "main.h"
#include "SoftwareRenderer.h"

class Game
{
public:
  
  Game();
  ~Game();
  void start();
  void update(TextureBuffer* screenBuffer, const Input* input, float lastDeltaMs);
  void cleanUp();
  
private:
  
  Vec2f offset;
  TextureBuffer testTexture;
  
  real32 rotAngleX = 30.0f;
  real32 rotAngleY = 30.0f;
  
  Vec3f cubePosition = Vec3f(0.25f, 0, 2.0f);
  
  void handleInput(const Input* input, float lastDeltaMs);
  void fillScreen(TextureBuffer* screenBuffer);
};


