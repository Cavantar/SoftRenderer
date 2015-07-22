#pragma once

#include <vector>
#include <jpb/Vector.h>

struct TextureBuffer {
  uint32* pixelData;
  Vec2i dimensions;
  int32 pitch;
  
  void setPixel(uint32 x, uint32 y, const Vec3f& color);
  Vec3f getPixel(uint32 x, uint32 y) const ;
  Vec3f getPixelUV(const Vec2f& uv) const;
  static void blitTexture(TextureBuffer* dst, TextureBuffer* src, const Vec2i& position);
};

typedef std::vector<Vec3f> Vertices;
typedef std::vector<Vec2f> Vertices2D;

struct Triangle {
  Vec3f vertices[3];
};

struct Polygon2D {
  Vertices2D vertices;
  Polygon2D toScreenSpace(const Vec2i& screenDimensions) const;
};
typedef std::vector<Vec3f> Vertices;

struct Polygon3D {
  Vertices vertices;
  
  Polygon3D clip(real32 nearZ) const ;
  Polygon2D toPolygon2D() const ;
};

typedef std::vector<Polygon3D> Polygons;

struct MappedVertex {
  Vec3f position;
  Vec2f uv;
  Vec3f normal;

  MappedVertex operator-(const MappedVertex& vertex) const;
  MappedVertex operator+(const MappedVertex& vertex) const;
  MappedVertex operator*(real32 scalar) const;
  
  static MappedVertex lerp(const MappedVertex& v1, const MappedVertex& v2, real32 t);
};
typedef std::vector<MappedVertex> MappedVertices;

struct MappedPolygon {
  std::vector<MappedVertex> vertices;

  static MappedPolygon clipNear(const MappedPolygon& polygon, real32 nearZ);
  static MappedPolygon clipSide(const MappedPolygon& polygon, real32 dfc);
  
  MappedPolygon clip(real32 nearZ, real32 dfc) const ;
  
  Polygon2D toPolygon2D() const ;
  MappedPolygon toScreenSpace(const Vec2i& screenDimensions) const;
};

typedef std::vector<MappedPolygon> MappedPolygons;

struct MappedTriangle
{
  MappedVertex vertices[3];
  MappedPolygon toPolygon() const;
};
typedef std::vector<MappedTriangle> MappedTriangles;

struct VertexCasted {
  Vec2f uv;
  Vec3f normal;
  real32 z;
};

struct IndexedTriangle
{
  uint32 indexes[3];
};

typedef std::vector<IndexedTriangle> TriangleIndices;
typedef std::vector<Triangle> TriangleVector;

struct ScanLine {
  uint32 y;
  
  uint32 startX;
  uint32 endX;
  
};

typedef std::vector<ScanLine> ScanLineVector;

struct MScanLine {
  uint32 y;
  
  int32 startX;
  int32 endX;
  
  uint32 minVertexIndex;
  uint32 maxVertexIndex;
};

typedef std::vector<MScanLine> MScanLineVector;

struct Range
{
  real32 min;
  real32 max;
};

struct Range2d
{
  Range x;
  Range y;
};

class MathHelper {
public:
  static Range2d getRange2d(const Vertices2D& vertices);
  static Range2d getRange2d(const MappedVertices& vertices);
};

class MeshHelper {
public:
  
  static Vec3f getCrossProduct(const Vertices& vertices, const IndexedTriangle& indexedTriangle);
  static Vec3f getCrossProduct(const MappedTriangle& triangle);
  static Vec3f castVertex(const Vec3f& position, real32 dfc);
  static Vec3f getCrossProductCasted(const MappedTriangle& triangle, real32 dfc);
  
  static TriangleVector getTrianglesFromIndices(const TriangleIndices& triangleIndexes, const Vertices& vertices);
  static MappedTriangles getTrianglesFromIndices(const TriangleIndices& triangleIndexes, const MappedVertices& vertices);
  
  static Polygons trianglesToPolys(const TriangleVector& triangles);
  static MappedPolygons trianglesToPolys(const MappedTriangles& triangles);

  static void rotateVertices(Vertices& vertices, const Vec3f& angles);
  static void rotateVertices(MappedVertices& vertices, const Vec3f& angles);
  
  static void translateVertices(Vertices& vertices, const Vec3f& translationVector);
  static void translateVertices(MappedVertices& vertices, const Vec3f& translationVector);

  static Vec3f getFaceNormal(const MappedVertices& vertices, const IndexedTriangle& indexedTriangle);
  static void calculateNormals(MappedVertices& vertices, const TriangleIndices& triangleIndices);
};

