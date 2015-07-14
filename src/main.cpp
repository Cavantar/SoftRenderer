#include <SDL.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <assert.h>

#include <jpb/misc.h>

#include "main.h"
#include "Game.h"

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
TextureBuffer::getPixel(uint32 x, uint32 y)
{
  uint32 color = pixelData[x + (uint32)dimensions.x * y];
  Vec3f result(color >> 24,  color >> 16, color >> 8);
  
  return result;
}

int main( int argc, char* args[] )
{
  // redirectIOToConsole();

  Game game;
  
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
			       SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    
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
      Input input = {};
      
      float lastDeltaMs = 2;
      
      TextureBuffer screenBuffer = {};
      screenBuffer.dimensions = Vec2i(1280, 720);
      
      SDL_Texture* screenTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
						     SDL_TEXTUREACCESS_STREAMING,
						     screenBuffer.dimensions.x,
						     screenBuffer.dimensions.y);
      
      game.start();
      
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
		if(event.key.state == SDL_PRESSED)
		{
		  uint8 key = (uint8)event.key.keysym.sym;
		  
		  input.keysDown[key] = true;
		  input.keysPressed[key] = true;
		}
		else // Key Released
		{
		  uint8 key = (uint8)event.key.keysym.sym;
		  
		  input.keysDown[key] = false;
		  input.keysReleased[key] = true;
		  
		}
	      }
	      break;
	    }
	  }
	}
	
	if(input.keysDown[SDLK_ESCAPE] || input.keysDown[SDLK_q]) quit = true;
	
	SDL_LockTexture(screenTexture, NULL, (void**)&screenBuffer.pixelData, &screenBuffer.pitch);
	game.update(&screenBuffer, &input, lastDeltaMs);
	SDL_UnlockTexture(screenTexture);
	
	SDL_RenderCopy(renderer, screenTexture, NULL, NULL);
	SDL_RenderPresent(renderer);
	
	memset(input.keysPressed, 0, sizeof(input.keysPressed));
	memset(input.keysReleased, 0, sizeof(input.keysReleased));
	
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

