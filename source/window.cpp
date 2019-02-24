#include "window.h"
#include "chip.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <sstream>
#undef main

#define DEFAULTSPEED 13950.0

Window::Window(const char *title, int width, int height, int scale) :
	title(title), width(width), height(height), scale(scale)
{
	closed = !init();
}


Window::~Window()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_DestroyTexture(texture);
	SDL_DestroyTexture(menuTexture);
	SDL_DestroyTexture(speedTexture);
	SDL_FreeSurface(textSurface);
//	TTF_CloseFont(font);
	//TTF_Quit();
	SDL_CloseAudioDevice(deviceId);
	SDL_FreeWAV(wavBuffer);
	SDL_Quit();
	delete speedString;
}


bool Window::init()
{
	emulationSpeed = DEFAULTSPEED; // Standard number of Ms to sleep after every cycle.

	//Initializes the use of SDL in general.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) == -1)
	{
		printf("Unable to initialize SDL : %s \n", SDL_GetError());
		return false;
	}

	//Create window and check if window was created.
	window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP);
	if (window == NULL)
	{
		printf("Failed to create window : %s\n", SDL_GetError());
		return false;
	}

	//Create renderer locked to afore mentioned window.
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL)
	{
		printf("Error Creating Renderer (Line 69) : %s\n", SDL_GetError());
		return false;
	}

	if (SDL_RenderSetLogicalSize(renderer, width , height ) == -1)
	{
		printf("Error setting render resolution : %s\n", SDL_GetError());
		return false;
	}

	//Create texture
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	if (texture == NULL)
	{
		printf("Error creating texture : %s\n", SDL_GetError());
		return false;
	}
	
	//Load sound WAV file
	if (SDL_LoadWAV("beep.wav", &wavSpec, &wavBuffer, &wavLength) == NULL)
	{
		printf("Error loading sound file : %s\n", SDL_GetError());
		//return false;
	}

	//Open Audio Device
	desired.freq = 1;
	desired.format = AUDIO_F32;
	desired.samples = 4096;


	deviceId = SDL_OpenAudioDevice(NULL, 0, &wavSpec, NULL, 0);
	if (deviceId == 0)
	{
		printf("Error identifying audio device : %s\n", SDL_GetError());
		//return false;
	}
	
	/*
	//Initialize SDL text
	if (TTF_Init() < 0)
	{
		printf("Error initializing TTF : %s\n", TTF_GetError());
		return false;
	}

	font = TTF_OpenFont("romfs:/font/Retro_Computer.ttf", 50); //SDL_RWFromMem((void*)Retro_Computer_ttf, Retro_Computer_ttf_size), 1, 36);
	printf("Font Opened :\n");

	//Initialize Main Menu Text

	textSurface = TTF_RenderText_Blended_Wrapped(font,
		"+---------------+\n|  PAUSE MENU |\n+---------------+\n(ESC) : Resume\n(Q)      : Quit\n(R)      : Restart\n(N)      : New ROM\n(A)      : + Speed\n(D)      : - Speed\n",
		white,
		width * scale);

	menuTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	//Set Menu to middle of screen
	menuRect.x = (width / 4) * scale;
	menuRect.y = 0;
	SDL_QueryTexture(menuTexture, NULL, NULL, &menuRect.w, &menuRect.h);
	*/
	return true;
}


void Window::draw()
{
	++drawCount;
	//Update Texture
	if (SDL_UpdateTexture(texture, NULL, pixel_buffer, width * sizeof(Uint32)) == 0)
	{
		//Render current pixel buffer.
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, NULL, NULL);
		SDL_RenderPresent(renderer);
	}
	else
		closed = false;
}

void Window::pollEvents(chip &chip8)
{
	u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);
	u64 kHeld = hidKeysHeld(CONTROLLER_P1_AUTO);
	u64 kUp   = hidKeysUp(CONTROLLER_P1_AUTO);

		if (!isPaused)
		{
			//KEYDOWN EVENTS
			if (kDown & KEY_LSTICK_RIGHT || kHeld & KEY_LSTICK_RIGHT || kDown & KEY_DRIGHT || kHeld & KEY_DRIGHT)
			{
				printf("Left Stick Right Yo! \n");
				chip8.key[6] = 1;
				
			}
			if (kDown & KEY_LSTICK_LEFT || kHeld & KEY_LSTICK_LEFT || kDown & KEY_DLEFT || kHeld & KEY_DLEFT)
			{
				printf("Left Stick Left Yo! \n");
				chip8.key[4] = 1;
			}
			if (kDown & KEY_LSTICK_DOWN || kHeld & KEY_LSTICK_DOWN)
			{
				printf("Left Stick Down Yo! \n");
				chip8.key[8] = 1;
			}
			if (kDown & KEY_LSTICK_UP || kHeld & KEY_LSTICK_UP)
			{
				printf("Left Stick Up Yo! \n");
				chip8.key[2] = 1;
			}

			if (kDown & KEY_DDOWN || kHeld & KEY_DDOWN)
			{
				printf("Dpad Down Yo! \n");
			}
			if (kDown & KEY_A || kHeld & KEY_A)
			{
				printf("A pressed Yo! \n");
				chip8.key[5] = 1;
			}

			else if (kDown & KEY_PLUS)
			{
				printf("+ pressed Yo! \n");
				isPaused = !isPaused;
				//displayPause();
			}

			//KEYUP EVENTS
			if (kUp & KEY_LSTICK_RIGHT || kUp & KEY_DRIGHT)
			{
				printf("Left Stick Right Yo! \n");
				chip8.key[6] = 0;

			}
			if (kUp & KEY_LSTICK_LEFT || kUp & KEY_DLEFT)
			{
				printf("Left Stick Left Yo! \n");
				chip8.key[4] = 0;
			}
			if (kUp & KEY_LSTICK_DOWN)
			{
				printf("Left Stick Down Yo! \n");
				chip8.key[8] = 0;
			}
			if (kUp & KEY_LSTICK_UP)
			{
				printf("Left Stick Up Yo! \n");
				chip8.key[2] = 0;
			}

		
			if (kUp & KEY_A || kUp & KEY_A)
			{
				printf("A released Yo! \n");
				chip8.key[5] = 0;
			}

			if (kUp & KEY_DUP)
			{
				printf("Dpad Up Yo! \n");
			}

			if (kUp & KEY_DDOWN)
			{
				printf("Dpad Down Yo! \n");
			}


			

		}
		else //isPaused
		{
			if (kDown & KEY_PLUS)
			{
				printf("+ pressed Yo! \n");
				isPaused = !isPaused;
			}
		}
	/*
		else if (isPaused == true)
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
				switch (event.key.keysym.sym)
				{
				case SDLK_q:
					closed = true;
					break;

				case SDLK_r:
					needsReset = true;
					isPaused = !isPaused;
					chip8.cycle_count = 0;
					break;

				case SDLK_n:
					needsNewRom = true;
					needsReset = true;
					isPaused = !isPaused;
					chip8.cycle_count = 0;
					break;

				case SDLK_ESCAPE:
					isPaused = !isPaused;
					draw();
					break;

					//Allow user to change emulation speed with UP and DOWN arrow keys.
				case SDLK_LEFT:
					emulationSpeed += 1000;

					//Update Current Speed
					emulationRatio = DEFAULTSPEED / emulationSpeed;
					printf("Emulation Speed Decreased : %F x\n Emulation Speed is %d\n ", emulationRatio,emulationSpeed);
					break;

				case SDLK_RIGHT:
					if (emulationSpeed > 1000)
						emulationSpeed -= 1000;
					else if (emulationSpeed > 100)
						emulationSpeed -= 100;
					else if (emulationSpeed > 10)
						emulationSpeed -= 10;
					else if (emulationSpeed > 1)
						emulationSpeed -= 1;

					//Update Current Speed
					emulationRatio = DEFAULTSPEED / emulationSpeed;
					printf("Emulation Speed Increased : %F x\n \n Emulation Speed is %d\n", emulationRatio, emulationSpeed);
	
					break;
				}
			}

		}
		*/
}

void Window::pixelCopy(uint8_t *gfx)
{

	for (int i = 0; i < width * height; i++)
	{
		if (gfx[i] != 0)
			pixel_buffer[i] = primaryColor;
		else
			pixel_buffer[i] = 0;
	}
}

void Window::playSound()
{
	SDL_QueueAudio(deviceId, wavBuffer, wavLength);
	SDL_PauseAudioDevice(deviceId, 0);
}

/*
void Window::displayPause()
{
	++drawCount;
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, menuTexture, NULL, &menuRect);
	SDL_RenderPresent(renderer);
}
*/