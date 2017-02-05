#include <iostream>
#include <algorithm>
#include <jpb/Types.h>
#include "Game.h"

#define sign(a) (a > 0 ? 1 : -1)

Game::Game()
{
  // Generating Texture
  Vec2i textureDimensions(20, 20);

  testTexture.dimensions = textureDimensions;
  testTexture.pixelData = new uint32[textureDimensions.x * textureDimensions.y];

  for(int y = 0; y < textureDimensions.y; y++)
  {
    for(int x = 0; x < textureDimensions.x; x++)
    {
      Vec3f color(((real32)x / textureDimensions.x) * 255.0f, ((real32)y / textureDimensions.y) * 255.0f, 0);
      if(x == y || (textureDimensions.x - x - 1) == y ) color = Vec3f(0, 0, 255);
      testTexture.setPixel(x, y, color);
    }
  }
};

Game::~Game()
{
  delete[] testTexture.pixelData;
}

void Game::start(const Vec2i& screenResolution)
{
  softRenderer.setZBufferSize(screenResolution);
  camera.setPosition(Vec3f(2.0f, 2.0f, -2.0f));
}

void Game::update(TextureBuffer* screenBuffer, const Input& input, real32 lastDeltaMs)
{
  handleInput(input, lastDeltaMs);
  fillScreen(screenBuffer);

  static real32 localTime = 0;
  localTime += lastDeltaMs;

  camera.handleInput(input, lastDeltaMs);
  softRenderer.setCamera(&camera);

  Vec3f directionalLight(-1.0f, 0, 0);
  directionalLight.rotateAroundZDeg(45.0f);// + sin(localTime * 0.005f) * 10.0f);
  directionalLight.rotateAroundYDeg(localTime * 0.0002f * 360);
  softRenderer.setDirectionalLight(directionalLight);

  if(0)
  {
    if(0)
    {
      Polygon2D polygon = { { Vec2f(100, 100), Vec2f(150, 50), Vec2f(100, 150) } };
      softRenderer.drawPolygon(screenBuffer, polygon, Vec3f(0, 255.0f, 0));
    }

    real32 sideLength = 200.0f;
    Vec2f startPosition(500,300);
    Vec2f offset(20.0f, 5.0f);

    {
      Vec2f v1 = startPosition;
      Vec2f v2 = startPosition + Vec2f(sideLength, 0);
      Vec2f v3 = startPosition + Vec2f(2, sideLength);
      Vec2f v4 = startPosition + Vec2f(sideLength, sideLength);

      Vec2f vp1 = startPosition + Vec2f(0, sideLength / 2.0f) + offset;
      Vec2f vp2 = startPosition + Vec2f(sideLength / 2.0f, sideLength) + offset;

      Vec2f vc = (vp1 + vp2) / 2.0f;

      Polygon2D polygon1 = { { v1, v2, vc, vp1 } };
      softRenderer.drawPolygon(screenBuffer, polygon1, Vec3f(0, 255.0f, 0), false);

      Polygon2D polygon2 = { { v2, v4, vp2, vc } };
      softRenderer.drawPolygon(screenBuffer, polygon2, Vec3f(0, 0, 255.0f), false);
    }
  }

  if(0)
  {
    Vec2f offset(100, 0);
    Polygon2D polygon2 = {
      {
	Vec2f(100, 100) + offset,
	Vec2f(150, 50) + offset,
	Vec2f(180, 30) + offset,
	Vec2f(250, 60) + offset,
	Vec2f(100, 150) + offset
      }
    };
    softRenderer.drawPolygon(screenBuffer, polygon2, Vec3f(0, 0, 255.0f));
  }

  if(0)
  {
    real32 testScale = 1.0f;
    real32 testDistance = 0.53f;

    Vertices vertices =
      {
	Vec3f(-0.1 * testScale, 0.1 * testScale, 0),
	Vec3f(0.1 * testScale, 0.1 * testScale, 0),
	Vec3f(-0.1 * testScale, -0.1 * testScale, 0),
	Vec3f(0.1 * testScale, -0.1 * testScale, 0),
	// Vec3f(-0.2 * testScale, 0.1 * testScale, 0),
      };

    MeshHelper::rotateVertices(vertices, Vec3f(rotAngleX * 0.01f, rotAngleY * 0.01f, 0));
    MeshHelper::translateVertices(vertices, Vec3f(0, 0, testDistance));

    TriangleIndices triangleIndices = {{0, 1, 2}, {1,3,2}}; //, {4, 0, 2}};
    softRenderer.drawTriangles3D(screenBuffer, vertices, triangleIndices, false);
  }

  if(1)
  {

    // Cube Rendering
    // -----------------------
    cubePosition.z = 0.6f; // + sin(localTime * 0.0025f) * 0.3f;
    rotAngleY += lastDeltaMs * 0.10f;
    Cube cube(cubePosition, 0.2f);
    softRenderer.drawCubeInPerspective(screenBuffer, cube, rotAngleX, rotAngleY);
  }

  // TextureBuffer::blitTexture(screenBuffer, &testTexture, Vec2i(200, 200));

  if(1)
  {
    real32 testScale = 0.5f;
    real32 testDistance = sin(localTime * 0.005f) * 0.2f + 1.0f;
    static real32 rotationValue = 0;

    // if(input.keysDown[SDLK_a])
    //   rotationValue = sin(localTime * 0.002f) * 3.0f;

    Vec3f v1 = Vec3f(-testScale, testScale, 0);
    Vec3f v2 = Vec3f(testScale, testScale, 0);

    Vec3f v3 = Vec3f(-testScale, -testScale, 0);
    Vec3f v4 = Vec3f(testScale, -testScale, 0);

    real32 textureScale = 3.0f;
    Vec2f textOffset(0, 0);
    TriangleIndices triangleIndices = {{0, 1, 2}, {1, 3, 2}};
    static const real32 rotationSpeed = 0.001f;

    {
      MappedVertices baseFace = {
	{ v1, Vec2f(textOffset.x, textOffset.y), Vec3f() },
	{ v2, Vec2f(textOffset.x + 1.0f * textureScale, textOffset.y), Vec3f() },
	{ v3, Vec2f(textOffset.x + 0, 1.0f * textureScale + textOffset.y), Vec3f() },
	{ v4, Vec2f(textOffset.x + 1.0f * textureScale, 1.0f * textureScale + textOffset.y), Vec3f() }
      };

      // Translating to the center
      MeshHelper::translateVertices(baseFace, Vec3f(0, 0, -testScale));

      MappedVertices frontFace = baseFace;
      MeshHelper::rotateVertices(frontFace, Vec3f(0, localTime * rotationSpeed, 0));
      MeshHelper::calculateNormals(frontFace, triangleIndices);
      softRenderer.drawMappedTriangles3D(screenBuffer, frontFace, triangleIndices, &testTexture);

      MeshHelper::rotateVertices(frontFace, Vec3f(0, 90.0f, 0));
      MeshHelper::calculateNormals(frontFace, triangleIndices);
      softRenderer.drawMappedTriangles3D(screenBuffer, frontFace, triangleIndices, &testTexture);

      MeshHelper::rotateVertices(frontFace, Vec3f(0, 90.0f, 0));
      MeshHelper::calculateNormals(frontFace, triangleIndices);
      softRenderer.drawMappedTriangles3D(screenBuffer, frontFace, triangleIndices, &testTexture);

      MeshHelper::rotateVertices(frontFace, Vec3f(0, 90.0f, 0));
      MeshHelper::calculateNormals(frontFace, triangleIndices);
      softRenderer.drawMappedTriangles3D(screenBuffer, frontFace, triangleIndices, &testTexture);

      frontFace = baseFace;

      MeshHelper::rotateVertices(frontFace, Vec3f(90.0f, 0, 0));
      MeshHelper::rotateVertices(frontFace, Vec3f(0, localTime * rotationSpeed, 0));
      MeshHelper::calculateNormals(frontFace, triangleIndices);
      softRenderer.drawMappedTriangles3D(screenBuffer, frontFace, triangleIndices, &testTexture);

      frontFace = baseFace;
      MeshHelper::rotateVertices(frontFace, Vec3f(-90.0f, 0, 0));
      MeshHelper::rotateVertices(frontFace, Vec3f(0, localTime * rotationSpeed, 0));
      MeshHelper::calculateNormals(frontFace, triangleIndices);
      softRenderer.drawMappedTriangles3D(screenBuffer, frontFace, triangleIndices, &testTexture);
    }
  }

  softRenderer.clearZBuffer();
}

void
Game::handleInput(const Input& input, float lastDeltaMs)
{
  static const real32 scrollSpeed = 0.25f;
  static const real32 rotationSpeed = 0.1f;

  static const real32 cubeMoveSpeed = 0.001f;

  if(input.isKeyDown(SDLK_w))
  {
    offset.y -= lastDeltaMs * scrollSpeed;
    cubePosition.z += lastDeltaMs * cubeMoveSpeed;
  }

  if(input.isKeyDown(SDLK_s))
  {
    offset.y += lastDeltaMs * scrollSpeed;
    cubePosition.z -= lastDeltaMs * cubeMoveSpeed;
  }

  if(input.isKeyDown(SDLK_a))
  {
    offset.x -= lastDeltaMs * scrollSpeed;
  }

  if(input.isKeyDown(SDLK_d))
  {
    offset.x += lastDeltaMs * scrollSpeed;
  }

  if(input.isKeyDown(SDLK_e))
  {
    rotAngleX += lastDeltaMs * rotationSpeed;
  }

  if(input.isKeyDown(SDLK_r))
  {
    rotAngleX -= lastDeltaMs * rotationSpeed;
  }

  if(input.isKeyDown(SDLK_n))
  {
    rotAngleY += lastDeltaMs * rotationSpeed;
  }

  if(input.isKeyDown(SDLK_m))
  {
    rotAngleY -= lastDeltaMs * rotationSpeed;
  }

}

void
Game::fillScreen(TextureBuffer* screenBuffer)
{
  uint8* pixelData = (uint8*)screenBuffer->pixelData;
  bool toggle = false;

  for(int y = 0; y < screenBuffer->dimensions.y; ++y)
  {
    uint8* row = pixelData + screenBuffer->pitch * y;
    for(int x = 0; x < screenBuffer->dimensions.x; ++x)
    {
      uint32* pixelValue = (uint32*)(row + x * 4);

      uint32 redValue = ((y + (uint32)offset.y) * (uint32)screenBuffer->dimensions.x) % 255;
      uint32 greenValue = (x + (uint32)offset.x) % 255;
      uint32 blueValue = 0;
      uint8 alphaValue = 0;
      redValue = 120;
      greenValue = 120;
      blueValue = 120;

      *pixelValue = alphaValue | blueValue << 8 | greenValue << 16 | redValue << 24;
    }
  }
}

void Game::cleanUp()
{
}
