#pragma once
#include <SDL.h>
#include <jpb/Vector.h>
#include <vector>
#include "main.h"
#include "SoftRenderer.h"

class Game {
public:
  
  Game();
  ~Game();
  
  void start(const Vec2i& screenResolution);
  void update(TextureBuffer* screenBuffer, const Input& input, float lastDeltaMs);
  void cleanUp();
private:
  
  SoftRenderer softRenderer;
  FPSCamera camera;
  
  Vec2f offset;
  TextureBuffer testTexture;
  
  real32 rotAngleX = 30.0f;
  real32 rotAngleY = 30.0f;
  
  Vec3f cubePosition = Vec3f(0.25f, 0, 2.0f);
  
  void handleInput(const Input& input, float lastDeltaMs);
  void fillScreen(TextureBuffer* screenBuffer);
};


