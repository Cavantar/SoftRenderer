#pragma once
#include <jpb/Vector.h>
#include "RenderPrimitives.h"
#include "main.h"

class Camera {
public:
  Camera(const Vec3f& initialPosition = Vec3f()) : position(initialPosition) {}
  const Vec3f& getPosition() const { return position; } 
  
  virtual void castVertices(MappedVertices& vertices) const = 0;
  virtual void castVertices(Vertices& vertices) const = 0;
  virtual real32 getFov() const = 0;
  
  // Getting Distance from camera
  real32 getDfc() const ;
protected:
  Vec3f position;
};

class FPSCamera : public Camera {
public:
  FPSCamera(const Vec3f& initialPosition = Vec3f()) :
    Camera(initialPosition), rotY(0), rotX(0) {}

  void handleInput(const Input& input, real32 lastDelta);
  
  void castVertices(MappedVertices& vertices) const;
  void castVertices(Vertices& vertices) const;
  
  real32 getFov() const;
private:
  
  real32 rotY;
  real32 rotX;
};
