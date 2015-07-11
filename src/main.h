#pragma once
#include <jpb/Types.h>
#include <jpb/Vector.h>

const int32 KEYCOUNT = 256;

const int32 SCREEN_WIDTH = 1280;
const int32 SCREEN_HEIGHT = 720;

struct Input {
  bool keysDown[KEYCOUNT];
  bool keysPressed[KEYCOUNT];
  bool keysReleased[KEYCOUNT];
};



