#include "SoftwareRenderer.h"

#include <algorithm>
#include <list>
#include <assert.h>

#define sign(a) (a > 0 ? 1 : -1)

Polygon2D
Polygon3D::toPolygon2D() const
{
  Polygon2D result;
  uint32 verticyCount = vertices.size();
  result.vertices.resize(vertices.size());
  
  for(int i = 0; i < verticyCount; i++)
  {
    result.vertices[i] = vertices[i].toVec2();
  }
  
  return result;
}

Polygon3D
Polygon3D::clip(real32 nearZ) const
{
  Polygon3D result;
  uint32 verticyCount = vertices.size();
  for(auto i = 0; i < verticyCount; i++)
  {
    const Vec3f& v1 = vertices[i];
    const Vec3f& v2 = vertices[(i + 1)%verticyCount];

    
    // Ignore if both
    if(v1.z >= nearZ && v2.z >= nearZ)
    {
      result.vertices.push_back(v1);
    }
    else if(v1.z >= nearZ && v2.z < nearZ)
    {
      result.vertices.push_back(v1);
      Vec3f delta = v1 - v2;
      real32 t = (v1.z - nearZ) / delta.z;
      
      // std::cout << "-----------------" << std::endl;
      // std::cout << "v1.z: " << v1.z << std::endl;
      // std::cout << "v2.z: " << v2.z << std::endl;
      // std::cout << "dz: " << delta.z << std::endl;
      // std::cout << "t: " << t << std::endl;
      
      Vec3f addedVector = v1 - (delta * t);
      result.vertices.push_back(addedVector);
    }
    // v1.z < nearZ && v2 >= nearZ
    else if(v1.z < nearZ && v2.z >= nearZ)
    {
      Vec3f delta = v2 - v1;
      real32 t = (v2.z - nearZ) / delta.z;
      
      Vec3f addedVector = v2 - (delta * t);
      result.vertices.push_back(addedVector);
      
      result.vertices.push_back(v2);
    }
  }
  return result;
}

Range2d
MathHelper::getRange2d(const VerticesVector2D& vertices)
{
  Range2d result = {};
  if(vertices.size() == 0) return result;
  result.x.max = vertices[0].x;
  result.x.min = vertices[0].x;

  result.y.max = vertices[0].y;
  result.y.min = vertices[0].y;
  
  for(auto it = vertices.begin()+1; it != vertices.end(); it++)
  {
    const Vec2f& currentVector = *it;
    result.x.max = std::max(currentVector.x, result.x.max);
    result.x.min = std::min(currentVector.x, result.x.min);
    
    result.y.max = std::max(currentVector.y, result.y.max);
    result.y.min = std::min(currentVector.y, result.y.min);
  }
  
  return result;
}

void
TextureHelper::blitTexture(TextureBuffer* dst, TextureBuffer* src, const Vec2i& position)
{
  Vec2i srcDimensions = src->dimensions;
  for(int y = 0; y < srcDimensions.y; y++)
  {
    for(int x = 0; x < srcDimensions.x; x++)
    {
      Vec3f srcColor = src->getPixel(x, y);
      dst->setPixel(x + position.x, y + position.y, srcColor);
      // srcColor.showData();
    }
  }
}

Vec3f
MeshHelper::getCrossProduct(const IndexedTriangle& indexedTriangle, const VerticesVector3D& vertices)
{
  Vec3f v1 = vertices[indexedTriangle.t1] - vertices[indexedTriangle.t0];
  Vec3f v2 = vertices[indexedTriangle.t2] - vertices[indexedTriangle.t1];

  Vec3f result = Vec3f::cross(v1, v2);
  return result;
}

TriangleVector
MeshHelper::getTrianglesFromIndices(const IndTriangleVector& triangleIndexes, const VerticesVector3D& vertices)
{
  TriangleVector triangles;
  const uint32 triangleCount = triangleIndexes.size();
  triangles.resize(triangleCount);
  
  for(int i = 0; i < triangleCount; i++)
  {
    const IndexedTriangle& indexedTriangle = triangleIndexes[i];
    
    triangles[i].vertices[0] = vertices[indexedTriangle.t0];
    triangles[i].vertices[1] = vertices[indexedTriangle.t1];
    triangles[i].vertices[2] = vertices[indexedTriangle.t2];
  }
  
  return triangles;
}
PolygonVector3D
MeshHelper::trianglesToPolys(const TriangleVector& triangles)
{
  PolygonVector3D result;
  for(auto it = triangles.begin(); it != triangles.end(); it++)
  {
    const Triangle& triangle = *it;
    Polygon3D poly;
    poly.vertices.push_back(triangle.vertices[0]);
    poly.vertices.push_back(triangle.vertices[1]);
    poly.vertices.push_back(triangle.vertices[2]);
    result.push_back(poly);
  }
  return result;
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

VerticesVector3D
Cube::getVertices(real32 rotAngleX, real32 rotAngleY) const
{
  
  VerticesVector3D vertices;
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

IndTriangleVector
Cube::getTriangleIndexes()
{
  IndTriangleVector indTriangleVector;
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
SoftwareRenderer::drawLine(TextureBuffer* texture, Vec2f p1, Vec2f p2, Vec3f color) const 
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

void
SoftwareRenderer::drawSquare(TextureBuffer* texture, Vec2f pos, float sideLength, Vec3f color) const
{
  Vec2f topLeft = pos + Vec2f(-sideLength / 2.0f, -sideLength / 2.0f);
  Vec2f topRight = pos + Vec2f(sideLength / 2.0f, -sideLength / 2.0f);
  
  Vec2f bottomLeft = pos + Vec2f(-sideLength / 2.0f, sideLength / 2.0f);
  Vec2f bottomRight = pos + Vec2f(sideLength / 2.0f, sideLength / 2.0f);
  
  drawLine(texture, topLeft, topRight, color);
  drawLine(texture, bottomLeft, bottomRight, color);
  
  drawLine(texture, topLeft, bottomLeft, color);
  drawLine(texture, topRight, bottomRight, color);
}

void
SoftwareRenderer::drawCubeInPerspective(TextureBuffer* texture, const Cube& cube, real32 rotAngleX, real32 rotAngleY)
{
  VerticesVector3D vertices = cube.getVertices(rotAngleX, rotAngleY);
  IndTriangleVector triangleIndices = Cube::getTriangleIndexes();
  drawTriangles3D(texture, vertices, triangleIndices);
}

void
SoftwareRenderer::drawTriangles3D(TextureBuffer* texture, const VerticesVector3D& vertices,
				  const IndTriangleVector& triangleIndices) const 
{
  VerticesVector3D castedVertices = castVertices(vertices, texture->dimensions);
  TriangleVector triangleVector = MeshHelper::getTrianglesFromIndices(triangleIndices, castedVertices);
  
  TriangleVector trianglesToProcess;
  uint32 triangleCount = triangleVector.size();
  
  for(int i = 0; i < triangleCount; i++)
  {
    Vec3f triangleNormal = MeshHelper::getCrossProduct(triangleIndices[i], castedVertices);
    Vec3f lookVector = Vec3f(0, 0, 1.0f);
    
    real32 dotProduct = Vec3f::dotProduct(lookVector, triangleNormal);
    if(dotProduct > 0) trianglesToProcess.push_back(triangleVector[i]);
  }
  
  PolygonVector3D polygons = MeshHelper::trianglesToPolys(trianglesToProcess);
  PolygonVector3D polygonsToDraw = clip(polygons, 0.5f);
  
  bool colorToggle = false;
  for(auto it = polygonsToDraw.begin(); it != polygonsToDraw.end(); it++)
  {
    const Polygon3D& polygon3D = *it;
    Polygon2D polygon2D = polygon3D.toPolygon2D();
    
    drawPolygon(texture, polygon2D, Vec3f(128.0f, colorToggle ? 128.0f : 0, 0));
    colorToggle = !colorToggle;
    // return ;
  }
  
}

void
SoftwareRenderer::drawTriangle(TextureBuffer* texture, const Triangle& triangle, Vec3f color) const
{
  Polygon2D polygon;
  polygon.vertices.resize(3);
  polygon.vertices[0] = triangle.vertices[0].toVec2();
  polygon.vertices[1] = triangle.vertices[1].toVec2();
  polygon.vertices[2] = triangle.vertices[2].toVec2();
  
  drawPolygon(texture, polygon, color);
}

void
SoftwareRenderer::drawPolygon(TextureBuffer* texture, Polygon2D& polygon, Vec3f color, bool outline) const 
{
  std::vector<ScanLine> scanLines = getScanLines(polygon);
  for(auto it = scanLines.begin(); it != scanLines.end(); it++)
  {
    ScanLine& scanLine =  *it;
    drawLine(texture, Vec2f(scanLine.startX, scanLine.y), Vec2f(scanLine.endX, scanLine.y), color);
  }

  if(outline)
  {
    int numbOfVertices = polygon.vertices.size();
    for(int i = 0; i != numbOfVertices; i++)
    {
      drawLine(texture, polygon.vertices[i], polygon.vertices[(i+1)%numbOfVertices], Vec3f());
    }
  }
  
}

ScanLineVector
SoftwareRenderer::getScanLines(const Polygon2D& polygon) const
{
  ScanLineVector result;
  const int numbOfVertices = polygon.vertices.size();
  
  const VerticesVector2D& vertices = polygon.vertices;
  Range2d range2d = MathHelper::getRange2d(vertices);
  
  std::vector<Vec2f> ls;
  ls.resize(numbOfVertices);
  
  for(int i = 0; i < numbOfVertices; i++)
  {
    ls[i] = vertices[i] - vertices[(i + 1)%numbOfVertices];
  }
  
  for(int y = range2d.y.min; y <= range2d.y.max; y++)
  {
    ScanLine scanLine;
    scanLine.y = y;

    std::vector<real32> xValues;
    xValues.resize(numbOfVertices);

    for(int i = 0; i < numbOfVertices; i++)
    {
      real32 xValue = ls[i].getXFor(y, vertices[i]);
      
      if((ls[i].y > 0 && (y > vertices[i].y || y < vertices[(i+1)%numbOfVertices].y)) ||
      	 (ls[i].y < 0 && (y < vertices[i].y || y > vertices[(i+1)%numbOfVertices].y)))
	xValue = range2d.x.max + 1;
      
      xValues[i] = xValue;
    }
    
    uint32 minScanX = -1;
    uint32 maxScanX = -1;
    
    for(int i = 0; i < numbOfVertices; i++)
    {
      if(xValues[i] >= range2d.x.min && xValues[i] <= range2d.x.max)
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
      }
      
    }
    
    scanLine.startX = minScanX;
    scanLine.endX = maxScanX;
    
    result.push_back(scanLine);    
  }
  
  return result;
}

VerticesVector3D
SoftwareRenderer::castVertices(const VerticesVector3D& vertices, const Vec2i& screenDimensions) const
{
  VerticesVector3D castedVertices;
  castedVertices.resize(vertices.size());
  
  // 90 Degrees
  float dfc = 1.0f / tan(fov / 2.0f);
  real32 aspectRatio = (real32)screenDimensions.x / screenDimensions.y;
  
  // Transforming Coordinates
  for(int i = 0; i != 8; i++)
  {
    uint32 pixelIndex = i;
    
    const Vec3f& src = vertices[i];
    Vec2f dst;
    
    // Perspective calculation
    dst.x = (src.x / src.z) * dfc;
    dst.y = (src.y / src.z) * dfc;
    
    // Transforming Into ScreenSpace
    dst.x = (dst.x * screenDimensions.x * 0.5) + screenDimensions.x * 0.5;
    dst.y = ((-dst.y * aspectRatio) * screenDimensions.y * 0.5) + screenDimensions.y * 0.5;

    castedVertices[i].x = dst.x;
    castedVertices[i].y = dst.y;
    castedVertices[i].z = src.z;
    
    // texture->setPixel(dst.x, dst.y, Vec3f(255.0f, 255.0f, 255.0f));
  }
  
  return castedVertices;
}

PolygonVector3D
SoftwareRenderer::clip(const PolygonVector3D& polygons, real32 nearZ) const
{
  PolygonVector3D result;
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
