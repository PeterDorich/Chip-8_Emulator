/****************************************************************************
 * Program: Chip-8 Emulator
 * Author: Peter Dorich
  
 * File: main.cpp
 * Main function that loads graphics support from SDL.
 * To learn the basics of SDL, I used the tutorial guides from LazyFoo.com
 * https://lazyfoo.net/tutorials/SDL/index.php#Key%20Presses
****************************************************************************/
#include <SDL2/SDL.h>
#include <iostream>
#include <thread>
#include <stdint.h>
#include "chip8.h"


using namespace std;

//keymap representing 16 keys from 1-v
uint8_t keymap[16] = {
    SDLK_1, SDLK_2, SDLK_3,
    SDLK_q, SDLK_w, SDLK_e,
    SDLK_a, SDLK_s, SDLK_d,
    SDLK_z, SDLK_x, SDLK_c,
    SDLK_4, SDLK_r, SDLK_f,
    SDLK_v,
};

int main(int argc, char** args)
{

	//Initialize Chip8 emulator
	Chip8 chip8;

	//Screen size
	int width = 64;
	int height = 32;

	//SDL window, renderer, and texture
	SDL_Window* window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	//Create window. I wanted it bigger, so it is scaled up by 15
	window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width *15, height*15, SDL_WINDOW_SHOWN);
	if (window == NULL){
        printf( "Error creating SDL window. SDL_Error: %s\n", SDL_GetError() );
		return 1;
    }

	//Create renderer for window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, width, height);
	if(renderer == NULL)
	{
		printf("Error creating renderer. SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

    // Create texture that stores frame buffer
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);


	//Sticking with just loading PONG as a way to show off emulator.
	const char *file_path = "roms/PONG";	
	
	//Load ROM:
	if(!chip8.load(file_path))
	{
		printf("Could not load ROM\n");
		return 1;
	}
	//Temporary buffer of pixels 
	uint32_t pixels[2048];

	//Quit flag for main loop
	bool quit = false;

	//Emulation Loop:
 	while (quit != true)
	{
		//Start emulating cycle
		chip8.emulateCycle();

		SDL_Event event;
		//Check for SDL events/ keypresses
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				
			//User requests to close the window
			case SDL_QUIT:
				quit = true;
				break;

			//User presses a key
			case SDL_KEYDOWN:
				//Escape key
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}

				//Check key pressed against keymap
				for(int i = 0; i < 16; i++)
				{
					if(event.key.keysym.sym == keymap[i])
					{
						chip8.key[i] = 1;
					}
				}
				break;

			case SDL_KEYUP:
				for(int i = 0; i < 16; i++)
				{
					if(event.key.keysym.sym == keymap[i])
					{
						chip8.key[i] = 0;
					}
				}
				break;
			}
		}

		//If a draw occured, update display
		if(chip8.draw_flag == true)
		{
			//reset draw_flag
			chip8.draw_flag = false; 	

			//Assign colors to pixels, in this case black and white
			for(int i = 0; i < 2048; i++)
			{
				int pixel = chip8.gfx[i];
				
				if(pixel == 1)
				{
					pixels[i] = 0x00FFFFFF;
				}
				if(pixel == 0)
				{
					pixels[i] = 0xFF000000;
				}
	
			}
	

			//Update texture to be displayed
			SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(int));
           		//Clear screen
			SDL_RenderClear(renderer);
			//Render texture to screen
			SDL_RenderCopy(renderer, texture, nullptr, nullptr);
			//Update screen
			SDL_RenderPresent(renderer);

		}
		//Set a small delay for each cycle to slow down emulation
		SDL_Delay(2);
	}

	//Clear memory for SDL texture, renderer, and window
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	window = NULL;
	renderer = NULL;
	texture = NULL;

	SDL_Quit();

	return 0;
}
