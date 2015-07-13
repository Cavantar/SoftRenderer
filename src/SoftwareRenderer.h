#pragma once
#include <vector>
#include <jpb/Vector.h>
#include "main.h"

typedef std::vector<Vec3f> VerticesVector3D;
typedef std::vector<Vec2f> VerticesVector2D;

struct Triangle {
  Vec3f vertices[3];
};

struct Polygon2D {
  VerticesVector2D vertices;
};
typedef std::vector<Vec3f> VerticesVector3D;

struct Polygon3D {
  VerticesVector3D vertices;
  Polygon2D toPolygon2D() const ;
  Polygon3D clip(real32 nearZ) const ;
};

typedef std::vector<Polygon3D> PolygonVector3D; 

struct IndexedTriangle
{
  uint32 t0;
  uint32 t1;
  uint32 t2;
};

typedef std::vector<IndexedTriangle> IndTriangleVector;
typedef std::vector<Triangle> TriangleVector;

struct ScanLine {
  uint32 y;
  
  uint32 startX;
  uint32 endX;
};
typedef std::vector<ScanLine> ScanLineVector;

struct TrianglePair
{
  Triangle t1;
  Triangle t2;
};

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
};

class TextureHelper {
public:
  static void blitTexture(TextureBuffer* dst, TextureBuffer* src, const Vec2i& position);
};

class MeshHelper {
public:
  
  static Vec3f getCrossProduct(const IndexedTriangle& indexedTriangle, const VerticesVector3D& vertices);
  
  static TriangleVector getTrianglesFromIndices(const IndTriangleVector& triangleIndexes, const VerticesVector3D& vertices);
  static PolygonVector3D trianglesToPolys(const TriangleVector& triangles);
};

class Cube {
public:
  
  Cube(Vec3f centerPosition = Vec3f(), float sideLength = 1.0f, Vec3f color = Vec3f(1.0f, 0, 0)) :
    centerPosition(centerPosition), sideLength(sideLength), color(color) {}
  
  // Vertices are arranged as follows
  // Four bottom ones back to front
  // Four top ones back to front
  VerticesVector3D getVertices(real32 rotAngleX, real32 rotAngleY) const ;
  Vec3f getColor() const { return color; }

  static IndTriangleVector getTriangleIndexes(); 
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
  
  void drawTriangles3D(TextureBuffer* texture, const VerticesVector3D& vertices,
		       const IndTriangleVector& triangleIndices) const;
  
  void drawTriangle(TextureBuffer* texture, const Triangle& triangle, Vec3f color) const ;
  void drawPolygon(TextureBuffer* texture, Polygon2D& triangle, Vec3f color, bool outline = true) const;
private:
  
  real32 fov;
  ScanLineVector getScanLines(const Polygon2D& polygon) const;
  
  // Perspective Transformation without clipping
  VerticesVector3D castVertices(const VerticesVector3D& vertices, const Vec2i& screenDimensions) const;
  PolygonVector3D clip(const PolygonVector3D& polygons, real32 nearZ) const;
};
