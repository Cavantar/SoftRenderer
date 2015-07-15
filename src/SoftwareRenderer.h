#pragma once
#include <vector>
#include <jpb/Vector.h>
#include "main.h"

typedef std::vector<Vec3f> Vertices;
typedef std::vector<Vec2f> VerticesVector2D;

struct Triangle {
  Vec3f vertices[3];
};

struct Polygon2D {
  VerticesVector2D vertices;
  Polygon2D toScreenSpace(const Vec2i& screenDimensions) const;
};
typedef std::vector<Vec3f> Vertices;

struct Polygon3D {
  Vertices vertices;
  
  Polygon3D clip(real32 nearZ) const ;
  Polygon2D toPolygon2D() const ;
};

typedef std::vector<Polygon3D> PolygonVector3D;

struct MappedVertex {
  Vec3f position;
  Vec2f uv;
  Vec3f normal;
};
typedef std::vector<MappedVertex> MappedVertices;

struct MappedTriangle
{
  MappedVertex vertices[3];
};
typedef std::vector<MappedTriangle> MappedTriangles;

struct MappedPolygon {
  std::vector<MappedVertex> vertices;
  
  MappedPolygon clip(real32 nearZ) const ;
  Polygon2D toPolygon2D() const ;
  MappedPolygon toScreenSpace(const Vec2i& screenDimensions) const;
};

typedef std::vector<MappedPolygon> MappedPolygons;

struct UVCasted {
  Vec2f uv;
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
  static Range2d getRange2d(const VerticesVector2D& vertices);
  static Range2d getRange2d(const MappedVertices& vertices);
};

class TextureHelper {
public:
  static void blitTexture(TextureBuffer* dst, TextureBuffer* src, const Vec2i& position);
};

class MeshHelper {
public:
  
  static Vec3f getCrossProduct(const IndexedTriangle& indexedTriangle, const Vertices& vertices);
  static Vec3f getCrossProduct(const MappedTriangle& triangle);
  
  static TriangleVector getTrianglesFromIndices(const TriangleIndices& triangleIndexes, const Vertices& vertices);
  static MappedTriangles getTrianglesFromIndices(const TriangleIndices& triangleIndexes, const MappedVertices& vertices);
  
  static PolygonVector3D trianglesToPolys(const TriangleVector& triangles);
  static MappedPolygons trianglesToPolys(const MappedTriangles& triangles);

  static void rotateVertices(Vertices& vertices, const Vec3f& angles);
  static void translateVertices(Vertices& vertices, const Vec3f& translationVector);
};

class Cube {
public:
  
  Cube(Vec3f centerPosition = Vec3f(), float sideLength = 1.0f, Vec3f color = Vec3f(1.0f, 0, 0)) :
    centerPosition(centerPosition), sideLength(sideLength), color(color) {}
  
  // Vertices are arranged as follows
  // Four bottom ones back to front
  // Four top ones back to front
  Vertices getVertices(real32 rotAngleX, real32 rotAngleY) const ;
  Vec3f getColor() const { return color; }

  static TriangleIndices getTriangleIndexes(); 
  static TriangleVector getTriangles(std::vector<Vec2f>& vertices2d);
private:
  
  Vec3f centerPosition;
  float sideLength;
  Vec3f color;
};

class SoftwareRenderer {
public:
  SoftwareRenderer(real32 fov) : fov(fov) {};
  
  void drawLine(TextureBuffer* texture, Vec2f p1, Vec2f p2, Vec3f color) const ;
  void drawSquare(TextureBuffer* texture, Vec2f pos, float sideLength, Vec3f color) const ;
  void drawCubeInPerspective(TextureBuffer* texture, const Cube& cube, real32 rotAngleX = 0, real32 rotAngleY = 0);
  
  void drawTriangles3D(TextureBuffer* texture, const Vertices& vertices,
		       const TriangleIndices& triangleIndices, bool outline = true) const;
  
  void drawMappedTriangles3D(TextureBuffer* texture, const MappedVertices& mappedVertices,
			     const TriangleIndices& triangleIndices, const TextureBuffer* srcTexture) const;
  
  void drawTriangle(TextureBuffer* texture, const Triangle& triangle, Vec3f color) const ;
  void drawPolygon(TextureBuffer* texture, Polygon2D& polygon, Vec3f color, bool outline = true) const;
  
  void drawPolygonMapped(TextureBuffer* texture, MappedPolygon& polygon, const TextureBuffer* srcTexture, bool outline = true) const;
  
private:
  
  real32 fov;
  ScanLineVector getScanLines(const Polygon2D& polygon) const;
  MScanLineVector getScanLinesMapped(const MappedPolygon& polygon) const;
    
  // Perspective Transformation without clipping
  Vertices castVertices(const Vertices& vertices) const;
  
  void castMappedVertices(MappedVertices& vertices) const;
  UVCasted castPerspective(const MappedVertex& v1, const MappedVertex& v2, real32 t) const ;
  
  PolygonVector3D clip(const PolygonVector3D& polygons, real32 nearZ) const;
  MappedPolygons clip(const MappedPolygons& polygons, real32 nearZ) const;
};
