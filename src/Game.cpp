#include <jpb/Types.h>
#include "Game.h"

#define sign(a) (a > 0 ? 1 : -1)

void SoftwareRenderer::drawLine(Vec2f p1, Vec2f p2, Vec3f color, TextureBuffer* texture) const 
{
  
  if(p1.x <= p2.x)
  {
    // First Quadrant
    //  |
    //==|==
    //  |xx

    // Second Quadrant
    //  |xx
    //==|==
    //  |
    
    Vec2f deltaVector = p2 - p1;
    
    // NOTE(jakub): fix this thing.
    int32 dx = (int32)deltaVector.x != 0 ? deltaVector.x : 1;
    
    real32 a = deltaVector.x != 0 ? abs(deltaVector.y / deltaVector.x) : abs(deltaVector.y);
    real32 tempDelta = 0;
    
    int y = p1.y;
    
    for(int x = 0; x < dx; x++)
    {
      int32 realX = p1.x + x;
      
      tempDelta += a;
      texture->setPixel(realX, y, color);
      
      while(tempDelta >= 1.0f)
      {
	y += sign(deltaVector.y);
	tempDelta -= 1.0f;
		
	if(deltaVector.y < 0 && y > p2.y ||
	   deltaVector.y > 0 && y < p2.y)
	{
	  texture->setPixel(realX, y, color);
	}

      }
    }
  }
  else 
  {
    
    // Third Quadrant
    //  |
    //==|==
    //xx|

    // Fourth Quadrant
    //xx|
    //==|==
    //  |
    
    Vec2f deltaVector = p2 - p1;
    
    // NOTE(jakub): fix this thing.
    int32 dx = abs((int32)deltaVector.x != 0 ? deltaVector.x : 1);
    
    real32 a = deltaVector.x != 0 ? abs(deltaVector.y / deltaVector.x) : abs(deltaVector.y);
    real32 tempDelta = 0;
    
    int y = p1.y;
    
    for(int x = 0; x < dx; x++)
    {
      int32 realX = p1.x - x;
      
      tempDelta += a;
      texture->setPixel(realX, y, color);
      
      while(tempDelta >= 1.0f)
      {
	y += sign(deltaVector.y);
	tempDelta -= 1.0f;
	
	if(deltaVector.y < 0 && y > p2.y ||
	   deltaVector.y > 0 && y < p2.y)
	{
	  texture->setPixel(realX, y, color);
	}
	
      }
    }
  }
}

void SoftwareRenderer::drawSquare(Vec2f pos, float sideLength, Vec3f color, TextureBuffer* texture) const
{
  Vec2f topLeft = pos + Vec2f(-sideLength / 2.0f, -sideLength / 2.0f);
  Vec2f topRight = pos + Vec2f(sideLength / 2.0f, -sideLength / 2.0f);
  
  Vec2f bottomLeft = pos + Vec2f(-sideLength / 2.0f, sideLength / 2.0f);
  Vec2f bottomRight = pos + Vec2f(sideLength / 2.0f, sideLength / 2.0f);
  
  drawLine(topLeft, topRight, color, texture);
  drawLine(bottomLeft, bottomRight, color, texture);
  
  drawLine(topLeft, bottomLeft, color, texture);
  drawLine(topRight, bottomRight, color, texture);
}

void Game::start()
{
  
}

void Game::update(TextureBuffer* screenBuffer, Input* input, float lastDeltaMs)
{
  static const real32 scrollSpeed = 0.25f;

  if(input->keysDown[SDLK_w])
  {
    offset.y -= lastDeltaMs * scrollSpeed;
  }

  if(input->keysDown[SDLK_s])
  {
    offset.y += lastDeltaMs * scrollSpeed;
  }

  if(input->keysDown[SDLK_a])
  {
    offset.x -= lastDeltaMs * scrollSpeed;
  }

  if(input->keysDown[SDLK_d])
  {
    offset.x += lastDeltaMs * scrollSpeed;
  }
  
  fillScreen(screenBuffer);

  static real32 localTime = 0;
  localTime += lastDeltaMs;

  const real32 period = 0.5f;
  real32 tempTest = fmodf(localTime / 1000.0f, period) / period;
  SoftwareRenderer softwareRenderer;
  
  Vec2f tempPosition(200, 200);
  softwareRenderer.drawLine(tempPosition, tempPosition + Vec2f::directionVector(tempTest * 360.0f) * 200.0f,
			    Vec3f(0, 255.0f, 0), screenBuffer);  
  
  
  // for(int i = 0; i < 10; i++)
  // {
  //   softwareRenderer.drawSquare(Vec2f(100 + i * 10, 100 + i * 10), 50.0f, Vec3f(255.0f, 0, 0), screenBuffer);
  // }
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
  SDL_DestroyTexture(screenBuffer);
}
