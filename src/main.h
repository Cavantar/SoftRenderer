#pragma once
#include <jpb/Types.h>
#include <jpb/Vector.h>

const int32 KEYCOUNT = 256;
const int32 BUTTONCOUNT = 16;

class Input {
public:
  Input();
  
  void handleKeyPress(uint8 key);
  void handleKeyRelease(uint8 key);
  void handleMouseMove(int32 x, int32 y);
  
  void handleButtonPress(uint8 button);
  void handleButtonRelease(uint8 button);
  
  // Has to be called at the end of each frame
  void clear();
  
  bool isKeyPressed(uint8 key) const { return keysPressed[key]; }
  bool isKeyReleased(uint8 key) const { return keysReleased[key]; }
  bool isKeyDown(uint8 key) const { return keysDown[key]; }
  
  bool isButtonPressed(uint8 button) const { return buttonsPressed[button]; }
  bool isButtonReleased(uint8 button) const { return buttonsReleased[button]; }
  bool isButtonDown(uint8 button) const { return buttonsDown[button]; }
  
  Vec2i getMouseDelta() const { return mousePositionDelta; }
private:
  
  bool keysDown[KEYCOUNT];
  bool keysPressed[KEYCOUNT];
  bool keysReleased[KEYCOUNT];

  bool buttonsDown[BUTTONCOUNT];
  bool buttonsPressed[BUTTONCOUNT];
  bool buttonsReleased[BUTTONCOUNT];

  Vec2i mousePosition;
  Vec2i mousePositionDelta;
};


