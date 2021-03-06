#include "SoftRenderer.h"

#include <algorithm>
#include <list>
#include <assert.h>

#define sign(a) (a > 0 ? 1 : -1)

/*
  Coordinate System
  |
  |y+ x+
  -------
  |y- x+
  |

  Z is forward
*/

Vertices
Cube::getVertices(real32 rotAngleX, real32 rotAngleY) const
{

  Vertices vertices;
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

  real32 radAngleY = (rotAngleY / 180.0f) * M_PI;
  real32 radAngleX = (rotAngleX / 180.0f) * M_PI;

  for(int i = 0; i < 8; i++)
  {
    Vec3f& src = vertices[i];
    src.rotateAroundY(radAngleY);
    src.rotateAroundX(radAngleX);
    src += centerPosition;
  }

  return vertices;
};

TriangleIndices
Cube::getTriangleIndexes()
{
  TriangleIndices indTriangleVector;
  indTriangleVector.resize(12);

  indTriangleVector[0] = {1, 0, 3};
  indTriangleVector[1] = {3, 2, 1};

  indTriangleVector[2] = {4, 5, 6};
  indTriangleVector[3] = {6, 7, 4};

  indTriangleVector[4] = {0, 4, 7};
  indTriangleVector[5] = {7, 3, 0};

  indTriangleVector[6] = {2, 6, 5};
  indTriangleVector[7] = {5, 1, 2};

  indTriangleVector[8] = {3, 7, 6};
  indTriangleVector[9] = {6, 2, 3};

  indTriangleVector[10] = {1, 5, 4};
  indTriangleVector[11] = {4, 0, 1};

  return indTriangleVector;
}

void
SoftRenderer::drawLine(TextureBuffer* screenBuffer, Vec2f p1, Vec2f p2, Vec3f color) const
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
      screenBuffer->setPixel(realX, y, color);

      while(tempDelta >= 1.0f)
      {
	y += sign(deltaVector.y);
	tempDelta -= 1.0f;

	if(deltaVector.y < 0 && y >= p2.y ||
	   deltaVector.y > 0 && y <= p2.y)
	{
	  screenBuffer->setPixel(realX, y, color);
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
      screenBuffer->setPixel(realX, y, color);

      while(tempDelta >= 1.0f)
      {
	y += sign(deltaVector.y);
	tempDelta -= 1.0f;

	if(deltaVector.y < 0 && y >= p2.y ||
	   deltaVector.y > 0 && y <= p2.y)
	{
	  screenBuffer->setPixel(realX, y, color);
	}

      }
    }
  }
}

void
SoftRenderer::drawSquare(TextureBuffer* screenBuffer, Vec2f pos, float sideLength, Vec3f color) const
{
  Vec2f topLeft = pos + Vec2f(-sideLength / 2.0f, -sideLength / 2.0f);
  Vec2f topRight = pos + Vec2f(sideLength / 2.0f, -sideLength / 2.0f);

  Vec2f bottomLeft = pos + Vec2f(-sideLength / 2.0f, sideLength / 2.0f);
  Vec2f bottomRight = pos + Vec2f(sideLength / 2.0f, sideLength / 2.0f);

  drawLine(screenBuffer, topLeft, topRight, color);
  drawLine(screenBuffer, bottomLeft, bottomRight, color);

  drawLine(screenBuffer, topLeft, bottomLeft, color);
  drawLine(screenBuffer, topRight, bottomRight, color);
}

void
SoftRenderer::drawCubeInPerspective(TextureBuffer* screenBuffer, const Cube& cube, real32 rotAngleX, real32 rotAngleY)
{
  Vertices vertices = cube.getVertices(rotAngleX, rotAngleY);
  TriangleIndices triangleIndices = Cube::getTriangleIndexes();

  drawTriangles3D(screenBuffer, vertices, triangleIndices);
}

void
SoftRenderer::drawTriangles3D(TextureBuffer* screenBuffer, const Vertices& vertices,
			      const TriangleIndices& triangleIndices, bool outline) const
{
  Vertices castedVertices = vertices;
  camera->castVertices(castedVertices);
  castedVertices = castVertices(castedVertices);

  TriangleVector triangleVector = MeshHelper::getTrianglesFromIndices(triangleIndices, castedVertices);

  TriangleVector trianglesToProcess;
  uint32 triangleCount = triangleVector.size();

  for(int i = 0; i < triangleCount; i++)
  {
    Vec3f triangleNormal = MeshHelper::getCrossProduct(castedVertices, triangleIndices[i]);
    Vec3f lookVector = Vec3f(0, 0, 1.0f);

    // Minus Because have to reverse lookVector direction.
    real32 dotProduct = -Vec3f::dotProduct(lookVector, triangleNormal);
    if(dotProduct > 0)
      trianglesToProcess.push_back(triangleVector[i]);
  }

  Polygons polygons = MeshHelper::trianglesToPolys(trianglesToProcess);

  real32 clipDistance = 0.5f;
  Polygons polygonsToDraw = clip(polygons, clipDistance);

  bool colorToggle = false;
  for(auto it = polygonsToDraw.begin(); it != polygonsToDraw.end(); it++)
  {
    const Polygon3D& polygon3D = *it;
    Polygon2D polygon2D = polygon3D.toPolygon2D();
    Polygon2D screenSpacePolygon = polygon2D.toScreenSpace(screenBuffer->dimensions);
    // screenSpacePolygon.vertices[0].showData();

    drawPolygon(screenBuffer, screenSpacePolygon, Vec3f(128.0f, colorToggle ? 128.0f : 0, 0), outline);
    // colorToggle = !colorToggle;
  }

}

void
SoftRenderer::drawMappedTriangles3D(TextureBuffer* screenBuffer, const MappedVertices& _mappedVertices,
				    const TriangleIndices& triangleIndices, const TextureBuffer* srcTexture)
{
  MappedVertices mappedVertices = _mappedVertices;
  camera->castVertices(mappedVertices);

  MappedTriangles triangles = MeshHelper::getTrianglesFromIndices(triangleIndices, mappedVertices);

  real32 dfc = camera->getDfc();
  uint32 triangleCount = triangles.size();
  MappedPolygons polygonsToDraw;

  for(int i = 0; i < triangleCount; i++)
  {
    // Checking if the plane is facing the camera
    Vec3f triangleNormal = MeshHelper::getCrossProductCasted(triangles[i], dfc);
    Vec3f lookVector = Vec3f(0, 0, 1.0f);

    real32 dotProduct = -Vec3f::dotProduct(lookVector, triangleNormal);

    // if so process the triangle further
    if(dotProduct > 0)
    {
      MappedPolygon polygon = triangles[i].toPolygon();
      real32 clipDistance = 0.5f;
      polygon = polygon.clip(clipDistance, dfc);

      // Perspective Cast Here !
      castPolygon(polygon);
      polygonsToDraw.push_back(polygon);
    }
  }

  for(auto it = polygonsToDraw.begin(); it != polygonsToDraw.end(); it++)
  {
    const MappedPolygon& mappedPolygon = *it;
    MappedPolygon screenSpacePolygon = mappedPolygon.toScreenSpace(screenBuffer->dimensions);
    drawPolygonMapped(screenBuffer, screenSpacePolygon, srcTexture, true);
  }
}

void
SoftRenderer::drawTriangle(TextureBuffer* screenBuffer, const Triangle& triangle, Vec3f color) const
{
  Polygon2D polygon;
  polygon.vertices.resize(3);
  polygon.vertices[0] = triangle.vertices[0].toVec2();
  polygon.vertices[1] = triangle.vertices[1].toVec2();
  polygon.vertices[2] = triangle.vertices[2].toVec2();

  drawPolygon(screenBuffer, polygon, color);
}

void
SoftRenderer::drawPolygon(TextureBuffer* screenBuffer, Polygon2D& polygon, Vec3f color, bool outline) const
{
  std::vector<ScanLine> scanLines = getScanLines(polygon);
  for(auto it = scanLines.begin(); it != scanLines.end(); it++)
  {
    ScanLine& scanLine =  *it;
    // drawLine(texture, Vec2f(scanLine.startX, scanLine.y), Vec2f(scanLine.endX, scanLine.y), color);
    for(uint32 x = scanLine.startX; x <= scanLine.endX; x++)
    {
      screenBuffer->setPixel(x, scanLine.y, color);
    }
  }

  if(outline)
  {
    int numbOfVertices = polygon.vertices.size();
    for(int i = 0; i != numbOfVertices; i++)
    {
      drawLine(screenBuffer, polygon.vertices[i], polygon.vertices[(i+1)%numbOfVertices], Vec3f());
    }
  }

}

void
SoftRenderer::drawPolygonMapped(TextureBuffer* screenBuffer, MappedPolygon& polygon, const TextureBuffer* srcTexture,
				bool outline)
{
  Vec3f color(128.0f, 0, 0);
  Vec3f castedDirectionalLight = camera->castDirectionalLight(directionalLight);

  real32 dfc = camera->getDfc();
  const Vec2i& screenResolution = screenBuffer->dimensions;
  real32 halfWidth = screenResolution.x / 2.0f;
  real32 halfHeight = screenResolution.y / 2.0f;

  MScanLineVector scanLines = getScanLinesMapped(polygon, screenBuffer->dimensions);
  for(auto it = scanLines.begin(); it != scanLines.end(); it++)
  {
    MScanLine& scanLine =  *it;

    MappedVertex v1Min = polygon.vertices[scanLine.minVertexIndex];
    MappedVertex v2Min = polygon.vertices[(scanLine.minVertexIndex + 1) % polygon.vertices.size()];

    Vec3f minDelta = v2Min.position - v1Min.position;
    real32 minT = ((real32)scanLine.y - v1Min.position.y) / minDelta.y;

    VertexCasted uvCastedLeft = castPerspective(v1Min, v2Min, minT);

    MappedVertex v1Max = polygon.vertices[scanLine.maxVertexIndex];
    MappedVertex v2Max = polygon.vertices[(scanLine.maxVertexIndex + 1) % polygon.vertices.size()];

    Vec3f maxDelta = v2Max.position - v1Max.position;
    real32 maxT = ((real32)scanLine.y - v1Max.position.y) / maxDelta.y;

    VertexCasted uvCastedRight = castPerspective(v1Max, v2Max, maxT);

    // Start for Calculating uv perPixel
    // Calculating divided by Z
    Vec2f SUVleft = uvCastedLeft.uv / uvCastedLeft.z;
    Vec2f SUVright = uvCastedRight.uv / uvCastedRight.z;

    Vec3f SNleft = uvCastedLeft.normal / uvCastedLeft.z;
    Vec3f SNright = uvCastedRight.normal / uvCastedRight.z;

    real32 currentSZ = 1.0f / uvCastedLeft.z;
    Vec2f currentSUV = SUVleft;
    Vec3f currentSN = SNleft;

    uint32 scanLineLength = scanLine.endX - scanLine.startX;

    real32 zDeltaPerPixel = ((1.0f / uvCastedRight.z) - currentSZ) / scanLineLength;
    Vec2f uvDeltaPerPixel = (SUVright - SUVleft) / scanLineLength;
    Vec3f nDeltaPerPixel = (SNright - SNleft) / scanLineLength;

    for(int x = scanLine.startX; x <= scanLine.endX; x++)
    {

      real32 currentZ = 1.0f / currentSZ;

      if(currentZ < zBuffer[scanLine.y][x])
      {
	zBuffer[scanLine.y][x] = currentZ;

	Vec2f resultUV = currentSUV / currentSZ;
	Vec3f resultN = currentSN / currentSZ;
	Vec3f textureColor = srcTexture->getPixelUV(resultUV);

	// Getting Original Pixel Position in 3D
	Vec3f pixelPosition;

	pixelPosition.x = x;
	pixelPosition.y = scanLine.y;

	// First Convert From ScreenSpace to objectSpace

	pixelPosition.x = (pixelPosition.x - halfWidth) / halfWidth;
	pixelPosition.y = (pixelPosition.y - halfHeight) / halfHeight;

	pixelPosition.x = (currentZ * pixelPosition.x) / dfc;
	pixelPosition.y = (currentZ * pixelPosition.y) / dfc;
	pixelPosition.z = currentZ;

	Vec3f deltaPosition = pixelPosition;
	real32 distanceFromPixel = deltaPosition.getLength();

	// ambientLight =  1.0f - ((distanceFromPixel - dfc) / 2.0f);

	// real32 tempT = (x - scanLine.startX) / scanLineLength;
	// VertexCasted pixelValues = castPerspective(uvCastedLeft, uvCastedRight, tempT);

	Vec3f pixelNormal = resultN;
	Vec3f lookVectorR(0, 0, -1);

	real32 lightValue = std::max(Vec3f::dotProduct(pixelNormal, castedDirectionalLight * -1.0f), 0.0f);
	lightValue += ambientLight;
	lightValue = std::max(std::min(lightValue, 1.0f), 0.0f);

	textureColor *= lightValue;
	screenBuffer->setPixel(x, scanLine.y, textureColor);
      }

      currentSZ += zDeltaPerPixel;
      currentSUV += uvDeltaPerPixel;
      currentSN += nDeltaPerPixel;
    }
  }

  Polygon2D _polygon = polygon.toPolygon2D();

  if(outline)
  {
    int numbOfVertices = _polygon.vertices.size();
    for(int i = 0; i != numbOfVertices; i++)
    {
      drawLine(screenBuffer, _polygon.vertices[i], _polygon.vertices[(i+1)%numbOfVertices], Vec3f());
    }
  }
}

void
SoftRenderer::setZBufferSize(const Vec2i& zBufferSize)
{

  zBuffer.resize(zBufferSize.y);
  for(auto it = zBuffer.begin(); it != zBuffer.end(); it++)
  {
    std::vector<real32>& pixelRow = *it;
    pixelRow.resize(zBufferSize.x);

    std::fill(pixelRow.begin(), pixelRow.end(), FLT_MAX);
  }
}

void
SoftRenderer::clearZBuffer()
{
  for(auto it = zBuffer.begin(); it != zBuffer.end(); it++)
  {
    std::vector<real32>& pixelRow = *it;
    std::fill(pixelRow.begin(), pixelRow.end(), FLT_MAX);
  }
}

ScanLineVector
SoftRenderer::getScanLines(const Polygon2D& polygon) const
{
  ScanLineVector result;
  const int numbOfVertices = polygon.vertices.size();

  const Vertices2D& vertices = polygon.vertices;
  Range2d range2d = MathHelper::getRange2d(vertices);

  for(int y = range2d.y.min; y <= range2d.y.max; y++)
  {
    std::vector<real32> xValues;
    xValues.resize(numbOfVertices);

    // Getting possible Values in correct ranges
    for(int i = 0; i < numbOfVertices; i++)
    {
      uint32 nextVertex = (i+1)%numbOfVertices;
      Vec2f ls = vertices[i] - vertices[nextVertex];

      real32 xValue = ls.getXFor(y, vertices[i]);

      // Checking the range
      // if a > 0 and checked Y Value is greater than position
      if((ls.y > 0 && (y > vertices[i].y || y < vertices[nextVertex].y)) ||
	 (ls.y < 0 && (y < vertices[i].y || y > vertices[nextVertex].y)))
	xValue = range2d.x.max + 1;

      xValues[i] = xValue;
    }

    uint32 minScanX = -1;
    uint32 maxScanX = -1;

    // Getting min and max value for given y value
    for(int i = 0; i < numbOfVertices; i++)
    {
      if(xValues[i] >= 0 && xValues[i] >= range2d.x.min && xValues[i] <= range2d.x.max )
      {
	uint32 floorValue = ceil(xValues[i]);
	uint32 ceilValue = ceil(xValues[i]);

	// First iteration
	if(minScanX == -1 && maxScanX == -1)
	{
	  minScanX = ceilValue;
	  maxScanX = floorValue;
	}
	else
	{
	  minScanX = std::min(minScanX, ceilValue);
	  maxScanX = std::max(maxScanX, floorValue);
	}
      }
    }

    if(minScanX != -1 && maxScanX != -1)
    {
      ScanLine scanLine;
      scanLine.y = y;

      scanLine.startX = minScanX;
      scanLine.endX = maxScanX;

      result.push_back(scanLine);
    }
  }

  return result;
}

MScanLineVector
SoftRenderer::getScanLinesMapped(const MappedPolygon& polygon, const Vec2i& screenResolution) const
{
  MScanLineVector result;
  const int numbOfVertices = polygon.vertices.size();

  const MappedVertices& vertices = polygon.vertices;
  Range2d range2d = MathHelper::getRange2d(vertices);

  range2d.y.min = std::max(range2d.y.min, 0.0f);
  range2d.y.max = std::min(range2d.y.max, (real32)(screenResolution.y - 1));

  struct XAssignedVertex{
    real32 value;
  };

  for(int y = range2d.y.min; y <= range2d.y.max; y++)
  {
    MScanLine scanLine;
    scanLine.y = y;

    std::vector<XAssignedVertex> xValues;
    xValues.resize(numbOfVertices);

    for(int i = 0; i < numbOfVertices; i++)
    {
      Vec2f ls = (vertices[i].position - vertices[(i + 1)%numbOfVertices].position).toVec2();
      real32 xValue = ls.getXFor(y, vertices[i].position.toVec2());

      if((ls.y > 0 && (y > vertices[i].position.y || y < vertices[(i+1)%numbOfVertices].position.y)) ||
	 (ls.y < 0 && (y < vertices[i].position.y || y > vertices[(i+1)%numbOfVertices].position.y)))
	xValue = range2d.x.max + 1;

      xValues[i].value = xValue;
    }

    int32 minScanX = -1;
    int32 maxScanX = -1;

    uint32 minVertexIndex;
    uint32 maxVertexIndex;

    for(int i = 0; i < numbOfVertices; i++)
    {
      if(xValues[i].value >= range2d.x.min && xValues[i].value <= range2d.x.max)
      {
	real32 ceilValue = ceil(xValues[i].value);
	real32 floorValue = floor(xValues[i].value);

	if(minScanX == -1 && maxScanX == -1)
	{
	  minScanX = ceilValue;
	  maxScanX = floorValue;

	  minVertexIndex = i;
	  maxVertexIndex = i;
	}
	else
	{
	  // MinScanX
	  if(ceilValue < minScanX)
	  {
	    minScanX = ceilValue;
	    minVertexIndex = i;
	  }

	  // MaxScanX
	  if(floorValue > maxScanX)
	  {
	    maxScanX = floorValue;
	    maxVertexIndex = i;
	  }
	}
      }
    }

    if(minScanX != -1 && maxScanX != -1)
    {
      scanLine.startX = std::max(minScanX, 0);
      scanLine.endX = std::min(maxScanX, screenResolution.x - 1);

      scanLine.minVertexIndex = minVertexIndex;
      scanLine.maxVertexIndex = maxVertexIndex;

      result.push_back(scanLine);
    }
  }

  return result;
}

Vertices
SoftRenderer::castVertices(const Vertices& vertices) const
{
  Vertices castedVertices;

  uint32 vertexCount = vertices.size();
  castedVertices.resize(vertexCount);

  // 90 Degrees
  real32 dfc = camera->getDfc();

  // Transforming Coordinates
  for(int i = 0; i < vertexCount; i++)
  {
    uint32 pixelIndex = i;

    const Vec3f& src = vertices[i];
    Vec2f dst;

    // Perspective calculation
    dst.x = (src.x / src.z) * dfc;
    dst.y = (src.y / src.z) * dfc;

    castedVertices[i].x = dst.x;
    castedVertices[i].y = dst.y;
    castedVertices[i].z = src.z;
  }

  return castedVertices;
}

void
SoftRenderer::castMappedVertices(MappedVertices& vertices) const
{
  uint32 vertexCount = vertices.size();

  real32 dfc = camera->getDfc();

  // Transforming Coordinates
  for(int i = 0; i < vertexCount; i++)
  {
    Vec3f& position = vertices[i].position;

    // Perspective calculation
    position.x = (position.x / position.z) * dfc;
    position.y = (position.y / position.z) * dfc;
  }

}
void
SoftRenderer::castPolygon(MappedPolygon& polygon) const
{
  uint32 vertexCount = polygon.vertices.size();
  real32 dfc = camera->getDfc();

  // Transforming Coordinates
  for(int i = 0; i < vertexCount; i++)
  {
    Vec3f& position = polygon.vertices[i].position;
    position = MeshHelper::castVertex(position, dfc);
  }
}

VertexCasted
SoftRenderer::castPerspective(const MappedVertex& v1, const MappedVertex& v2, real32 t) const
{
  VertexCasted result;

  // Divided By Z Values
  Vec2f SUVv1 = v1.uv / v1.position.z;
  Vec2f SUVv2 = v2.uv / v2.position.z;

  Vec3f SNv1 = v1.normal / v1.position.z;
  Vec3f SNv2 = v2.normal / v2.position.z;

  // Deltas
  Vec2f SUVDelta = SUVv2 - SUVv1;
  Vec3f SNDelta = SNv2 - SNv1;

  // Z Calculations
  real32 SZv1 = 1.0f / v1.position.z;
  real32 SZv2 = 1.0f / v2.position.z;
  real32 SZDelta = SZv2 - SZv1;

  // Result
  Vec2f SUVResult = SUVv1 + (SUVDelta  * t);
  Vec3f SNResult = SNv1 + (SNDelta  * t);
  real32 ZResult = SZv1 + (SZDelta  * t);

  real32 resultZ = 1.0f / ZResult;

  Vec2f resultUV = SUVResult * resultZ;
  Vec3f resultN = SNResult * resultZ;

  result.uv = resultUV;
  result.normal = resultN;
  result.z = resultZ;

  return result;
}

Polygons
SoftRenderer::clip(const Polygons& polygons, real32 nearZ) const
{
  Polygons result;
  for(auto it = polygons.begin(); it != polygons.end(); it++)
  {
    const Polygon3D& src = *it;
    Polygon3D dst = src.clip(nearZ);
    if(dst.vertices.size() >= 3)
    {
      result.push_back(dst);
    }
  }
  return result;
}

MappedPolygons
SoftRenderer::clip(const MappedPolygons& polygons, real32 nearZ) const
{
  MappedPolygons result;
  for(auto it = polygons.begin(); it != polygons.end(); it++)
  {
    const MappedPolygon& src = *it;
    // MappedPolygon dst = src;
    MappedPolygon dst = src.clip(nearZ, camera->getDfc());

    if(dst.vertices.size() >= 3)
    {
      result.push_back(dst);
    }
  }
  return result;
}
