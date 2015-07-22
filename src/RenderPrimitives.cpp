
#include "RenderPrimitives.h"
#include <algorithm>

void
TextureBuffer::setPixel(uint32 x, uint32 y, const Vec3f& color)
{
  if(x >= 0 && x < dimensions.x &&
     y >= 0 && y < dimensions.y)
  {
    pixelData[x + (uint32)dimensions.x * y] =
      (uint32)(color.x) << 24 | (uint32)(color.y) << 16  | (uint32)(color.z) << 8;
  }
}

Vec3f
TextureBuffer::getPixel(uint32 x, uint32 y) const
{
  uint32 color = pixelData[x + (uint32)dimensions.x * y];
  Vec3f result(uint8(color >> 24),  uint8(color >> 16), uint8(color >> 8));
  
  return result;
}

Vec3f
TextureBuffer::getPixelUV(const Vec2f& uv) const
{
  uint32 x = std::max(fmodf(uv.x, 1.0f), 0.0f) * (real32)dimensions.x;
  uint32 y = std::max(fmodf(uv.y, 1.0f), 0.0f) * (real32)dimensions.y;
  
  return getPixel(x, y);
}

void
TextureBuffer::blitTexture(TextureBuffer* dst, TextureBuffer* src, const Vec2i& position)
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


Polygon2D
Polygon2D::toScreenSpace(const Vec2i& screenDimensions) const 
{
  real32 aspectRatio = (real32)screenDimensions.x / screenDimensions.y;
  real32 halfResX = screenDimensions.x * 0.5f;
  real32 halfResY = screenDimensions.y * 0.5f;
  
  uint32 vertexCount = vertices.size();
  Polygon2D result;
  result.vertices.resize(vertexCount);
  for(uint32 i = 0; i < vertexCount; i++)
  {
    const Vec2f& src = vertices[i];
    
    // std::cout << "src: " << src.x << " " << src.y << std::endl;
    Vec2f& dst = result.vertices[i];
    
    // // Transforming Into ScreenSpace
    dst.x = (src.x * halfResX) + halfResX;
    dst.y = ((-src.y * aspectRatio) * halfResY) + halfResY;
    
    // std::cout << "dst: " << dst.x << " " << dst.y << "\n\n";
    
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

MappedVertex
MappedVertex::operator-(const MappedVertex& vertex) const
{
  MappedVertex result = *this;
  
  result.position -= vertex.position;
  result.uv -= vertex.uv;
  result.normal -= vertex.normal;
  
  return result;
}

MappedVertex
MappedVertex::operator+(const MappedVertex& vertex) const
{
  MappedVertex result = *this;
  
  result.position += vertex.position;
  result.uv += vertex.uv;
  result.normal += vertex.normal;
  
  return result;
}

MappedVertex
MappedVertex::operator*(real32 scalar) const
{
  MappedVertex result = *this;
  
  result.position *= scalar;
  result.uv *= scalar;
  result.normal *= scalar;
  
  return result;  
}

MappedVertex
MappedVertex::lerp(const MappedVertex& v1, const MappedVertex& v2, real32 t)
{
  MappedVertex result;
  MappedVertex deltaVertex = v2 - v1;

  result = v1 + (deltaVertex * t);
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
    
    // result.vertices.push_back(v1);
    // continue;
    
    // Ignore if both
    if(v1.z >= nearZ && v2.z >= nearZ)
    {
      result.vertices.push_back(v1);
    }
    else if(v1.z >= nearZ && v2.z < nearZ)
    {
      result.vertices.push_back(v1);
      
      real32 t = (v1.z - nearZ) / (v1.z - v2.z);
      // std::cout << "tf: " << t << std::endl;
      Vec3f delta = v2 - v1;
      Vec3f addedVertex = v1 + (delta * t);
      
      result.vertices.push_back(addedVertex);
    }
    // v1.z < nearZ && v2 >= nearZ
    else if(v1.z < nearZ && v2.z >= nearZ)
    {
      real32 t = (v2.z - nearZ) / (v2.z - v1.z);

      // std::cout << "t: " << t << std::endl;
      Vec3f delta = v1 - v2;
      Vec3f addedVertex = v2 + (delta * t);
      
      result.vertices.push_back(addedVertex);
    }
  }
  return result;
}
MappedPolygon
MappedTriangle::toPolygon() const
{
  MappedPolygon polygon;
  polygon.vertices.resize(3);
  
  for(int i = 0; i < 3; i++)
  {
    polygon.vertices[i] = vertices[i];
  }
  return polygon;
}

MappedPolygon
MappedPolygon::clipNear(const MappedPolygon& polygon, real32 nearZ)
{
  MappedPolygon result;
  std::vector<MappedVertex> dstVertices;
  
  const std::vector<MappedVertex>& srcVertices = polygon.vertices;
  uint32 vertexCount = srcVertices.size();
  for(auto i = 0; i < vertexCount; i++)
  {
    uint32 nextVertexIndex = (i + 1)%vertexCount;
    
    const Vec3f& v1 = srcVertices[i].position;
    const Vec3f& v2 = srcVertices[nextVertexIndex].position;
    
    // Ignore if both
    if(v1.z >= nearZ && v2.z >= nearZ)
    {
      dstVertices.push_back(srcVertices[i]);
    }
    else if(v1.z >= nearZ && v2.z < nearZ)
    {
      dstVertices.push_back(srcVertices[i]);
      
      real32 deltaZ = v1.z - v2.z;
      real32 t = (v1.z - nearZ) / deltaZ;
      
      MappedVertex addedVertex = MappedVertex::lerp(srcVertices[i], srcVertices[nextVertexIndex], t);
      dstVertices.push_back(addedVertex);
    }
    else if(v1.z < nearZ && v2.z >= nearZ)
    {
      real32 deltaZ = v2.z - v1.z;
      real32 t = (v2.z - nearZ) / deltaZ;
      
      MappedVertex addedVertex = MappedVertex::lerp(srcVertices[nextVertexIndex], srcVertices[i], t);
      dstVertices.push_back(addedVertex);
    }
  }
  
  result.vertices = dstVertices;
  return result;
}

MappedPolygon
MappedPolygon::clipSide(const MappedPolygon& polygon, real32 dfc)
{
  MappedPolygon result;
  std::vector<MappedVertex> dstVertices;
  
  const std::vector<MappedVertex>& srcVertices = polygon.vertices;
  uint32 vertexCount = srcVertices.size();
  real32 a1 = dfc * -1.0f;

  for(auto i = 0; i < vertexCount; i++)
  {
    uint32 nextVertexIndex = (i + 1)%vertexCount;
    
    const Vec3f& v1 = srcVertices[i].position;
    const Vec3f& v2 = srcVertices[nextVertexIndex].position;
    
    // x is conventional x in linear equation, z is my result

    real32 minZV1 = a1 * v1.x;
    real32 minZV2 = a1 * v2.x;
    
    // Ignore if both
    if(v1.z >= minZV1 && v2.z >= minZV2)
    {
      dstVertices.push_back(srcVertices[i]);
    }
    else if(v1.z >= minZV1 && v2.z < minZV2)
    {
      dstVertices.push_back(srcVertices[i]);

      real32 dx = v1.x - v2.x;
      real32 dz = v1.z - v2.z;
      
      real32 a2 = 0;
      real32 t = 0;
      
      real32 z = 0;
      real32 x = 0;

      // If it's not aligned horizontally or vertically
      if(dx != 0 && dz != 0)
      {
	a2 = dz/dx;
	
	const Vec3f& startPosition = v2;
	
	x = ((-a2*startPosition.x) + startPosition.z) / (a1 - a2);
	t = (v1.x - x) / dx;
      }
      else
      {
	if(dx == 0)
	{
	  // Vertical Alignment
	  x = v1.x; // Both Vertices have the same x

	  // LinearEquation
	  z = a1 * x;
	  
	  t = (v1.z - z) / dz;
	}
	else
	{
	  // Horizontal Alignment
	  z = v1.z; // Both Vertices have the same z
	  
	  // LinearEquation - Solved For X
	  x = z / a1;
	  
	  t = (v1.x - x) / dx; 
	}
      }
      
      MappedVertex addedVertex = MappedVertex::lerp(srcVertices[i], srcVertices[nextVertexIndex], t);
      dstVertices.push_back(addedVertex);
    }
    else if(v1.z < minZV1 && v2.z >= minZV2)
    {
      
      real32 dx = (v2.x - v1.x);
      real32 dz = (v2.z - v1.z);
      
      real32 a2 = 0;
      real32 t = 0;
      
      real32 z = 0; 
      real32 x = 0;
      
      const Vec3f& startPosition = v1;
      
      if(dx != 0 && dz != 0)
      {
	a2 = dz/dx;
	
	x = ((-a2*startPosition.x) + startPosition.z) / (a1 - a2);
	t = (v2.x - x) / dx;
      }
      else
      {
	if(dx == 0)
	{
	  // Vertical Alignment
	  
	  x = startPosition.x;
	  z = a1 * x;
	  t = (v2.z - z) / dz;
	}
	else
	{
	  z = startPosition.z;
	  x = z / a1;
	  t = (v2.x - x) / dx;
	}
      }
      
      MappedVertex addedVertex = MappedVertex::lerp(srcVertices[nextVertexIndex], srcVertices[i], t);
      dstVertices.push_back(addedVertex);
    }
  }
  
  result.vertices = dstVertices;
  return result;
}

MappedPolygon
MappedPolygon::clip(real32 nearZ, real32 dfc) const
{
  MappedPolygon nearClipped = clipNear(*this, nearZ);
  MappedPolygon leftClipped = clipSide(nearClipped, dfc);
  leftClipped = clipSide(leftClipped, -dfc);
  
  return leftClipped;
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
MathHelper::getRange2d(const Vertices2D& vertices)
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

Vec3f
MeshHelper::getCrossProduct(const Vertices& vertices, const IndexedTriangle& indexedTriangle)
{
  Vec3f v1 = vertices[indexedTriangle.indexes[1]] - vertices[indexedTriangle.indexes[0]];
  Vec3f v2 = vertices[indexedTriangle.indexes[2]] - vertices[indexedTriangle.indexes[0]];
  
  Vec3f result = Vec3f::cross(v1, v2);
  return result;
}

Vec3f
MeshHelper::getCrossProduct(const MappedTriangle& triangle)
{
  Vec3f v1 = triangle.vertices[1].position - triangle.vertices[0].position;
  Vec3f v2 = triangle.vertices[2].position - triangle.vertices[0].position;
  
  Vec3f result = Vec3f::cross(v1, v2);
  return result;
}

Vec3f
MeshHelper::castVertex(const Vec3f& position, real32 dfc)
{
  Vec3f result;
  result.x = (position.x / position.z) * dfc;
  result.y = (position.y / position.z) * dfc;
  result.z = position.z;
  return result;
}

Vec3f
MeshHelper::getCrossProductCasted(const MappedTriangle& triangle, real32 dfc)
{
  MappedTriangle castedTriangle = triangle;
  Vec3f result;

  for(int i = 0; i < 3; i++)
  {
    Vec3f& position = castedTriangle.vertices[i].position;
    position = castVertex(position, dfc);
  }

  result = getCrossProduct(castedTriangle);
  
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
    triangles[i].vertices[0] = vertices[indexedTriangle.indexes[0]];
    triangles[i].vertices[1] = vertices[indexedTriangle.indexes[1]];
    triangles[i].vertices[2] = vertices[indexedTriangle.indexes[2]];
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

Polygons
MeshHelper::trianglesToPolys(const TriangleVector& triangles)
{
  Polygons result;
  for(auto it = triangles.begin(); it != triangles.end(); it++)
  {
    const Triangle& triangle = *it;
    Polygon3D poly;
    poly.vertices.resize(3);
    
    poly.vertices[0] = triangle.vertices[0];
    poly.vertices[1] = triangle.vertices[1];
    poly.vertices[2] = triangle.vertices[2];
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
  Vec3f radAngles = angles.degToRad();
  
  for(auto it = vertices.begin(); it != vertices.end(); it++)
  {
    Vec3f& vertex = *it;
    vertex.rotateAroundY(radAngles.y);
    vertex.rotateAroundX(radAngles.x);
    vertex.rotateAroundZ(radAngles.z);
  }
}

void
MeshHelper::rotateVertices(MappedVertices& vertices, const Vec3f& angles)
{
  Vec3f radAngles = angles.degToRad();
  
  for(auto it = vertices.begin(); it != vertices.end(); it++)
  {
    Vec3f& vertex = it->position;
    Vec3f& normal = it->normal;
    
    vertex.rotateAroundY(radAngles.y);
    normal.rotateAroundY(radAngles.y);
    
    vertex.rotateAroundX(radAngles.x);
    normal.rotateAroundX(radAngles.x);
    
    vertex.rotateAroundZ(radAngles.z);
    normal.rotateAroundZ(radAngles.z);
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

void
MeshHelper::translateVertices(MappedVertices& vertices, const Vec3f& translationVector)
{
  for(auto it = vertices.begin(); it != vertices.end(); it++)
  {
    Vec3f& vertex = it->position;
    vertex += translationVector;
  }
}

Vec3f
MeshHelper::getFaceNormal(const MappedVertices& vertices, const IndexedTriangle& indexedTriangle)
{
  Vec3f result;
  
  Vec3f v1 = vertices[indexedTriangle.indexes[1]].position - vertices[indexedTriangle.indexes[0]].position;
  Vec3f v2 = vertices[indexedTriangle.indexes[2]].position - vertices[indexedTriangle.indexes[0]].position;
  
  result = Vec3f::cross(v1, v2);
  result = Vec3f::normalize(result);
  
  return result;
}

void
MeshHelper::calculateNormals(MappedVertices& vertices, const TriangleIndices& triangleIndices)
{
  
  uint32 triangleCount = triangleIndices.size();
  Vertices normals(triangleCount);

  // Calculating Face Normals
  for(uint32 i = 0; i != triangleCount; i++)
  {
    const IndexedTriangle& indexedTriangle = triangleIndices[i];
    normals[i] = getFaceNormal(vertices, indexedTriangle);
  }

  // Checking What Face Normals belongs to what vertex 
  uint32 vertexCount = vertices.size();
  for(int i = 0; i != vertexCount; i++)
  {
    MappedVertex& vertex = vertices[i];
    Vec3f directionSum;
    
    for(int j = 0; j < triangleCount; j++)
    {
      if(triangleIndices[j].indexes[0] == i ||
	 triangleIndices[j].indexes[1] == i ||
	 triangleIndices[j].indexes[2] == i )
      {
	directionSum += normals[j];
      }
    }
    // Normalizing direction sum
    vertex.normal = Vec3f::normalize(directionSum);
  }
}
