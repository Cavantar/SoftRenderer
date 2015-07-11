#include <jpb/Types.h>
#include <iostream>
#include "Game.h"


#define sign(a) (a > 0 ? 1 : -1)

void TextureBuffer::setPixel(uint32 x, uint32 y, const Vec3f& color)
{
  if(x > 0 && x < textureDimensions.x &&
     y > 0 && y < textureDimensions.y)
  {
    pixelData[x + (uint32)textureDimensions.x * y] =
      (uint32)(color.x) << 24 | (uint32)(color.z) << 16  | (uint32)(color.z) << 8;
  }
}

/*
  Coordinate System
  |
  |y+ x+
  -------
  |y- x+
  |

  Z is forward
*/

VerticesVector Cube::getVertices(real32 rotAngle) const
{
  
  VerticesVector vertices;
  vertices.resize(8);
  float halfSideLength = sideLength / 2.0f;
  
  // Bottom Part
  vertices[0] = Vec3f(-halfSideLength, -halfSideLength, -halfSideLength);
  vertices[1] = Vec3f(-halfSideLength, -halfSideLength, halfSideLength);
  vertices[2] = Vec3f(halfSideLength, -halfSideLength, halfSideLength);
  vertices[3] = Vec3f(halfSideLength, -halfSideLength, -halfSideLength);

  // Top Part
  vertices[4] = vertices[0] + Vec3f(0, sideLength, 0);
  vertices[5] = vertices[1] + Vec3f(0, sideLength, 0);
  vertices[6] = vertices[2] + Vec3f(0, sideLength, 0); 
  vertices[7] = vertices[3] + Vec3f(0, sideLength, 0);

  real32 rotAngleRad = (rotAngle / 180.0f) * M_PI;
  
  for(int i = 0; i < 8; i++)
  {
    Vec3f& src = vertices[i];
    Vec3f rotatedPosition;
    
    // Rotation
    rotatedPosition.x = src.x * cos(rotAngleRad) - src.z * sin(rotAngleRad);
    rotatedPosition.z = src.x * sin(rotAngleRad) + src.z * cos(rotAngleRad);
    rotatedPosition.y = src.y;
    
    src = rotatedPosition;
    src += centerPosition;
  }
  
  return vertices;
};

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

void SoftwareRenderer::drawCubeInPerspective(const Cube& cube, TextureBuffer* texture, real32 rotAngle)
{
  VerticesVector vertices = cube.getVertices(rotAngle);
  
  std::vector<Vec2f> vertices2d;
  vertices2d.resize(8);
  
  // 90 Degrees
  float fov = M_PI / 2.0f;
  float dfc = tan(fov / 2.0f);
  
  // std::cout << dfc << std::endl;
  
  real32 aspectRatio = texture->textureDimensions.x / texture->textureDimensions.y;
  
  // Transforming Coordinates
  for(int i = 0; i != 8; i++)
  {
    uint32 pixelIndex = i;
    
    Vec3f& src = vertices[i];
    
    // std::cout << pixelIndex << ": " << src.x << " " << src.z << std::endl;
    
    Vec2f& dst = vertices2d[pixelIndex];
    
    // Perspective calculation
    dst.x = (src.x / src.z) * 1.0f;
    dst.y = (src.y / src.z) * 1.0f;

    // Transforming Into ScreenSpace
    dst.x = (dst.x * texture->textureDimensions.x * 0.5) + texture->textureDimensions.x * 0.5;
    dst.y = ((dst.y * aspectRatio) * texture->textureDimensions.y * 0.5) + texture->textureDimensions.y * 0.5;
    
    texture->setPixel(dst.x, dst.y, Vec3f(255.0f, 255.0f, 255.0f));
    
    // std::cout << pixelIndex << ": " << dst.x << " " << dst.y << std::endl;
  }

  Vec3f cubeColor = cube.getColor();
  
  // Drawing Lines
  for(int i = 0; i != 2; i++)
  {
    
    // Horizontal Lines
    drawLine(vertices2d[i * 4 + 0], vertices2d[i * 4 + 1],
	     cubeColor, texture);
    
    drawLine(vertices2d[i * 4 + 1], vertices2d[i * 4 + 2],
	     cubeColor, texture);
    
    drawLine(vertices2d[i * 4 + 2], vertices2d[i * 4 + 3],
	     cubeColor, texture);
    
    drawLine(vertices2d[i * 4 + 3], vertices2d[i * 4 + 0],
	     cubeColor, texture);

    // Vertical Lines
    drawLine(vertices2d[i*2], vertices2d[4 + i*2],
	     cubeColor, texture);
    
    drawLine(vertices2d[i*2 + 1], vertices2d[4 + (i*2) + 1],
	     cubeColor, texture);
    
  }
}

void Game::start()
{
  
}

void Game::update(TextureBuffer* screenBuffer, Input* input, float lastDeltaMs)
{
  static const real32 scrollSpeed = 0.25f;
  static Vec3f cubePosition(0, 0, 5.0f);
  real32 cubeMoveSpeed = 0.01f;
  
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
  
  fillScreen(screenBuffer);

  static real32 localTime = 0;
  localTime += lastDeltaMs;

  const real32 period = 0.5f;
  real32 tempTest = fmodf(localTime / 1000.0f, period) / period;
  SoftwareRenderer softwareRenderer;
  
  Vec2f tempPosition(200, 200);
  softwareRenderer.drawLine(tempPosition, tempPosition + Vec2f::directionVector(tempTest * 360.0f) * 200.0f,
			    Vec3f(0, 255.0f, 0), screenBuffer);  
  
  static real32 rotAngle = 0;
  rotAngle += lastDeltaMs / 100.0f;
  
  Cube cube(cubePosition, 0.2f);
  softwareRenderer.drawCubeInPerspective(cube, screenBuffer, rotAngle);
  
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
}
