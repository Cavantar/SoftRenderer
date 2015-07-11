#include <iostream>
#include <algorithm>
#include <jpb/Types.h>
#include "Game.h"


#define sign(a) (a > 0 ? 1 : -1)

void TextureBuffer::setPixel(uint32 x, uint32 y, const Vec3f& color)
{
  if(x > 0 && x < textureDimensions.x &&
     y > 0 && y < textureDimensions.y)
  {
    pixelData[x + (uint32)textureDimensions.x * y] =
      (uint32)(color.x) << 24 | (uint32)(color.y) << 16  | (uint32)(color.z) << 8;
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

  real32 radAngle = (rotAngle / 180.0f) * M_PI;
  
  for(int i = 0; i < 8; i++)
  {
    Vec3f& src = vertices[i];
    src.rotateAroundY(radAngle);
    src.rotateAroundX(radAngle);
    
    src.x += sideLength * 2.0f;
    src.rotateAroundY(radAngle);
    
    src += centerPosition;
  }
  
  return vertices;
};

void SoftwareRenderer::drawLine(Vec2f p1, Vec2f p2, Vec3f color, TextureBuffer* texture) const 
{
  Vec2f deltaVector = p2 - p1;
  
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
    
    // NOTE(jakub): fix this thing.
    int32 dx = deltaVector.x != 0 ? ceil(deltaVector.x) : 1;
    
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
	
	if(deltaVector.y < 0 && y >= p2.y ||
	   deltaVector.y > 0 && y <= p2.y)
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
    
    // NOTE(jakub): fix this thing.
    int32 dx = deltaVector.x != 0 ? abs(floor(deltaVector.x)) : 1;
    
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
	
	if(deltaVector.y < 0 && y >= p2.y ||
	   deltaVector.y > 0 && y <= p2.y)
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
  float fov = (M_PI / 2.0f) * 1.5f;
  float dfc = 1.0f / tan(fov / 2.0f);
  
  real32 aspectRatio = texture->textureDimensions.x / texture->textureDimensions.y;
  
  // Transforming Coordinates
  for(int i = 0; i != 8; i++)
  {
    uint32 pixelIndex = i;
    
    Vec3f& src = vertices[i];
    Vec2f& dst = vertices2d[pixelIndex];
    
    // Perspective calculation
    dst.x = (src.x / src.z) * dfc;
    dst.y = (src.y / src.z) * dfc;

    // Transforming Into ScreenSpace
    dst.x = (dst.x * texture->textureDimensions.x * 0.5) + texture->textureDimensions.x * 0.5;
    dst.y = ((dst.y * aspectRatio) * texture->textureDimensions.y * 0.5) + texture->textureDimensions.y * 0.5;
    
    texture->setPixel(dst.x, dst.y, Vec3f(255.0f, 255.0f, 255.0f));
  }

  Vec3f cubeColor = cube.getColor();


  Triangle triangles[12];
  
  triangles[0].vertices[0] = vertices2d[0];
  triangles[0].vertices[1] = vertices2d[1];
  triangles[0].vertices[2] = vertices2d[2];
  
  triangles[1].vertices[0] = vertices2d[2];
  triangles[1].vertices[1] = vertices2d[3];
  triangles[1].vertices[2] = vertices2d[0];
  
  drawTriangle(triangles[0], texture, Vec3f(128.0f, 128.0f, 0));
  drawTriangle(triangles[1], texture, Vec3f(128.0f, 0, 0));
  
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

void SoftwareRenderer::drawTriangle(Triangle& triangle, TextureBuffer* texture, Vec3f color)
{
  std::vector<ScanLine> scanLines = getScanLines(triangle);
  for(auto it = scanLines.begin(); it != scanLines.end(); it++)
  {
    ScanLine& scanLine =  *it;
    drawLine(Vec2f(scanLine.startX, scanLine.y), Vec2f(scanLine.endX, scanLine.y), color, texture);
  }
}

ScanLineVector SoftwareRenderer::getScanLines(Triangle& triangle)
{
  ScanLineVector result;
  
  real32 minY = std::min(std::min(triangle.vertices[0].y,
				  triangle.vertices[1].y),
			 triangle.vertices[2].y);
  
  real32 maxY = std::max(std::max(triangle.vertices[0].y,
				  triangle.vertices[1].y),
			 triangle.vertices[2].y);
  
  real32 minX = std::min(std::min(triangle.vertices[0].x,
				  triangle.vertices[1].x),
			 triangle.vertices[2].x);
  
  real32 maxX = std::max(std::max(triangle.vertices[0].x,
				  triangle.vertices[1].x),
			 triangle.vertices[2].x);

  Vec2f st[3] =
    {
      triangle.vertices[1],
      triangle.vertices[2],
      triangle.vertices[0]
    };

  Vec2f ls[3] =
    {
      st[0] - triangle.vertices[0],
      st[1] - triangle.vertices[1],
      st[2] - triangle.vertices[2]
    };
  
  for(int y = minY; y <= maxY; y++)
  {
    ScanLine scanLine;
    scanLine.y = y;

    real32 xValues[3];

    for(int i = 0; i < 3; i++)
    {
      real32 xValue = ls[i].getXFor(y, st[i]);
      
      if((ls[i].y < 0 && (y < st[i].y || y > triangle.vertices[i].y)) ||
	 (ls[i].y > 0 && (y > st[i].y || y < triangle.vertices[i].y)))
      	xValue = maxX + 1;
      
      xValues[i] = xValue;
    }
    
    uint32 minScanX = -1;
    uint32 maxScanX = -1;
    
    for(int i = 0; i < 3; i++)
    {
      if(xValues[i] >= minX && xValues[i] <= maxX)
      {
	if(minScanX == -1 && maxScanX == -1)
	{
	  minScanX = ceil(xValues[i]);
	  maxScanX = floor(xValues[i]);
	}
	else
	{
	  minScanX = std::min(minScanX, (uint32)ceil(xValues[i]));
	  maxScanX = std::max(maxScanX, (uint32)floor(xValues[i]));
	}
	// std::cout << i << " msx: "<< xValues[i] << std::endl;
      }
      
    }

    // std::cout << "y: " << y << " min: " << minScanX << " max: " << maxScanX << std::endl;
    
    scanLine.startX = minScanX;
    scanLine.endX = maxScanX;
    
    result.push_back(scanLine);    
  }
  
  // ScanLine scanLine = { minY, minX, (minX + 10) };
  // result.push_back(scanLine);

  return result;
}

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
  static Vec3f cubePosition(0, 0, 2.0f);
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
  
  fillScreen(screenBuffer);

  static real32 localTime = 0;
  localTime += lastDeltaMs;

  const real32 period = 0.5f;
  real32 tempTest = fmodf(localTime / 1000.0f, period) / period;
  SoftwareRenderer softwareRenderer;

  static real32 rotAngle = 0;
  static bool rotToggle = true;
  if(input->keysPressed[SDLK_c]) rotToggle = !rotToggle;
  if(rotToggle)
  {
    rotAngle += lastDeltaMs / 100.0f;
  }
  
  
#if 0
  
  // Triangle Thing
  //--------------------------------
  real32 triangleDistance = 25.0f; 
  
  Vec2f trianglePosition(100.0f, 100.0f);
  
  Vec2f position1 = Vec2f(-triangleDistance, triangleDistance);
  Vec2f position2 = Vec2f(0, -triangleDistance);
  Vec2f position3 = Vec2f(triangleDistance, triangleDistance);
  
  position1.rotate(rotAngle);
  position2.rotate(rotAngle);
  position3.rotate(rotAngle);
  
  Triangle triangle = {trianglePosition + position1,
		       trianglePosition + position2,
		       trianglePosition + position3};
  
  softwareRenderer.drawTriangle(triangle, screenBuffer, Vec3f(0, 255.0f, 0));
  
  softwareRenderer.drawLine(triangle.vertices[0], triangle.vertices[1],
  			    Vec3f(255.0f, 0, 0), screenBuffer);  
  
  softwareRenderer.drawLine(triangle.vertices[1], triangle.vertices[2],
  			    Vec3f(255.0f, 0, 0), screenBuffer);  
  
  softwareRenderer.drawLine(triangle.vertices[2], triangle.vertices[0],
  			    Vec3f(255.0f, 0, 0), screenBuffer);
  
  //--------------------------------
#else 
  
  Triangle triangle = {Vec2f(100, 100),
		       Vec2f(150, 50),
		       Vec2f(100, 150)};
  
  softwareRenderer.drawTriangle(triangle, screenBuffer, Vec3f(0, 255.0f, 0));
  
  softwareRenderer.drawLine(triangle.vertices[0], triangle.vertices[1],
  			    Vec3f(255.0f, 0, 0), screenBuffer);  
  
  softwareRenderer.drawLine(triangle.vertices[1], triangle.vertices[2],
  			    Vec3f(255.0f, 0, 0), screenBuffer);  
  
  softwareRenderer.drawLine(triangle.vertices[2], triangle.vertices[0],
  			    Vec3f(255.0f, 0, 0), screenBuffer);
  
#endif
  
  Vec2f tempPosition(200, 200);
  // softwareRenderer.drawLine(tempPosition, tempPosition + Vec2f::directionVector(rotAngle) * 200.0f,
  // 			    Vec3f(0, 255.0f, 0), screenBuffer);  
  
   
  // softwareRenderer.drawLine(tempPosition, tempPosition + Vec2f(2.0f, 10.0f),
  // Vec3f(0, 255.0f, 0), screenBuffer);    
  
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
