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
  void update(TextureBuffer* screenBuffer, Input* input, float lastDeltaMs);
  void cleanUp();
  
private:
  Vec2f offset;
  TextureBuffer testTexture;
  void fillScreen(TextureBuffer* screenBuffer);
};


