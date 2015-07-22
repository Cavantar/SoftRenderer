#pragma once
#include <vector>
#include <jpb/Vector.h>

#include "main.h"
#include "RenderPrimitives.h"
#include "Camera.h"

typedef std::vector<std::vector<real32>> FloatMatrix;

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

class SoftRenderer {
public:
  
  void drawLine(TextureBuffer* screenBuffer, Vec2f p1, Vec2f p2, Vec3f color) const ;
  void drawSquare(TextureBuffer* screenBuffer, Vec2f pos, float sideLength, Vec3f color) const ;
  void drawCubeInPerspective(TextureBuffer* screenBuffer, const Cube& cube, real32 rotAngleX = 0, real32 rotAngleY = 0);
  
  void drawTriangles3D(TextureBuffer* screenBuffer, const Vertices& vertices,
		       const TriangleIndices& triangleIndices, bool outline = true) const;
  
  void drawMappedTriangles3D(TextureBuffer* screenBuffer, const MappedVertices& mappedVertices,
			     const TriangleIndices& triangleIndices, const TextureBuffer* srcTexture);
  
  void drawTriangle(TextureBuffer* screenBuffer, const Triangle& triangle, Vec3f color) const ;
  void drawPolygon(TextureBuffer* screenBuffer, Polygon2D& polygon, Vec3f color, bool outline = true) const;
  
  void drawPolygonMapped(TextureBuffer* screenBuffer, MappedPolygon& polygon, const TextureBuffer* srcTexture, bool outline = true);
  void setCamera(Camera* camera) { this->camera = camera; }
  void setDirectionalLight(const Vec3f& directionalLight) { this->directionalLight = directionalLight; }
  
  void setZBufferSize(const Vec2i& zBufferSize);
  void clearZBuffer();
  
private:
  
  Camera* camera;
  FloatMatrix zBuffer;
  real32 ambientLight = 0.3f;
  Vec3f directionalLight = Vec3f(-0.707f, -0.707f, -0.707f);
  
  ScanLineVector getScanLines(const Polygon2D& polygon) const;
  MScanLineVector getScanLinesMapped(const MappedPolygon& polygon, const Vec2i& screenResolution) const;
    
  // Perspective Transformation without clipping
  Vertices castVertices(const Vertices& vertices) const;
  
  void castMappedVertices(MappedVertices& vertices) const;
  VertexCasted castPerspective(const MappedVertex& v1, const MappedVertex& v2, real32 t) const ;
  
  Polygons clip(const Polygons& polygons, real32 nearZ) const;
  MappedPolygons clip(const MappedPolygons& polygons, real32 nearZ) const;
};
