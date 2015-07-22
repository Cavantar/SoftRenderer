#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <assert.h>

#include <jpb/misc.h>

#include "main.h"
#include "Game.h"
#include "RenderPrimitives.h"


Input::Input()
{
  memset(keysPressed, 0, sizeof(keysPressed));
  memset(keysReleased, 0, sizeof(keysReleased));
  memset(keysDown, 0, sizeof(keysReleased));
  
  memset(buttonsPressed, 0, sizeof(buttonsPressed));
  memset(buttonsReleased, 0, sizeof(buttonsReleased));
  memset(buttonsDown, 0, sizeof(buttonsReleased));
}

void
Input::handleKeyPress(uint8 key)
{
  keysDown[key] = true;
  keysPressed[key] = true;
}

void
Input::handleKeyRelease(uint8 key)
{
  keysDown[key] = false;
  keysReleased[key] = true;
}

void
Input::handleButtonPress(uint8 button)
{
  buttonsDown[button] = true;
  buttonsPressed[button] = true;
}

void
Input::handleButtonRelease(uint8 button)
{
  buttonsDown[button] = false;
  buttonsReleased[button] = true;
}

void
Input::handleMouseMove(int32 x, int32 y)
{
  Vec2i newMousePosition = Vec2i(x, y);
  mousePositionDelta = mousePosition - newMousePosition;
  
  mousePosition = newMousePosition;
}

void
Input::clear()
{
  memset(keysPressed, 0, sizeof(keysPressed));
  memset(keysReleased, 0, sizeof(keysReleased));
  
  memset(buttonsPressed, 0, sizeof(buttonsPressed));
  memset(buttonsReleased, 0, sizeof(buttonsReleased));
  
  mousePositionDelta = Vec2i();
}

int main( int argc, char* args[] )
{
  // redirectIOToConsole();

  Game game;
  Vec2i screenResolution(1280, 720);
  
  //The window we'll be rendering to
  SDL_Window* window = NULL;
  
  //Initialize SDL
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
  {
    printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
  }
  else
  {
    window = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED,
			       SDL_WINDOWPOS_UNDEFINED, screenResolution.x, screenResolution.y, SDL_WINDOW_SHOWN );
    
    if( window == NULL )
    {
      printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
      // Time Stuff
      // --------------------
      
      LARGE_INTEGER countsPerSecond;
      QueryPerformanceFrequency(&countsPerSecond);
      
      LARGE_INTEGER countValue = {};
      LARGE_INTEGER prevCountValue = {};
      QueryPerformanceCounter(&prevCountValue);
      
      SDL_Renderer* renderer = NULL;
      renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
      
      bool quit = false;
      Input input;
      
      float lastDeltaMs = 2;
      
      TextureBuffer screenBuffer = {};
      screenBuffer.dimensions = screenResolution;
      
      SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
						     SDL_TEXTUREACCESS_STREAMING,
						     screenBuffer.dimensions.x,
						     screenBuffer.dimensions.y);
      
      game.start(screenResolution);
      
      while(!quit){
	SDL_Event event;
	while( SDL_PollEvent( &event ) != 0 )
	{
	  switch( event.type) 
	  {
	  case SDL_QUIT:
	    {
	      quit = true;
	      break;
	    }
	  case SDL_KEYUP:
	  case SDL_KEYDOWN:
	    {
	      if(!event.key.repeat)
	      {
		uint8 key = (uint8)event.key.keysym.sym;

		if(event.key.state == SDL_PRESSED)
		{
		  input.handleKeyPress(key);
		}
		else // Key Released
		{
		  input.handleKeyRelease(key);
		}
	      }
	      break;
	    }
	  case SDL_MOUSEMOTION:
	    {
	      // std::cout << "Mouse Position: " << event.motion.x << " " << event.motion.y << std::endl;
	      uint32 x = event.motion.x;
	      uint32 y = event.motion.y;
	      
	      input.handleMouseMove(x, y);
	    }
	    break;
	  case SDL_MOUSEBUTTONUP:
	  case SDL_MOUSEBUTTONDOWN:
	    {
	      // std::cout << "Mouse Position: " << event.motion.x << " " << event.motion.y << std::endl;
	      uint8 button = event.button.button;
	      if(event.button.state == SDL_PRESSED)
	      {
		input.handleButtonPress(button);
	      }
	      else
	      {
		input.handleButtonRelease(button);
	      }
	      
	    }
	    break;
	  }
	}
	
	// input.keysDown
	
	if(input.isKeyDown(SDLK_ESCAPE) || input.isKeyDown(SDLK_q)) quit = true;
	
	SDL_LockTexture(screenTexture, NULL, (void**)&screenBuffer.pixelData, &screenBuffer.pitch);
	game.update(&screenBuffer, input, lastDeltaMs);
	SDL_UnlockTexture(screenTexture);
	
	SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
	SDL_RenderPresent(renderer);

	input.clear();
	
	// Time Stuff
	// --------------------
	
	QueryPerformanceCounter(&countValue);
	int64 countDelta = countValue.QuadPart - prevCountValue.QuadPart; 
	prevCountValue = countValue;

	static real32 localTime = 0;
	static const real32 updatePeriod = 500;
	
	// What Part Of Second Elapsed
	lastDeltaMs = (float)countDelta / (float)countsPerSecond.QuadPart;
	lastDeltaMs *= 1000;
	
	localTime += lastDeltaMs;
	
	if(localTime > updatePeriod)
	{
	  localTime = fmodf(localTime, updatePeriod);
	  char tempBuffer[255] = {};
	  
	  sprintf(tempBuffer,"SoftRenderer %f ms/frame, %f fps", lastDeltaMs, 1000.0f/lastDeltaMs);
	  SDL_SetWindowTitle(window, tempBuffer);
	}
      }
    }
  }
  
  SDL_DestroyWindow( window );
  SDL_Quit();
  
  return 0;
}

