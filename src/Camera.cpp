#include "Camera.h"
#include <SDL.h>

real32
Camera::getDfc() const
{
  real32 fovRad = (M_PI/180.0f) * getFov();
  float dfc = 1.0f / tan(fovRad / 2.0f);

  return dfc;
}

void
FPSCamera::handleInput(const Input& input, real32 lastDelta)
{
  static const real32 rotationSpeed = 0.01f;

  if(input.isKeyDown(SDLK_8)) rotY += 0.001f * lastDelta;
  if(input.isKeyDown(SDLK_9)) rotY -= 0.001f * lastDelta;


  if(input.isButtonDown(SDL_BUTTON_LEFT))
  {
    Vec2i mouseDelta = input.getMouseDelta();

    rotY += rotationSpeed * lastDelta * -mouseDelta.x;
    rotX += rotationSpeed * lastDelta * -mouseDelta.y;
  }

  Vec3f lookVector(0, 0, 1.0f);
  lookVector.rotateAroundXDeg(rotX);
  lookVector.rotateAroundYDeg(-rotY);

  if(input.isKeyDown(SDLK_w)) position += lookVector * 0.001f * lastDelta;
  if(input.isKeyDown(SDLK_s)) position -= lookVector * 0.001f * lastDelta;

  Vec3f upVector(0, 1.0f, 0);
  Vec3f rightVector = Vec3f::cross(upVector, lookVector);

  if(input.isKeyDown(SDLK_d)) position += rightVector * 0.001f * lastDelta;
  if(input.isKeyDown(SDLK_a)) position -= rightVector * 0.001f * lastDelta;

}

void
FPSCamera::castVertices(MappedVertices& vertices) const
{
  MeshHelper::translateVertices(vertices, position * -1);
  MeshHelper::rotateVertices(vertices, Vec3f(0, rotY, 0));
  MeshHelper::rotateVertices(vertices, Vec3f(-rotX, 0, 0));
}

void
FPSCamera::castVertices(Vertices& vertices) const
{
  MeshHelper::translateVertices(vertices, position * -1);
  MeshHelper::rotateVertices(vertices, Vec3f(0, rotY, 0));
  MeshHelper::rotateVertices(vertices, Vec3f(-rotX, 0, 0));

}

Vec3f
FPSCamera::castDirectionalLight(const Vec3f& dirLight) const
{
  Vec3f result = dirLight;
  result.rotateAroundYDeg(rotY);
  result.rotateAroundXDeg(-rotX);
  return result;
}

real32
FPSCamera::getFov() const
{
  return 90.0f;
}
