#include <iostream>
#include <algorithm>
#include <jpb/Types.h>
#include "Game.h"

#define sign(a) (a > 0 ? 1 : -1)

void Game::start()
{
  // // Test

  // Vec2f startV1(10.0f, 0);
  // Vec2f tempV1(10.0f, 10.0f);

  // // For what x y value will be 0
  // // y = ax + b
  // real32 a = tempV1.y / tempV1.x;

  // real32 searchY = 1;
  // real32 x = startV1.x + (searchY - startV1.y)/a ;
  
  // std::cout << "y: " << searchY << " " << "x: " << x << std::endl;
}

void Game::update(TextureBuffer* screenBuffer, Input* input, float lastDeltaMs)
{
  static const real32 scrollSpeed = 0.25f;
  static const real32 rotationSpeed = 0.1f;
  static real32 rotAngleX = 30.0f;
  static real32 rotAngleY = 30.0f;
  
  static Vec3f cubePosition(0, 0.5f, 2.0f);
  real32 cubeMoveSpeed = 0.001f;
  
  if(input->keysDown[SDLK_w])
  {
    offset.y -= lastDeltaMs * scrollSpeed;
    cubePosition.z += lastDeltaMs * cubeMoveSpeed;
  }
  
  if(input->keysDown[SDLK_s])
  {
    offset.y += lastDeltaMs * scrollSpeed;
    cubePosition.z -= lastDeltaMs * cubeMoveSpeed;
  }

  if(input->keysDown[SDLK_a])
  {
    offset.x -= lastDeltaMs * scrollSpeed;
  }

  if(input->keysDown[SDLK_d])
  {
    offset.x += lastDeltaMs * scrollSpeed;
  }

  if(input->keysDown[SDLK_e])
  {
    rotAngleX += lastDeltaMs * rotationSpeed;
  }
  
  if(input->keysDown[SDLK_r])
  {
    rotAngleX -= lastDeltaMs * rotationSpeed;
  }

  if(input->keysDown[SDLK_n])
  {
    rotAngleY += lastDeltaMs * rotationSpeed;
  }
  
  if(input->keysDown[SDLK_m])
  {
    rotAngleY -= lastDeltaMs * rotationSpeed;
  }
  
  fillScreen(screenBuffer);

  static real32 localTime = 0;
  localTime += lastDeltaMs;

  const real32 period = 0.5f;
  real32 tempTest = fmodf(localTime / 1000.0f, period) / period;
  
  SoftwareRenderer softwareRenderer((M_PI / 2.0f) * 1.0f);

  static real32 rotAngle = 0;
  static bool rotToggle = true;
  if(input->keysPressed[SDLK_c]) rotToggle = !rotToggle;
  
  if(rotToggle)
  {
    rotAngle += lastDeltaMs / 100.0f;
  }
  
  
  Triangle triangle = {Vec3f(100, 100),
		       Vec3f(150, 50),
		       Vec3f(100, 150)};
  
  VerticesVector2D polygonVertices1 = { Vec2f(100, 100), Vec2f(150, 50), Vec2f(100, 150) };
  Polygon2D polygon1 = { polygonVertices1 };
  softwareRenderer.drawPolygon(screenBuffer, polygon1, Vec3f(0, 255.0f, 0));

  Vec2f offset(100, 0);
  VerticesVector2D polygonVertices2 =
    {
      Vec2f(100, 100) + offset,
      Vec2f(150, 50) + offset,
      Vec2f(180, 30) + offset,
      Vec2f(250, 60) + offset,
      Vec2f(100, 150) + offset
    };
  
  Polygon2D polygon2 = { polygonVertices2 };
  softwareRenderer.drawPolygon(screenBuffer, polygon2, Vec3f(0, 0, 255.0f));
  
  Cube cube(cubePosition, 0.2f);
  softwareRenderer.drawCubeInPerspective(screenBuffer, cube, rotAngleX, rotAngleY);
}

void Game::fillScreen(TextureBuffer* screenBuffer)
{
  uint8* pixelData = (uint8*)screenBuffer->pixelData;
  bool toggle = false;
  
  for(int y = 0; y < screenBuffer->textureDimensions.y; ++y)
  {
    uint8* row = pixelData + screenBuffer->pitch * y;
    for(int x = 0; x < screenBuffer->textureDimensions.x; ++x)
    {
      uint32* pixelValue = (uint32*)(row + x * 4);
      
      uint32 redValue = ((y + (uint32)offset.y) * (uint32)screenBuffer->textureDimensions.x) % 255; 
      uint32 greenValue = (x + (uint32)offset.x) % 255; 
      uint32 blueValue = 0; 
      uint8 alphaValue = 0;       
      
      *pixelValue = alphaValue | blueValue << 8 | greenValue << 16 | redValue << 24;
    }
  }
}

void Game::cleanUp()
{
}
