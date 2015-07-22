
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
      // result.vertices.push_back(vertices[nextVertexIndex]);
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
