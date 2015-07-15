#include "SoftwareRenderer.h"

#include <algorithm>
#include <list>
#include <assert.h>

#define sign(a) (a > 0 ? 1 : -1)

Polygon2D
Polygon2D::toScreenSpace(const Vec2i& screenDimensions) const 
{
  real32 aspectRatio = (real32)screenDimensions.x / screenDimensions.y;

  uint32 vertexCount = vertices.size();
  Polygon2D result;
  result.vertices.resize(vertexCount);
  for(uint32 i = 0; i != vertexCount; i++)
  {
    const Vec2f& src = vertices[i];
    Vec2f& dst = result.vertices[i];
    
    // // Transforming Into ScreenSpace
    dst.x = (src.x * screenDimensions.x * 0.5) + screenDimensions.x * 0.5;
    dst.y = ((-src.y * aspectRatio) * screenDimensions.y * 0.5) + screenDimensions.y * 0.5;
  }
  return result;
}

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

MappedPolygon
MappedPolygon::clip(real32 nearZ) const
{
  MappedPolygon result;
  uint32 verticyCount = vertices.size();
  for(auto i = 0; i < verticyCount; i++)
  {
    uint32 nextVertexIndex = (i + 1)%verticyCount;
    
    const Vec3f& v1 = vertices[i].position;
    const Vec3f& v2 = vertices[nextVertexIndex].position;
    
    // Ignore if both
    if(v1.z >= nearZ && v2.z >= nearZ)
    {
      result.vertices.push_back(vertices[i]);
    }
    else if(v1.z >= nearZ && v2.z < nearZ)
    {
      result.vertices.push_back(vertices[i]);
      
      Vec3f delta = v1 - v2;
      real32 t = (v1.z - nearZ) / delta.z;
      
      Vec3f addedVector = v1 - (delta * t);
      Vec2f textureCoordsDelta = vertices[i].uv - vertices[nextVertexIndex].uv;
      
      //TODO(jakub): Interpolate using perspective
      Vec2f textureCoords = vertices[i].uv - (textureCoordsDelta * t);
      
      MappedVertex addedVertex;
      addedVertex.uv = textureCoords;
      
      // Important ------------------ 
      // addedVertex.uv = vertices[i].uv;
      addedVertex.position = addedVector;
      
      result.vertices.push_back(addedVertex);
    }
    else if(v1.z < nearZ && v2.z >= nearZ)
    {
      Vec3f delta = v2 - v1;
      real32 t = (v2.z - nearZ) / delta.z;
      
      Vec3f addedVector = v2 - (delta * t);
      
      
      Vec2f textureCoordsDelta = vertices[nextVertexIndex].uv - vertices[i].uv;
      
      //TODO(jakub): Interpolate using perspective
      Vec2f textureCoords = vertices[nextVertexIndex].uv - (textureCoordsDelta * t);
      
      MappedVertex addedVertex;
      addedVertex.uv = textureCoords;
      
      // Important ------------------ 
      // addedVertex.uv = vertices[nextVertexIndex].uv;
      addedVertex.position = addedVector;
      
      result.vertices.push_back(addedVertex);
      result.vertices.push_back(vertices[nextVertexIndex]);
    }
  }
  return result;
}

Polygon2D
MappedPolygon::toPolygon2D() const
{
  Polygon2D result;
  uint32 verticyCount = vertices.size();
  result.vertices.resize(vertices.size());
  
  for(int i = 0; i < verticyCount; i++)
  {
    result.vertices[i] = vertices[i].position.toVec2();
  }
  
  return result;
}

MappedPolygon
MappedPolygon::toScreenSpace(const Vec2i& screenDimensions) const
{
  real32 aspectRatio = (real32)screenDimensions.x / screenDimensions.y;

  uint32 vertexCount = vertices.size();
  MappedPolygon result;
  result.vertices.resize(vertexCount);
  for(uint32 i = 0; i != vertexCount; i++)
  {
    const Vec3f& src = vertices[i].position;
    
    result.vertices[i] = vertices[i];
    Vec3f& dst = result.vertices[i].position;
    
    // // Transforming Into ScreenSpace
    dst.x = (src.x * screenDimensions.x * 0.5) + screenDimensions.x * 0.5;
    dst.y = ((-src.y * aspectRatio) * screenDimensions.y * 0.5) + screenDimensions.y * 0.5;
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

Range2d
MathHelper::getRange2d(const MappedVertices& vertices)
{
  Range2d result = {};
  if(vertices.size() == 0) return result;
  result.x.max = vertices[0].position.x;
  result.x.min = vertices[0].position.x;
  
  result.y.max = vertices[0].position.y;
  result.y.min = vertices[0].position.y;
  
  for(auto it = vertices.begin()+1; it != vertices.end(); it++)
  {
    const Vec3f& currentVector = it->position;
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
    }
  }
}

Vec3f
MeshHelper::getCrossProduct(const IndexedTriangle& indexedTriangle, const Vertices& vertices)
{
  Vec3f v1 = vertices[indexedTriangle.indexes[0]] - vertices[indexedTriangle.indexes[1]];
  Vec3f v2 = vertices[indexedTriangle.indexes[2]] - vertices[indexedTriangle.indexes[1]];

  Vec3f result = Vec3f::cross(v1, v2);
  return result;
}

Vec3f
MeshHelper::getCrossProduct(const MappedTriangle& triangle)
{
  Vec3f v1 = triangle.vertices[0].position - triangle.vertices[1].position;
  Vec3f v2 = triangle.vertices[2].position - triangle.vertices[1].position;
  
  Vec3f result = Vec3f::cross(v1, v2);
  return result;
}

TriangleVector
MeshHelper::getTrianglesFromIndices(const TriangleIndices& triangleIndexes, const Vertices& vertices)
{
  TriangleVector triangles;
  const uint32 triangleCount = triangleIndexes.size();
  triangles.resize(triangleCount);
  
  for(int i = 0; i < triangleCount; i++)
  {
    const IndexedTriangle& indexedTriangle = triangleIndexes[i];
#if 1
    triangles[i].vertices[0] = vertices[indexedTriangle.indexes[0]];
    triangles[i].vertices[1] = vertices[indexedTriangle.indexes[1]];
    triangles[i].vertices[2] = vertices[indexedTriangle.indexes[2]];
#else    
    Triangle triangle;
    triangle.vertices[0] = vertices[indexedTriangle.indexes[0]];
    triangle.vertices[1] = vertices[indexedTriangle.indexes[1]];
    triangle.vertices[2] = vertices[indexedTriangle.indexes[2]];
    triangles.push_back(triangle);
#endif
  }
  
  return triangles;
}

MappedTriangles
MeshHelper::getTrianglesFromIndices(const TriangleIndices& triangleIndexes, const MappedVertices& vertices)
{
  MappedTriangles triangles;
  const uint32 triangleCount = triangleIndexes.size();
  triangles.resize(triangleCount);
  
  for(int i = 0; i < triangleCount; i++)
  {
    const IndexedTriangle& indexedTriangle = triangleIndexes[i];
    
    triangles[i].vertices[0] = vertices[indexedTriangle.indexes[0]];
    triangles[i].vertices[1] = vertices[indexedTriangle.indexes[1]];
    triangles[i].vertices[2] = vertices[indexedTriangle.indexes[2]];
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

MappedPolygons
MeshHelper::trianglesToPolys(const MappedTriangles& triangles)
{
  MappedPolygons result;
  for(auto it = triangles.begin(); it != triangles.end(); it++)
  {
    const MappedTriangle& triangle = *it;
    MappedPolygon polygon;
    polygon.vertices.resize(3);

    for(int i = 0; i < 3; i++)
    {
      polygon.vertices[i] = triangle.vertices[i];
    }
    result.push_back(polygon);
  }
  return result;
}

void
MeshHelper::rotateVertices(Vertices& vertices, const Vec3f& angles)
{
  for(auto it = vertices.begin(); it != vertices.end(); it++)
  {
    Vec3f& vertex = *it;
    vertex.rotateAroundY(angles.y);
    vertex.rotateAroundX(angles.x);
    vertex.rotateAroundZ(angles.z);
  }
}

void
MeshHelper::translateVertices(Vertices& vertices, const Vec3f& translationVector)
{
  for(auto it = vertices.begin(); it != vertices.end(); it++)
  {
    Vec3f& vertex = *it;
    vertex += translationVector;
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
  Vertices vertices = cube.getVertices(rotAngleX, rotAngleY);
  TriangleIndices triangleIndices = Cube::getTriangleIndexes();
  
  drawTriangles3D(texture, vertices, triangleIndices);
}

void
SoftwareRenderer::drawTriangles3D(TextureBuffer* texture, const Vertices& vertices,
				  const TriangleIndices& triangleIndices, bool outline) const 
{
  Vertices castedVertices = castVertices(vertices);
  TriangleVector triangleVector = MeshHelper::getTrianglesFromIndices(triangleIndices, castedVertices);

  TriangleVector trianglesToProcess;
  uint32 triangleCount = triangleVector.size();
  
  for(int i = 0; i < triangleCount; i++)
  {
    Vec3f triangleNormal = MeshHelper::getCrossProduct(triangleIndices[i], castedVertices);
    Vec3f lookVector = Vec3f(0, 0, 1.0f);
    
    real32 dotProduct = Vec3f::dotProduct(lookVector, triangleNormal);
    if(dotProduct >= 0)
      trianglesToProcess.push_back(triangleVector[i]);
  }
  
  PolygonVector3D polygons = MeshHelper::trianglesToPolys(trianglesToProcess);
  
  real32 clipDistance = 0.5f;
  PolygonVector3D polygonsToDraw = clip(polygons, clipDistance);
  
  bool colorToggle = false;
  for(auto it = polygonsToDraw.begin(); it != polygonsToDraw.end(); it++)
  {
    const Polygon3D& polygon3D = *it;
    Polygon2D polygon2D = polygon3D.toPolygon2D();
    Polygon2D screenSpacePolygon = polygon2D.toScreenSpace(texture->dimensions);
    // screenSpacePolygon.vertices[0].showData();
    
    drawPolygon(texture, screenSpacePolygon, Vec3f(128.0f, colorToggle ? 128.0f : 0, 0), outline);
    colorToggle = !colorToggle;
  }
  
}

void
SoftwareRenderer::drawMappedTriangles3D(TextureBuffer* texture, const MappedVertices& _mappedVertices,
					const TriangleIndices& triangleIndices, const TextureBuffer* srcTexture) const
{
  MappedVertices mappedVertices = _mappedVertices;
  castMappedVertices(mappedVertices);
  
  MappedTriangles triangles = MeshHelper::getTrianglesFromIndices(triangleIndices, mappedVertices);
  MappedTriangles trianglesToProcess;
  
  uint32 triangleCount = triangles.size();
  
  for(int i = 0; i < triangleCount; i++)
  {
    // Checking if the plane is facing the camera
    Vec3f triangleNormal = MeshHelper::getCrossProduct(triangles[i]);
    Vec3f lookVector = Vec3f(0, 0, 1.0f);
    
    real32 dotProduct = Vec3f::dotProduct(lookVector, triangleNormal);
    
    // if so process the triangle further
    if(dotProduct >= 0)
      trianglesToProcess.push_back(triangles[i]);
  }
  
  MappedPolygons polygons = MeshHelper::trianglesToPolys(trianglesToProcess);
  real32 clipDistance = 0.5f;
  
  MappedPolygons polygonsToDraw = clip(polygons, clipDistance);
  for(auto it = polygonsToDraw.begin(); it != polygonsToDraw.end(); it++)
  {
    const MappedPolygon& mappedPolygon = *it;
    MappedPolygon screenSpacePolygon = mappedPolygon.toScreenSpace(texture->dimensions);
    drawPolygonMapped(texture, screenSpacePolygon, srcTexture, false);
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

void
SoftwareRenderer::drawPolygonMapped(TextureBuffer* texture, MappedPolygon& polygon, const TextureBuffer* srcTexture,
				    bool outline) const
{
  Vec3f color(128.0f, 0, 0);
  
  MScanLineVector scanLines = getScanLinesMapped(polygon);
  for(auto it = scanLines.begin(); it != scanLines.end(); it++)
  {
    MScanLine& scanLine =  *it;
    
    MappedVertex v1Min = polygon.vertices[scanLine.minVertexIndex];
    MappedVertex v2Min = polygon.vertices[(scanLine.minVertexIndex + 1) % polygon.vertices.size()];
    
    Vec3f minDelta = v2Min.position - v1Min.position;
    real32 minT = ((real32)scanLine.y - v1Min.position.y) / minDelta.y;
    
    UVCasted uvCastedLeft = castPerspective(v1Min, v2Min, minT);
    
    MappedVertex v1Max = polygon.vertices[scanLine.maxVertexIndex];
    MappedVertex v2Max = polygon.vertices[(scanLine.maxVertexIndex + 1) % polygon.vertices.size()];
    
    Vec3f maxDelta = v2Max.position - v1Max.position;
    real32 maxT = ((real32)scanLine.y - v1Max.position.y) / maxDelta.y;
    
    UVCasted uvCastedRight = castPerspective(v1Max, v2Max, maxT);
    
    for(int x = scanLine.startX; x < scanLine.endX; x++)
    {
      MappedVertex v1;
      v1.uv = uvCastedLeft.uv;
      v1.position = Vec3f(x, scanLine.y, uvCastedLeft.z);
      
      MappedVertex v2;
      v2.uv = uvCastedRight.uv;
      v2.position = Vec3f(x, scanLine.y, uvCastedRight.z);

      real32 t = (x-scanLine.startX)/(real32(scanLine.endX - scanLine.startX));
      UVCasted uvCasted = castPerspective(v1, v2, t);
      
      Vec3f startColor = srcTexture->getPixelUV(uvCasted.uv);
      texture->setPixel(x, scanLine.y, startColor);
    }
    
    // if(std::distance(scanLines.begin(), it) == 16)
    // {
    //   std::cout << "-----------------------\n";
    //   break;
    // }
  }
  
  Polygon2D _polygon = polygon.toPolygon2D();
  
  if(outline)
  {
    int numbOfVertices = _polygon.vertices.size();
    for(int i = 0; i != numbOfVertices; i++)
    {
      drawLine(texture, _polygon.vertices[i], _polygon.vertices[(i+1)%numbOfVertices], Vec3f());
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
	uint32 ceilValue = floor(xValues[i]);
	uint32 floorValue = floor(xValues[i]);
	
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
    
    scanLine.startX = minScanX;
    scanLine.endX = maxScanX;
    
    result.push_back(scanLine);    
  }
  
  return result;
}

MScanLineVector
SoftwareRenderer::getScanLinesMapped(const MappedPolygon& polygon) const
{
  MScanLineVector result;
  const int numbOfVertices = polygon.vertices.size();
  
  const MappedVertices& vertices = polygon.vertices;
  Range2d range2d = MathHelper::getRange2d(vertices);
  
  std::vector<Vec2f> ls;
  ls.resize(numbOfVertices);
  
  for(int i = 0; i < numbOfVertices; i++)
  {
    ls[i] = (vertices[i].position - vertices[(i + 1)%numbOfVertices].position).toVec2();
  }

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
      real32 xValue = ls[i].getXFor(y, vertices[i].position.toVec2());
      
      if((ls[i].y > 0 && (y > vertices[i].position.y || y < vertices[(i+1)%numbOfVertices].position.y)) ||
      	 (ls[i].y < 0 && (y < vertices[i].position.y || y > vertices[(i+1)%numbOfVertices].position.y)))
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
	// TODO(jakub): Check What the fuck is going on
	
	real32 ceilValue = floor(xValues[i].value);
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
	  
	  // minScanX = std::min(minScanX, (int32)ceilValue);
	  // maxScanX = std::max(maxScanX, (int32)floorValue);
	}
      }
    }
    
    if(minScanX != -1 && maxScanX != -1)
    {
      scanLine.startX = minScanX;
      scanLine.endX = maxScanX;
      
      scanLine.minVertexIndex = minVertexIndex;
      scanLine.maxVertexIndex = maxVertexIndex;
      
      result.push_back(scanLine);    
    }
  }
  
  return result;  
}

Vertices
SoftwareRenderer::castVertices(const Vertices& vertices) const
{
  Vertices castedVertices;
  
  uint32 vertexCount = vertices.size();
  castedVertices.resize(vertexCount);
  
  // 90 Degrees
  float dfc = 1.0f / tan(fov / 2.0f);
  
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
SoftwareRenderer::castMappedVertices(MappedVertices& vertices) const
{
  uint32 vertexCount = vertices.size();
  
  // 90 Degrees
  float dfc = 1.0f / tan(fov / 2.0f);
  
  // Transforming Coordinates
  for(int i = 0; i < vertexCount; i++)
  {
    Vec3f& position = vertices[i].position;
    
    // Perspective calculation
    
    // position /= position.z;
    // position *= dfc;
    
    position.x = (position.x / position.z) * dfc;
    position.y = (position.y / position.z) * dfc;
  }
  
}

UVCasted
SoftwareRenderer::castPerspective(const MappedVertex& v1, const MappedVertex& v2, real32 t) const
{
  UVCasted result;
  
  Vec2f SUVv1 = v1.uv / v1.position.z; 
  Vec2f SUVv2 = v2.uv / v2.position.z;

  // UvDelta
  Vec2f SUVDelta = SUVv2 - SUVv1;
  
  real32 SZv1 = 1.0f / v1.position.z;
  real32 SZv2 = 1.0f / v2.position.z;
  real32 SZDelta = SZv2 - SZv1;
  
  Vec2f SUVResult = SUVv1 + (SUVDelta  * t);
  real32 ZResult = SZv1 + (SZDelta  * t);
  
  Vec2f resultUV = SUVResult / ZResult;
  real32 resultZ = 1.0f / ZResult;
  
  result.uv = resultUV;
  result.z = resultZ;
  
  return result;
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

MappedPolygons
SoftwareRenderer::clip(const MappedPolygons& polygons, real32 nearZ) const
{
  MappedPolygons result;
  for(auto it = polygons.begin(); it != polygons.end(); it++)
  {
    const MappedPolygon& src = *it;
    // MappedPolygon dst = src;
    MappedPolygon dst = src.clip(nearZ);
    
    if(dst.vertices.size() >= 3)
    {
      result.push_back(dst);
    }
  }
  return result;
}

